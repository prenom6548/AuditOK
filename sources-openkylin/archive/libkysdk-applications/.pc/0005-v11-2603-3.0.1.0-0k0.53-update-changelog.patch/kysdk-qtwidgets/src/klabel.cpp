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

#include "klabel.h"
#include "themeController.h"
#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRect>

namespace kdk
{

static const int spacing = 4;
class KLabelPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KLabel)
public:
    KLabelPrivate(KLabel *parent);

protected:
    void changeTheme();

private:
    KLabel *q_ptr;
    KLabelType m_labelType;
    QPixmap m_pixmap;
    QString m_text;
    QColor m_backgroundColor;
    QColor m_fontColor;
    QIcon m_icon;
    QPalette::ColorRole m_fontColorRole;
    bool m_dataHightColorFlag;
    bool m_isBackgroundFlag;
    int m_topLeftRadius;
    int m_topRightRadius;
    int m_bottomLeftRadius;
    int m_bottomRightRadius;
    int diffPointSize;
    int m_pixmapWidth;
    int m_pixmapHeight;
    bool m_isHighlightBackgroundFlag;
    bool m_wordWrap;
    bool m_fontColorFlag;
    bool m_isIcon;
    bool m_fontColorRoleFlag;
    bool m_isTristate;
    bool m_clicked;
};

KLabel::KLabel(QWidget *parent)
    : QLabel(parent)
    , d_ptr(new KLabelPrivate(this))
{
    Q_D(KLabel);
    int pointSize = QApplication::font().pointSize();
    connect(d->m_gsetting, &QGSettings::changed, this, [=](const QString &key) {
        d->changeTheme();
        int now = this->font().pointSize();
        if (now != QApplication::font().pointSize()) {
            if (d->diffPointSize == 0)
                d->diffPointSize = now - pointSize;
            QFont font = this->font();
            font.setPointSize(d->diffPointSize + QApplication::font().pointSize());
            setFont(font);
        }

        if ("styleName" == key) {
            QColor textColor = ThemeController::getPaletteColorFromDT("placeholdertext-active");
        }
    });
}

void KLabel::setLabelType(KLabelType type)
{
    Q_D(KLabel);
    d->m_labelType = type;
}

void KLabel::setDataHightColor(bool flag)
{
    Q_D(KLabel);
    d->m_dataHightColorFlag = flag;
}

void KLabel::setPixmap(const QPixmap &pixmap)
{
    Q_D(KLabel);
    d->m_isIcon = false;
    d->m_pixmap = pixmap;
    update();
}

void KLabel::setPixmap(QIcon icon, int width, int height)
{
    Q_D(KLabel);
    d->m_isIcon = true;
    d->m_pixmapWidth = width;
    d->m_pixmapHeight = height;
    d->m_icon = icon;
    update();
}

const QPixmap *KLabel::pixmap() const
{
    Q_D(const KLabel);
    return &d->m_pixmap;
}

void KLabel::setText(const QString &str)
{
    Q_D(KLabel);
    QLabel::setText(str);
}

QString KLabel::text()
{
    Q_D(KLabel);
    return QLabel::text();
}

void KLabel::setBackgroundColor(bool flag, QColor color)
{
    Q_D(KLabel);
    if (d->m_isHighlightBackgroundFlag)
        d->m_isHighlightBackgroundFlag = false;
    d->m_isBackgroundFlag = flag;
    d->m_backgroundColor = color;
    update();
}

void KLabel::setHighlightBackground(bool flag)
{
    Q_D(KLabel);
    if (d->m_isBackgroundFlag)
        d->m_isBackgroundFlag = false;
    d->m_isHighlightBackgroundFlag = flag;
    update();
}

void KLabel::setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight)
{
    Q_D(KLabel);
    d->m_bottomRightRadius = bottomRight;
    d->m_topLeftRadius = topLeft;
    d->m_bottomLeftRadius = bottomLeft;
    d->m_topRightRadius = topRight;
}

void KLabel::setBorderRadius(int radius)
{
    Q_D(KLabel);
    d->m_bottomRightRadius = radius;
    d->m_topLeftRadius = radius;
    d->m_bottomLeftRadius = radius;
    d->m_topRightRadius = radius;
}

void KLabel::setWordWrap(bool flag)
{
    Q_D(KLabel);
    d->m_wordWrap = flag;
    QLabel::setWordWrap(true);
}

bool KLabel::wordWrap()
{
    Q_D(KLabel);
    return d->m_wordWrap;
}

void KLabel::setFontColor(QColor color)
{
    Q_D(KLabel);
    d->m_fontColor = color;
    d->m_fontColorFlag = true;
    d->m_fontColorRoleFlag = false;
    update();
}

void KLabel::setFontColorRole(QPalette::ColorRole role)
{
    Q_D(KLabel);
    d->m_fontColorRoleFlag = true;
    d->m_fontColorFlag = false;
    d->m_fontColorRole = role;
    update();
}

void KLabel::setTristate(bool flag)
{
    Q_D(KLabel);
    d->m_isTristate = flag;
    if (d->m_isTristate) {
        QColor textColor = ThemeController::getPaletteColorFromDT("placeholdertext-active");
        setFontColor(textColor);
    } else {
        d->m_pixmap = ThemeController::drawColoredPixmap(d->m_pixmap, ThemeController::getPaletteColorFromDT("highlight-active"));
        if (d->m_fontColorRoleFlag)
            setFontColorRole(d->m_fontColorRole);
        else if (d->m_fontColorRoleFlag)
            setFontColorRole(d->m_fontColorRole);
        else if (d->m_dataHightColorFlag)
            setFontColorRole(QPalette::Highlight);
        else
            setFontColorRole(QPalette::Text);
    }
}

bool KLabel::isTristate()
{
    Q_D(KLabel);
    return d->m_isTristate;
}

void KLabel::paintEvent(QPaintEvent *event)
{
    Q_D(KLabel);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    QColor bkgColor;
    QRect drawRect = rect();
    QRect iconRect;
    QRect textRect;

    painter.setPen(Qt::NoPen);
    QString str = text();

    // 绘制背景
    QPainterPath path;
    path.moveTo(drawRect.topRight() - QPoint(d->m_topRightRadius, 0)); // 右上
    path.lineTo(drawRect.topLeft() + QPointF(d->m_topLeftRadius, 0)); // 上方线
    path.quadTo(drawRect.topLeft(), drawRect.topLeft() + QPointF(0, d->m_topLeftRadius)); // 圆角
    path.lineTo(drawRect.bottomLeft() + QPointF(0, -d->m_bottomLeftRadius)); // 左方线
    path.quadTo(drawRect.bottomLeft(), drawRect.bottomLeft() + QPointF(d->m_bottomLeftRadius, 0)); // 圆角
    path.lineTo(drawRect.bottomRight() - QPointF(d->m_bottomRightRadius, 0)); // 下方线
    path.quadTo(drawRect.bottomRight(), drawRect.bottomRight() + QPointF(0, -d->m_bottomRightRadius)); // 圆角
    path.lineTo(drawRect.topRight() + QPointF(0, d->m_topRightRadius)); // 右方线
    path.quadTo(drawRect.topRight(), drawRect.topRight() - QPointF(d->m_topRightRadius, 0)); // 圆角

    switch (d->m_labelType) {
    case DataType:
        bkgColor = ThemeController::getPaletteColorFromDT("highlight-active");
        bkgColor.setAlphaF(0.15);
        if (d->m_isBackgroundFlag) {
            painter.setBrush(d->m_backgroundColor);
            painter.setPen(Qt::NoPen);
        } else {
            painter.setBrush(bkgColor);
            painter.setPen(Qt::NoPen);
        }
        painter.drawPath(path);
        break;
    case NormalType:
        if (d->m_isBackgroundFlag)
            painter.setBrush(d->m_backgroundColor);
        else if (d->m_isHighlightBackgroundFlag)
            painter.setBrush(ThemeController::getPaletteColorFromDT("highlight-active"));
        else
            painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::NoPen);
        painter.drawPath(path);
        break;
    case CircleType:
        if (d->m_isBackgroundFlag)
            painter.setBrush(d->m_backgroundColor);
        else if (d->m_isHighlightBackgroundFlag)
            painter.setBrush(ThemeController::getPaletteColorFromDT("highlight-active"));
        else
            painter.setBrush(ThemeController::getPaletteColorFromDT("button-active"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect());
        break;
    default:
        break;
    }

    QMargins margins = contentsMargins();

    // 绘制图标和文字
    if (d->m_isIcon)
    {
        d->m_pixmap = d->m_icon.pixmap(d->m_pixmapWidth, d->m_pixmapHeight);
        if ((d->m_pixmap.size().width() != d->m_pixmapWidth || d->m_pixmap.size().height() != d->m_pixmapHeight) && !d->m_pixmap.isNull())
            d->m_pixmap = d->m_pixmap.scaled(d->m_pixmapWidth, d->m_pixmapHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    if (!wordWrap()) {
        int drawLength;
        if (!text().isEmpty() && !d->m_pixmap.isNull())
            drawLength = d->m_pixmap.width() + spacing + fontMetrics().width(text());
        else if (!text().isEmpty() && d->m_pixmap.isNull())
            drawLength = fontMetrics().width(text());
        else if (text().isEmpty() && !d->m_pixmap.isNull())
            drawLength = d->m_pixmap.width();

        if (!text().isEmpty() && !d->m_pixmap.isNull()) {
            int drawLength = d->m_pixmap.width() + spacing + fontMetrics().width(text());
            if (drawRect.width() >= drawLength) {
                iconRect = QRect(/*drawRect.center().x() - drawLength/2*/ drawRect.x() + margins.left(),
                                 drawRect.center().y() - d->m_pixmap.height() / 2 + margins.top(),
                                 d->m_pixmap.width(),
                                 d->m_pixmap.height());
                textRect = QRect(iconRect.right() + spacing,
                                 /*drawRect.center().y() - fontMetrics().height()/2 +*/ margins.top(),
                                 fontMetrics().width(text()) + 1,
                                 /*fontMetrics().height()*/
                                 drawRect.height());
                setToolTip("");

            } else {
                iconRect = QRect(/*drawRect.x()*/ drawRect.x() + margins.left(),
                                 drawRect.center().y() - d->m_pixmap.height() / 2 + margins.top(),
                                 d->m_pixmap.width(),
                                 d->m_pixmap.height());
                textRect = QRect(iconRect.right() + spacing,
                                 /*drawRect.center().y() - fontMetrics().height()/2 +*/ margins.top(),
                                 drawRect.width() - spacing - iconRect.width(),
                                 /*fontMetrics().height()*/
                                 drawRect.height());
                setToolTip(str);
                str = fontMetrics().elidedText(str, Qt::ElideRight, textRect.width());
            }
        } else {
            if (text().isEmpty()) {
                iconRect = QRect(drawRect.center().x() - drawLength / 2 + margins.left(),
                                 drawRect.center().y() - d->m_pixmap.height() / 2 + margins.top(),
                                 d->m_pixmap.width(),
                                 d->m_pixmap.height());
            } else {
                int drawLength = fontMetrics().width(text());
                if (drawRect.width() >= drawLength) {
                    textRect = QRect(/*drawRect.center().x() - drawLength/2*/ drawRect.x() + margins.left(),
                                     /*drawRect.center().y() - fontMetrics().height()/2 +*/ margins.top(),
                                     fontMetrics().width(text()) + 2,
                                     /*fontMetrics().height()*/
                                     drawRect.height());
                    setToolTip("");
                } else {
                    textRect = QRect(drawRect.x() + margins.left(),
                                     /*drawRect.center().y() - fontMetrics().height()/2 + */ margins.top(),
                                     drawRect.width(),
                                     /*fontMetrics().height()*/
                                     drawRect.height());
                    setToolTip(str);
                    str = fontMetrics().elidedText(str, Qt::ElideRight, textRect.width());
                }
            }
        }

        if (d->m_dataHightColorFlag) {
            d->m_pixmap = ThemeController::drawColoredPixmap(d->m_pixmap, ThemeController::getPaletteColorFromDT("highlight-active"));
            if (d->m_fontColorFlag)
                painter.setPen(d->m_fontColor);
            else if (d->m_fontColorRoleFlag)
                painter.setPen(palette().color(d->m_fontColorRole));
            else
                painter.setPen(ThemeController::getPaletteColorFromDT("highlight-active"));
        } else if (d->m_isBackgroundFlag || d->m_isHighlightBackgroundFlag) {
            d->m_pixmap = ThemeController::drawSymbolicColoredPixmap(d->m_pixmap);
            if (d->m_fontColorFlag)
                painter.setPen(d->m_fontColor);
            else if (d->m_fontColorRoleFlag)
                painter.setPen(palette().color(d->m_fontColorRole));
            else
                painter.setPen(ThemeController::getPaletteColorFromDT("text-active"));
        } else {
            if (d->m_fontColorFlag)
                painter.setPen(d->m_fontColor);
            else if (d->m_fontColorRoleFlag)
                painter.setPen(palette().color(d->m_fontColorRole));
            else
                painter.setPen(ThemeController::getPaletteColorFromDT("text-active"));
            if (!d->m_pixmap.isNull()) {
                if (ThemeController::isPixmapPureColor(d->m_pixmap)) {
                    if (ThemeController::themeMode() == DarkTheme)
                        d->m_pixmap = ThemeController::drawColoredPixmap(d->m_pixmap, Qt::white);
                    else
                        d->m_pixmap = ThemeController::drawColoredPixmap(d->m_pixmap, ThemeController::getPaletteColorFromDT("windowtext-active"));
                }
            }
        }

        if (alignment() == Qt::AlignCenter) {
            if (!text().isEmpty() && !d->m_pixmap.isNull()) {
                iconRect.moveLeft(drawRect.center().x() - drawLength / 2);
                textRect.moveLeft(iconRect.right() + spacing);
            } else {
                if (text().isEmpty())
                    iconRect.moveLeft(drawRect.center().x() - drawLength / 2);
                else
                    textRect.moveLeft(drawRect.center().x() - drawLength / 2);
            }
        }
        painter.drawText(textRect, Qt::AlignVCenter, str);
        painter.drawPixmap(iconRect, d->m_pixmap);
    } else {
        QFontMetrics metrics(this->font());

        int lineHeight = metrics.height();
        int xPos = drawRect.left(); // 起始x位置
        int yPos = metrics.leading() + metrics.ascent(); // 起始y位置

        while (!str.isEmpty()) {
            QString line;
            int widthUsed = 0;
            if (rect().height() > (yPos + lineHeight)) {
                QChar ch;
                for (int i = 0; i < str.length(); ++i) {
                    ch = str[i];
                    int charWidth = metrics.horizontalAdvance(ch);
                    if (ch == '\n') {
                        str.remove(ch);
                        break;
                    }
                    if (widthUsed + charWidth > this->width() - 2) {
                        break; // 当前行已满，开始新行
                    }

                    line += ch;
                    widthUsed += charWidth;
                }
                if (d->m_fontColorFlag)
                    painter.setPen(d->m_fontColor);
                else if (d->m_fontColorRoleFlag)
                    painter.setPen(palette().color(d->m_fontColorRole));
                else if (d->m_dataHightColorFlag)
                    painter.setPen(ThemeController::getPaletteColorFromDT("highlight-active"));
                else
                    painter.setPen(ThemeController::getPaletteColorFromDT("text-active"));
                painter.drawText(xPos, yPos, line);
                yPos += lineHeight;
                setToolTip("");
            } else {
                line = fontMetrics().elidedText(str, Qt::ElideRight, rect().width());
                if (d->m_fontColorFlag)
                    painter.setPen(d->m_fontColor);
                else if (d->m_fontColorRoleFlag)
                    painter.setPen(palette().color(d->m_fontColorRole));
                else if (d->m_dataHightColorFlag)
                    painter.setPen(ThemeController::getPaletteColorFromDT("highlight-active"));
                else
                    painter.setPen(ThemeController::getPaletteColorFromDT("text-active"));
                if (line != text())
                    setToolTip(text());
                painter.drawText(xPos, yPos, line);
                break;
            }
            // 准备下一行
            str = str.mid(line.length());
        }
    }
}

void KLabel::resizeEvent(QResizeEvent *event)
{
    Q_D(KLabel);
    int height = d->m_pixmapHeight > fontMetrics().height() ? d->m_pixmapHeight : fontMetrics().height();
    if (height > size().height()) {
        setMinimumHeight(height);
    }
    QLabel::resizeEvent(event);
}

void KLabel::enterEvent(QEvent *event)
{
    Q_D(KLabel);
    if (d->m_isTristate) {
        QColor textColor = ThemeController::getPaletteColorFromDT("placeholdertext-active");
        QColor highlight = ThemeController::getPaletteColorFromDT("highlight-active");
        QColor mix = ThemeController::getPaletteColorFromDT("brighttext-active");
        textColor = ThemeController::mixColor(highlight, mix, 0.05);
        setFontColor(textColor);
    }

    Q_EMIT enterWidget();
    QLabel::enterEvent(event);
}

void KLabel::leaveEvent(QEvent *event)
{
    Q_D(KLabel);
    if (d->m_isTristate) {
        QColor textColor = ThemeController::getPaletteColorFromDT("placeholdertext-active");
        setFontColor(textColor);
    }
    Q_EMIT leaveWidget();

    QLabel::leaveEvent(event);
}

void KLabel::mousePressEvent(QMouseEvent *event)
{
    Q_D(KLabel);
    if (event->button() == Qt::LeftButton && d->m_isTristate) {
        QColor textColor = ThemeController::getPaletteColorFromDT("placeholdertext-active");
        QColor highlight = ThemeController::getPaletteColorFromDT("highlight-active");
        QColor mix = ThemeController::getPaletteColorFromDT("brighttext-active");
        textColor = ThemeController::mixColor(highlight, mix, 0.2);
        setFontColor(textColor);
    }
}

void KLabel::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(KLabel);
    if (event->button() == Qt::LeftButton && d->m_isTristate) {
        QColor textColor = ThemeController::getPaletteColorFromDT("placeholdertext-active");
        QColor highlight = ThemeController::getPaletteColorFromDT("highlight-active");
        QColor mix = ThemeController::getPaletteColorFromDT("brighttext-active");
        textColor = ThemeController::mixColor(highlight, mix, 0.05);
        setFontColor(textColor);
    }
    d->m_clicked = !d->m_clicked;
    Q_EMIT clicked(d->m_clicked);
}

void KLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_EMIT doubleClicked();
    return;
}

QSize KLabel::sizeHint() const
{
    Q_D(const KLabel);
    return QLabel::sizeHint();
}

KLabelPrivate::KLabelPrivate(KLabel *parent)
    : q_ptr(parent)
    , m_isBackgroundFlag(false)
    , m_dataHightColorFlag(false)
    , m_bottomRightRadius(6)
    , m_topLeftRadius(6)
    , m_bottomLeftRadius(6)
    , m_topRightRadius(6)
    , diffPointSize(0)
    , m_pixmapWidth(16)
    , m_pixmapHeight(16)
    , m_isHighlightBackgroundFlag(false)
    , m_wordWrap(false)
    , m_fontColorFlag(false)
    , m_isIcon(false)
    , m_fontColorRoleFlag(false)
    , m_isTristate(false)
    , m_clicked(false)
{
    Q_Q(KLabel);
    m_labelType = NormalType;
}

void KLabelPrivate::changeTheme()
{
    Q_Q(KLabel);
    initThemeStyle();
}

}

#include "klabel.moc"
#include "moc_klabel.cpp"
