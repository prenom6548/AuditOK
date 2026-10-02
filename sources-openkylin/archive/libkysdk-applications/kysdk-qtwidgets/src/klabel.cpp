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
#include <QTextDocument>
#include <QStyle>
#include <QTextCursor>

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
    QPixmap *m_pixmap;
    QPixmap *scaledpixmap;
    QImage *cachedimage;
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
    int diffPixelSize;
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
    int pointSize = QApplication::font().pointSize(); //获取初始化时系统字体大小 pt
    int pixelSize = QApplication::font().pixelSize(); //获取初始化时系统字体大小 px
    connect(d->m_gsetting, &QGSettings::changed, this, [=](const QString &key) {
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            d->changeTheme();
        }
        if(key == "systemFontSize" || key == "systemFont")
        {
            if(this->font().pixelSize() != pixelSize) //如果控件字体px != 系统默认字体
            {
                int now = this->font().pixelSize();
                if(now != pixelSize || d->diffPixelSize != 0)
                {
                    if(d->diffPixelSize == 0)
                        d->diffPixelSize = now - pointSize;
                    QFont font = this->font();
                    font.setPixelSize(d->diffPixelSize + QApplication::font().pointSize());
                    setFont(font);
                }
            }
            else if(this->font().pointSize() != pointSize || d->diffPointSize != 0)
            {
                int now = this->font().pointSize();
                if(now != QApplication::font().pointSize() || d->diffPointSize != 0)
                {
                    if(d->diffPointSize == 0)
                        d->diffPointSize = now - pointSize;
                    QFont font = this->font();
                    font.setPointSize(d->diffPointSize + QApplication::font().pointSize());
                    setFont(font);
                }
            }
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
    if (!d->m_pixmap || d->m_pixmap->cacheKey() != pixmap.cacheKey()) {
        d->m_pixmap = new QPixmap(pixmap);
    }
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
    if(d->m_pixmap && d->m_pixmap->isNull())
        return d->m_pixmap;
    else
        return nullptr;
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
        setFontColorRole(QPalette::PlaceholderText);
    } else {
        if(d->m_pixmap)
            *d->m_pixmap = ThemeController::drawColoredPixmap(*d->m_pixmap, ThemeController::getPaletteColorFromDT("highlight-active"));
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
    QPixmap pix;
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    QColor bkgColor;
    QMargins margins = contentsMargins();
    QRect drawRect = rect().adjusted(margins.left(),margins.top(),-margins.right(),-margins.bottom());
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

    // 绘制图标和文字
    if (d->m_isIcon)
    {
        if (!d->m_pixmap || d->m_pixmap->cacheKey() != d->m_icon.pixmap(d->m_pixmapWidth, d->m_pixmapHeight).cacheKey()) {
            d->m_pixmap = new QPixmap(d->m_icon.pixmap(d->m_pixmapWidth, d->m_pixmapHeight));
        }
    }
    if (!wordWrap()) {
        int drawLength = this->rect().width();
        if (!text().isEmpty() && d->m_pixmap && !d->m_pixmap->isNull())
            drawLength = d->m_pixmap->width() + spacing + fontMetrics().width(text());
        else if (!text().isEmpty())
            drawLength = fontMetrics().width(text());
        else if (text().isEmpty() && d->m_pixmap && !d->m_pixmap->isNull())
            drawLength = d->m_pixmap->width();

        if (!text().isEmpty() && d->m_pixmap && !d->m_pixmap->isNull()) {
            iconRect = QRect(drawRect.x() ,
                             drawRect.height()/2 - d->m_pixmap->height() / 2 + 1,
                             d->m_pixmap->width(),
                             d->m_pixmap->height());
            textRect = QRect(iconRect.right() + spacing,
                             drawRect.top(),
                             drawRect.width() - spacing - iconRect.width(),
                             drawRect.height());
            drawLength = d->m_pixmap->width() + spacing + fontMetrics().width(text());
            if (drawRect.width() >= drawLength) {
                setToolTip("");
            } else {
                setToolTip(str);
                str = fontMetrics().elidedText(str, Qt::ElideRight, textRect.width());
            }
        } else {
            if (text().isEmpty() && d->m_pixmap) {
                iconRect = QRect(drawRect.width()/2 - drawLength / 2 ,
                                 drawRect.height()/2 - d->m_pixmap->height() / 2 ,
                                 d->m_pixmap->width(),
                                 d->m_pixmap->height());
            } else {
                textRect = QRect(drawRect.x(),
                                 drawRect.top(),
                                 drawRect.width(),
                                 drawRect.height());
                if (drawRect.width() >= drawLength) {
                    setToolTip("");
                } else {
                    setToolTip(str);
                    str = fontMetrics().elidedText(str, Qt::ElideRight, textRect.width());
                }
            }
        }

        if(hasSelectedText())
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(ThemeController::getCustomColorFromDT("highlight-active"));
            QRect backgroundRect ;
            if (drawRect.width() >= drawLength)
            {
                backgroundRect = QRect(textRect.x(),textRect.height() /2 - fontMetrics().height()/2,drawRect.width(),fontMetrics().height());
            }
            else
            {
                backgroundRect = QRect(textRect.x(),textRect.height() /2 - fontMetrics().height()/2,fontMetrics().width(str),fontMetrics().height());
            }
            painter.drawRoundedRect(backgroundRect,0,0);
            painter.setPen(ThemeController::getCustomColorFromDT("highlightedtext-active"));
        }
        else
        {
            if (d->m_dataHightColorFlag) {
                if(d->m_pixmap)
                    *d->m_pixmap = ThemeController::drawColoredPixmap(*d->m_pixmap, ThemeController::getPaletteColorFromDT("highlight-active"));
                if (d->m_fontColorFlag)
                    painter.setPen(d->m_fontColor);
                else if (d->m_fontColorRoleFlag)
                    painter.setPen(palette().color(d->m_fontColorRole));
                else
                    painter.setPen(ThemeController::getPaletteColorFromDT("highlight-active"));
            } else if (d->m_isHighlightBackgroundFlag) {
                if(d->m_pixmap)
                    *d->m_pixmap = ThemeController::drawSymbolicColoredPixmap(*d->m_pixmap);
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
                if(d->m_isIcon)
                {
                    if (d->m_pixmap && !d->m_pixmap->isNull() && !d->m_icon.isNull() && d->m_icon.name()!="") {
                        if (ThemeController::isPixmapPureColor(*d->m_pixmap)) {
                            if (ThemeController::themeMode() == DarkTheme)
                                *d->m_pixmap = ThemeController::drawColoredPixmap(*d->m_pixmap, Qt::white);
                            else
                                *d->m_pixmap = ThemeController::drawColoredPixmap(*d->m_pixmap, ThemeController::getPaletteColorFromDT("windowtext-active"));
                        }
                    }
                }
                else
                {
                    if (d->m_pixmap && !d->m_pixmap->isNull()) {
                        if (ThemeController::isPixmapPureColor(*d->m_pixmap)) {
                            if (ThemeController::themeMode() == DarkTheme)
                                *d->m_pixmap = ThemeController::drawColoredPixmap(*d->m_pixmap, Qt::white);
                            else
                                *d->m_pixmap = ThemeController::drawColoredPixmap(*d->m_pixmap, ThemeController::getPaletteColorFromDT("windowtext-active"));
                        }
                    }
                }
            }
        }

        if (alignment() == Qt::AlignCenter || alignment() == Qt::AlignHCenter) {
          if (!text().isEmpty() && d->m_pixmap && !d->m_pixmap->isNull()) {
                if (drawRect.width() >= drawLength)
                    iconRect.moveLeft(drawRect.width() / 2 - drawLength / 2);
                else
                    iconRect.moveLeft(drawRect.left());
          } else {
            if (text().isEmpty())
              iconRect.moveLeft(drawRect.width() / 2 - drawLength / 2);
          }
        }
        else if(alignment() == Qt::AlignRight)
            iconRect.moveLeft(drawRect.width() - fontMetrics().width(str) - spacing - iconRect.width());

        bool rtl = false;
        for (const QChar& ch : text())
        {
            if (ch.unicode() >= 0x0590 && ch.unicode() <= 0x08FF)
            {
                // 希伯来文、阿拉伯文等 RTL 字符范围
                rtl = true;
                break;
            }
        }

        Qt::Alignment m_alignment = this->alignment();

//        if (rtl)
//        {
//            if (m_alignment & Qt::AlignLeft)
//            {
//                m_alignment = (m_alignment & ~Qt::AlignLeft) | Qt::AlignRight;
//            }
//            else if (m_alignment & Qt::AlignRight)
//            {
//                m_alignment = (m_alignment & ~Qt::AlignRight) | Qt::AlignLeft;
//            }

//        }

        if(d->m_pixmap && !d->m_pixmap->isNull())
        {
            pix = *d->m_pixmap;

            qreal scale = pix.devicePixelRatio();
            QRect aligned = this->style()->alignedRect(QGuiApplication::layoutDirection(), QFlag(m_alignment), pix.size() / scale, contentsRect());
            QRect inter = aligned.intersected(contentsRect());
            if(!text().isEmpty() && d->m_pixmap && !d->m_pixmap->isNull())
            {
                if(m_alignment & Qt::AlignHCenter)
                {
                    inter.moveLeft(inter.x() - fontMetrics().width(text())/2);
                    textRect.setLeft(inter.right() + spacing);
                    m_alignment &= ~Qt::AlignHCenter;
                }
            }
            painter.drawPixmap(inter.x(), inter.y(), inter.width(), inter.height(), pix);
        }

        painter.drawText(textRect, m_alignment | Qt::AlignVCenter, str);
    } else {
        QRect contentRect = drawRect.adjusted(margins.left(), margins.top(), -margins.right(), -margins.bottom());
        if (d->m_pixmap && !d->m_pixmap->isNull()) {
            int iconX = contentRect.x();
            int iconY = contentRect.y() + (contentRect.height() - d->m_pixmap->height()) / 2;
            if (alignment() & Qt::AlignHCenter) {
                iconX = contentRect.x() + (contentRect.width() - d->m_pixmap->width()) / 2;
            } else if (alignment() & Qt::AlignRight) {
                iconX = contentRect.right() - d->m_pixmap->width();
            }

            iconRect = QRect(iconX, iconY, d->m_pixmap->width(), d->m_pixmap->height());

            if (alignment() & Qt::AlignLeft) {
                contentRect.setLeft(iconRect.right() + spacing);
            } else if (alignment() & Qt::AlignRight) {
                contentRect.setRight(iconRect.left() - spacing);
            }
            if(d->m_pixmap && !d->m_pixmap->isNull())
            {
                pix = *d->m_pixmap;
                qreal scale = pix.devicePixelRatio();
                QRect aligned = this->style()->alignedRect(QGuiApplication::layoutDirection(), QFlag(alignment()), pix.size() / scale, contentsRect());
                QRect inter = aligned.intersected(contentsRect());
                if(!text().isEmpty() && d->m_pixmap && !d->m_pixmap->isNull())
                {
                    textRect.setLeft(inter.right());
                }
                painter.drawPixmap(inter.x(), inter.y(), inter.width(), inter.height(), pix);
            }
        }

        if (!text().isEmpty()) {
            QTextDocument doc;
            doc.setDefaultFont(font());
            doc.setDocumentMargin(0);
            doc.setPlainText(text());

            QTextCursor cursor(&doc);
            cursor.select(QTextCursor::Document);

            QTextCharFormat format;

            QColor penColor;
            if (d->m_fontColorFlag)
                penColor = d->m_fontColor;
            else if (d->m_fontColorRoleFlag)
                penColor = palette().color(d->m_fontColorRole);
            else
                penColor = ThemeController::getPaletteColorFromDT("text-active");
            format.setForeground(penColor);
            cursor.mergeCharFormat(format);

            QTextOption textOption;
            textOption.setWrapMode(QTextOption::WordWrap);
            textOption.setAlignment(alignment());
            doc.setDefaultTextOption(textOption);
            doc.setTextWidth(contentRect.width());

            qreal textHeight = doc.size().height();
            qreal yPos = static_cast<qreal>(contentRect.y());

            if (alignment() & Qt::AlignBottom) {
                yPos = static_cast<qreal>(contentRect.bottom()) - textHeight;
            } else if (alignment() & Qt::AlignVCenter) {
                yPos = static_cast<qreal>(contentRect.y()) + (static_cast<qreal>(contentRect.height()) - textHeight) / 2;
            }

            yPos = qMax(static_cast<qreal>(contentRect.y()),
                        qMin(yPos, static_cast<qreal>(contentRect.bottom()) - textHeight));
            painter.save();
            painter.translate(contentRect.x(), static_cast<int>(yPos));
            painter.setClipRect(QRect(0, 0, contentRect.width(), contentRect.height()));
            doc.drawContents(&painter, QRectF(0, 0, contentRect.width(), contentRect.height()));
            painter.restore();
        }
    }
}

void KLabel::resizeEvent(QResizeEvent *event)
{
    Q_D(KLabel);
    int height = d->m_pixmapHeight > fontMetrics().height() ? d->m_pixmapHeight : fontMetrics().height();
    if(d->m_pixmap)
        height = d->m_pixmap->height() > height ? d->m_pixmap->height() : height;
    if (height > size().height() && (minimumHeight() != maximumHeight()))
    {
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
        setFontColorRole(QPalette::PlaceholderText);
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
    QLabel::mousePressEvent(event);
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
    QLabel::mouseReleaseEvent(event);
}

void KLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_EMIT doubleClicked();
    QLabel::mouseDoubleClickEvent(event);
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
    , diffPixelSize(0)
    , m_pixmapWidth(16)
    , m_pixmapHeight(16)
    , m_isHighlightBackgroundFlag(false)
    , m_wordWrap(false)
    , m_fontColorFlag(false)
    , m_isIcon(false)
    , m_fontColorRoleFlag(false)
    , m_isTristate(false)
    , m_clicked(false)
    , m_pixmap(nullptr)
    , scaledpixmap(nullptr)
    , cachedimage(nullptr)
{
    Q_Q(KLabel);
    setParent(parent);
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
