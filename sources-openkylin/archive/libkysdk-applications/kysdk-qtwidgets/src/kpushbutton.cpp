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
 * Authors: Xiangwei Liu <liuxiangwei@kylinos.cn>
 *
 */

#include "kpushbutton.h"
#include "themeController.h"
#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QDesktopWidget>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>
#include <QTimer>
#include "accessinfohelper.h"

namespace kdk
{
static int margin = 5; // 左右边距
static int spacing = 4; // 图标文本间距

QStringList iconList{
    "ukui-priority-symbolic",
    "ukui-priority-low-symbolic",
    "ukui-priority-middling-symbolic",
    "ukui-priority-high-symbolic"
};

class KPushButtonPrivate : public QObject, public ThemeController
{
    Q_DECLARE_PUBLIC(KPushButton)
    Q_OBJECT
public:
    KPushButtonPrivate(KPushButton *parent)
        : q_ptr(parent)
    {
        setParent(parent);
    }

    void changeTheme();

    void updateColor(QPainter &painter, QStyleOptionButton &option, QColor &backgroundColor, QColor &highlight, QColor &mix, QColor &fontColor, QColor &pixmapColor);
    void drawBackground(QPainter &painter, QStyleOptionButton &option, QColor &backgroundColor);
    void updateArrowDirection(QPixmap &arrowPixmap, QPoint pos, QPoint desktopPos, QPoint globalPos, QIcon::Mode mode, QIcon::State state);
    void updatePixmap(QPixmap &arrowPixmap, QPixmap &pixmap, QStyleOptionButton &option, QColor &highlight, QColor &fontColor, QColor &pixmapColor, QIcon::Mode mode, QIcon::State state);

    void updateGrayColor(QPainter &painter, QStyleOptionButton &option, QColor &backgroundColor, QColor &pixmapColor);

    void drawContent(QPainter &painter, QStyleOptionButton &option, QColor &fontColor, QRect &arrowRect, QRect &textRect, QRect &iconRect, QFontMetrics fm, QPixmap &pixmap, QPixmap &arrowPixmap, uint tf, QPoint iconPosition, int iconWidth, int iconHeight, int textWidth);

private:
    KPushButton *q_ptr;
    int m_radius;
    int m_bottomLeft;
    int m_topLeft;
    int m_topRight;
    int m_bottomRight;
    int m_flashState;
    QColor m_backgroundColor;
    bool m_useCustomColor;
    bool m_hasOneParam;
    bool m_isTranslucent;
    bool m_isIconHighlight;
    QColor m_iconColor;
    bool m_useCustomIconColor;
    bool m_isBackgroundColorHighlight;
    bool m_isTranslucentFlag;
    bool m_hasArrow;
    bool m_isLoading;
    KPushButton::ArrowDirection m_arrowDirection;
    KPushButton::ButtonType m_buttonType;
    QGraphicsDropShadowEffect *m_pShadowEffect;
    QTimer *m_pTimer;
    QIcon m_icon;
    QLinearGradient m_linearGradient ;
    QColor m_borderColor;
};

KPushButton::KPushButton(QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new KPushButtonPrivate(this))
{
    Q_D(KPushButton);
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1) {
        d->m_radius = 6;
        d->m_bottomLeft = 6;
        d->m_topLeft = 6;
        d->m_topRight = 6;
        d->m_bottomRight = 6;
    } else {
        d->m_radius = radius;
        d->m_bottomLeft = radius;
        d->m_topLeft = radius;
        d->m_topRight = radius;
        d->m_bottomRight = radius;
    }
    d->m_flashState = 0;
    d->m_backgroundColor = ThemeController::getPaletteColorFromDT("button-active");
    d->m_useCustomColor = false;
    d->m_hasOneParam = true;
    d->m_buttonType = NormalType;
    d->m_isTranslucent = false;
    d->m_isIconHighlight = false;
    d->m_iconColor = ThemeController::getPaletteColorFromDT("buttontext-active");
    d->m_useCustomIconColor = false;
    d->m_isBackgroundColorHighlight = false;
    d->m_isTranslucentFlag = false;
    d->m_hasArrow = false;
    d->m_isLoading = false;
    d->m_arrowDirection = ArrowBottom;
    d->m_pShadowEffect = new QGraphicsDropShadowEffect(this);

    KDK_OBJ_INFO_FORMAT(d->m_pShadowEffect);

    d->m_pTimer = new QTimer(this);
    d->m_pTimer->setInterval(100);
    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            d->changeTheme();
        }
    });
    connect(d->m_pTimer, &QTimer::timeout, this, [=]() {
        if (d->m_flashState < 7)
            d->m_flashState++;
        else
            d->m_flashState = 0;
        setIcon(QIcon::fromTheme(QString("ukui-loading-%1.symbolic").arg(d->m_flashState)));
    });
}

KPushButton::~KPushButton()
{
}

void KPushButton::setBorderRadius(int radius)
{
    Q_D(KPushButton);
    d->m_hasOneParam = true;
    d->m_radius = radius;
    update();
}

void KPushButton::setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight)
{
    Q_D(KPushButton);
    d->m_hasOneParam = false;
    d->m_bottomLeft = bottomLeft;
    d->m_topLeft = topLeft;
    d->m_topRight = topRight;
    d->m_bottomRight = bottomRight;
    update();
}

int KPushButton::borderRadius()
{
    Q_D(KPushButton);
    return d->m_radius;
}

void KPushButton::setBackgroundColor(QColor color)
{
    Q_D(KPushButton);
    if (d->m_isBackgroundColorHighlight)
        d->m_isBackgroundColorHighlight = false;
    d->m_useCustomColor = true;
    d->m_backgroundColor = color;
    update();
}

QColor KPushButton::backgroundColor()
{
    Q_D(KPushButton);
    return d->m_backgroundColor;
}

void KPushButton::setButtonType(ButtonType type)
{
    Q_D(KPushButton);
    if (type == KPushButton::ShadowType) {
        d->m_pShadowEffect->setOffset(0, 3);
        // 阴影颜色
        QColor color(0, 0, 0);
        // 阴影半径
        if (ThemeController::themeMode() == LightTheme)
            color.setAlphaF(0.25);
        else
            color.setAlphaF(0.45);
        d->m_pShadowEffect->setColor(color);
        d->m_pShadowEffect->setBlurRadius(8);
        setGraphicsEffect(d->m_pShadowEffect);
    }
    if (type == KPushButton::GrayType)
        setFocusPolicy(Qt::StrongFocus);
    else
        setFocusPolicy(Qt::NoFocus);
    d->m_buttonType = type;
}

KPushButton::ButtonType KPushButton::buttonType()
{
    Q_D(KPushButton);
    return d->m_buttonType;
}

void KPushButton::setTranslucent(bool flag)
{
    Q_D(KPushButton);
    d->m_isTranslucentFlag = flag;
    d->m_isTranslucent = flag;
    d->changeTheme();
}

bool KPushButton::isTranslucent()
{
    Q_D(KPushButton);
    return d->m_isTranslucent;
}

void KPushButton::setIconHighlight(bool flag)
{
    Q_D(KPushButton);
    d->m_isIconHighlight = flag;
}

bool KPushButton::isIconHighlight()
{
    Q_D(KPushButton);
    return d->m_isIconHighlight;
}

void KPushButton::setIconColor(QColor color)
{
    Q_D(KPushButton);
    d->m_useCustomIconColor = true;
    d->m_iconColor = color;
    update();
}

QColor KPushButton::IconColor()
{
    Q_D(KPushButton);
    return d->m_iconColor;
}

void KPushButton::setBackgroundColorHighlight(bool flag)
{
    Q_D(KPushButton);
    if (d->m_useCustomColor)
        d->m_useCustomColor = false;
    d->m_isBackgroundColorHighlight = flag;
}

bool KPushButton::isBackgroundColorHighlight()
{
    Q_D(KPushButton);
    return d->m_isBackgroundColorHighlight;
}

void KPushButton::setArrow(bool flag, KPushButton::ArrowDirection arrowDirection)
{
    Q_D(KPushButton);
    d->m_hasArrow = flag;
    d->m_arrowDirection = arrowDirection;
}

bool KPushButton::hasArrow() const
{
    Q_D(const KPushButton);
    return d->m_hasArrow;
}

void KPushButton::setLoaingStatus(bool flag)
{
    Q_D(KPushButton);
    if (!d->m_isLoading) {
        d->m_icon = icon();
    }
    d->m_isLoading = flag;
    if (flag) {
        d->m_pTimer->start();
    } else {
        d->m_pTimer->stop();
        setIcon(d->m_icon);
    }
}

bool KPushButton::isLoading()
{
    Q_D(KPushButton);
    return d->m_isLoading;
}

bool KPushButton::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KPushButton);
    return QPushButton::eventFilter(watched, event);
}

void KPushButton::paintEvent(QPaintEvent *event)
{
    Q_D(KPushButton);
    d->m_linearGradient = QLinearGradient(width() / 2,rect().top(), width() / 2, rect().bottom());
    QStyleOptionButton option;
    initStyleOption(&option);
    QColor pixmapColor;

    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QColor fontColor = ThemeController::getPaletteColorFromDT("buttontext-disable");
    QColor mix = ThemeController::getPaletteColorFromDT("brighttext-active");
    // QColor highlight = ThemeController::getPaletteColorFromDT("highlight-active");
    QColor highlight = ThemeController::getCustomColorFromDT("kbrand-normal");
    QColor backgroundColor;

    if (ThemeController::widgetTheme() == ClassicTheme) {
        if (painter.pen().width() == 1)
            painter.translate(0.5, 0.5);
    }

    d->updateColor(painter, option, backgroundColor, highlight, mix, fontColor, pixmapColor);
    d->drawBackground(painter, option, backgroundColor);

    /*绘制图标和文字*/
    QPen pen;
    pen.setColor(fontColor);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(pen);
    QPoint point;
    QRect ir = option.rect;
    uint tf = Qt::AlignVCenter;
    QPixmap pixmap;
    QPixmap arrowPixmap;

    QIcon::Mode mode = option.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    if (mode == QIcon::Normal && option.state & QStyle::State_HasFocus)
        mode = QIcon::Active;
    QIcon::State state = QIcon::Off;
    if (option.state & QStyle::State_On)
        state = QIcon::On;
    QPoint pos;
    QPoint desktopPos(QApplication::desktop()->availableGeometry().width(), QApplication::desktop()->availableGeometry().height());
    QPoint globalPos;

    d->updateArrowDirection(arrowPixmap, pos, desktopPos, globalPos, mode, state);
    d->updatePixmap(arrowPixmap, pixmap, option, highlight, fontColor, pixmapColor, mode, state);

    if (!option.icon.isNull()) {
        int w = pixmap.width() / pixmap.devicePixelRatio();
        int h = pixmap.height() / pixmap.devicePixelRatio();

        if (!option.text.isEmpty())
            w += option.fontMetrics.boundingRect(option.rect, tf, option.text).width() + 2;

        if(hasArrow())
            w += arrowPixmap.width();

        point = QPoint(ir.x() + ir.width() / 2 - w / 2 ,
                       ir.y() + ir.height() / 2 - h / 2);

        w = pixmap.width() / pixmap.devicePixelRatio();

        if (option.direction == Qt::RightToLeft)
            point.rx() += w;

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

    QFontMetrics fm = option.fontMetrics;

    QPoint iconPosition = this->style()->visualPos(option.direction, option.rect, point);
    if(iconPosition.x() < 0)
        iconPosition.setX(margin);
    else if(iconPosition.x() > option.rect.right() - margin - option.iconSize.width() )
        iconPosition.setX(option.rect.width() - margin - option.iconSize.width());

    int iconWidth = option.iconSize.width();
    int iconHeight = option.iconSize.height();
    int textWidth = fm.width(option.text);
    QRect arrowRect;
    QRect textRect;
    QRect iconRect = QRect(iconPosition.x(), iconPosition.y(), iconWidth, iconHeight);
    d->drawContent(painter, option, fontColor, arrowRect, textRect, iconRect, fm, pixmap, arrowPixmap, tf, iconPosition, iconWidth, iconHeight, textWidth);
}

void KPushButtonPrivate::changeTheme()
{
    Q_Q(KPushButton);
    if (m_isTranslucentFlag) {
        if (ThemeController::widgetTheme() == ClassicTheme)
            m_isTranslucent = false;
        else
            m_isTranslucent = true;
    } else {
        m_isTranslucent = false;
    }
    initThemeStyle();
    if (m_buttonType == KPushButton::ShadowType) {
        m_pShadowEffect->setOffset(0, 3);
        // 阴影颜色
        QColor color(0, 0, 0);
        // 阴影半径
        if (ThemeController::themeMode() == LightTheme)
            color.setAlphaF(0.25);
        else
            color.setAlphaF(0.45);
        m_pShadowEffect->setColor(color);
        m_pShadowEffect->setBlurRadius(8);
        q->setGraphicsEffect(m_pShadowEffect);
    }
}

void KPushButtonPrivate::updateColor(QPainter &painter, QStyleOptionButton &option, QColor &backgroundColor, QColor &highlight, QColor &mix, QColor &fontColor, QColor &pixmapColor)
{
    Q_Q(KPushButton);
    QColor startColor;
    QColor endColor;
    if(!q->isDown())
    {
        if (m_buttonType == KPushButton::NormalType || m_buttonType == KPushButton::CircleType || m_buttonType == KPushButton::ShadowType)
        {
            if (m_isTranslucent)
            {
                /*判断使用用户设置的背景色、跟随系统高亮色或是使用默认的背景色*/
                if (!q->isFlat()) {
                    if (m_useCustomColor)
                    {
                        backgroundColor = m_backgroundColor;
                        m_linearGradient.setColorAt(0,m_backgroundColor);
                        m_linearGradient.setColorAt(1,m_backgroundColor);
                    }
                    else if (m_isBackgroundColorHighlight)
                    {
                        backgroundColor = highlight;
                        backgroundColor.setAlphaF(0.15);
                        m_linearGradient.setColorAt(0,highlight);
                        m_linearGradient.setColorAt(1,highlight);
                    }
                    else if (m_buttonType == KPushButton::ShadowType)
                    {
                        backgroundColor = ThemeController::getPaletteColorFromDT("base-active");
                    }
                    else
                    {
                        ThemeController::getGradientFromDT("kcomponent-alpha-normal",startColor,endColor);
                        m_linearGradient.setColorAt(0,startColor);
                        m_linearGradient.setColorAt(1,endColor);
                    }
                } else {
                    backgroundColor = Qt::transparent;
                    m_linearGradient.setColorAt(0,Qt::transparent);
                    m_linearGradient.setColorAt(1,Qt::transparent);
                }
    
                if (!option.state.testFlag(QStyle::State_Enabled))
                {
                    fontColor = ThemeController::getPaletteColorFromDT("buttontext-disable");
                    ThemeController::getGradientFromDT("kcomponent-alpha-disable",startColor,endColor);
                    m_linearGradient.setColorAt(0,startColor);
                    m_linearGradient.setColorAt(1,endColor);
                    pixmapColor = fontColor;
                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-disable");
                }
                else /*按钮处于可用（Enabled）状态*/
                {
                    fontColor = ThemeController::getCustomColorFromDT("kfont-primary");
                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-normal");
                    pixmapColor = ThemeController::getCustomColorFromDT("kfont-primary");
                    if (option.state.testFlag(QStyle::State_MouseOver)) /*鼠标在按钮上(hover状态)*/
                    {
                        if (m_buttonType == KPushButton::ShadowType) {
                            backgroundColor = ThemeController::getCustomColorFromDT("kbrand-normal");
                        }
                        if (option.state.testFlag(QStyle::State_Sunken)) /*按钮被选中（clicked）*/
                        {
                            m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
                            if (m_useCustomColor || m_isBackgroundColorHighlight || m_buttonType == KPushButton::ShadowType)
                            {
                                fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
                                backgroundColor.setAlphaF(0.45);
                                m_linearGradient.setColorAt(0,backgroundColor);
                                m_linearGradient.setColorAt(1,backgroundColor);
                            }
                            else
                            {
                                fontColor = ThemeController::getCustomColorFromDT("kfont-primary");
                                ThemeController::getGradientFromDT("kcomponent-alpha-click",startColor,endColor);
                                m_linearGradient.setColorAt(0,startColor);
                                m_linearGradient.setColorAt(1,endColor);
                                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
                            }
                        }
                        else
                        {
                            m_borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
                            if (m_useCustomColor || m_isBackgroundColorHighlight)
                            {
                                backgroundColor.setAlphaF(0.3);
                                m_linearGradient.setColorAt(0,backgroundColor);
                                m_linearGradient.setColorAt(1,backgroundColor);
                                fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
                            }
                            else
                            {
                                ThemeController::getGradientFromDT("kcomponent-alpha-hover",startColor,endColor);
                                m_linearGradient.setColorAt(0,startColor);
                                m_linearGradient.setColorAt(1,endColor);
                                fontColor = ThemeController::getCustomColorFromDT("buttontext-active");
                                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
                            }
                        }
                    }
                    else
                    {
                        if (m_useCustomColor || m_isBackgroundColorHighlight)
                        {
                            fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
                            backgroundColor.setAlphaF(0.15);
                            m_linearGradient.setColorAt(0,backgroundColor);
                            m_linearGradient.setColorAt(1,backgroundColor);
                        }
                        else if (m_buttonType == KPushButton::ShadowType) {
                            if (ThemeController::themeMode() == DarkTheme)
                            {
                                backgroundColor.setAlphaF(0.2);
                                m_linearGradient.setColorAt(0,backgroundColor);
                                m_linearGradient.setColorAt(1,backgroundColor);
                            }
                        }
                    }
                }
            }
            else
            {
                /*判断使用用户设置的背景色或是使用默认的背景色*/
                if (!q->isFlat()) {
                    if (m_useCustomColor) {
                        backgroundColor = m_backgroundColor;
                        m_linearGradient.setColorAt(0,m_backgroundColor);
                        m_linearGradient.setColorAt(1,m_backgroundColor);
                    } else if (m_isBackgroundColorHighlight) {
                        ThemeController::getGradientFromDT("kbrand-normal",startColor,endColor);
                        m_linearGradient.setColorAt(0,startColor);
                        m_linearGradient.setColorAt(1,endColor);
                    } else {
                        ThemeController::getGradientFromDT("kcomponent-normal",startColor,endColor);
                        m_linearGradient.setColorAt(0,startColor);
                        m_linearGradient.setColorAt(1,endColor);
                    }
                } else {
                    backgroundColor = Qt::transparent;
                    m_linearGradient.setColorAt(0,Qt::transparent);
                    m_linearGradient.setColorAt(1,Qt::transparent);
                }
    
                /*按钮处于不可用（Disabled）状态*/
                if (!option.state.testFlag(QStyle::State_Enabled))
                {
                    if(m_isBackgroundColorHighlight)
                    {
                        fontColor = ThemeController::getCustomColorFromDT("kfont-white-disable");
                        ThemeController::getGradientFromDT("kbrand-disable",startColor,endColor);
                        m_linearGradient.setColorAt(0,startColor);
                        m_linearGradient.setColorAt(1,endColor);
                    }
                    else
                    {
                        fontColor = ThemeController::getPaletteColorFromDT("buttontext-disable");
                        pixmapColor = ThemeController::getPaletteColorFromDT("buttontext-disable");
                        ThemeController::getGradientFromDT("kcomponent-disable",startColor,endColor);
                        startColor = ThemeController::getCustomColorFromDT("kcomponent-disable");
                        m_linearGradient.setColorAt(0,startColor);
                        m_linearGradient.setColorAt(1,endColor);
                    }
                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-disable");
                    if(q->isFlat())
                    {
                        backgroundColor = Qt::transparent;
                        m_linearGradient.setColorAt(0,Qt::transparent);
                        m_linearGradient.setColorAt(1,Qt::transparent);
                    }
                }
                else
                {
                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-normal");
                    pixmapColor = ThemeController::getPaletteColorFromDT("buttontext-active");
                    if (m_useCustomColor || m_isBackgroundColorHighlight)
                    {
                        if(m_isBackgroundColorHighlight)
                            fontColor = ThemeController::getCustomColorFromDT("kfont-white");
                        else
                            fontColor = ThemeController::getPaletteColorFromDT("buttontext-active");
    
                        if (option.state.testFlag(QStyle::State_MouseOver)) /*鼠标在按钮上(hover状态)*/
                        {
                            if (option.state.testFlag(QStyle::State_Sunken)) /*按钮被选中（clicked）*/
                            {
                                if(m_isBackgroundColorHighlight)
                                {
                                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
                                    ThemeController::getGradientFromDT("kbrand-click",startColor,endColor);
                                    m_linearGradient.setColorAt(0,startColor);
                                    m_linearGradient.setColorAt(1,endColor);
                                }
                                else
                                {
                                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
                                    backgroundColor = ThemeController::mixColor(backgroundColor, mix, 0.2);
                                    m_linearGradient.setColorAt(0,backgroundColor);
                                    m_linearGradient.setColorAt(1,backgroundColor);
                                }
                            }
                            else
                            {
                                if(m_isBackgroundColorHighlight)
                                {
                                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
                                    ThemeController::getGradientFromDT("kbrand-hover",startColor,endColor);
                                    m_linearGradient.setColorAt(0,startColor);
                                    m_linearGradient.setColorAt(1,endColor);
                                }
                                else
                                {
                                    m_borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
                                    backgroundColor = ThemeController::mixColor(backgroundColor, mix, 0.05);
                                    m_linearGradient.setColorAt(0,backgroundColor);
                                    m_linearGradient.setColorAt(1,backgroundColor);
                                }
                            }
                        }
                    }
                    else
                    {
                        /*按钮处于可用（Enabled）状态*/
                        fontColor = ThemeController::getPaletteColorFromDT("buttontext-active");
                        if (option.state.testFlag(QStyle::State_MouseOver)) /*鼠标在按钮上(hover状态)*/
                        {
                            if (option.state.testFlag(QStyle::State_Sunken)) /*按钮被选中（clicked）*/
                            {
                                ThemeController::getGradientFromDT("kcomponent-click",startColor,endColor);
                                m_linearGradient.setColorAt(0,startColor);
                                m_linearGradient.setColorAt(1,endColor);
                                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
                            }
                            else
                            {
                                ThemeController::getGradientFromDT("kcomponent-hover",startColor,endColor);
                                m_linearGradient.setColorAt(0,startColor);
                                m_linearGradient.setColorAt(1,endColor);
                                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-hover");
                            }
                        }
                    }
                }
            }
            if (q->isChecked())
            {
                if(q->isEnabled())
                {
                    fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
                    m_linearGradient.setColorAt(0,highlight);
                    m_linearGradient.setColorAt(1,highlight);
                }
                else
                {
                    fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-disable");
                    ThemeController::getGradientFromDT("kbrand-disable",startColor,endColor);
                    m_linearGradient.setColorAt(0,startColor);
                    m_linearGradient.setColorAt(1,endColor);
                }
            }
        } else if (m_buttonType == KPushButton::GrayType) {
            updateGrayColor(painter, option, backgroundColor, pixmapColor);
        }
    }
    else
    {
        if (m_isTranslucent)
        {
            if (m_useCustomColor)
            {
                backgroundColor = m_backgroundColor;
            }
            else if (m_isBackgroundColorHighlight)
            {
                backgroundColor = highlight;
            }
            else if (m_buttonType == KPushButton::ShadowType)
            {
                backgroundColor = ThemeController::getPaletteColorFromDT("base-active");
            }
            else
            {
                ThemeController::getGradientFromDT("kcomponent-alpha-normal",startColor,endColor);
                m_linearGradient.setColorAt(0,startColor);
                m_linearGradient.setColorAt(1,endColor);
            }

            m_borderColor =ThemeController::getCustomColorFromDT("kline-component-click");
            if (m_useCustomColor || m_isBackgroundColorHighlight ||m_buttonType == KPushButton::ShadowType)
            {
                fontColor =ThemeController::getPaletteColorFromDT("highlightedtext-active");
                backgroundColor.setAlphaF(0.45);
                m_linearGradient.setColorAt(0, backgroundColor);
                m_linearGradient.setColorAt(1, backgroundColor);
            }
            else
            {
                fontColor = ThemeController::getCustomColorFromDT("kfont-primary");
                ThemeController::getGradientFromDT("kcomponent-alpha-click",startColor, endColor);
                m_linearGradient.setColorAt(0, startColor);
                m_linearGradient.setColorAt(1, endColor);
                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
            }
        }
        else
        {
            if (m_useCustomColor || m_isBackgroundColorHighlight)
            {
                if (m_useCustomColor)
                {
                    backgroundColor = m_backgroundColor;
                }
                else if (m_isBackgroundColorHighlight)
                {
                    backgroundColor = highlight;
                }

                if (m_isBackgroundColorHighlight)
                    fontColor = ThemeController::getCustomColorFromDT("kfont-white");
                else
                    fontColor =ThemeController::getPaletteColorFromDT("buttontext-active");
                if (m_isBackgroundColorHighlight)
                {
                    m_borderColor =ThemeController::getCustomColorFromDT("kline-component-click");
                    ThemeController::getGradientFromDT("kbrand-click", startColor,endColor);
                    m_linearGradient.setColorAt(0, startColor);
                    m_linearGradient.setColorAt(1, endColor);
                }
                else
                {
                    m_borderColor =ThemeController::getCustomColorFromDT("kline-component-click");
                    backgroundColor =ThemeController::mixColor(backgroundColor, mix, 0.2);
                    m_linearGradient.setColorAt(0, backgroundColor);
                    m_linearGradient.setColorAt(1, backgroundColor);
                }
            }
            else
            {
                fontColor =ThemeController::getPaletteColorFromDT("buttontext-active");
                ThemeController::getGradientFromDT("kcomponent-click", startColor,endColor);
                m_linearGradient.setColorAt(0, startColor);
                m_linearGradient.setColorAt(1, endColor);
                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
            }
        }
        if (q->isChecked())
        {
            if(q->isEnabled())
            {
                fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-active");
                ThemeController::getGradientFromDT("kbrand-click", startColor,endColor);
                m_linearGradient.setColorAt(0, startColor);
                m_linearGradient.setColorAt(1, endColor);
                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-click");
            }
            else
            {
                fontColor = ThemeController::getPaletteColorFromDT("highlightedtext-disable");
                ThemeController::getGradientFromDT("kbrand-disable", startColor,endColor);
                m_linearGradient.setColorAt(0, startColor);
                m_linearGradient.setColorAt(1, endColor);
                m_borderColor = ThemeController::getCustomColorFromDT("kline-component-disable");
            }
        }
    }
}

void KPushButtonPrivate::drawBackground(QPainter &painter, QStyleOptionButton &option, QColor &backgroundColor)
{
    Q_Q(KPushButton);
    /*绘制背景色和rect*/
    painter.save();
    if (((m_buttonType == KPushButton::NormalType || m_buttonType == KPushButton::CircleType) && !m_useCustomColor) || q->isChecked()) {
        painter.setBrush(m_linearGradient);
        painter.setPen(m_borderColor);
    } else {
        painter.setBrush(backgroundColor);
        painter.setPen(Qt::NoPen);
    }

    switch (m_buttonType) {
    case KPushButton::NormalType:
    {
        if (m_hasOneParam)
        {
            if (ThemeController::widgetTheme() == ClassicTheme)
                painter.drawRoundedRect(option.rect.adjusted(0, 0, -1, -1), 0, 0);
            else
                painter.drawRoundedRect(option.rect.adjusted(0, 0, -1, -1), m_radius, m_radius);
        }
        else
        {
            QPainterPath path;
            if (ThemeController::widgetTheme() == ClassicTheme)
                path.addRect(option.rect.adjusted(0, 0, -1, -1));
            else {
                path.moveTo(option.rect.topLeft() + QPointF(0, m_topLeft));
                path.lineTo(option.rect.bottomLeft() - QPointF(0, m_bottomLeft));
                path.quadTo(option.rect.bottomLeft(), option.rect.bottomLeft() + QPointF(m_bottomLeft, 0));
                path.lineTo(option.rect.bottomRight() - QPointF(m_bottomRight, 0));
                path.quadTo(option.rect.bottomRight(), option.rect.bottomRight() - QPointF(0, m_bottomRight));
                path.lineTo(option.rect.topRight() + QPointF(0, m_topRight));
                path.quadTo(option.rect.topRight(), option.rect.topRight() - QPointF(m_topRight, 0));
                path.lineTo(option.rect.topLeft() + QPointF(m_topLeft, 0));
                path.quadTo(option.rect.topLeft(), option.rect.topLeft() + QPointF(0, m_topLeft));
            }
            painter.drawPath(path);
        }
        painter.restore();
        break;
    }
    case KPushButton::CircleType:
        painter.drawRoundedRect(option.rect.adjusted(0, 0, 0, 0), q->width() / 2, q->height() / 2);
        painter.restore();
        break;
    case KPushButton::GrayType: {
        if (option.state.testFlag(QStyle::State_HasFocus)) {
            QPen pen;
            pen.setWidth(2);
            pen.setColor(ThemeController::getCustomColorFromDT("kbrand-normal"));
            painter.setPen(pen);
        } else {
            painter.setPen(Qt::NoPen);
        }
        if (m_hasOneParam) {
            painter.drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), m_radius, m_radius);
        } else {
            QPainterPath path;

            path.moveTo(option.rect.topLeft() + QPointF(0, m_topLeft));
            path.lineTo(option.rect.bottomLeft() - QPointF(0, m_bottomLeft));
            path.quadTo(option.rect.bottomLeft(), option.rect.bottomLeft() + QPointF(m_bottomLeft, 0));
            path.lineTo(option.rect.bottomRight() - QPointF(m_bottomRight, 0));
            path.quadTo(option.rect.bottomRight(), option.rect.bottomRight() - QPointF(0, m_bottomRight));
            path.lineTo(option.rect.topRight() + QPointF(0, m_topRight));
            path.quadTo(option.rect.topRight(), option.rect.topRight() - QPointF(m_topRight, 0));
            path.lineTo(option.rect.topLeft() + QPointF(m_topLeft, 0));
            path.quadTo(option.rect.topLeft(), option.rect.topLeft() + QPointF(0, m_topLeft));

            painter.setRenderHint(QPainter::HighQualityAntialiasing);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::TextAntialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);

            painter.drawPath(path);
        }
        painter.restore();
        break;
    }
    case KPushButton::ShadowType: {
        QColor penColor = Qt::white;
        if (!q->isEnabled()) {
            if (ThemeController::themeMode() == LightTheme)
                penColor.setAlphaF(0.05);
            else
                penColor.setAlphaF(0.1);
        }
        painter.setPen(penColor);
        painter.setBrush(backgroundColor);
        painter.drawEllipse(option.rect.adjusted(0, 0, -1, -1));
        painter.restore();
    } break;
    default:
        break;
    }
}

void KPushButtonPrivate::updateArrowDirection(QPixmap &arrowPixmap, QPoint pos, QPoint desktopPos, QPoint globalPos, QIcon::Mode mode, QIcon::State state)
{
    Q_Q(KPushButton);
    if (q->hasArrow()) {
        switch (m_arrowDirection) {
        case KPushButton::ArrowTop:
            arrowPixmap = QIcon::fromTheme("ukui-up-symbolic").pixmap(QSize(16, 16), mode, state);
            if (q->menu()) {
                pos.setX(q->rect().left());
                pos.setY(-q->menu()->height());
                globalPos = q->mapToGlobal(pos); // 现在
                if (globalPos.x() > (desktopPos.x() - q->menu()->width()))
                    globalPos.setX(desktopPos.x() - q->menu()->width());
                if (globalPos.x() < 0)
                    globalPos.setX(0);
                if (globalPos.y() < 0) {
                    pos.setY(q->rect().bottom());
                    globalPos.setY(q->mapToGlobal(pos).y());
                }
                q->menu()->move(globalPos);
            }
            break;
        case KPushButton::ArrowBottom:
            arrowPixmap = QIcon::fromTheme("ukui-down-symbolic").pixmap(QSize(16, 16), mode, state);
            break;
        case KPushButton::ArrowLeft:
            arrowPixmap = QIcon::fromTheme("ukui-start-symbolic").pixmap(QSize(16, 16), mode, state);
            if (q->menu()) {
                pos.setX(q->rect().left() - q->menu()->width());
                pos.setY(q->rect().center().y() / 2);
                globalPos = q->mapToGlobal(pos);
                if (globalPos.x() < 0) {
                    pos.setY(q->rect().bottom());
                    globalPos.setX(0);
                    globalPos.setY(q->mapToGlobal(pos).y());
                }
                if (globalPos.y() > desktopPos.y() - q->menu()->height()) {
                    globalPos.setY(desktopPos.y() - q->menu()->height());
                }
                q->menu()->move(globalPos);
            }
            break;
        case KPushButton::ArrowRight:
            arrowPixmap = QIcon::fromTheme("ukui-end-symbolic").pixmap(QSize(16, 16), mode, state);
            if (q->menu()) {
                pos.setX(q->rect().right());
                q->menu()->move(q->mapToGlobal(pos));
                globalPos = q->mapToGlobal(pos);
                if (globalPos.x() > (desktopPos.x() - q->menu()->width())) {
                    pos.setX(q->rect().left() - q->menu()->width());
                    globalPos.setX(q->mapToGlobal(pos).x());
                }
                if (globalPos.y() > desktopPos.y() - q->menu()->height()) {
                    globalPos.setY(desktopPos.y() - q->menu()->height());
                }
                q->menu()->move(globalPos);
            }
            break;
        default:
            break;
        }
    }

    if(arrowPixmap.size() != QSize(16,16))
        arrowPixmap = arrowPixmap.scaled(QSize(16,16),Qt::KeepAspectRatio,Qt::SmoothTransformation);
}

void KPushButtonPrivate::updatePixmap(QPixmap &arrowPixmap, QPixmap &pixmap, QStyleOptionButton &option, QColor &highlight, QColor &fontColor, QColor &pixmapColor, QIcon::Mode mode, QIcon::State state)
{
    Q_Q(KPushButton);
    pixmap = option.icon.pixmap(option.iconSize, mode, state);
    if(q->isChecked())
        pixmapColor = ThemeController::getCustomColorFromDT("kwhite");

    if (mode != QIcon::Disabled) {
        if (m_buttonType == KPushButton::NormalType || m_buttonType == KPushButton::CircleType || m_buttonType == KPushButton::ShadowType) {
            if (m_isIconHighlight) {
                pixmap = ThemeController::drawColoredPixmap(q->icon().pixmap(q->iconSize()), highlight);
                arrowPixmap = ThemeController::drawColoredPixmap(arrowPixmap, highlight);
            } else if (m_useCustomIconColor) {
                pixmap = ThemeController::drawColoredPixmap(q->icon().pixmap(q->iconSize()), m_iconColor);
                arrowPixmap = ThemeController::drawColoredPixmap(arrowPixmap, pixmapColor);
            } else {
                if (!pixmap.isNull()) {
                  if (iconList.contains(option.icon.name())) {
                    pixmap = q->icon().pixmap(q->iconSize());
                    if(option.icon.name() == "ukui-priority-symbolic"){
                        pixmap = ThemeController::drawColoredPixmap(q->icon().pixmap(q->iconSize()), pixmapColor);
                    }
                  } else if (ThemeController::isPixmapPureColor(pixmap))
                    pixmap = ThemeController::drawColoredPixmap(
                        q->icon().pixmap(q->iconSize()), fontColor);
                }

                if (!arrowPixmap.isNull()) {
                    if (ThemeController::isPixmapPureColor(arrowPixmap))
                        arrowPixmap = ThemeController::drawColoredPixmap(arrowPixmap, fontColor);
                }
            }
        } else {
            pixmap = ThemeController::drawColoredPixmap(q->icon().pixmap(q->iconSize()), pixmapColor);
            arrowPixmap = ThemeController::drawColoredPixmap(arrowPixmap, pixmapColor);
        }
    } else {
        arrowPixmap = ThemeController::drawColoredPixmap(arrowPixmap, ThemeController::getPaletteColorFromDT("buttontext-disable"));
        if (ThemeController::isPixmapPureColor(pixmap) && !iconList.contains(option.icon.name()))
            pixmap = ThemeController::drawColoredPixmap(pixmap, ThemeController::getPaletteColorFromDT("buttontext-disable"));
    }
}

void KPushButtonPrivate::updateGrayColor(QPainter &painter, QStyleOptionButton &option, QColor &backgroundColor, QColor &pixmapColor)
{
    Q_Q(KPushButton);
    if (!m_isTranslucent) {
        pixmapColor = ThemeController::getPaletteColorFromDT("text-active");
        if (!q->isEnabled()) {
            backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-disable");
            pixmapColor = ThemeController::getPaletteColorFromDT("text", QPalette::Disabled);
            painter.setPen(Qt::NoPen);
        } else {
            if (q->isLoading()) {
                backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-normal");
                painter.setPen(Qt::NoPen);
            } else {
                if (option.state.testFlag(QStyle::State_MouseOver)) {
                    if (option.state.testFlag(QStyle::State_Sunken)) {
                        backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-click");
                    } else {
                        backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-hover");
                    }
                } else {
                    backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-normal");
                }
            }
        }
    } else {
        pixmapColor = ThemeController::getPaletteColorFromDT("text-active");
        if (!q->isEnabled()) {
            backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-alpha-disable");
            pixmapColor = ThemeController::getPaletteColorFromDT("text-disable");
        } else {
            if (q->isLoading()) {
                backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-alpha-normal");
                painter.setPen(Qt::NoPen);
            } else {
                if (option.state.testFlag(QStyle::State_MouseOver)) {
                    if (option.state.testFlag(QStyle::State_Sunken)) {
                        backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-alpha-click");
                    } else {
                        backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-alpha-hover");
                    }
                } else {
                    backgroundColor = ThemeController::getCustomColorFromDT("kcomponent-alpha-normal");
                }
            }
        }
    }
}

void KPushButtonPrivate::drawContent(QPainter &painter, QStyleOptionButton &option, QColor &fontColor, QRect &arrowRect, QRect &textRect, QRect &iconRect, QFontMetrics fm, QPixmap &pixmap, QPixmap &arrowPixmap, uint tf, QPoint iconPosition, int iconWidth, int iconHeight, int textWidth)
{
    Q_Q(KPushButton);
    bool layoutDirection = ThemeController::systemLang();//(q->layoutDirection() == Qt::RightToLeft)
    QString elidedText;
    if (m_isLoading) {
        iconRect = QRect((q->rect().width() - iconWidth) / 2 + 1, iconPosition.y() - 1, iconWidth, iconHeight);
        painter.drawPixmap(iconRect, pixmap);
    } else {
        if (q->hasArrow()) {
            if (!q->isEnabled()) {
                QPen pen;
                pen.setColor(fontColor);
                painter.setPen(pen);
                if (option.icon.isNull()) {
                    if(layoutDirection)
                        textRect = QRect(iconWidth + margin + spacing,(option.rect.height() - fm.height()) / 2,option.rect.width() - iconWidth - margin*2 - spacing,fm.height());
                    else
                        textRect = QRect(margin, (option.rect.height() - fm.height()) / 2, option.rect.width() - iconWidth - margin*2 - spacing , fm.height());

                    if ((fm.width(option.text) >= textRect.width())) {

                        if(layoutDirection)
                        {
                            arrowRect = QRect(margin,(option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                            elidedText = fm.elidedText(option.text,Qt::ElideLeft,textRect.width());
                        }
                        else
                        {
                            arrowRect = QRect(textRect.right() + spacing, (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                            elidedText = fm.elidedText(option.text, Qt::ElideRight, textRect.width());
                        }
                        painter.drawText(textRect, tf, elidedText);
                        painter.drawPixmap(arrowRect, arrowPixmap);
                    } else {
                        if(layoutDirection)
                            arrowRect = QRect(margin,(option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                        else
                            arrowRect = QRect(textRect.right() + spacing, (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                        painter.drawText(textRect, tf, option.text);
                        painter.drawPixmap(arrowRect, arrowPixmap);
                    }

                } else {
                    painter.drawPixmap(iconRect, pixmap);
                    if(layoutDirection)
                    {
                        textRect = QRect(arrowPixmap.width() + margin + spacing, (option.rect.height() - fm.height()) / 2,iconPosition.x()- spacing*2 - margin*2-arrowRect.width() - option.iconSize.width()  , fm.height());
                    }
                    else
                    {
                        textRect = QRect(iconPosition.x() + spacing + iconWidth, 0, option.rect.width() - margin*2 - spacing*2 - option.iconSize.width()*2, option.rect.height());
                    }

                    if (fm.width(option.text) > textRect.width()) {
                        if(layoutDirection)
                        {
                            arrowRect = QRect(margin,(option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                            textRect = QRect(arrowRect.right() + spacing +margin, 0, iconPosition.x()- spacing*2 - margin*2-arrowRect.width() , option.rect.height());
                            elidedText = fm.elidedText(option.text, Qt::ElideLeft, textRect.width());
                            tf =  Qt::AlignVCenter | Qt::AlignRight | Qt::AlignAbsolute;
                        }
                        else
                        {
                            arrowRect = QRect(textRect.right() + spacing, (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                            elidedText = fm.elidedText(option.text, Qt::ElideRight, textRect.width());
                        }
                        painter.drawText(textRect, tf, elidedText);
                        painter.drawPixmap(arrowRect, arrowPixmap);
                    } else {
                        if(layoutDirection)
                        {
                            arrowRect = QRect(margin,(option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                            textRect = QRect(arrowRect.right() + spacing +margin, 0, iconPosition.x()- spacing - margin*2-arrowRect.width() , option.rect.height());
                            tf =  Qt::AlignVCenter | Qt::AlignRight | Qt::AlignAbsolute;
                        }
                        else
                        {
                            textRect = QRect(iconPosition.x() + spacing + iconWidth, 0, fm.width(option.text), option.rect.height());
                            arrowRect = QRect(textRect.right() + spacing, (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                        }

                        painter.drawText(textRect, tf, option.text);
                        painter.drawPixmap(arrowRect, arrowPixmap);
                    }
                }
            } else {
                if (option.icon.isNull()) {
                    if(layoutDirection)
                    {
                        arrowRect = QRect(margin , (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                        textRect = QRect(arrowRect.right() + spacing, (option.rect.height() - fm.height()) / 2, option.rect.width() - iconWidth - margin*2 - spacing, fm.height());
                    }
                    else
                    {
                        textRect = QRect(margin, (option.rect.height() - fm.height()) / 2, option.rect.width() - iconWidth - margin*2 - spacing, fm.height());
                        arrowRect = QRect(textRect.right() + spacing, (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                    }

                    if (fm.width(option.text) > textRect.width()) {
                        if(layoutDirection)
                            elidedText = fm.elidedText(option.text, Qt::ElideLeft, textRect.width());
                        else
                            elidedText = fm.elidedText(option.text, Qt::ElideRight, textRect.width());
                        painter.drawText(textRect, tf, elidedText);
                        painter.drawPixmap(arrowRect, arrowPixmap);
                    } else {
                        painter.drawText(textRect, tf, option.text);
                        painter.drawPixmap(arrowRect, arrowPixmap);
                    }

                } else {
                    if(layoutDirection)
                    {
                        textRect = QRect(arrowPixmap.width() + margin + spacing, (option.rect.height() - fm.height()) / 2,iconPosition.x()- spacing*2 - margin*2-arrowRect.width() - option.iconSize.width()  , fm.height());
                    }
                    else
                    {
                        textRect = QRect(iconPosition.x() + spacing + iconWidth, 0, option.rect.width() - margin*2 - spacing*2 - option.iconSize.width()*2, option.rect.height());
                    }
                    painter.drawPixmap(iconRect, pixmap);

                    if (fm.width(option.text) > textRect.width()) {
                        if(layoutDirection)
                        {
                            arrowRect = QRect(margin, (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                            textRect = QRect(arrowRect.right() + spacing +margin, 0, iconPosition.x()- spacing - margin*2 - arrowRect.width() , option.rect.height());
                            elidedText = fm.elidedText(option.text, Qt::ElideLeft, textRect.width());
                            tf =  Qt::AlignVCenter | Qt::AlignRight | Qt::AlignAbsolute;
                        }
                        else
                        {
                            arrowRect = QRect(option.rect.width() - arrowPixmap.width() - margin, (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                            elidedText = fm.elidedText(option.text, Qt::ElideRight, textRect.width());
                        }

                        painter.drawText(textRect, tf, elidedText);
                        painter.drawPixmap(arrowRect, arrowPixmap);
                    } else {
                        if(layoutDirection)
                        {
                            arrowRect = QRect(/*textRect.right() + spacing*/margin, (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                            textRect = QRect(arrowRect.right() + spacing +margin, 0, iconPosition.x()- spacing - margin*2-arrowRect.width() , option.rect.height());
                            tf =  Qt::AlignVCenter | Qt::AlignRight | Qt::AlignAbsolute;
                        }
                        else
                        {
                            textRect = QRect(iconPosition.x() + spacing + iconWidth, 0, fm.width(option.text), option.rect.height());
                            arrowRect = QRect(option.rect.width() - arrowPixmap.width() - margin, (option.rect.height() - arrowPixmap.width()) / 2, arrowPixmap.width(), arrowPixmap.height());
                        }
                        painter.drawText(textRect, tf, option.text);
                        painter.drawPixmap(arrowRect, arrowPixmap);
                    }
                }
            }
        } else {
            if (!q->isEnabled()) {
                qreal opacity = 0.35;
                fontColor.setAlphaF(opacity);
                QPen pen;
                pen.setColor(fontColor);
                painter.setPen(pen);
                if (option.icon.isNull()) {
                    if(layoutDirection)
                        elidedText = fm.elidedText(option.text, Qt::ElideLeft, option.rect.width() - 10);
                    else
                        elidedText = fm.elidedText(option.text, Qt::ElideRight, option.rect.width() - 10);
                    if (fm.width(option.text) > option.rect.width() - 10) {
                        painter.drawText(option.rect.adjusted(margin, 0, -margin, 0), tf, elidedText);
                        q->setToolTip(option.text);
                    } else {
                        painter.drawText(option.rect.adjusted(margin, 0, -margin, 0), tf, option.text);
                        q->setToolTip("");
                    }
                } else {
                    elidedText = option.text;
                    if (elidedText.isEmpty()) {
                        iconRect = QRect((q->rect().width() - iconWidth) / 2 + 1, iconPosition.y() - 1, iconWidth, iconHeight);
                    } else {
                        textWidth = option.rect.width() - margin * 2 - spacing - iconWidth;
                        if (fm.width(option.text) > textWidth) {
                            if(layoutDirection)
                            {
                                textRect = QRect(margin, 0, iconPosition.x()- spacing - margin*2, option.rect.height());
                                elidedText = fm.elidedText(option.text, Qt::ElideLeft, textRect.width());
                                tf =  Qt::AlignVCenter | Qt::AlignRight | Qt::AlignAbsolute;
                            }
                            else
                            {
                                textRect = QRect(iconRect.right() + spacing, 0, option.rect.width() - margin * 2 - spacing - iconWidth, option.rect.height());
                                elidedText = fm.elidedText(option.text, Qt::ElideRight, textRect.width());
                            }
                            q->setToolTip(option.text);
                        } else {
                            if(layoutDirection)
                            {
                                textRect = QRect(margin, 0, iconPosition.x()- spacing - margin*2, option.rect.height());
                                tf =  Qt::AlignVCenter | Qt::AlignRight | Qt::AlignAbsolute;
                            }
                            else
                                textRect = QRect(iconRect.right() + spacing, 0, option.rect.width() - margin * 2 - spacing - iconWidth, option.rect.height());
                        }
                    }
                    painter.drawPixmap(iconRect, pixmap);
                    painter.drawText(textRect, tf, elidedText);
                }
            } else {
                if (option.icon.isNull()) {
                    if (fm.width(option.text) > option.rect.width() - 10) {
                        if(layoutDirection)
                        {
                            elidedText = fm.elidedText(option.text, Qt::ElideLeft, option.rect.width() - 10);
                            tf =  Qt::AlignVCenter | Qt::AlignRight | Qt::AlignAbsolute;
                        }
                        else
                            elidedText = fm.elidedText(option.text, Qt::ElideRight, option.rect.width() - 10);
                        painter.drawText(option.rect.adjusted(margin, 0, -margin, 0), tf, elidedText);
                        q->setToolTip(option.text);
                    } else {
                        painter.drawText(option.rect.adjusted(margin, 0, -margin, 0), tf, option.text);
                        q->setToolTip("");
                    }
                } else {
                    elidedText = option.text;
                    if (elidedText.isEmpty()) {
                        iconRect = QRect((q->rect().width() - iconWidth) / 2 , iconPosition.y() - 1, iconWidth, iconHeight);
                    } else {
                        if ((fm.width(option.text) > option.rect.width() - margin * 2 - spacing - iconWidth)) {
                            if(layoutDirection)
                            {
                                textRect = QRect(margin, 0,  iconPosition.x()- spacing - margin*2-arrowRect.width(), option.rect.height());
                                elidedText = fm.elidedText(option.text, Qt::ElideLeft, textRect.width());
                                tf =  Qt::AlignVCenter | Qt::AlignRight | Qt::AlignAbsolute;
                            }
                            else
                            {
//                                iconRect = QRect(margin, iconPosition.y(), iconWidth, iconHeight);
                                textRect = QRect(iconRect.right() + spacing, 0, option.rect.width() - margin * 2 - spacing - iconWidth, option.rect.height());
                                elidedText = fm.elidedText(option.text, Qt::ElideRight, textRect.width());
                            }
                            q->setToolTip(option.text);
                        } else {
                            if(layoutDirection)
                            {
                                textRect = QRect(margin, 0, iconPosition.x()- spacing - margin*2-arrowRect.width(), option.rect.height());
                                tf =  Qt::AlignVCenter | Qt::AlignRight | Qt::AlignAbsolute;
                            }
                            else
                                textRect = QRect(iconRect.right() + spacing, 0, option.rect.width() - margin * 2 - spacing - iconWidth, option.rect.height());
                        }
                    }
                    painter.drawPixmap(iconRect, pixmap);
                    painter.drawText(textRect, tf, elidedText);
                }
            }
        }
    }
}

}
#include "kpushbutton.moc"
#include "moc_kpushbutton.cpp"
