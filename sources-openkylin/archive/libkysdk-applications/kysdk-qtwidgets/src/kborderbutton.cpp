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

#include "kborderbutton.h"
#include "parmscontroller.h"
#include "themeController.h"
#include <QDebug>
#include <QIcon>
#include <QPainter>
#include <QStyleOptionButton>

namespace kdk
{
class KBorderButtonPrivate : public QObject, public ThemeController
{
    Q_DECLARE_PUBLIC(KBorderButton)
    Q_OBJECT
public:
    KBorderButtonPrivate(KBorderButton *parent);

protected:
    void changeTheme();

private:
    KBorderButton *q_ptr;
    bool m_hoverd;
    int m_radius;
};
KBorderButton::KBorderButton(QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new KBorderButtonPrivate(this))
{
    Q_D(KBorderButton);
    connect(Parmscontroller::self(), &Parmscontroller::modeChanged, this, [=]() {
        updateGeometry();
    });
}

KBorderButton::~KBorderButton()
{
}

QSize KBorderButton::sizeHint() const
{
    auto size = QPushButton::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_PushButtonHeight));
    return size;
}
bool KBorderButton::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KBorderButton);
    return QPushButton::eventFilter(watched, event);
}

void KBorderButton::paintEvent(QPaintEvent *event)
{
    Q_D(KBorderButton);
    QStyleOptionButton option;
    initStyleOption(&option);
    QPainter p(this);
    QColor borderColor;
    QColor fontColor;
    int borderWidth = 1;
    QColor startColor;
    QColor endColor;
    if (!option.state.testFlag(QStyle::State_Enabled)) {
        borderColor = ThemeController::getCustomColorFromDT("kfont-primary-disable");
        fontColor = ThemeController::getCustomColorFromDT("kfont-primary-disable");
    } else if (option.state.testFlag(QStyle::State_MouseOver)) {
        if (option.state.testFlag(QStyle::State_Sunken))
        {
            ThemeController::getGradientFromDT("kbrand-click",startColor,endColor);
            fontColor = endColor;
            borderColor  = endColor;
        }
        else
        {
            ThemeController::getGradientFromDT("kbrand-hover",startColor,endColor);
            fontColor = endColor;
            borderColor = endColor;
        }
    }
    else
    {
        fontColor = ThemeController::getPaletteColorFromDT("buttontext-active");
        borderColor = ThemeController::getPaletteColorFromDT("button-active");
    }
    p.setBrush(Qt::NoBrush);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    p.save();
    pen.setWidth(borderWidth);
    pen.setColor(borderColor);
    p.setPen(pen);
    p.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), d->m_radius, d->m_radius);
    p.restore();
    pen.setWidth(1);
    pen.setColor(fontColor);
    p.setPen(pen);
    QPoint point;
    QRect ir = option.rect;
    uint tf = Qt::AlignVCenter;
    if (!option.icon.isNull()) {
        // Center both icon and text

        QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                : QIcon::Disabled;
        if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
            mode = QIcon::Active;
        QIcon::State state = QIcon::Off;
        if (option.state & QStyle::State_On)
            state = QIcon::On;

        QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);
        pixmap = ThemeController::drawColoredPixmap(pixmap, fontColor);
        int w = pixmap.width() / pixmap.devicePixelRatio();
        int h = pixmap.height() / pixmap.devicePixelRatio();

        if (!option.text.isEmpty())
            w += option.fontMetrics.boundingRect(option.rect, tf, option.text).width() + 2;

        point = QPoint(ir.x() + ir.width() / 2 - w / 2,
                       ir.y() + ir.height() / 2 - h / 2);

        w = pixmap.width() / pixmap.devicePixelRatio();

        if (option.direction == Qt::RightToLeft)
            point.rx() += w;

        p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);

        if (option.direction == Qt::RightToLeft)
            ir.translate(-point.x() - 2, 0);
        else
            ir.translate(point.x() + w + 4, 0);

        // left-align text if there is
        if (!option.text.isEmpty())
            tf |= Qt::AlignLeft;
    } else {
        tf |= Qt::AlignHCenter;
    }
    p.drawText(ir, tf, option.text);
}

KBorderButton::KBorderButton(const QString &text, QWidget *parent)
    : KBorderButton(parent)
{
    setText(text);
}

KBorderButton::KBorderButton(const QIcon &icon, const QString &text, QWidget *parent)
    : KBorderButton(parent)
{
    setIcon(icon);
    setText(text);
}

void KBorderButton::setIcon(const QIcon &icon)
{
    Q_D(KBorderButton);
    QPushButton::setIcon(icon);
}

KBorderButtonPrivate::KBorderButtonPrivate(KBorderButton *parent)
    : q_ptr(parent)
    , m_hoverd(false)
{
    setParent(parent);
    m_radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (m_radius == -1)
        m_radius = 6;
}

void KBorderButtonPrivate::changeTheme()
{
    Q_Q(KBorderButton);
    initThemeStyle();
}

}
#include "kborderbutton.moc"
#include "moc_kborderbutton.cpp"
