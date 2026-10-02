/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "blur-helper.h"
#include "settings/ukui-style-settings.h"
#include <QWidget>
#include <KWindowEffects>
#include <QGSettings>
#include <QVariant>
#include <QEvent>
#include <QIcon>
#include <QPainterPath>

#include <QMenu>

#include "settings/black-list.h"

#include <QApplication>
#include <QX11Info>

#include <QDebug>

BlurHelper::BlurHelper(QObject *parent, bool systemEnable) : QObject(parent)
{
    if (QGSettings::isSchemaInstalled("org.ukui.control-center.personalise")) {
        m_peripheralsGsettings = new QGSettings("org.ukui.control-center.personalise", QByteArray(), this);
        connect(m_peripheralsGsettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "effect") {
                bool enable = m_peripheralsGsettings->get(key).toBool();
                this->onBlurEnableChanged(enable);
            }
        });
        bool enable = m_peripheralsGsettings->get("effect").toBool();
        m_system_enableBlur = systemEnable;
        this->onBlurEnableChanged(enable);

        if (!m_system_enableBlur/*!KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind)*/)
            confirmBlurEnableDelay();
    }
    m_windowHelperHash = new QHash<QWidget *, UkuiWindowHelper*>();
    m_timer.setSingleShot(true);

    m_timer.setInterval(100);
}

BlurHelper::~BlurHelper()
{
    m_blur_widgets.clear();
    m_update_list.clear();
    if(m_peripheralsGsettings){
        m_peripheralsGsettings->deleteLater();
        m_peripheralsGsettings = nullptr;
    }
    auto windowHelperHashs= m_windowHelperHash->values();
    for (auto windowHelperHash : windowHelperHashs) {
        if (windowHelperHash) {
            delete windowHelperHash;
            windowHelperHash = nullptr;
        }
    }
    if(m_windowHelperHash){
        m_windowHelperHash->clear();
        delete m_windowHelperHash;
        m_windowHelperHash = nullptr;
    }

}

bool BlurHelper::eventFilter(QObject *obj, QEvent *e)
{
    if (!m_blur_enable)
        return false;

    QWidget* widget = qobject_cast<QWidget*>(obj);

    if (!widget || !widget->isWindow())
        return false;

    //FIXME:
    //qDebug()<<e->type()<<obj;
    //qDebug()<<KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind);
    switch (e->type()) {
    case QEvent::UpdateRequest: {
        //QWidget* widget = qobject_cast<QWidget*>(obj);
        delayUpdate(widget, true);
        break;
    }
    case QEvent::LayoutRequest:
    {
        //QWidget* widget = qobject_cast<QWidget*>(obj);
        delayUpdate(widget);
        break;
    }
    case QEvent::Hide: {
        //QWidget* widget = qobject_cast<QWidget*>(obj);
        // KWindowEffects::enableBlurBehind(widget->winId(), false);
        QRegion region(widget->geometry());
        UkuiWindowHelper *windowHelper = getWindowHelper(widget);
        windowHelper->setBlurEffect(region, 0, false);

    }

    default:
        break;
    }
    return false;
}

/*!
 * \brief BlurHelper::registerWidget
 * \param widget
 * \note
 *
 * we can't blur a widget before it shown, because some times
 * there might be problems about window painting.
 * this usually happend on a widget which has graphics effect.
 *
 * to avoid them, never try get winid and do a blur for that case.
 */
#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
void BlurHelper::registerWidget(QWidget *widget)
{
//    // FIXME: how to blur window on wayland?
//    if (!QX11Info::isPlatformX11())
//        return;

    if (!widget)
        return;

    if (shouldSkip(widget))
        return;
    if (isApplicationInBlackList())
        return;
    if (widget->property("doNotBlur").toBool())
        return;
    if (!m_blur_widgets.contains(widget)) {
        m_blur_widgets<<widget;
        //qDebug()<<KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind);
        /*!
          \note
          never try enableBlurBehind when register widget.
          it might cause serious problems about paint.
          usually it happend with a widget which has graphics effect.
          */
//        if (!widget->mask().isEmpty()) {
//            KWindowEffects::enableBlurBehind(widget->winId(), true, widget->mask());
//        } else {
//            KWindowEffects::enableBlurBehind(widget->winId(), true);
//        }

        connect(widget, &QWidget::destroyed, this, [=]() {
            this->onWidgetDestroyed(widget);
        });
    }
    widget->removeEventFilter(this);
    widget->installEventFilter(this);

    if (!widget->mask().isEmpty()) {
        widget->update(widget->mask());
    } else {
        widget->update();
    }
}

void BlurHelper::unregisterWidget(QWidget *widget)
{
    if (!QX11Info::isPlatformX11())
        return;

    if (!widget)
        return;

    if (shouldSkip(widget))
        return;
    if (isApplicationInBlackList())
        return;
    if (widget->property("doNotBlur").toBool())
        return;
    m_blur_widgets.removeOne(widget);
    widget->removeEventFilter(this);
    if (widget){
        QRegion region(widget->geometry());
        UkuiWindowHelper *windowHelper = getWindowHelper(widget);
        windowHelper->setBlurEffect(region, 0, false);
    }
}
#endif

bool BlurHelper::isApplicationInBlackList()
{
    return blackList().contains(qAppName());
}

const QStringList BlurHelper::blackList()
{
    return blackAppListWithBlurHelper();
}

bool BlurHelper::shouldSkip(QWidget *w)
{
    if (w->property("useSystemStyleBlur").isValid()) {
        return !w->property("useSystemStyleBlur").toBool();
    }

    bool skip = true;
    if (w && (w->inherits("QMenu") || w->inherits("QTipLabel") || w->inherits("QComboBoxPrivateContainer"))) {
        return false;
    }

    return skip;
}

void BlurHelper::onBlurEnableChanged(bool enable)
{
    m_blur_enable = enable;

    if (enable) {
        qApp->setProperty("blurEnable", true);
    } else {
        qApp->setProperty("blurEnable", false);
    }

    if (!m_system_enableBlur)//!KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind))
        return;

    for (auto widget : qApp->allWidgets()) {
        widget->update();
        if (m_blur_widgets.contains(widget)) {
            if (widget) {
                QRegion region(widget->geometry());
                UkuiWindowHelper *windowHelper = getWindowHelper(widget);
                windowHelper->setBlurEffect(region, 0, enable);
            }
        }
    }
//    QTimer::singleShot(100, this, [=](){
//        for (auto widget : m_blur_widgets) {
//            if (!widget)
//                continue;
//            if (widget->winId() > 0)
//                KWindowEffects::enableBlurBehind(widget->winId(), enable);
//        }
//    });
}

void BlurHelper::onWidgetDestroyed(QWidget *widget)
{
    widget->removeEventFilter(this);
    m_blur_widgets.removeOne(widget);
    //unregisterWidget(widget);
}

void BlurHelper::delayUpdate(QWidget *w, bool updateBlurRegionOnly)
{
    if (w->winId() <= 0)
        return;

    if (!m_system_enableBlur)//!KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind))
        return;

    m_update_list.append(w);
    if (!m_timer.isActive()) {
        for (auto widget : m_update_list) {
            // cast to widget and check
            //KWindowEffects::enableBlurBehind(widget->winId(), false);

            if (!widget)
                continue;

            if (widget->winId() <= 0)
                continue;

            bool hasMask = false;
            if (widget->mask().isNull())
                hasMask = true;

            QVariant regionValue = widget->property("blurRegion");
            QRegion region = qvariant_cast<QRegion>(regionValue);

            if (widget->inherits("QMenu")) {
                //skip menu which has style sheet.
                if (!widget->styleSheet().isEmpty() || qApp->styleSheet().contains("QMenu")) {
                    break;
                }
                QPainterPath path;
                int radius = 8;
                if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
                    auto settings = UKUIStyleSettings::globalInstance();
                    if(settings->keys().contains("windowRadius"))
                        radius = settings->get("windowRadius").toInt();
                }
                path.addRoundedRect(widget->rect(), radius, radius);
                // KWindowEffects::enableBlurBehind(widget->winId(), true, path.toFillPolygon().toPolygon());

                UkuiWindowHelper *windowHelper = getWindowHelper(widget);
                windowHelper->setBlurEffect(path.toFillPolygon().toPolygon());

                if (!updateBlurRegionOnly)
                    widget->update();
                break;
            }

            if (widget->inherits("QTipLabel") || widget->inherits("QComboBoxPrivateContainer")) {
                QPainterPath path;
                int radius = 8;
                if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
                    auto settings = UKUIStyleSettings::globalInstance();
                    if(settings->keys().contains("windowRadius"))
                        radius = settings->get("windowRadius").toInt();
                }
                path.addRoundedRect(widget->rect(), radius, radius);
                // KWindowEffects::enableBlurBehind(widget->winId(), true, path.toFillPolygon().toPolygon());
                UkuiWindowHelper *windowHelper = getWindowHelper(widget);
                windowHelper->setBlurEffect(path.toFillPolygon().toPolygon());

                if (!updateBlurRegionOnly)
                    widget->update();
                break;
            }

            if (!hasMask && region.isEmpty())
                break;

            //qDebug()<<regionValue<<region;
            //qDebug()<<widget->metaObject()->className()<<widget->geometry()<<widget->mask();
            if (!region.isEmpty()) {
                //qDebug()<<"blur region"<<region;
                // KWindowEffects::enableBlurBehind(widget->winId(), true, region);
                UkuiWindowHelper *windowHelper = getWindowHelper(widget);
                windowHelper->setBlurEffect(region);

                if (!updateBlurRegionOnly)
                    widget->update();
            } else {
                //qDebug()<<widget->mask();
                // KWindowEffects::enableBlurBehind(widget->winId(), true, widget->mask());
                UkuiWindowHelper *windowHelper = getWindowHelper(widget);
                windowHelper->setBlurEffect(widget->mask());

                if (!updateBlurRegionOnly)
                    widget->update(widget->mask());
            }

            //NOTE: we can not setAttribute Qt::WA_TranslucentBackground here,
            //because the window is about to be shown.
            //widget->setAttribute(Qt::WA_TranslucentBackground);
            //KWindowEffects::enableBlurBehind(widget->winId(), true);
            //widget->update();
        }
        m_update_list.clear();
    } else {
        m_timer.start();
    }
}

void BlurHelper::confirmBlurEnableDelay()
{
    QTimer::singleShot(3000, this, [=]() {
        bool enable = m_blur_enable;
        if (enable) {
            qApp->setProperty("blurEnable", true);
        } else {
            qApp->setProperty("blurEnable", false);
        }
        for (auto widget : qApp->allWidgets()) {
            widget->update();
            if (m_blur_widgets.contains(widget)) {
                if (widget->winId() > 0){
                    // KWindowEffects::enableBlurBehind(widget->winId(), enable);
                    QRegion region(widget->geometry());
                    UkuiWindowHelper *windowHelper = getWindowHelper(widget);
                    windowHelper->setBlurEffect(region, 0, enable);
                }
            }
        }
//        QTimer::singleShot(100, this, [=](){
//            for (auto widget : m_blur_widgets) {
//                if (!widget)
//                    continue;
//                if (widget->winId() > 0)
//                    KWindowEffects::enableBlurBehind(widget->winId(), enable);
//            }
//        });
    });
}

UkuiWindowHelper* BlurHelper::getWindowHelper(QWidget* w)
{
    if(!m_windowHelperHash){
        m_windowHelperHash = new  QHash<QWidget *, UkuiWindowHelper*>();
    }
    if(m_windowHelperHash && w && m_windowHelperHash->contains(w)){
        return m_windowHelperHash->value(w);
    }
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(w);
    m_windowHelperHash->insert(w, windowHelper);
    QObject::connect(w, &QWidget::destroyed, this, [this, w]() {
        if (m_windowHelperHash && m_windowHelperHash->contains(w)) {
            UkuiWindowHelper* helper = m_windowHelperHash->take(w);
            delete helper; // 释放 helper 内存
            helper = nullptr;
        }
    });
    return windowHelper;
}
