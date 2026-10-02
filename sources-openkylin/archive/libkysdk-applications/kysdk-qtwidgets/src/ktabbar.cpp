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

#include "ktabbar.h"
#include "parmscontroller.h"
#include "themeController.h"
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QVariantAnimation>

namespace kdk
{
class KTabBarPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTabBar)

public:
    KTabBarPrivate(KTabBar *parent)
        : q_ptr(parent)
        , m_animation(new QVariantAnimation(this))
        , m_nextTabWidth(0)
        , m_animationStarted(false)
    {
        Q_Q(KTabBar);
        setParent(parent);
        parent->installEventFilter(this);
        m_animation->setDuration(300);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        connect(m_animation, &QVariantAnimation::valueChanged, q, [=]() {
            q->update();
        });
        connect(m_animation, &QVariantAnimation::finished, this, [=]() {
            m_animationStarted = false;
            q->update();
        });
    }
    void changeTheme();
    int getIndexAtPos(const QPoint &p);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    KTabBar *q_ptr;
    KTabBarStyle m_kTabBarStyle;
    int m_borderRadius = 6;
    int m_horizontalMargin = 1;
    int m_topMargin = 0;
    QColor m_bkgrdColor;
    bool m_useCustomColor;
    QVariantAnimation *m_animation;
    int m_nextTabWidth;
    bool m_animationStarted;
    QLinearGradient m_linearGradient ;
};

KTabBar::KTabBar(KTabBarStyle barStyle, QWidget *parent)
    : QTabBar(parent)
    , d_ptr(new KTabBarPrivate(this))
{
    Q_D(KTabBar);
    d->m_borderRadius = ThemeController::getRadiusFromDT("kradius-normal");
    if (d->m_borderRadius == -1)
        d->m_borderRadius = 6;
    d->m_kTabBarStyle = barStyle;
    d->m_useCustomColor = false;

    // this->setObjectName("KTabbar");
    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            d->changeTheme();
        }
    });
    connect(Parmscontroller::self(), &Parmscontroller::modeChanged, this, [=](bool flag) {
        updateGeometry();
    });
}

KTabBar::~KTabBar()
{
}

void KTabBar::setTabBarStyle(KTabBarStyle barStyle)
{
    Q_D(KTabBar);
    d->m_kTabBarStyle = barStyle;
    d->changeTheme();
}
KTabBarStyle KTabBar::barStyle()
{
    Q_D(KTabBar);
    return d->m_kTabBarStyle;
}

void KTabBar::setBorderRadius(int radius)
{
    Q_D(KTabBar);
    if (radius < 0 || radius > 20)
        return;
    d->m_borderRadius = radius;
    update();
}

int KTabBar::borderRadius()
{
    Q_D(KTabBar);
    if (d->m_kTabBarStyle == Sliding)
        return 0;
    else
        return d->m_borderRadius;
}

void KTabBar::setBackgroundColor(const QColor &color)
{
    Q_D(KTabBar);
    d->m_useCustomColor = true;
    d->m_bkgrdColor = color;
}

QSize KTabBar::sizeHint() const
{
    auto size = QTabBar::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_TabBarHeight));
    return size;
}

QSize KTabBar::minimumTabSizeHint(int index) const
{
    Q_UNUSED(index)
    QSize size(100, Parmscontroller::parm(Parmscontroller::Parm::PM_PushButtonHeight));
    return size;
}

QSize KTabBar::tabSizeHint(int index) const
{
    Q_UNUSED(index)
    auto size = QTabBar::tabSizeHint(index);
    if(this->maximumHeight() == this->minimumHeight())
        size.setHeight(this->minimumHeight());
    else
        size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_TabBarHeight));
    return size;
}

void KTabBar::paintEvent(QPaintEvent *event)
{
    Q_D(KTabBar);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    QColor fontColor;
    QColor mix;
    QColor borderColor;
    QColor startColor;
    QColor endColor;

    QFontMetrics fm = p.fontMetrics();
    for (int i = 0; i < count(); ++i) {
        QStyleOptionTab option;
        initStyleOption(&option, i);
        QRect rect = option.rect.adjusted(d->m_horizontalMargin, 0, 0, -d->m_topMargin);
        rect.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_PushButtonHeight));
        d->m_linearGradient = QLinearGradient(rect.left() + rect.width()/2, rect.top(),rect.left() + rect.width()/2, rect.bottom());
        switch (d->m_kTabBarStyle) {
        case SegmentDark: {
            mix = ThemeController::getPaletteColorFromDT("brighttext-active");
            fontColor = ThemeController::getPaletteColorFromDT("buttontext-active");
            QColor bkgrdColor = d->m_bkgrdColor;
            d->m_linearGradient.setColorAt(0,bkgrdColor);
            d->m_linearGradient.setColorAt(1,bkgrdColor);
            borderColor = Qt::transparent;
            if (option.state.testFlag(QStyle::State_Selected)) {
                if (option.state.testFlag(QStyle::State_Sunken)) {
                    borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
                    ThemeController::getGradientFromDT("kbrand-click", startColor, endColor);
                }
                else if (option.state.testFlag(QStyle::State_MouseOver)) {
                    borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
                    ThemeController::getGradientFromDT("kbrand-hover", startColor, endColor);
                }
                else {
                    borderColor = ThemeController::getCustomColorFromDT("kline-component-normal");
                    ThemeController::getGradientFromDT("kbrand-normal", startColor, endColor);
                }
                d->m_linearGradient.setColorAt(0,startColor);
                d->m_linearGradient.setColorAt(1,endColor);
                fontColor = ThemeController::getCustomColorFromDT("kwhite");
            } else if (option.state.testFlag(QStyle::State_MouseOver)) {
                ThemeController::getGradientFromDT("kcomponent-hover",startColor,endColor);
                d->m_linearGradient.setColorAt(0,startColor);
                d->m_linearGradient.setColorAt(1,endColor);
            }
            if (option.position == QStyleOptionTab::Middle) {
                p.save();
                p.setBrush(d->m_linearGradient);
                p.setPen(borderColor);
                // 利用quadto绘制圆角矩形会出现一个像素的偏差，修正一下QRect底部高度
                p.drawRect(rect.adjusted(0, 1, -1, -1)); // 调整启典 上描边
                p.restore();
                p.setBrush(Qt::NoBrush);
                p.setPen(fontColor);
                QPoint point;
                uint tf = Qt::AlignVCenter;
                if (!option.icon.isNull()) {
                    QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                            : QIcon::Disabled;
                    if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (option.state & QStyle::State_On)
                        state = QIcon::On;

                    QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);
                    if(ThemeController::isPixmapPureColor(pixmap))
                        pixmap = ThemeController::drawColoredPixmap(pixmap,fontColor);


                    int w = pixmap.width() / pixmap.devicePixelRatio();
                    int h = pixmap.height() / pixmap.devicePixelRatio();

                    if (!tabText(i).isEmpty())
                        w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 2;

                    point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                                   rect.y() + rect.height() / 2 - h / 2);

                    w = pixmap.width() / pixmap.devicePixelRatio();

                    //                    if (option.direction == Qt::RightToLeft)
                    //                        point.rx() += w;

                    if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7)
                        p.drawPixmap(option.rect.x() + 4, this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                    else {
                        if (option.direction == Qt::RightToLeft)
                            p.drawPixmap(point, pixmap);
                        else
                            p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);
                    }

                    if (!tabText(i).isEmpty()) {
                        int subH = std::max(option.iconSize.height(), option.fontMetrics.height());
                        int icon_Y = (rect.height() - subH) / 2;
                        int text_X = point.x() + option.iconSize.width() + 4;
                        int text_Y = icon_Y;

                        QRect textRect;
                        if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7) {
                            textRect = QRect(option.rect.x() + option.iconSize.width() + 8, text_Y, option.rect.width() - option.iconSize.width() - 7, option.fontMetrics.height());
                            setTabToolTip(i, tabText(i));
                            QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width() - option.iconSize.width() - 7);
                            p.drawText(textRect, tf, elidedText);
                        } else {
                            textRect = QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                            setTabToolTip(i, "");
                            p.drawText(textRect, tf, tabText(i));
                        }
                    }
                } else {
                    tf |= Qt::AlignHCenter;
                    if (fm.width(tabText(i)) >= option.rect.width()) {
                        QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width());
                        p.drawText(rect, tf, elidedText);
                        setTabToolTip(i, tabText(i));
                    } else {
                        setTabToolTip(i, "");
                        p.drawText(rect, tf, tabText(i));
                    }
                }
            } else if (option.position == QStyleOptionTab::Beginning) {
                p.save();
                p.setBrush(d->m_linearGradient);
                p.setPen(Qt::NoPen);
                QPainterPath path;
                auto tempRect = rect.adjusted(0, 1, 0, 0); // 调整启典 上描边
                if (layoutDirection() == Qt::LeftToRight)
                {
                    path.moveTo(tempRect.topLeft() + QPointF(0, d->m_borderRadius));
                    path.lineTo(tempRect.bottomLeft() - QPointF(0, d->m_borderRadius));
                    path.quadTo(tempRect.bottomLeft(), tempRect.bottomLeft() + QPointF(d->m_borderRadius, 0));
                    path.lineTo(tempRect.bottomRight());
                    path.lineTo(tempRect.topRight());
                    path.lineTo(tempRect.topLeft() + QPointF(d->m_borderRadius, 0));
                    path.quadTo(tempRect.topLeft(), tempRect.topLeft() + QPointF(0, d->m_borderRadius));
                }
                else
                {
                    path.moveTo(tempRect.topLeft());
                    path.lineTo(tempRect.bottomLeft());
                    path.lineTo(tempRect.bottomRight() - QPointF(d->m_borderRadius, 0));
                    path.quadTo(tempRect.bottomRight(), tempRect.bottomRight() - QPointF(0, d->m_borderRadius));
                    path.lineTo(tempRect.topRight() + QPointF(0, d->m_borderRadius));
                    path.quadTo(tempRect.topRight(), tempRect.topRight() - QPointF(d->m_borderRadius, 0));
                    path.lineTo(tempRect.topLeft());
                }
                p.drawPath(path);
                p.restore();
                p.setBrush(Qt::NoBrush);
                p.setPen(fontColor);
                QPoint point;
                uint tf = Qt::AlignVCenter;
                if (!option.icon.isNull()) {
                    QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                            : QIcon::Disabled;
                    if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (option.state & QStyle::State_On)
                        state = QIcon::On;

                    QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);
                    if(ThemeController::isPixmapPureColor(pixmap))
                        pixmap = ThemeController::drawColoredPixmap(pixmap,fontColor);

                    int w = pixmap.width() / pixmap.devicePixelRatio();
                    int h = pixmap.height() / pixmap.devicePixelRatio();

                    if (!tabText(i).isEmpty())
                        w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 2;

                    point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                                   rect.y() + rect.height() / 2 - h / 2);

                    w = pixmap.width() / pixmap.devicePixelRatio();

                    //                    if (option.direction == Qt::RightToLeft)
                    //                        point.rx() += w;

                    // p.setBrush(Qt::red);
                    // p.drawRect(option.rect.x() + 4, this->style()->visualPos(option.direction, option.rect, point).y(), pixmap.width(), pixmap.height());
                    // p.setBrush(Qt::NoBrush);

                    if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7)
                        p.drawPixmap(option.rect.x() + 4, this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                    else {
                        if (option.direction == Qt::RightToLeft)
                            p.drawPixmap(point, pixmap);
                        else
                            p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);
                    }

                    if (option.direction == Qt::RightToLeft)
                        rect.translate(-point.x() - 2, 0);
                    else
                        rect.translate(point.x() + w + 4, 0);

                    if (!tabText(i).isEmpty()) {
                        tf |= Qt::AlignLeft;
                        int subH = std::max(option.iconSize.height(), option.fontMetrics.height());
                        int icon_Y = (rect.height() - subH) / 2;
                        int text_X = point.x() + option.iconSize.width() + 4;
                        int text_Y = icon_Y;
                        QRect textRect;
                        if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7) {
                            textRect = QRect(option.rect.x() + option.iconSize.width() + 8, text_Y, option.rect.width() - option.iconSize.width() - 8, option.fontMetrics.height());
                            setTabToolTip(i, tabText(i));
                            QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width() - option.iconSize.width() - 8);
                            p.drawText(textRect, tf, elidedText);
                        } else {
                            textRect = QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                            setTabToolTip(i, "");
                            p.drawText(textRect, tf, tabText(i));
                        }
                    }
                } else {
                    tf |= Qt::AlignHCenter;
                    if (fm.width(tabText(i)) >= option.rect.width()) {
                        QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width());
                        p.drawText(rect, tf, elidedText);
                        setTabToolTip(i, tabText(i));
                    } else {
                        p.drawText(rect, tf, tabText(i));
                        setTabToolTip(i, "");
                    }
                }
            } else {
                p.save();
                p.setBrush(d->m_linearGradient);
                rect = rect.adjusted(0, 1, 0, 0); // 调整启典 上描边
                p.setPen(Qt::NoPen);
                QPainterPath path;
                if (count() == 1)
                {
                    path.moveTo(rect.topLeft() + QPoint(d->m_borderRadius, 0));
                    path.quadTo(rect.topLeft(), rect.topLeft() + QPoint(0, d->m_borderRadius));
                    path.lineTo(rect.bottomLeft() - QPoint(0, d->m_borderRadius));
                    path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPoint(d->m_borderRadius, 0));
                    path.lineTo(rect.bottomRight() - QPoint(d->m_borderRadius, 0));
                    path.quadTo(rect.bottomRight(), rect.bottomRight() - QPoint(0, d->m_borderRadius));
                    path.lineTo(rect.topRight() + QPoint(0, d->m_borderRadius));
                    path.quadTo(rect.topRight(), rect.topRight() - QPoint(d->m_borderRadius, 0));
                    path.lineTo(rect.topLeft() + QPoint(d->m_borderRadius, 0));
                }
                else
                {
                    if (layoutDirection() == Qt::LeftToRight)
                    {
                        path.moveTo(rect.topLeft());
                        path.lineTo(rect.bottomLeft());
                        path.lineTo(rect.bottomRight() - QPointF(d->m_borderRadius, 0));
                        path.quadTo(rect.bottomRight(), rect.bottomRight() - QPointF(0, d->m_borderRadius));
                        path.lineTo(rect.topRight() + QPointF(0, d->m_borderRadius));
                        path.quadTo(rect.topRight(), rect.topRight() - QPointF(d->m_borderRadius, 0));
                        path.lineTo(rect.topLeft());
                    }
                    else
                    {
                        path.moveTo(rect.topLeft() + QPointF(0, d->m_borderRadius));
                        path.lineTo(rect.bottomLeft() - QPointF(0, d->m_borderRadius));
                        path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(d->m_borderRadius, 0));
                        path.lineTo(rect.bottomRight());
                        path.lineTo(rect.topRight());
                        path.lineTo(rect.topLeft() + QPointF(d->m_borderRadius, 0));
                        path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, d->m_borderRadius));
                    }
                }
                p.drawPath(path);
                p.restore();
                p.setBrush(Qt::NoBrush);
                p.setPen(fontColor);
                QPoint point;
                uint tf = Qt::AlignVCenter;
                if (!option.icon.isNull()) {

                    QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                            : QIcon::Disabled;
                    if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (option.state & QStyle::State_On)
                        state = QIcon::On;

                    QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);
                    if(ThemeController::isPixmapPureColor(pixmap))
                        pixmap = ThemeController::drawColoredPixmap(pixmap,fontColor);

                    int w = pixmap.width() / pixmap.devicePixelRatio();
                    int h = pixmap.height() / pixmap.devicePixelRatio();

                    if (!tabText(i).isEmpty())
                        w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 4;

                    point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                                   rect.y() + rect.height() / 2 - h / 2);

                    w = pixmap.width() / pixmap.devicePixelRatio();

                    //                    if (option.direction == Qt::RightToLeft)
                    //                        point.rx() += w;

                    if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7)
                        p.drawPixmap(option.rect.x() + 4, this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                    else {
                        if (option.direction == Qt::RightToLeft)
                            p.drawPixmap(point, pixmap);
                        else
                            p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);
                    }

                    if (!tabText(i).isEmpty()) {
                        int subH = std::max(option.iconSize.height(), option.fontMetrics.height());
                        int icon_Y = (rect.height() - subH) / 2;
                        int text_X = point.x() + option.iconSize.width() + 4;
                        int text_Y = icon_Y;
                        QRect textRect;
                        if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7) {
                            textRect = QRect(option.rect.x() + option.iconSize.width() + 8, text_Y, option.rect.width() - option.iconSize.width() - 8, option.fontMetrics.height());
                            setTabToolTip(i, tabText(i));
                            QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width() - option.iconSize.width() - 8);
                            p.drawText(textRect, elidedText);
                        } else {
                            textRect = QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                            setTabToolTip(i, "");
                            p.drawText(textRect, tabText(i));
                        }
                    }
                } else {
                    tf |= Qt::AlignHCenter;
                    if (fm.width(tabText(i)) >= option.rect.width()) {
                        QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width());
                        p.drawText(rect, tf, elidedText);
                        setTabToolTip(i, tabText(i));
                    } else {
                        p.drawText(rect, tf, tabText(i));
                        setTabToolTip(i, "");
                    }
                }
            }
            break;
        }
        case SegmentLight: {
            mix = ThemeController::getPaletteColorFromDT("brighttext-active");
            fontColor = ThemeController::getPaletteColorFromDT("buttontext-active");
            QColor bkgrdColor = d->m_bkgrdColor;
            d->m_linearGradient.setColorAt(0,bkgrdColor);
            d->m_linearGradient.setColorAt(1,bkgrdColor);
            if (option.state.testFlag(QStyle::State_Selected)) {
                if (option.state.testFlag(QStyle::State_Sunken)) {
                    borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
                    ThemeController::getGradientFromDT("kbrand-click", startColor, endColor);
                }
                else if (option.state.testFlag(QStyle::State_MouseOver)) {
                    borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
                    ThemeController::getGradientFromDT("kbrand-hover", startColor, endColor);
                }
                else {
                    borderColor = ThemeController::getCustomColorFromDT("kline-component-normal");
                    ThemeController::getGradientFromDT("kbrand-normal", startColor, endColor);
                }
                d->m_linearGradient.setColorAt(0,startColor);
                d->m_linearGradient.setColorAt(1,endColor);
                fontColor = ThemeController::getCustomColorFromDT("kwhite");
            } else if (option.state.testFlag(QStyle::State_MouseOver)) {
                ThemeController::getGradientFromDT("kcomponent-hover",startColor,endColor);
                d->m_linearGradient.setColorAt(0,startColor);
                d->m_linearGradient.setColorAt(1,endColor);
            }
            p.save();
            p.setBrush(d->m_linearGradient);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(/*option.rect.adjusted*/ rect.adjusted(d->m_horizontalMargin, 0, 0, -d->m_topMargin),
                              d->m_borderRadius, d->m_borderRadius);
            p.restore();
            p.setBrush(Qt::NoBrush);
            p.setPen(fontColor);
            /*QRect*/ rect = /*option.*/ rect.adjusted(d->m_horizontalMargin, 0, 0, -d->m_topMargin);
            QPoint point;
            uint tf = Qt::AlignVCenter;
            if (!option.icon.isNull()) {
                QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                        : QIcon::Disabled;
                if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (option.state & QStyle::State_On)
                    state = QIcon::On;

                QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);
                if(ThemeController::isPixmapPureColor(pixmap))
                    pixmap = ThemeController::drawColoredPixmap(pixmap,fontColor);

                int w = pixmap.width() / pixmap.devicePixelRatio();
                int h = pixmap.height() / pixmap.devicePixelRatio();

                if (!tabText(i).isEmpty())
                    w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 2;

                point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                               rect.y() + rect.height() / 2 - h / 2);

                w = pixmap.width() / pixmap.devicePixelRatio();

                //                if (option.direction == Qt::RightToLeft)
                //                    point.rx() += w;

                if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7)
                    p.drawPixmap(option.rect.x() + 4, this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                else {
                    if (option.direction == Qt::RightToLeft)
                        p.drawPixmap(point, pixmap);
                    else
                        p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);
                }

                if (!tabText(i).isEmpty()) {
                    int subH = std::max(option.iconSize.height(), option.fontMetrics.height());
                    int icon_Y = (rect.height() - subH) / 2;
                    int text_X = point.x() + option.iconSize.width() + 4;
                    int text_Y = icon_Y;
                    QRect textRect;
                    if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7) {
                        textRect = QRect(option.rect.x() + option.iconSize.width() + 8, text_Y, option.rect.width() - option.iconSize.width() - 8, option.fontMetrics.height());
                        setTabToolTip(i, tabText(i));
                        QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width() - option.iconSize.width() - 8);
                        p.drawText(textRect, elidedText);
                    } else {
                        textRect = QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                        setTabToolTip(i, "");
                        p.drawText(textRect, tabText(i));
                    }
                }
            } else {
                tf |= Qt::AlignHCenter;
                if (fm.width(tabText(i)) >= option.rect.width()) {
                    QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width());
                    p.drawText(rect, tf, elidedText);
                    setTabToolTip(i, tabText(i));
                } else {
                    p.drawText(rect, tf, tabText(i));
                    setTabToolTip(i, "");
                }
            }
            break;
        }
        case Sliding: {
            QColor lineColor = ThemeController::getCustomColorFromDT("kline-normal");
            // mix = ThemeController::getPaletteColorFromDT("brighttext-active");
            fontColor = ThemeController::getPaletteColorFromDT("buttontext-active");
            QColor bkgrdColor = d->m_bkgrdColor;
            if (option.state.testFlag(QStyle::State_Selected))
            {
                if (option.state.testFlag(QStyle::State_Sunken)) {
                    fontColor = ThemeController::getCustomColorFromDT("kbrand-normal");
                    lineColor = ThemeController::getCustomColorFromDT("kbrand-click");
                }
                else if (option.state.testFlag(QStyle::State_MouseOver)) {
                    fontColor = ThemeController::getCustomColorFromDT("kbrand-normal");
                    lineColor = ThemeController::getCustomColorFromDT("kbrand-hover");
                }
                else {
                    fontColor = ThemeController::getCustomColorFromDT("kbrand-normal");
                    lineColor = ThemeController::getCustomColorFromDT("kbrand-normal");
                }
            }
            else if (option.state.testFlag(QStyle::State_MouseOver))
            {
                lineColor = ThemeController::getCustomColorFromDT("kline-normal");
            }

            p.save();
            QPen pen;
            if (i != currentIndex())
                pen.setColor(lineColor);
            else
                pen.setColor(d->m_bkgrdColor);
            pen.setWidth(2);
            p.setPen(pen);
            p.drawLine(rect.bottomLeft(), rect.bottomRight());
            p.restore();

            if (d->m_animationStarted) {
                int left_border = d->m_animation->currentValue().toInt();
                p.save();
                QPen pen;
                pen.setWidth(2);
                pen.setColor(ThemeController::getCustomColorFromDT("kbrand-normal"));
                p.setPen(pen);
                p.drawLine(QPoint(left_border, rect.height() - 1), QPoint(left_border + d->m_nextTabWidth, rect.height() - 1));
                p.restore();
            } else {
                p.save();
                QPen pen;
                pen.setColor(lineColor);
                pen.setWidth(2);
                p.setPen(pen);
                p.drawLine(rect.bottomLeft(), rect.bottomRight());
                p.restore();
            }

            p.setBrush(Qt::NoBrush);
            p.setPen(fontColor);
            QPoint point;
            uint tf = Qt::AlignVCenter;
            if (!option.icon.isNull()) {
                QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal
                                                                        : QIcon::Disabled;
                if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (option.state & QStyle::State_On)
                    state = QIcon::On;

                QPixmap pixmap = option.icon.pixmap(option.iconSize, mode, state);
                if(ThemeController::isPixmapPureColor(pixmap))
                    pixmap = ThemeController::drawColoredPixmap(pixmap,fontColor);

                int w = pixmap.width() / pixmap.devicePixelRatio();
                int h = pixmap.height() / pixmap.devicePixelRatio();

                if (!tabText(i).isEmpty())
                    w += option.fontMetrics.boundingRect(option.rect, tf, tabText(i)).width() + 2;

                point = QPoint(rect.x() + rect.width() / 2 - w / 2,
                               rect.y() + rect.height() / 2 - h / 2);

                w = pixmap.width() / pixmap.devicePixelRatio();

                //                if (option.direction == Qt::RightToLeft)
                //                    point.rx() += w;

                if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7)
                    p.drawPixmap(option.rect.x() + 4, this->style()->visualPos(option.direction, option.rect, point).y(), pixmap);
                else {
                    if (option.direction == Qt::RightToLeft)
                        p.drawPixmap(point, pixmap);
                    else
                        p.drawPixmap(this->style()->visualPos(option.direction, option.rect, point), pixmap);
                }

                if (!tabText(i).isEmpty()) {
                    int subH = std::max(option.iconSize.height(), option.fontMetrics.height());
                    int icon_Y = (rect.height() - subH) / 2;
                    int text_X = point.x() + option.iconSize.width() + 4;
                    int text_Y = icon_Y;
                    QRect textRect;
                    if (fm.width(tabText(i)) >= option.rect.width() - option.iconSize.width() - 7) {
                        textRect = QRect(option.rect.x() + option.iconSize.width() + 8, text_Y, option.rect.width() - option.iconSize.width() - 8, option.fontMetrics.height());
                        setTabToolTip(i, tabText(i));
                        QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width() - option.iconSize.width() - 8);
                        p.drawText(textRect, elidedText);
                    } else {
                        textRect = QRect(text_X, text_Y, option.fontMetrics.width(tabText(i)), option.fontMetrics.height());
                        setTabToolTip(i, "");
                        p.drawText(textRect, tabText(i));
                    }
                }
            } else {
                tf |= Qt::AlignHCenter;
                if (fm.width(tabText(i)) >= option.rect.width()) {
                    QString elidedText = fm.elidedText(tabText(i), Qt::ElideRight, option.rect.width());
                    p.drawText(rect, tf, elidedText);
                    setTabToolTip(i, tabText(i));
                } else {
                    p.drawText(rect, tf, tabText(i));
                    setTabToolTip(i, "");
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

void KTabBarPrivate::changeTheme()
{
    Q_Q(KTabBar);
    switch (m_kTabBarStyle) {
    case SegmentDark: {
        if (m_useCustomColor)
            return;
        else
            m_bkgrdColor = ThemeController::getPaletteColorFromDT("button-active");
        break;
    }
    case SegmentLight: {
        if (m_useCustomColor)
            return;
        else
            m_bkgrdColor = ThemeController::getCustomColorFromDT("kgray-alpha0");
        break;
    }
    case Sliding:
        if (m_useCustomColor)
            return;
        else
            m_bkgrdColor = ThemeController::getPaletteColorFromDT("button-active");
        break;
    default:
        break;
    };
    q->update();
}

int KTabBarPrivate::getIndexAtPos(const QPoint &p)
{
    Q_Q(KTabBar);
    if (q->tabRect(q->currentIndex()).contains(p))
        return q->currentIndex();
    for (int i = 0; i < q->count(); ++i)
        if (q->tabRect(i).contains(p))
            return i;
    return -1;
}

bool KTabBarPrivate::eventFilter(QObject *watched, QEvent *event)
{
    Q_Q(KTabBar);
    if (event->type() == QEvent::MouseButtonPress) {
        if (q->isEnabled()) {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
            if (mouseEvent && mouseEvent->button() == Qt::MouseButton::LeftButton) {
                int nextIndex = getIndexAtPos(mouseEvent->pos());
                int currentIndex = q->currentIndex();
                if (nextIndex != currentIndex) {
                    int cur_left_border = q->tabRect(currentIndex).left();
                    int next_left_border = q->tabRect(nextIndex).left();
                    m_nextTabWidth = q->tabRect(nextIndex).width();
                    m_animation->setStartValue(cur_left_border);
                    m_animation->setEndValue(next_left_border);
                    m_animation->start();
                    m_animationStarted = true;
                }
            }
            if (mouseEvent && mouseEvent->button() == Qt::MouseButton::RightButton) {
                emit q->rightlicked(mouseEvent->globalPos());
            }
            q->update();
        }
    }
    return QObject::eventFilter(watched, event);
}

}
#include "ktabbar.moc"
#include "moc_ktabbar.cpp"
