/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "ukmedia_custom_class.h"
#include <QPalette>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include <QStyleOptionComplex>
#include <QStylePainter>
#include <QDebug>
#include <QGSettings>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QApplication>


#define TIMER_TIMEOUT   (2*1000)
#define FOREGROUND_COLOR_NORMAL qApp->palette().text().color()

extern double transparency;

UkuiApplicationWidget::UkuiApplicationWidget(QWidget *parent)
{
//    this->setAttribute(Qt::WA_TranslucentBackground);
//    this->setStyleSheet("QWiget{background:rgba(0,0,0,0);}");
    Q_UNUSED(parent);
}

void UkuiApplicationWidget::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    double transparence = transparency * 255;
    QColor color = palette().color(QPalette::Base);
    color.setAlpha(transparence);
    QBrush brush = QBrush(QColor(0,0,0,0));
    p.setBrush(brush);
    p.setBrush(this->palette().base());
    p.setBrush(QBrush(QColor(19, 19, 20, 0)));
    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,0,0);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,0,0);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

}

UkuiApplicationWidget::~UkuiApplicationWidget()
{

}

static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}

QColor mixColor(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }
    if (qIsNaN(bias)) {
        return c1;
    }

    qreal r = mixQreal(c1.redF(),   c2.redF(),   bias);
    qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = mixQreal(c1.blueF(),  c2.blueF(),  bias);
    qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}

void UkuiSettingButton::onPaletteChanged()
{
    m_foregroundColor = FOREGROUND_COLOR_NORMAL;
    this->repaint();
}

void UkuiSettingButton::setPressColor()
{
    QColor hightlight = this->palette().color(QPalette::Active,QPalette::Highlight);
    QColor mix = this->palette().color(QPalette::Active,QPalette::BrightText);
    m_foregroundColor = mixColor(hightlight, mix, 0.05);
}

void UkuiSettingButton::setHoverColor()
{
    QColor hightlight = this->palette().color(QPalette::Active,QPalette::Highlight);
    QColor mix = this->palette().color(QPalette::Active,QPalette::BrightText);
    m_foregroundColor = mixColor(hightlight, mix, 0.2);
}

void UkuiSettingButton::setNormalColor()
{
    m_foregroundColor = FOREGROUND_COLOR_NORMAL;
}

UkuiSettingButton::UkuiSettingButton(QWidget *parent)
{
    connect(qApp, &QApplication::paletteChanged, this, &UkuiSettingButton::onPaletteChanged);
    onPaletteChanged();
}

UkuiSettingButton::~UkuiSettingButton()
{

}

void UkuiSettingButton::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();
    pal.setColor(QPalette::WindowText, m_foregroundColor);
    this->setPalette(pal);
    return QLabel::paintEvent(event);
}

void UkuiSettingButton::enterEvent(QEvent *event)
{
    setHoverColor();
    this->update();
}

void UkuiSettingButton::leaveEvent(QEvent *event)
{
    setNormalColor();
    this->update();
}

void UkuiSettingButton::mousePressEvent(QMouseEvent *event)
{
    setPressColor();
    this->update();
    return QLabel::mousePressEvent(event);
}

void UkuiSettingButton::mouseReleaseEvent(QMouseEvent *event)
{
    setHoverColor();
    this->update();
    Q_EMIT clicked();
    return QLabel::mouseReleaseEvent(event);
}

AudioSlider::AudioSlider(QWidget *parent)
    : KSlider(parent)
{
    Q_UNUSED(parent);
    setRange(0,100);
    setSliderType(SmoothSlider);
    setOrientation(Qt::Horizontal);
    setFocusPolicy(Qt::StrongFocus);
    setNodeVisible(false);
}

void AudioSlider::wheelEvent(QWheelEvent *e)
{
    if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::wheelEvent(e);
}

void AudioSlider::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_VolumeMute) {
        qDebug() << "AudioSlider keyReleaseEvent " << e->key();
        // 过滤掉快捷键的操作
        e->ignore(); // 忽略该事件
        return;
    }

    //start 长按左键或者下键调低音量
    if (e->isAutoRepeat())
    {
        if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
            blueValue = this->value();
            Q_EMIT blueValueChanged(blueValue);
        }
        return;
    }
    //end

    if (this->value() != blueValue)
    {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::keyReleaseEvent(e);
}

void AudioSlider::mouseMoveEvent(QMouseEvent *e)
{
    isMouseWheel = true;

    if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::mouseMoveEvent(e);
}

void AudioSlider::mouseReleaseEvent(QMouseEvent *e)
{
    isMouseWheel = false;
    if (this->value() != blueValue)
    {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::mouseReleaseEvent(e);
}

AudioSlider::~AudioSlider()
{

}

void UkuiButtonDrawSvg::init(QImage img, QColor color)
{
    themeIcon.image = img;
    themeIcon.color = color;
}

void UkuiButtonDrawSvg::paintEvent(QPaintEvent *event)
{
    QStyleOptionComplex opt;
    opt.init(this);
    QPainter p(this);

    QColor color;
    switch (buttonState) {
    case PUSH_BUTTON_NORMAL:
        color = QColor(0x25,0x25,0x25,0xFF);
        break;
    case PUSH_BUTTON_CLICK:
        color = QColor(0x37,0x90,0xFA,0xFF);
        break;
    case PUSH_BUTTON_PRESS:
        color = QColor(0x24,0x6D,0xD4,0xFF);
        break;
    default:
        break;
    }

    p.setBrush(QBrush(color));
    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,18,18);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,18,18);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawComplexControl(QStyle::CC_ToolButton,&opt,&p,this);

//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    p.setBrush(QBrush(QColor(0x13,0x13,0x14,0x00)));
//    p.setPen(Qt::NoPen);
//    QPainterPath path;
//    opt.rect.adjust(0,0,0,0);
//    path.addRoundedRect(opt.rect,18,18);
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.drawRoundedRect(opt.rect,18,18);
//    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QRect UkuiButtonDrawSvg::IconGeometry()
{
    QRect res = QRect(QPoint(0,0),QSize(24,24));
    res.moveCenter(QRect(0,0,width(),height()).center());
    return res;
}

void UkuiButtonDrawSvg::draw(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    QRect iconRect = IconGeometry();
    if (themeIcon.image.size() != iconRect.size())
    {
        themeIcon.image = themeIcon.image.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect r = themeIcon.image.rect();
        r.moveCenter(iconRect.center());
        iconRect = r;
    }

    this->setProperty("fillIconSymbolicColor", true);
    filledSymbolicColoredPixmap(themeIcon.image,themeIcon.color);
    painter.drawImage(iconRect, themeIcon.image);
}

bool UkuiButtonDrawSvg::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Paint:
        draw(static_cast<QPaintEvent*>(event));
        break;

    case QEvent::Move:
    case QEvent::Resize:
    {
        QRect rect = IconGeometry();
    }
        break;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
        event->accept();
        break;

    default:
        break;
    }

    return QPushButton::event(event);
}


UkuiButtonDrawSvg::UkuiButtonDrawSvg(QWidget *parent)
{
    Q_UNUSED(parent);
}
UkuiButtonDrawSvg::~UkuiButtonDrawSvg()
{

}

QPixmap UkuiButtonDrawSvg::filledSymbolicColoredPixmap(QImage &img, QColor &baseColor)
{

    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                int hue = color.hue();
                if (!qAbs(hue - symbolic_color.hue()) < 10) {
                    color.setRed(baseColor.red());
                    color.setGreen(baseColor.green());
                    color.setBlue(baseColor.blue());
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }

    return QPixmap::fromImage(img);
}

FixLabel::FixLabel(QWidget *parent):
    QLabel(parent)
{

}

FixLabel::FixLabel(QString text , QWidget *parent):
    QLabel(parent)
{
    this->setText(text);
}

FixLabel::~FixLabel()
{

}

void FixLabel::paintEvent(QPaintEvent *event)
{
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(fullText);
    if (fontSize > this->width()) {
        this->setText(fontMetrics.elidedText(fullText, Qt::ElideRight, this->width()), false);
        this->setToolTip(fullText);
    } else {
        this->setText(fullText, false);
        this->setToolTip("");
    }
    QLabel::paintEvent(event);
}

void FixLabel::setText(const QString & text, bool saveTextFlag)
{
    if (saveTextFlag)
        fullText = text;
    QLabel::setText(text);
}

Divider::Divider(QWidget * parent) : QFrame(parent)
{
    this->setFixedSize(420,1);
}

void Divider::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    QColor color = qApp->palette().color(QPalette::BrightText);
    color.setAlphaF(0.08);
    p.save();
    p.setBrush(color);
    p.setPen(Qt::transparent);
    p.drawRoundedRect(this->rect(), 6, 6);
    p.restore();
    return QFrame::paintEvent(e);
}
