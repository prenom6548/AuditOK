/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "themeiconlabel.h"
#include <QPainter>
#include <QPainterPath>
#include <QGridLayout>
#include <QDebug>
#include "themebutton.h"

ThemeIconLabel::ThemeIconLabel(QSize size, QWidget *parent) : QLabel(parent)
{
    setFixedSize(size);
    initMaskLabel();
}

ThemeIconLabel::ThemeIconLabel(QPixmap pixmap, QSize size, QWidget *parent) : QLabel(parent)
{
    setFixedSize(size);
    initMaskLabel();
    QIcon icon(pixmap);
    m_background = icon;
}

ThemeIconLabel::ThemeIconLabel(QStringList iStringList, QSize size, QWidget *parent):
    mIconList(iStringList),
    QLabel(parent)
{
    setFixedSize(size);
    initMaskLabel();
    QGridLayout *gLyt = new QGridLayout(this);
    gLyt->setContentsMargins(0, 0, 0, 0);
    gLyt->setSpacing(10);
    for (int i = 0; i < iStringList.count(); i++) {
        QLabel * label = new QLabel(this);
        label->setFixedSize(32, 32);
        label->setPixmap(pixmapToRound(iStringList.at(i), QSize(32, 32)));
        gLyt->addWidget(label, i / 3 , i % 3);
    }
}

ThemeIconLabel::ThemeIconLabel(QList<QPixmap> iconList, QSize size, QWidget *parent):
    QLabel(parent)
{
    setFixedSize(size);
    initMaskLabel();
    QGridLayout *gLyt = new QGridLayout(this);
    gLyt->setContentsMargins(0, 0, 0, 0);
    gLyt->setSpacing(10);
    for (int i = 0; i < iconList.count(); i++) {
        QLabel * label = new QLabel(this);
        label->setFixedSize(24, 24);
        label->setPixmap(iconList.at(i));
        gLyt->addWidget(label, i / 3 , i % 3);
    }
}

ThemeIconLabel::ThemeIconLabel(const QIcon &icon, QSize size, QWidget *parent) : QLabel(parent)
{
    setFixedSize(size);
    initMaskLabel();
    QHBoxLayout *gLyt = new QHBoxLayout(this);
    gLyt->setContentsMargins(0, 0, 0, 0);
    gLyt->setAlignment(Qt::AlignCenter);
    m_pIconLabel = new QLabel(this);
    m_pIconLabel->setFixedSize(size);
    m_pIconLabel->setPixmap(pixmapToRound(icon.pixmap(size), size));

}

void ThemeIconLabel::initMaskLabel() {
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_AcceptTouchEvents);
    installEventFilter(this);
    setScaledContents(true);
    m_maskLabel = new MaskLabel(this, true);
    m_maskLabel->setGeometry(0, 0, this->width(), this->height());
    m_maskLabel->hide();
}

void ThemeIconLabel::setIconCenter()
{
    m_pIconLabel->move(this->x() + (this->width() - m_pIconLabel->width())/2, this->y()+(this->height() -m_pIconLabel->height())/2);
}

QPixmap ThemeIconLabel::pixmapToRound(const QString &filePath, const QSize &scaledSize)
{
    QPixmap pixmap(filePath);

    qreal dpi = devicePixelRatioF();

    QSize realSize = scaledSize * dpi;
    QPixmap resultPixmap(realSize);

    resultPixmap.fill(Qt::transparent);
    QPainter painter(&resultPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    //画圆
//    QPainterPath path;
//    path.addEllipse(0, 0, realSize.width(), realSize.height());
//    painter.setClipPath(path);


    QPoint drawLeftTop(0, 0);
    QSize drawSize = realSize;

    drawSize.setWidth(realSize.width());
    drawSize.setHeight(realSize.width() * scaledSize.height() / scaledSize.width());
    drawLeftTop.setX(0);
    drawLeftTop.setY((realSize.height() - drawSize.height()) / 2);

    // scaled draw
    painter.drawPixmap(drawLeftTop.x(), drawLeftTop.y(), drawSize.width(), drawSize.height(), pixmap.scaled(drawSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    resultPixmap.setDevicePixelRatio(dpi);
    return resultPixmap;
}

QPixmap ThemeIconLabel::pixmapToRound(const QPixmap &pixmap, const QSize &scaledSize)
{
    qreal dpi = devicePixelRatioF();

    QSize realSize = scaledSize * dpi;
    QPixmap resultPixmap(realSize);

    resultPixmap.fill(Qt::transparent);
    QPainter painter(&resultPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPoint drawLeftTop(0, 0);
    QSize drawSize = realSize;

    drawSize.setWidth(realSize.width());
    drawSize.setHeight(realSize.width() * scaledSize.height() / scaledSize.width());
    drawLeftTop.setX(0);
    drawLeftTop.setY((realSize.height() - drawSize.height()) / 2);

    // scaled draw
    painter.drawPixmap(drawLeftTop.x(), drawLeftTop.y(), drawSize.width(), drawSize.height(), pixmap.scaled(drawSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    resultPixmap.setDevicePixelRatio(dpi);
    return resultPixmap;
}

void ThemeIconLabel::setRadius(int radius)
{
    mRadius = radius - 2;
}

void ThemeIconLabel::setIcon(QPixmap pixmap)
{
    m_background = pixmap;
    this->repaint();
}

void ThemeIconLabel::setMaskSize(const QSize &)
{
    m_maskLabel->setGeometry(0, 0, this->width(), this->height());
}

void ThemeIconLabel::changeClickedFlag(bool flag)
{
    m_clickedFlag = flag;
    m_maskLabel->setVisible(m_clickedFlag);
}

bool ThemeIconLabel::eventFilter(QObject *w, QEvent *e)
{
    ThemeButton *parent = static_cast<ThemeButton *>(this->parent());
    if (parent && (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::Enter || e->type() == QEvent::Leave)) {
        if (!parent->isEnabled() && !parent->isChecked() && m_maskLabel) {
            m_maskLabel->hide();
            return QWidget::eventFilter(w, e);
        }
    }
    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent *>(e);
        if (mouseEvent->button() == Qt::LeftButton ) {
            m_clickedFlag = true;
            m_maskLabel->show();
        }
    }
    if (e->type() == QEvent::Enter) {
        Q_EMIT enterWidget();
        m_maskLabel->show();
    } else if (e->type() == QEvent::Leave) {
        Q_EMIT leaveWidget();
        if (!m_clickedFlag) {
            m_maskLabel->hide();
        }
    }
    return QWidget::eventFilter(w, e);
}

void ThemeIconLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const QRect bgRect(4, 4, width() - 8, height() - 8);
    QPainterPath path;
    path.addRoundedRect(bgRect, 8, 8);
    painter.fillPath(path, palette().window());
    painter.setClipPath(path);
    QSize size = bgRect.size();
    QPixmap pixmap = m_background.pixmap(size);
    painter.drawPixmap(bgRect, pixmap, pixmap.rect());

    // 键盘焦点框与选中框（MaskLabel）同几何：整块图标区、2px、圆角 8；深色主题用文字色反白
    ThemeButton *btn = qobject_cast<ThemeButton *>(parentWidget());
    if (btn && btn->hasFocus()) {
        painter.setClipping(false);
        QPainterPath focusPath;
        const QRectF borderRect = rect().adjusted(1, 1, -1, -1);
        focusPath.addRoundedRect(borderRect, 8, 8);
        QPen pen(palette().color(QPalette::WindowText), 2);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(focusPath);
    }
}
