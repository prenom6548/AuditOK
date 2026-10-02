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

#include "kballontip.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QColor>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>
#include <kwindoweffects.h>
#include "accessinfohelper.h"

namespace kdk
{
class KBallonTipPrivate : public QObject, public ThemeController
{
    Q_DECLARE_PUBLIC(KBallonTip)
    Q_OBJECT

public:
    KBallonTipPrivate(KBallonTip *parent);
    void adjustStyle();
    void setAccessInfo();

protected:
    void changeTheme();

private:
    KBallonTip *q_ptr;
    QLabel *m_pContentLabel;
    QLabel *m_pIconLabel;
    TipType m_type;
    QIcon m_icon;
    QPixmap m_variablePixmap;
    QPixmap m_pixmap;
    QColor m_color;
    QHBoxLayout *m_pLayout;
    QVBoxLayout *m_pVLayout;
    int m_time;
    QGraphicsDropShadowEffect *m_pShadow_effect;
    QTimer *m_pTimer;
    int m_flashState;
};

KBallonTip::KBallonTip(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KBallonTipPrivate(this))
{
    Q_D(KBallonTip);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlag(Qt::ToolTip);
    setFixedWidth(544);
    d->m_pVLayout = new QVBoxLayout(this);
    d->m_pLayout = new QHBoxLayout();
    d->m_pLayout->setSpacing(12);
    this->setContentsMargins(24, 18, 24, 18);
    d->m_pContentLabel = new QLabel(this);
    //    d->m_pContentLabel->setWordWrap(true);
    // 设置具体阴影
    d->m_pShadow_effect = new QGraphicsDropShadowEffect(this);
    d->m_pShadow_effect->setOffset(0, 0);
    // 阴影颜色
    d->m_pShadow_effect->setColor(ThemeController::getPaletteColorFromDT("text-disable"));
    // 阴影半径
    if (ThemeController::themeMode() == LightTheme)
        d->m_pShadow_effect->setBlurRadius(15);
    else
        d->m_pShadow_effect->setBlurRadius(0);
    this->setGraphicsEffect(d->m_pShadow_effect);

    d->m_pIconLabel = new QLabel(this);
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(d->m_pIconLabel);
    //    vLayout->addStretch();
    d->m_pLayout->addLayout(vLayout);
    d->m_pLayout->addWidget(d->m_pContentLabel);
    d->m_pLayout->setAlignment(Qt::AlignVCenter);
    d->m_type = TipType::Nothing;
    d->m_pVLayout->addStretch();
    d->m_pVLayout->addLayout(d->m_pLayout);
    d->m_pVLayout->addStretch();

    d->adjustStyle();
    d->m_pVLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    if (ThemeController::systemFontSize() > 11)
        vLayout->setContentsMargins(0, (ThemeController::systemFontSize() - 11), 0, 0);
    else
        vLayout->setContentsMargins(0, 0, 0, 0);

    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if (key.contains("systemFontSize"))
        {
            if (ThemeController::systemFontSize() > 11)
                vLayout->setContentsMargins(0, (ThemeController::systemFontSize() - 11), 0, 0);
            else
                vLayout->setContentsMargins(0, 0, 0, 0);
        }
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            if(key == "styleName")
            {
                if (ThemeController::themeMode() == LightTheme)
                    d->m_pShadow_effect->setBlurRadius(15);
                else
                    d->m_pShadow_effect->setBlurRadius(0);
            }
            d->changeTheme();
        }
    });

    connect(d->m_pTimer, &QTimer::timeout, this, [=]() {
        if (d->m_flashState < 7)
            d->m_flashState++;
        else
            d->m_flashState = 0;
        if (ThemeController::themeMode() == LightTheme) {
            d->m_pIconLabel->setPixmap(QIcon::fromTheme(QString("ukui-loading-%1.symbolic").arg(d->m_flashState)).pixmap(22, 22));
        } else {
            d->m_pIconLabel->setPixmap(ThemeController::drawColoredPixmap(QIcon::fromTheme(QString("ukui-loading-%1.symbolic").arg(d->m_flashState)).pixmap(22, 22), ThemeController::getCustomColorFromDT("kwhite")));
        }
    });

    d->setAccessInfo();
}

KBallonTip::KBallonTip(const QString &content, const TipType &type, QWidget *parent)
    : KBallonTip(parent)
{
    Q_D(KBallonTip);
    d->m_pContentLabel->setText(content);
    d->m_type = type;
    d->adjustStyle();
    setAttribute(Qt::WA_TranslucentBackground);
}

void KBallonTip::showInfo()
{
    Q_D(KBallonTip);

    show();
    QTimer *timer = new QTimer(this);
    timer->start(d->m_time);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeupDestroy()));
}

void KBallonTip::setTipType(const TipType &type)
{
    Q_D(KBallonTip);
    d->m_type = type;
    d->adjustStyle();
}

TipType KBallonTip::tipType()
{
    Q_D(KBallonTip);
    return d->m_type;
}

void KBallonTip::setText(const QString &text)
{
    Q_D(KBallonTip);
    d->m_pContentLabel->setText(text);
}

QString KBallonTip::text()
{
    Q_D(KBallonTip);
    return d->m_pContentLabel->text();
}

void KBallonTip::setContentsMargins(int left, int top, int right, int bottom)
{
    Q_D(KBallonTip);
    d->m_pLayout->setContentsMargins(left, top, right, bottom);
    repaint();
}

void KBallonTip::setContentsMargins(const QMargins &margins)
{
    Q_D(KBallonTip);
    d->m_pLayout->setContentsMargins(margins);
    repaint();
}

void KBallonTip::setTipTime(int my_time)
{
    Q_D(KBallonTip);
    d->m_time = my_time;
}

QHBoxLayout *KBallonTip::hBoxLayout()
{
    Q_D(KBallonTip);
    return d->m_pLayout;
}

void KBallonTip::setVariableIcon(QIcon icon)
{
    Q_D(KBallonTip);
    d->m_variablePixmap = icon.pixmap(22, 22);
    update();
}

QPixmap KBallonTip::variableIcon()
{
    Q_D(KBallonTip);
    return d->m_variablePixmap;
}

void KBallonTip::onTimeupDestroy()
{
    this->close();
}

void KBallonTip::paintEvent(QPaintEvent *event)
{
    Q_D(KBallonTip);
    // bug 173714 173165
    adjustSize();

    d->m_pShadow_effect->setColor(ThemeController::getPaletteColorFromDT("text-disable"));
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;

    auto color = ThemeController::getCustomColorFromDT("kline-window-inactive");
    QPen pen;
    pen.setColor(color);
    pen.setWidthF(0.2);
    painter.setPen(pen);
    painter.setBrush(ThemeController::getPaletteColorFromDT("window-active"));
    QRect rect = this->rect();
    if (!mask().isNull())
        clearMask();
    painter.setBrush(ThemeController::getPaletteColorFromDT("window-active"));
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1)
        radius = 6;
    painter.drawRoundedRect(rect.adjusted(8, 8, -8, -8), radius, radius);
    if (!d->m_variablePixmap.isNull()) {
        if (ThemeController::isPixmapPureColor(d->m_variablePixmap)) {
            if (ThemeController::themeMode() == LightTheme)
                d->m_pixmap = d->m_variablePixmap;
            else
                d->m_pixmap = ThemeController::drawColoredPixmap(d->m_variablePixmap, Qt::white);
        }
    }
    d->adjustStyle();
}

KBallonTipPrivate::KBallonTipPrivate(KBallonTip *parent)
    : q_ptr(parent)
{
    Q_Q(KBallonTip);
    m_pTimer = new QTimer(this);
    m_pTimer->setInterval(100);
    m_time = 1000;
    setParent(parent);
}

void KBallonTipPrivate::adjustStyle()
{
    Q_Q(KBallonTip);
    switch (m_type) {
    case TipType::Nothing:
        m_icon = QIcon();
        m_pIconLabel->setPixmap(m_icon.pixmap(22, 22));
        m_pIconLabel->hide();
        if (m_pTimer->isActive())
            m_pTimer->stop();
        break;
    case TipType::Normal:
        //        m_icon = QIcon::fromTheme("ukui-dialog-success");
        m_icon = QIcon::fromTheme("ukui-dialog-success", QIcon::fromTheme("emblem-default"));
        m_pIconLabel->setPixmap(m_icon.pixmap(22, 22));
        m_pIconLabel->show();
        if (m_pTimer->isActive())
            m_pTimer->stop();
        break;
    case TipType::Info:;
        m_icon = QIcon::fromTheme("dialog-info");
        m_pIconLabel->setPixmap(m_icon.pixmap(22, 22));
        m_pIconLabel->show();
        if (m_pTimer->isActive())
            m_pTimer->stop();
        break;
    case TipType::Warning:
        m_icon = QIcon::fromTheme("dialog-warning");
        m_pIconLabel->setPixmap(m_icon.pixmap(22, 22));
        m_pIconLabel->show();
        if (m_pTimer->isActive())
            m_pTimer->stop();
        break;
    case TipType::Error:
        m_icon = QIcon::fromTheme("dialog-error");
        m_pIconLabel->setPixmap(m_icon.pixmap(22, 22));
        m_pIconLabel->show();
        if (m_pTimer->isActive())
            m_pTimer->stop();
        break;
    case TipType::Variable:
        m_pIconLabel->setPixmap(m_pixmap);
        m_pIconLabel->show();
        if (m_pTimer->isActive())
            m_pTimer->stop();
        break;
    case TipType::Loading:
        if (!m_pTimer->isActive()) {
            m_pTimer->start();
            m_pIconLabel->show();
        }
        break;
    default:
        break;
    }
    m_pIconLabel->setFixedSize(22, 22);
    //    m_pContentLabel->setAlignment(Qt::AlignLeft);
}

void KBallonTipPrivate::setAccessInfo()
{
    Q_Q(KBallonTip);
    KDK_ALL_INFO_FORMAT(m_pContentLabel,"");
    KDK_ALL_INFO_FORMAT(m_pIconLabel,"");
    KDK_OBJ_INFO_FORMAT(m_pShadow_effect);
}

void KBallonTipPrivate::changeTheme()
{
    Q_Q(KBallonTip);
    m_pShadow_effect->setOffset(0, 0);
    // 阴影颜色
    m_pShadow_effect->setColor(ThemeController::getPaletteColorFromDT("text-disable"));
    // 阴影半径
    if (ThemeController::themeMode() == LightTheme) {
        m_pShadow_effect->setBlurRadius(15);
    } else
        m_pShadow_effect->setBlurRadius(0);
    q->setGraphicsEffect(m_pShadow_effect);
}

}

#include "kballontip.moc"
#include "moc_kballontip.cpp"
