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

#include "kprogressbar.h"
#include "themeController.h"
#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QRect>
#include <QStyleOptionProgressBar>
#include <QVariantAnimation>

namespace kdk
{
class KProgressBarPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KProgressBar)

public:
    KProgressBarPrivate(KProgressBar *parent);
    void calculateTextRect();
    void calculateRect();
    void calculateContenteRect();
    void changeTheme();
    int computeHardLightChannel(int base, int blend);
    QColor hardLight(QColor blendColor, QColor baseColor = Qt::white);

Q_SIGNALS:
    void progressStateChanged();

private:
    KProgressBar *q_ptr;
    ProgressBarState m_state;
    int m_contentMargin;
    QRect m_textRect;
    QRect m_contentRect;
    QRect m_rect;
    int m_bodyWidth;
    QLinearGradient m_linearGradient;
    bool isLinearGradient;
    QColor m_color1;
    QColor m_color2;
    QColor m_backgroundColor;
    bool isBackgroundColor;
    QColor m_fillColor;
    bool isFillColor;
    QVariantAnimation *m_pAnimation;
    qreal m_animationPos;
    bool linearGradient;
    bool animationsFlag;
};

KProgressBar::KProgressBar(QWidget *parent)
    : QProgressBar(parent)
    , d_ptr(new KProgressBarPrivate(this))
{
    Q_D(KProgressBar);
    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            d->changeTheme();
        }
    });
    connect(this, &KProgressBar::valueChanged, this, [=]() {
        if (this->value() == this->maximum()) {
            setState(SuccessProgress);
        }
    });

    connect(d->m_pAnimation, &QVariantAnimation::valueChanged, this, [=](const QVariant &value) {
        if (orientation() == Qt::Horizontal)
            d->m_animationPos = value.toDouble() / d->m_contentRect.width();
        else {
            d->m_animationPos = (value.toDouble() - d->m_contentRect.top()) / d->m_contentRect.height();
        }
        update();
    });

    setContentsMargins(6, 6, 6, 6);
    setValue(0);
}

ProgressBarState KProgressBar::state()
{
    Q_D(KProgressBar);
    return d->m_state;
}

void KProgressBar::setState(ProgressBarState state)
{
    Q_D(KProgressBar);
    d->m_state = state;
    if (d->animationsFlag && state == ProgressBarState::NormalProgress) {
        d->m_pAnimation->start();
    } else {
        d->m_pAnimation->stop();
    }
    update();
}

void KProgressBar::paintEvent(QPaintEvent *event)
{
    Q_D(KProgressBar);
    d->calculateTextRect();
    d->calculateRect();
    d->calculateContenteRect();
    // 以上三个函数有逻辑关系，相对位置不能改变

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1)
        radius = 6;

    if (this->orientation() == Qt::Horizontal)
    {
        if (d->isBackgroundColor)
            painter.setBrush(d->m_backgroundColor);
        else
            painter.setBrush(ThemeController::getCustomColorFromDT("kcomponent-alpha-disable"));
        painter.setPen(ThemeController::getCustomColorFromDT("kline-component-normal"));
        painter.drawRoundedRect(d->m_rect, radius, radius);
        //        QLinearGradient linear(this->rect().topLeft(), this->rect().bottomRight());
        QLinearGradient linear(d->m_contentRect.left(), 0, d->m_contentRect.right(), 0);
        d->m_linearGradient = QLinearGradient(d->m_contentRect.topLeft(), d->m_contentRect.bottomRight());
        QColor color = ThemeController::getPaletteColorFromDT("highlight-active");
        switch (d->m_state) {
        case NormalProgress:
            if (d->animationsFlag) {
                linear.setColorAt(0, color);
                linear.setColorAt(d->m_animationPos, d->hardLight(color));
                linear.setColorAt(1, color);
            } else {
                QColor startColor;
                QColor endColor;
                ThemeController::getGradientFromDT("kbrand-normal",startColor,endColor);
                linear.setColorAt(0, startColor);
                linear.setColorAt(1, endColor);
            }
            linear.setSpread(QGradient::PadSpread);
            if (d->isLinearGradient) {
                if (!d->linearGradient) {
                    d->m_color1 = ThemeController::mixColor(color, Qt::white, 0.2);
                    d->m_color2 = ThemeController::mixColor(color, Qt::white, 0.05);
                }
                d->m_linearGradient.setColorAt(0, d->m_color1);
                d->m_linearGradient.setColorAt(1, d->m_color2);
                painter.setBrush(d->m_linearGradient);
            } else if (d->isFillColor)
                painter.setBrush(d->m_fillColor);
            else
                painter.setBrush(linear);
            painter.drawRoundedRect(d->m_contentRect, 6, 6);
            if (isTextVisible()) {
                painter.setPen(ThemeController::getCustomColorFromDT("kfont-primary"));
                painter.drawText(d->m_textRect, Qt::AlignCenter, text());
            }
            break;
        case FailedProgress:
        {
            QColor startColor;
            QColor endColor;
            ThemeController::getGradientFromDT("kerror-normal",startColor,endColor);
            linear.setColorAt(0,startColor);
            linear.setColorAt(1,endColor);
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            painter.drawRoundedRect(d->m_contentRect, 6, 6);
            if (isTextVisible()) {
                QPixmap pixmap = QIcon::fromTheme("dialog-error").pixmap(16, 16);
                QRect rect(0, 0, 16, 16);
                rect.moveCenter(d->m_textRect.center());
                painter.drawPixmap(rect, pixmap);
            }
            break;
        }
        case SuccessProgress:
        {
            QColor startColor;
            QColor endColor;
            ThemeController::getGradientFromDT("ksuccess-normal",startColor,endColor);
            linear.setColorAt(0,startColor);
            linear.setColorAt(1,endColor);
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            painter.drawRoundedRect(d->m_contentRect, 6, 6);
            if (isTextVisible()) {
                QPixmap pixmap = QIcon::fromTheme("ukui-dialog-success").pixmap(16, 16);
                QRect rect(0, 0, 16, 16);
                rect.moveCenter(d->m_textRect.center());
                painter.drawPixmap(rect, pixmap);
            }
            break;
        }
        default:
            break;
        }
    } else {
        if (d->isBackgroundColor)
            painter.setBrush(d->m_backgroundColor);
        else
        {
            painter.setBrush(ThemeController::getCustomColorFromDT("kcomponent-alpha-disable"));
        }
        painter.setPen(ThemeController::getCustomColorFromDT("kline-component-normal"));
        painter.drawRoundedRect(d->m_rect, 6, 6);
        QLinearGradient linear(0, d->m_contentRect.top(), 0, d->m_contentRect.bottom());

        d->m_linearGradient = QLinearGradient(d->m_contentRect.topLeft(), d->m_contentRect.bottomRight());
        QColor color = ThemeController::getPaletteColorFromDT("highlight-active");
        switch (d->m_state) {
        case NormalProgress:
            if (d->animationsFlag) {
                linear.setColorAt(0, color);
                linear.setColorAt(d->m_animationPos, d->hardLight(color));
                linear.setColorAt(1, color);
            } else {
                QColor startColor;
                QColor endColor;
                ThemeController::getGradientFromDT("kbrand-normal",startColor,endColor);
                linear.setColorAt(0, startColor);
                linear.setColorAt(1, endColor);
            }
            linear.setSpread(QGradient::PadSpread);
            if (d->isLinearGradient)
            {
                if (!d->linearGradient) {
                    d->m_color1 = ThemeController::mixColor(color, Qt::white, 0.2);
                    d->m_color2 = ThemeController::mixColor(color, Qt::white, 0.05);
                }
                d->m_linearGradient.setColorAt(0, d->m_color1);
                d->m_linearGradient.setColorAt(1, d->m_color2);
                painter.setBrush(d->m_linearGradient);
            } else if (d->isFillColor)
                painter.setBrush(d->m_fillColor);
            else
                painter.setBrush(linear);
            painter.drawRoundedRect(d->m_contentRect, 6, 6);
            if (isTextVisible()) {
                painter.setPen(ThemeController::getCustomColorFromDT("kfont-primary"));
                painter.drawText(d->m_textRect, Qt::AlignCenter, text());
            }
            break;
        case FailedProgress: {
            QColor startColor;
            QColor endColor;
            ThemeController::getGradientFromDT("kerror-normal",startColor,endColor);
            linear.setColorAt(0,startColor);
            linear.setColorAt(1,endColor);
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            painter.drawRoundedRect(d->m_contentRect, 6, 6);
            if (isTextVisible()) {
                QPixmap pixmap = QIcon::fromTheme("dialog-error").pixmap(16, 16);
                QRect rect(0, 0, 16, 16);
                rect.moveCenter(d->m_textRect.center());
                painter.drawPixmap(rect, pixmap);
            }
            break;
        }
        case SuccessProgress: {
            QColor startColor;
            QColor endColor;
            ThemeController::getGradientFromDT("ksuccess-normal",startColor,endColor);
            linear.setColorAt(0,startColor);
            linear.setColorAt(1,endColor);
            linear.setSpread(QGradient::PadSpread);
            painter.setBrush(linear);
            painter.drawRoundedRect(d->m_contentRect, 6, 6);
            if (isTextVisible()) {
                QPixmap pixmap = QIcon::fromTheme("ukui-dialog-success").pixmap(16, 16);
                // painter.drawPixmap(d->m_textRect,pixmap);
                QRect rect(0, 0, 16, 16);
                rect.moveCenter(d->m_textRect.center());
                painter.drawPixmap(rect, pixmap);
            }
            break;
        }
        default:
            break;
        }
    }
}

QSize KProgressBar::sizeHint() const
{
    QSize size = QProgressBar::sizeHint();
    if (this->orientation() == Qt::Horizontal)
        size.setHeight(30);
    else
        size.setWidth(30);
    return size;
}

QString KProgressBar::text() const
{
    Q_D(const KProgressBar);
    if ((maximum() == 0 && minimum() == 0) || value() < minimum()
        || (minimum() == INT_MIN && minimum() == INT_MIN))
        return QString();
    qint64 totalSteps = qint64(maximum()) - minimum();

    QString result = format();
    QLocale locale = this->locale(); // Omit group separators for compatibility with previous versions that were non-localized.
    locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);
    result.replace(QLatin1String("%m"), locale.toString(totalSteps));
    result.replace(QLatin1String("%v"), locale.toString(value()));

    // If max and min are equal and we get this far, it means that the
    // progress bar has one step and that we are on that step. Return
    // 100% here in order to avoid division by zero further down.
    if (totalSteps == 0) {
        result.replace(QLatin1String("%p"), locale.toString(100));
        return result;
    }

    const auto progress = static_cast<int>((qint64(value()) - minimum()) * 100.0 / totalSteps);
    result.replace(QLatin1String("%p"), locale.toString(progress));
    return result;
}

void KProgressBar::setOrientation(Qt::Orientation orientation)
{
    if (orientation == Qt::Vertical)
        this->setMinimumHeight(200);
    QProgressBar::setOrientation(orientation);
}

void KProgressBar::setBodyWidth(int width)
{
    Q_D(KProgressBar);
    d->m_bodyWidth = width;
    update();
}

void KProgressBar::setGradationColor(QColor color1, QColor color2)
{
    Q_D(KProgressBar);
    d->linearGradient = true;
    d->m_color1 = color1;
    d->m_color2 = color2;
}

void KProgressBar::setBackgroundColor(QColor color)
{
    Q_D(KProgressBar);
    d->m_backgroundColor = color;
    d->isBackgroundColor = true;
}

void KProgressBar::setFillColor(QColor color)
{
    Q_D(KProgressBar);
    d->m_fillColor = color;
    d->isFillColor = true;
}

void KProgressBar::setGradation(bool flag)
{
    Q_D(KProgressBar);
    d->isLinearGradient = flag;
}

bool KProgressBar::gradation()
{
    Q_D(KProgressBar);
    return d->isLinearGradient;
}

void KProgressBar::setAnimation(bool flag)
{
    Q_D(KProgressBar);
    d->animationsFlag = flag;

    if (!flag) {
        d->m_pAnimation->stop();
    } else {
        d->m_pAnimation->start();
    }
}

bool KProgressBar::animation()
{
    Q_D(KProgressBar);
    return d->animationsFlag;
}

KProgressBarPrivate::KProgressBarPrivate(KProgressBar *parent)
    : q_ptr(parent)
{
    Q_Q(KProgressBar);
    m_contentMargin = 2;
    m_state = NormalProgress;
    setParent(parent);
    m_bodyWidth = 0;
    isLinearGradient = false;
    isBackgroundColor = false;
    isFillColor = false;
    linearGradient = false;
    animationsFlag = false;

    m_animationPos = 0.00;
    m_pAnimation = new QVariantAnimation();
    m_pAnimation->setDuration(1000);
    m_pAnimation->setEasingCurve(QEasingCurve::Linear);
    m_pAnimation->setLoopCount(-1);
}

void KProgressBarPrivate::calculateTextRect()
{
    Q_Q(KProgressBar);
    if (!q->isTextVisible())
        m_textRect = QRect();
    else {
        QFont font(QApplication::font());
        QFontMetrics fm(font);
        m_textRect = QRect(0, 0, fm.width(q->text()), fm.height());
        m_textRect.moveCenter(q->rect().center());
        if (q->orientation() == Qt::Horizontal) {
            if (q->alignment() & Qt::AlignCenter)
                return;
            else {
                if (ThemeController::systemLang())
                    m_textRect.moveLeft(q->rect().left());
                else
                    m_textRect.moveRight(q->rect().right());
            }
        } else {
            if (q->alignment() & Qt::AlignCenter)
                return;
            else
                m_textRect.moveTop(q->rect().top());
        }
    }
}

void KProgressBarPrivate::calculateRect() // 背景矩形
{
    Q_Q(KProgressBar);
    QMargins margins = q->contentsMargins();
    m_rect = q->rect();
    if (q->orientation() == Qt::Horizontal) {
        if (m_bodyWidth != 0)
            m_rect.setHeight(m_bodyWidth);
        if (!q->isTextVisible())
            return;
        else {
            m_rect.moveCenter(q->rect().center());
            if (q->alignment() & Qt::AlignCenter)
                return;
            else {
                if (ThemeController::systemLang())
                    m_rect.setLeft(margins.right() + m_textRect.width() /*-gSpace*/);
                else
                    m_rect.setRight(q->rect().width() - margins.right() - m_textRect.width() /*-gSpace*/);
            }
        }
    } else {
        if (m_bodyWidth != 0)
            m_rect.setWidth(m_bodyWidth);
        if (!q->isTextVisible())
            return;
        else {
            m_rect.moveCenter(q->rect().center());
            if (q->alignment() & Qt::AlignCenter)
                return;
            else {
                m_rect.setTop(margins.top() + m_textRect.height() /* + gSpace*/);
            }
        }
    }
}

void KProgressBarPrivate::calculateContenteRect() // 填充矩形
{
    Q_Q(KProgressBar);
    m_contentRect = m_rect;
    if (q->orientation() == Qt::Horizontal) {
        int width;
        qint64 totalSteps = qint64(q->maximum()) - q->minimum();
        width = m_rect.width() * (q->value() - q->minimum()) / totalSteps;
        if (!width) {
            m_contentRect = QRect();
        }
        // 没设置反方向的情况下，即普通情况下
        if (!q->invertedAppearance()) {
            m_contentRect.setRight(width + m_rect.left());
        } else {
            m_contentRect.setLeft(m_rect.width() - width);
        }
        m_pAnimation->setStartValue(m_contentRect.x());
        m_pAnimation->setEndValue(m_contentRect.width());
    } else {
        int height;
        qint64 totalSteps = q->maximum() - q->minimum();
        height = m_rect.height() * (q->value() - q->minimum()) / totalSteps;
        if (!height) {
            m_contentRect = QRect();
        }
        // 没设置反方向的情况下，即普通情况下
        if (!q->invertedAppearance()) {
            m_contentRect.setTop(m_rect.top() + m_rect.height() - height);
        } else {
            m_contentRect.setBottom(m_rect.top() + height);
        }
        m_pAnimation->setStartValue(m_contentRect.bottom());
        m_pAnimation->setEndValue(m_contentRect.top());
    }
}

void KProgressBarPrivate::changeTheme()
{
    Q_Q(KProgressBar);
    initThemeStyle();
    q->repaint();
}

int KProgressBarPrivate::computeHardLightChannel(int base, int blend)
{
    if (blend <= 128) {
        return (base * blend) / 128;
    }
    return 255 - ((255 - blend) * (255 - base)) / 128;
}

QColor KProgressBarPrivate::hardLight(QColor blendColor, QColor baseColor)
{
    int r = computeHardLightChannel(baseColor.red(), blendColor.red());
    int g = computeHardLightChannel(baseColor.green(), blendColor.green());
    int b = computeHardLightChannel(baseColor.blue(), blendColor.blue());

    return QColor(r, g, b, blendColor.alpha());
}


}

#include "kprogressbar.moc"
#include "moc_kprogressbar.cpp"
