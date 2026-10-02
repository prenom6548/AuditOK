/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "Ukui4CustomControl.h"

#include <QKeyEvent>
#include <QToolTip>
#include <QStyleOptionSlider>
#include <QCoreApplication>

AutoOmitLabel::AutoOmitLabel(QWidget* parent)
    : QLabel(parent)
{

}

AutoOmitLabel::AutoOmitLabel(const QString& text, QWidget* parent)
    : QLabel(parent)
{
    setText(text);
}

void AutoOmitLabel::paintEvent(QPaintEvent* event)
{
    QFontMetrics fontMetrics(font());
    int fontSize = fontMetrics.width(m_fullText);
    if (fontSize > width()) {
        setText(fontMetrics.elidedText(m_fullText, Qt::ElideRight, width()), false);
        setToolTip(m_fullText);
    } else {
        setText(m_fullText, false);
        setToolTip("");
    }
    QLabel::paintEvent(event);
}

void AutoOmitLabel::setText(const QString& text, bool save)
{
    if (save)
        m_fullText = text;
    QLabel::setText(text);
}


UkmediaVolumeSlider::UkmediaVolumeSlider(QWidget* parent)
    : kdk::KSlider(parent)
{
    Q_UNUSED(parent);
    setSliderType(kdk::SmoothSlider);
    setNodeVisible(false);
}

void UkmediaVolumeSlider::updateValue(QMouseEvent* e)
{
    int value = 0;
    int currentX = e->pos().x();
    double per = currentX * 1.0 / this->width();
    if ((this->maximum() - this->minimum()) >= 50) {
        value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
        if (value <= (this->maximum() / 2 - this->maximum() / 10 + this->minimum() / 10)) {
            value = qRound(per*(this->maximum() - this->minimum() - 1)) + this->minimum();
        } else if (value > (this->maximum() / 2 + this->maximum() / 10 + this->minimum() / 10)) {
            value = qRound(per*(this->maximum() - this->minimum() + 1)) + this->minimum();
        } else {
            value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
        }
    } else {
        value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
    }
    this->setValue(value);
}

void UkmediaVolumeSlider::mousePressEvent(QMouseEvent* e)
{
    m_bMousePress = true;

    // 向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
    QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
    QCoreApplication::sendEvent(parentWidget(), &evEvent);

    return KSlider::mousePressEvent(e);
}

void UkmediaVolumeSlider::mouseReleaseEvent(QMouseEvent* e)
{
    m_bMousePress = false;

    return KSlider::mouseReleaseEvent(e);
}

void UkmediaVolumeSlider::mouseMoveEvent(QMouseEvent* e)
{

    return KSlider::mouseMoveEvent(e);
}

void UkmediaVolumeSlider::wheelEvent(QWheelEvent* e)
{
    return KSlider::wheelEvent(e);
}

void UkmediaVolumeSlider::keyReleaseEvent(QKeyEvent* e)
{
    return KSlider::keyReleaseEvent(e);
}

void UkmediaVolumeSlider::initStyleOption(QStyleOptionSlider* option)
{
    return KSlider::initStyleOption(option);
}

void UkmediaVolumeSlider::leaveEvent(QEvent* e)
{
    return KSlider::leaveEvent(e);
}

void UkmediaVolumeSlider::enterEvent(QEvent* e)
{
    return KSlider::enterEvent(e);
}

UkmediaVolumeSlider::~UkmediaVolumeSlider()
{
//    delete m_pTiplabel;
}

UkBalanceVolumeSlider::UkBalanceVolumeSlider(QWidget* parent)
    : kdk::KSlider(parent)
{
    Q_UNUSED(parent);
    setSliderType(kdk::SingleSelectSlider);
    setRange(-100,100);
    setSingleStep(100);
    setTickInterval(100);
    setOrientation(Qt::Horizontal);
    setFocusPolicy(Qt::StrongFocus);
    this->installEventFilter(this);
}

void UkBalanceVolumeSlider::showTooltip()
{
    QString percent = QString::number(this->value());
    percent.append("%");

    QStyleOptionSlider opt;
    this->initStyleOption(&opt);
    QRect handleRect = this->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    QPoint point = this->mapToGlobal(handleRect.topLeft());
    QFontMetrics fontMetrics = QFontMetrics(this->font());
    QRect fontRect = fontMetrics.boundingRect(percent);

    QToolTip::showText(point - QPoint(fontRect.width()/2 +3, fontRect.height() + 40), percent, this);
    return;
}

bool UkBalanceVolumeSlider::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == this) {
        if (event->type() == QEvent::HoverEnter) {
            showTooltip();
        } else if (event->type() == QEvent::HoverLeave) {
            QToolTip::hideText();
        }
    }
    return kdk::KSlider::eventFilter(watched, event);
}

void UkBalanceVolumeSlider::initStyleOption(QStyleOptionSlider* option)
{
    kdk::KSlider::initStyleOption(option);
}

UkBalanceVolumeSlider::~UkBalanceVolumeSlider()
{
}
