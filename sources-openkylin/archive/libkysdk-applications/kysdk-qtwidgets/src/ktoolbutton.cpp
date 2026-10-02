/*
 * libkysdk-qtwidgets's Library
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "ktoolbutton.h"
#include "parmscontroller.h"
#include "themeController.h"
#include <QApplication>
#include <QDebug>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QStyleOptionToolButton>
#include <QTimer>

namespace kdk
{
class KToolButtonPrivate : public QObject, public ThemeController
{
    Q_DECLARE_PUBLIC(KToolButton)
    Q_OBJECT

public:
    KToolButtonPrivate(KToolButton *parent);
    ~KToolButtonPrivate()
    {
    }
    void changePalette();
    void doLoadingFlash();

protected:
    void changeTheme();

private:
    KToolButton *q_ptr;
    KToolButtonType m_type;
    QLinearGradient m_pLinearGradient;
    bool m_isLoading;
    QTimer *m_pTimer;
    int m_flashState;
    bool m_hasArrow;
    QPixmap m_arrowPixmap;
    QColor m_pixColor;
    QSize m_pixmapSize;
    bool m_isIconColor;
    QColor m_iconColor;
    int m_bottomLeftRadius;
    int m_topLeftRadius;
    int m_topRightRadius;
    int m_bottomRightRadius;
    bool m_isBkgColorEnable;
};

KToolButton::KToolButton(QWidget *parent)
    : QToolButton(parent)
    , d_ptr(new KToolButtonPrivate(this))
{
    Q_D(KToolButton);
    d->m_pTimer = new QTimer(this);
    d->m_pTimer->setInterval(100);
    d->m_flashState = 0;
    d->m_isLoading = false;
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1) {
        d->m_bottomLeftRadius = 6;
        d->m_topLeftRadius = 6;
        d->m_topRightRadius = 6;
        d->m_bottomRightRadius = 6;
    } else {
        d->m_bottomLeftRadius = radius;
        d->m_topLeftRadius = radius;
        d->m_topRightRadius = radius;
        d->m_bottomRightRadius = radius;
    }
    d->m_isBkgColorEnable = false;
    setType(Flat);
    installEventFilter(this);
    QToolButton::setIcon(QIcon::fromTheme("open-menu-symbolic"));
    setFocusPolicy(Qt::FocusPolicy::TabFocus);
    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            d->changeTheme();
        }
    });
    connect(d->m_pTimer, &QTimer::timeout, d, &KToolButtonPrivate::doLoadingFlash);
    connect(Parmscontroller::self(), &Parmscontroller::modeChanged, this, [=]() {
        updateGeometry();
    });
    connect(this, &KToolButton::toggled, this, [=](bool checked) {
        emit checkedChanged(checked);
    });
}

KToolButtonType KToolButton::type()
{
    Q_D(KToolButton);
    return d->m_type;
}

void KToolButton::setType(KToolButtonType type)
{
    Q_D(KToolButton);
    d->m_type = type;
    d->changePalette();
}

void KToolButton::setIcon(const QIcon &icon)
{
    Q_D(KToolButton);
    QToolButton::setIcon(icon);
    d->changeTheme();
}

void KToolButton::setLoading(bool flag)
{
    Q_D(KToolButton);
    if (!isEnabled())
        return;
    if (hasArrow())
        return;
    d->m_isLoading = flag;
    if (flag)
        d->m_pTimer->start();
    else
        d->m_pTimer->stop();
}

bool KToolButton::isLoading()
{
    Q_D(KToolButton);
    return d->m_isLoading;
}

QIcon KToolButton::icon()
{
    return QToolButton::icon();
}

void KToolButton::setArrow(bool flag)
{
    Q_D(KToolButton);
    if (!d->m_isLoading)
        d->m_hasArrow = flag;
    update();
}

bool KToolButton::hasArrow() const
{
    Q_D(const KToolButton);
    return d->m_hasArrow;
}

void KToolButton::setIconColor(bool flag, QColor color)
{
    Q_D(KToolButton);
    d->m_isIconColor = flag;
    d->m_iconColor = color;
    update();
}

void KToolButton::setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight)
{
    Q_D(KToolButton);
    d->m_bottomLeftRadius = bottomLeft;
    d->m_topLeftRadius = topLeft;
    d->m_topRightRadius = topRight;
    d->m_bottomRightRadius = bottomRight;
}

void KToolButton::setBackgroundColorEnabled(bool flag)
{
    Q_D(KToolButton);
    d->m_isBkgColorEnable = flag;
}

bool KToolButton::backgroundColorEnabled()
{
    Q_D(KToolButton);
    return d->m_isBkgColorEnable;
}

bool KToolButton::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KToolButton);
    QColor highlightColor = ThemeController::getPaletteColorFromDT("highlight-active");
    if (watched == this) {
        // 根据不同状态重绘icon颜色
        if (highlightColor == Qt::transparent) {
            if (ThemeController::themeMode() == LightTheme)
                d->m_pixColor = ThemeController::getCustomColorFromDT("kgray-17");
            else
                d->m_pixColor = ThemeController::getCustomColorFromDT("kwhite");
        } else {
            switch (event->type()) {
            case QEvent::MouseButtonPress:
                if (isEnabled() && !d->m_isLoading) {
                    if (ThemeController::themeMode() == LightTheme)
                        d->m_pixColor = highlightColor.darker(120);
                    else
                        d->m_pixColor = highlightColor.lighter(120);
                }
                break;
            case QEvent::Enter:
                if (isEnabled() && !d->m_isLoading) {
                    if (ThemeController::themeMode() == LightTheme)
                        d->m_pixColor = highlightColor.darker(105);
                    else
                        d->m_pixColor = highlightColor.lighter(105);
                }
                break;
            case QEvent::FocusIn:
                if (isEnabled() && !d->m_isLoading) {
                    if (ThemeController::themeMode() == LightTheme)
                        d->m_pixColor = highlightColor.darker(120);
                    else
                        d->m_pixColor = highlightColor.lighter(120);
                }
                break;
            case QEvent::EnabledChange: {
                if (!isEnabled() && !d->m_isLoading) {
                    if (d->m_isLoading) {
                        d->m_isLoading = false;
                        d->m_pTimer->stop();
                    }
                    if (ThemeController::themeMode() == LightTheme)
                        d->m_pixColor = ThemeController::getCustomColorFromDT("kgray-10");
                    else
                        d->m_pixColor = ThemeController::getCustomColorFromDT("kgray-14");
                } else if (isEnabled() && !d->m_isLoading) {
                    if (ThemeController::themeMode() == LightTheme)
                        d->m_pixColor = ThemeController::getCustomColorFromDT("kgray-17");
                    else
                        d->m_pixColor = ThemeController::getCustomColorFromDT("kwhite");
                }
            } break;
            case QEvent::MouseButtonRelease:
                if (isEnabled() && !d->m_isLoading) {
                    if (ThemeController::themeMode() == LightTheme)
                        d->m_pixColor = highlightColor.darker(105);
                    else
                        d->m_pixColor = highlightColor.lighter(105);
                }
                break;
            case QEvent::Leave:
            case QEvent::FocusOut:
                if (isEnabled() && !d->m_isLoading) {
                    if (ThemeController::themeMode() == LightTheme)
                        d->m_pixColor = ThemeController::getCustomColorFromDT("kgray-17");
                    else
                        d->m_pixColor = ThemeController::getCustomColorFromDT("kwhite");
                }
                break;
            default:
                break;
            }
        }
    }
    return QToolButton::eventFilter(watched, event);
}

QSize KToolButton::sizeHint() const
{
    Q_D(const KToolButton);

    QSize size(Parmscontroller::parm(Parmscontroller::Parm::PM_ToolButtonHeight),
               Parmscontroller::parm(Parmscontroller::Parm::PM_ToolButtonHeight));
    if (d->m_hasArrow)
        size.setWidth(Parmscontroller::parm(Parmscontroller::Parm::PM_ToolButtonHeight) + 24);
    return size;
}

void KToolButton::paintEvent(QPaintEvent *event)
{
    Q_D(KToolButton);
    d->m_pLinearGradient.setStart(this->width() / 2, 0);
    d->m_pLinearGradient.setFinalStop(this->width() / 2, this->height());
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制边框
    QStyleOptionToolButton option;
    initStyleOption(&option);
    d->m_arrowPixmap = QIcon::fromTheme("ukui-down-symbolic").pixmap(option.iconSize);

    QPen pen;
    QColor color = ThemeController::getPaletteColorFromDT("highlight-active");
    pen.setColor(color);
    pen.setWidth(2);
    if (((QStyle::State_HasFocus & option.state) && isEnabled() && !isLoading() && (this->focusPolicy() != Qt::FocusPolicy::NoFocus)) && d->m_type != Frameless)
        painter.setPen(pen);
    else
        painter.setPen(Qt::NoPen);

    QColor startColor;
    QColor endColor;

    if(!isChecked())
    {
        if (!isEnabled())
        {
            if(type() != Flat)
            {
                ThemeController::getGradientFromDT("kcomponent-disable", startColor, endColor);
                d->m_pLinearGradient.setColorAt(0, startColor);
                d->m_pLinearGradient.setColorAt(1, endColor);
                painter.setBrush(d->m_pLinearGradient);
            }
            else
            {
                painter.setBrush(Qt::NoBrush);
            }
        } else if (QStyle::State_Sunken & option.state) {
            if(type() != Flat)
            {
                ThemeController::getGradientFromDT("kcomponent-click", startColor, endColor);
                d->m_pLinearGradient.setColorAt(0, startColor);
                d->m_pLinearGradient.setColorAt(1, endColor);
                painter.setBrush(d->m_pLinearGradient);
            }
            else
            {
                painter.setBrush(Qt::NoBrush);
            }
        } else if (QStyle::State_MouseOver & option.state) {
            if(type() != Flat)
            {
                ThemeController::getGradientFromDT("kcomponent-hover", startColor, endColor);
                d->m_pLinearGradient.setColorAt(0, startColor);
                d->m_pLinearGradient.setColorAt(1, endColor);
                painter.setBrush(d->m_pLinearGradient);
            }
            else
            {
                painter.setBrush(Qt::NoBrush);
            }
        } else {
            if(type() != Flat && type() != SemiFlat)
            {
                ThemeController::getGradientFromDT("kcomponent-normal", startColor, endColor);
                d->m_pLinearGradient.setColorAt(0, startColor);
                d->m_pLinearGradient.setColorAt(1, endColor);
                painter.setBrush(d->m_pLinearGradient);
            }
            else
            {
                painter.setBrush(Qt::NoBrush);
            }
        }
    }
    else
    {
        if (!isEnabled())
        {
            ThemeController::getGradientFromDT("kbrand-disable", startColor, endColor);
            d->m_pLinearGradient.setColorAt(0, startColor);
            d->m_pLinearGradient.setColorAt(1, endColor);
            painter.setBrush(d->m_pLinearGradient);
        } else if (QStyle::State_Sunken & option.state) {
            ThemeController::getGradientFromDT("kbrand-click", startColor, endColor);
            d->m_pLinearGradient.setColorAt(0, startColor);
            d->m_pLinearGradient.setColorAt(1, endColor);
            painter.setBrush(d->m_pLinearGradient);
        } else if (QStyle::State_MouseOver & option.state) {
            ThemeController::getGradientFromDT("kbrand-hover", startColor, endColor);
            d->m_pLinearGradient.setColorAt(0, startColor);
            d->m_pLinearGradient.setColorAt(1, endColor);
            painter.setBrush(d->m_pLinearGradient);
        } else {
            ThemeController::getGradientFromDT("kbrand-normal", startColor, endColor);
            d->m_pLinearGradient.setColorAt(0, startColor);
            d->m_pLinearGradient.setColorAt(1, endColor);
            painter.setBrush(d->m_pLinearGradient);
        }
    }

    if(testAttribute(Qt::WA_TranslucentBackground))
        painter.setBrush(Qt::NoBrush);

    if (ThemeController::widgetTheme() == ClassicTheme)
        painter.drawRoundedRect(this->rect().adjusted(1, 1, -1, -1), 0, 0);
    else {
        QRect drawRect = this->rect().adjusted(1, 1, 0, 0);
        QPainterPath path;
        path.moveTo(drawRect.topLeft() + QPoint(0, d->m_topLeftRadius));
        path.lineTo(drawRect.bottomLeft() - QPoint(0, d->m_bottomLeftRadius));
        path.quadTo(drawRect.bottomLeft(), drawRect.bottomLeft() + QPoint(d->m_bottomLeftRadius, 0));
        path.lineTo(drawRect.bottomRight() - QPoint(d->m_bottomRightRadius, 0));
        path.quadTo(drawRect.bottomRight(), drawRect.bottomRight() - QPoint(0, d->m_bottomRightRadius));
        path.lineTo(drawRect.topRight() + QPoint(0, d->m_topRightRadius));
        path.quadTo(drawRect.topRight(), drawRect.topRight() - QPoint(d->m_topRightRadius, 0));
        path.lineTo(drawRect.topLeft() + QPoint(d->m_topLeftRadius, 0));
        path.quadTo(drawRect.topLeft(), drawRect.topLeft() + QPoint(0, d->m_topLeftRadius));
        painter.drawPath(path);
    }

    // 绘制图标
    QRect rect(0, 0, option.iconSize.width(), option.iconSize.height());
    QPixmap pixmap = this->icon().pixmap(option.iconSize);

    if (!pixmap.isNull()) {
        if (ThemeController::isPixmapPureColor(pixmap)) {
            if(isChecked())
                pixmap = ThemeController::drawColoredPixmap(pixmap, ThemeController::getCustomColorFromDT("kwhite"));
            else
            {
                if (d->m_type == KToolButtonType::Flat || !isEnabled())
                    pixmap = ThemeController::drawColoredPixmap(pixmap, d->m_pixColor);
                else if (ThemeController::themeMode() == DarkTheme)
                    pixmap = ThemeController::drawColoredPixmap(pixmap , palette().text().color());
            }
        }
    }
    if (d->m_isIconColor) {
        pixmap = ThemeController::drawColoredPixmap(pixmap, d->m_iconColor);
    }

    if (!hasArrow()) {
        rect.moveCenter(this->rect().center());
        painter.drawPixmap(rect, pixmap);
    } else {
        QRect newRect = this->rect().adjusted(0, 0, -20, 0);
        rect.moveCenter(newRect.center());

        if (sizePolicy().horizontalPolicy() == QSizePolicy::Expanding)
            this->style()->drawItemPixmap(&painter, this->rect(), Qt::AlignCenter, pixmap);
        else
            this->style()->drawItemPixmap(&painter, rect, Qt::AlignCenter, pixmap);
        QRect arrowRect(0, 0, option.iconSize.width(), option.iconSize.height());
        arrowRect.moveLeft(this->rect().width() - option.iconSize.width() - 8);
        arrowRect.moveTop((this->height() - option.iconSize.height()) / 2);
        QPixmap arrowPixmap = d->m_arrowPixmap;
        if(isChecked())
            arrowPixmap = ThemeController::drawSymbolicColoredPixmap(arrowPixmap);
        else if (d->m_type == KToolButtonType::Flat || !isEnabled())
            arrowPixmap = ThemeController::drawColoredPixmap(arrowPixmap, d->m_pixColor);
        else if (ThemeController::themeMode() == DarkTheme)
            arrowPixmap = ThemeController::drawColoredPixmap(arrowPixmap , palette().text().color());

        if(property("isWindowButton").toBool() && !isActiveWindow())
        {
            if(ThemeController::themeMode() == LightTheme)
            {
                QIcon icon = QIcon::fromTheme("ukui-down-symbolic");
                QIcon::State state = QIcon::Off;
                if (option.state & QStyle::State_On)
                    state = QIcon::On;
                arrowPixmap = icon.pixmap(option.iconSize, QIcon::Disabled, state);
            }
            else
            {
                arrowPixmap = ThemeController::drawColoredPixmap(arrowPixmap,ThemeController::getCustomColorFromDT("kgray-14"));
            }
        }
        painter.drawPixmap(arrowRect, arrowPixmap);
    }
}

KToolButtonPrivate::KToolButtonPrivate(KToolButton *parent)
    : q_ptr(parent)
    , m_hasArrow(false)
{
    m_isIconColor = false;
    setParent(parent);
}

void KToolButtonPrivate::changePalette()
{
    Q_Q(KToolButton);

}

void KToolButtonPrivate::doLoadingFlash()
{
    Q_Q(KToolButton);
    if (m_flashState < 7)
        m_flashState++;
    else
        m_flashState = 0;
    if (ThemeController::themeMode() == LightTheme) {
        q->QToolButton::setIcon(QIcon::fromTheme(QString("ukui-loading-%1.symbolic").arg(m_flashState)));
    } else {
        q->QToolButton::setIcon(ThemeController::drawColoredPixmap(QIcon::fromTheme(QString("ukui-loading-%1.symbolic").arg(m_flashState)).pixmap(q->iconSize()), ThemeController::getCustomColorFromDT("kwhite")));
    }
}

void KToolButtonPrivate::changeTheme()
{
    Q_Q(KToolButton);

    initThemeStyle();
    changePalette();
    if (ThemeController::themeMode() == LightTheme) {
        if (q->isEnabled())
            m_pixColor = ThemeController::getCustomColorFromDT("kgray-17");
        else
            m_pixColor = ThemeController::getCustomColorFromDT("kgray-10");
    } else {
        if (q->isEnabled())
            m_pixColor = ThemeController::getCustomColorFromDT("kwhite");
        else
            m_pixColor = ThemeController::getCustomColorFromDT("kgray-14");
    }
}
}

#include "ktoolbutton.moc"
#include "moc_ktoolbutton.cpp"
