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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "kpressbutton.h"
#include "themeController.h"
#include <QDebug>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>
#include <QTimer>

namespace kdk
{

class KPressButtonPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KPressButton)
public:
    KPressButtonPrivate(KPressButton *parent);
    void changeTheme();

private:
    KPressButton *q_ptr;
    QColor m_pbkgColor;
    QTimer *m_pTimer;
    KPressButton::ButtonType m_pButtonType;
    QIcon m_pIcon;
    int m_flashState;
    int m_pTopLeftRadius;
    int m_pTopRightRadius;
    int m_pBottomLeftRadius;
    int m_pBottomRightRadius;
    bool m_pLoading; // loafing标志位
    bool m_pCheackable; // 开启标志位
    bool m_pTranslucent;
    bool m_isTranslucentFlag;
};

KPressButtonPrivate::KPressButtonPrivate(KPressButton *parent)
    : q_ptr(parent)
{
    Q_Q(KPressButton);
}

void KPressButtonPrivate::changeTheme()
{
    Q_Q(KPressButton);
    if (m_isTranslucentFlag) {
        if (ThemeController::widgetTheme() == ClassicTheme)
            m_pTranslucent = false;
        else
            m_pTranslucent = true;
    } else {
        m_pTranslucent = false;
    }
    initThemeStyle();
}

KPressButton::KPressButton(QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new KPressButtonPrivate(this))
{
    Q_D(KPressButton);
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1) {
        d->m_pTopLeftRadius = 6;
        d->m_pTopRightRadius = 6;
        d->m_pBottomLeftRadius = 6;
        d->m_pBottomRightRadius = 6;
    } else {
        d->m_pTopLeftRadius = radius;
        d->m_pTopRightRadius = radius;
        d->m_pBottomLeftRadius = radius;
        d->m_pBottomRightRadius = radius;
    }
    d->m_pButtonType = KPressButton::NormalType;
    d->m_pCheackable = false;
    d->m_pTranslucent = false;
    d->m_isTranslucentFlag = false;
    d->m_pTimer = new QTimer(this);
    d->m_pTimer->setInterval(100);
    d->m_flashState = 0;
    setCheckable(true);
    connect(d->m_pTimer, &QTimer::timeout, this, [=]() {
        if (d->m_flashState < 7)
            d->m_flashState++;
        else
            d->m_flashState = 0;
        setIcon(QIcon::fromTheme(QString("ukui-loading-%1.symbolic").arg(d->m_flashState)));
    });
    connect(d->m_gsetting, &QGSettings::changed, this, [=]() {
        d->changeTheme();
    });
}

void KPressButton::setBorderRadius(int radius)
{
    Q_D(KPressButton);
    d->m_pTopLeftRadius = radius;
    d->m_pTopRightRadius = radius;
    d->m_pBottomLeftRadius = radius;
    d->m_pBottomRightRadius = radius;
}

void KPressButton::setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight)
{
    Q_D(KPressButton);
    d->m_pBottomLeftRadius = bottomLeft;
    d->m_pTopLeftRadius = topLeft;
    d->m_pTopRightRadius = topRight;
    d->m_pBottomRightRadius = bottomRight;
}

void KPressButton::setCheckable(bool flag)
{
    QPushButton::setCheckable(flag);
}

bool KPressButton::isCheckable() const
{
    return QPushButton::isCheckable();
}

void KPressButton::setChecked(bool flag)
{
    QPushButton::setChecked(flag);
}

bool KPressButton::isChecked() const
{
    return QPushButton::isChecked();
}

void KPressButton::setButtonType(ButtonType type)
{
    Q_D(KPressButton);
    d->m_pButtonType = type;
}

KPressButton::ButtonType KPressButton::buttonType()
{
    Q_D(KPressButton);
    return d->m_pButtonType;
}

void KPressButton::setLoaingStatus(bool flag)
{
    Q_D(KPressButton);
    d->m_pLoading = flag;
    if (flag)
        d->m_pTimer->start();
    else
        d->m_pTimer->stop();
    update();
}

bool KPressButton::isLoading()
{
    Q_D(KPressButton);
    return d->m_pLoading;
}

void KPressButton::setTranslucent(bool flag)
{
    Q_D(KPressButton);
    d->m_isTranslucentFlag = flag;
    d->m_pTranslucent = flag;
    d->changeTheme();
}

bool KPressButton::isTranslucent()
{
    Q_D(KPressButton);
    return d->m_pTranslucent;
}

void KPressButton::paintEvent(QPaintEvent *event)
{
    Q_D(KPressButton);
    QStyleOptionButton opt;
    initStyleOption(&opt);
    QPainter p(this);
    QColor borderColor;
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    if (ThemeController::widgetTheme() == ClassicTheme) {
        d->m_pTranslucent = false;
        if (p.pen().width() == 1)
            p.translate(0.5, 0.5);
    }

    if (d->m_pTranslucent)
    {
        if (isChecked() /*&& !isLoading()*/)
            d->m_pbkgColor = ThemeController::getPaletteColorFromDT("highlight-active");
        else {
            d->m_pbkgColor = ThemeController::getCustomColorFromDT("kcomponent-alpha-normal");
        }

        if (opt.state.testFlag(QStyle::State_MouseOver) && isCheckable() /*&& !isLoading()*/) {
            if (isChecked()) {
                if (opt.state & QStyle::State_Sunken) {
                    d->m_pbkgColor = ThemeController::mixColor(d->m_pbkgColor, ThemeController::getPaletteColorFromDT("brighttext-active"), 0.2); // 点击变深
                } else {
                    d->m_pbkgColor = ThemeController::mixColor(d->m_pbkgColor, ThemeController::getPaletteColorFromDT("brighttext-active"), 0.05); // 悬浮变浅
                }
            } else {
                if (opt.state & QStyle::State_Sunken) {
                    d->m_pbkgColor = ThemeController::getCustomColorFromDT("kcomponent-alpha-click"); // 点击变深
                } else {
                    d->m_pbkgColor = ThemeController::getCustomColorFromDT("kcomponent-alpha-hover"); // 悬浮变浅
                }
            }
        } else if (opt.state.testFlag(QStyle::State_Selected)) {
            if (isChecked())
                d->m_pbkgColor = ThemeController::getPaletteColorFromDT("highlight-active");
            else {
                d->m_pbkgColor = ThemeController::getCustomColorFromDT("kcomponent-alpha-normal");
            }
        }
    } else // 关闭透明度
    {
        if (isChecked() /*&& !isLoading()*/)
            d->m_pbkgColor = ThemeController::getPaletteColorFromDT("highlight-active");
        else
            d->m_pbkgColor = ThemeController::getPaletteColorFromDT("button-active");


        if (opt.state.testFlag(QStyle::State_MouseOver) && isCheckable() /*&& !isLoading()*/)
        {
            if (isChecked())
            {
                if (opt.state & QStyle::State_Sunken)
                    d->m_pbkgColor = ThemeController::mixColor(d->m_pbkgColor, ThemeController::getPaletteColorFromDT("brighttext-active"), 0.2); // 点击变深
                else
                    d->m_pbkgColor = ThemeController::mixColor(d->m_pbkgColor, ThemeController::getPaletteColorFromDT("brighttext-active"), 0.05); // 悬浮变浅
            }
            else
            {
                if (opt.state & QStyle::State_Sunken)
                    d->m_pbkgColor = ThemeController::getCustomColorFromDT("kcomponent-click");
                else
                    d->m_pbkgColor = ThemeController::getCustomColorFromDT("kcomponent-hover");
            }
        }
        else if (opt.state.testFlag(QStyle::State_Selected))
        {
            if (isChecked())
                d->m_pbkgColor = ThemeController::getPaletteColorFromDT("highlight-active");
            else
                d->m_pbkgColor = ThemeController::getPaletteColorFromDT("button-active");
        }
    }

    if (opt.icon.name() != "ukui-loading-0.symbolic" && opt.icon.name() != "ukui-loading-1.symbolic" && opt.icon.name() != "ukui-loading-2.symbolic" && opt.icon.name() != "ukui-loading-3.symbolic" && opt.icon.name() != "ukui-loading-4.symbolic" && opt.icon.name() != "ukui-loading-5.symbolic" && opt.icon.name() != "ukui-loading-6.symbolic" && opt.icon.name() != "ukui-loading-7.symbolic")
        d->m_pIcon = opt.icon;

    QRect rect = this->rect();
    QPainterPath path;
    switch (d->m_pButtonType) {
    case NormalType:
        if (ThemeController::widgetTheme() == ClassicTheme) {
            path.addRect(rect.adjusted(0, 0, -1, -1));
            p.setPen(borderColor);
        } else {
            path.moveTo(rect.topLeft() + QPointF(0, d->m_pTopLeftRadius));
            path.lineTo(rect.bottomLeft() - QPointF(0, d->m_pBottomLeftRadius));
            path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(d->m_pBottomLeftRadius, 0));
            path.lineTo(rect.bottomRight() - QPointF(d->m_pBottomRightRadius, 0));
            path.quadTo(rect.bottomRight(), rect.bottomRight() - QPointF(0, d->m_pBottomRightRadius));
            path.lineTo(rect.topRight() + QPointF(0, d->m_pTopRightRadius));
            path.quadTo(rect.topRight(), rect.topRight() - QPointF(d->m_pTopRightRadius, 0));
            path.lineTo(rect.topLeft() + QPointF(d->m_pTopLeftRadius, 0));
            path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, d->m_pTopLeftRadius));
            p.setPen(Qt::NoPen);
        }
        p.setBrush(d->m_pbkgColor);
        p.drawPath(path);
        break;
    case CircleType:
        if (ThemeController::widgetTheme() == ClassicTheme)
            p.setPen(borderColor);
        else
            p.setPen(Qt::NoPen);
        p.setBrush(d->m_pbkgColor);
        p.drawEllipse(rect.adjusted(1, 1, -1, -1));
    default:
        break;
    }

    int iconSize = opt.iconSize.width();

    if(!isEnabled())
    {
        opt.icon = ThemeController::drawColoredPixmap(opt.icon.pixmap(iconSize, iconSize), ThemeController::getPaletteColorFromDT("buttontext-disable"));
        d->m_pIcon = ThemeController::drawColoredPixmap(d->m_pIcon.pixmap(iconSize, iconSize), ThemeController::getPaletteColorFromDT("buttontext-disable"));
    }
    else if (isChecked() || ThemeController::themeMode() == DarkTheme) {
        opt.icon = ThemeController::drawColoredPixmap(opt.icon.pixmap(iconSize, iconSize), QColor(255, 255, 255));
        d->m_pIcon = ThemeController::drawColoredPixmap(d->m_pIcon.pixmap(iconSize, iconSize), QColor(255, 255, 255));
    } else {
        opt.icon = ThemeController::drawColoredPixmap(opt.icon.pixmap(iconSize, iconSize), QColor(0, 0, 0));
        d->m_pIcon = ThemeController::drawColoredPixmap(d->m_pIcon.pixmap(iconSize, iconSize), QColor(0, 0, 0));
    }

    if (isLoading())
        p.drawPixmap(rect.center().x() - iconSize / 2, rect.center().y() - iconSize / 2, iconSize, iconSize, opt.icon.pixmap(iconSize, iconSize)); // opt.icon loading
    else
        p.drawPixmap(rect.center().x() - iconSize / 2, rect.center().y() - iconSize / 2, iconSize, iconSize, d->m_pIcon.pixmap(iconSize, iconSize));
}

}

#include "kpressbutton.moc"
#include "moc_kpressbutton.cpp"
