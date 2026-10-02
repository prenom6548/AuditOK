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
#include "pictureunit.h"
#include <QApplication>
#include <QFileInfo>
#include "maskwidget.h"
#include <QTouchEvent>
#include <QGSettings>
#include <QDebug>
#include <qmath.h>
#include <QPainter>
#include <QPainterPath>
#include <QtGlobal>
#include <limits>
#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

PictureUnit::PictureUnit(QWidget *parent, QSize size)
    : QLabel(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(size);
    setScaledContents(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
    // 壁纸缩略图默认可点选；组内仅选中项 StrongFocus，避免 Tab 跳到「回到主页」
    setFocusPolicy(Qt::ClickFocus);
    installEventFilter(this);

    m_maskLabel = new MaskLabel(this, true);
    m_maskLabel->setGeometry(0, 0, this->width(), this->height());
    m_maskLabel->setFocusPolicy(Qt::NoFocus);
    m_maskLabel->hide();
}

PictureUnit::~PictureUnit()
{
}

void PictureUnit::setFilenameText(QString fn){
    m_filename = fn;
    // 覆盖 KDK 统一物件名（各缩略图都是 …_picUnit），读屏按文件名区分
    if (!fn.isEmpty()) {
        setAccessibleName(QFileInfo(fn).fileName());
        setAccessibleDescription(QString());
    }
}

void PictureUnit::setColorName(QString colorName) {
    m_colorName = colorName;
}

QString PictureUnit::getColorName() {
    return m_colorName;
}

QString PictureUnit::filenameText(){
    return m_filename;
}

void PictureUnit::setBackground(QPixmap background)
{
    m_background = background;
    repaint();
}

void PictureUnit::setColor(QColor color)
{
    m_color = color;
    m_isColorUnit = true;
    // 颜色组默认 ClickFocus：Tab 只进组内一个 StrongFocus 项，组内用方向键只移焦
    setFocusPolicy(Qt::ClickFocus);
    repaint();
}

void PictureUnit::takeGroupTabStop()
{
    if (!parentWidget()) {
        return;
    }
    const QObjectList children = parentWidget()->children();
    for (QObject *obj : children) {
        PictureUnit *unit = qobject_cast<PictureUnit *>(obj);
        if (!unit || unit->m_isColorUnit != m_isColorUnit) {
            continue;
        }
        unit->setFocusPolicy(unit == this ? Qt::StrongFocus : Qt::ClickFocus);
    }
}

PictureUnit *PictureUnit::findGroupNeighbor(int key) const
{
    if (!parentWidget()) {
        return nullptr;
    }

    QList<PictureUnit *> units;
    const QObjectList children = parentWidget()->children();
    for (QObject *obj : children) {
        PictureUnit *unit = qobject_cast<PictureUnit *>(obj);
        if (unit && unit->m_isColorUnit == m_isColorUnit && unit->isVisible()) {
            units.append(unit);
        }
    }
    if (units.size() < 2) {
        return nullptr;
    }

    const QPoint center = geometry().center();
    PictureUnit *best = nullptr;
    qint64 bestScore = std::numeric_limits<qint64>::max();

    for (PictureUnit *unit : units) {
        if (unit == this) {
            continue;
        }
        const QPoint other = unit->geometry().center();
        const int dx = other.x() - center.x();
        const int dy = other.y() - center.y();
        qint64 score = -1;

        switch (key) {
        case Qt::Key_Left:
            if (dx >= 0) {
                break;
            }
            // 同行优先，再取最近
            score = qAbs(dy) * 100000LL + qAbs(dx);
            break;
        case Qt::Key_Right:
            if (dx <= 0) {
                break;
            }
            score = qAbs(dy) * 100000LL + qAbs(dx);
            break;
        case Qt::Key_Up:
            if (dy >= 0) {
                break;
            }
            score = qAbs(dx) * 100000LL + qAbs(dy);
            break;
        case Qt::Key_Down:
            if (dy <= 0) {
                break;
            }
            score = qAbs(dx) * 100000LL + qAbs(dy);
            break;
        default:
            break;
        }

        if (score >= 0 && score < bestScore) {
            bestScore = score;
            best = unit;
        }
    }
    return best;
}

void PictureUnit::activateColorUnit()
{
    m_clickedFlag = true;
    m_maskLabel->show();
    takeGroupTabStop();
    Q_EMIT clicked(m_filename);
}

void PictureUnit::activatePictureUnit()
{
    m_clickedFlag = true;
    m_maskLabel->show();
    takeGroupTabStop();
    Q_EMIT clicked(m_filename);
}

void PictureUnit::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        takeGroupTabStop();
        setFocus(Qt::MouseFocusReason);
        m_clickedFlag = true;
        m_maskLabel->show();
        Q_EMIT clicked(m_filename);
    }
}

void PictureUnit::keyPressEvent(QKeyEvent *e)
{
    const int key = e->key();
    if (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down) {
        // 与 ThemeButton 一致：方向键只切换焦点，空格/回车或鼠标再选中
        if (PictureUnit *neighbor = findGroupNeighbor(key)) {
            // TabFocusReason 才能稳定画出键盘焦点框；勿 takeGroupTabStop，Tab 仍落在已选中项
            neighbor->setFocus(Qt::TabFocusReason);
        }
        e->accept();
        return;
    }
    if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter) {
        if (m_isColorUnit) {
            activateColorUnit();
        } else {
            activatePictureUnit();
        }
        e->accept();
        return;
    }
    QLabel::keyPressEvent(e);
}

void PictureUnit::focusInEvent(QFocusEvent *e)
{
    QLabel::focusInEvent(e);
    // 焦点框由 paintEvent 用 WindowText 绘制（同 ThemeIconLabel），勿用蓝色 MaskLabel
    update();
}

void PictureUnit::focusOutEvent(QFocusEvent *e)
{
    QLabel::focusOutEvent(e);
    update();
}

bool PictureUnit::getClickedFlag()
{
    return m_clickedFlag;
}

void PictureUnit::changeClickedFlag(bool flag)
{   
    m_clickedFlag = flag;
    m_maskLabel->setVisible(m_clickedFlag);
}

bool PictureUnit::eventFilter(QObject *w, QEvent *e)
{
    if (e->type() == QEvent::Enter) {
        m_maskLabel->show();
    } else if (e->type() == QEvent::Leave) {
        if (!m_clickedFlag) {
            m_maskLabel->hide();
        }
    }
    if (w == this) {
        if (e->type() == QEvent::TouchBegin) {
            m_isTouchControl = true;
            QTouchEvent *touchE = static_cast<QTouchEvent*>(e);
            m_touchBeginPoint = touchE->touchPoints().at(0).screenPos();
        }
    } else if (e->type() == QEvent::TouchEnd && m_isTouchControl) {
        m_isTouchControl = false;
        QTouchEvent *touchE = static_cast<QTouchEvent*>(e);
        m_touchEndPoint = touchE->touchPoints().at(0).screenPos();
        if (qFabs(m_touchEndPoint.x() - m_touchBeginPoint.x()) < 1.0 &&
            qFabs(m_touchEndPoint.y() - m_touchBeginPoint.y()) < 1.0) {
            emit clicked(m_filename);
        }
    }
    return QWidget::eventFilter(w, e);
}

void PictureUnit::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if (m_isColorUnit) {
        QPainterPath path;
        const QRect bgRect(1, 1, width() - 2, height() - 2);
        path.addRoundedRect(bgRect, 8, 8);
        painter.fillRect(rect(), palette().color(QPalette::Base));
        painter.setClipPath(path);
        painter.fillPath(path, m_color);
        painter.drawPath(path);
    } else {
        painter.fillRect(rect(), palette().color(QPalette::Base));
        const QRect bgRect(4, 4, width() - 8, height() - 8);
        QPainterPath path;
        path.addRoundedRect(bgRect, 8, 8);
        painter.setClipPath(path);
        painter.drawPixmap(bgRect, m_background, m_background.rect());
    }

    // 键盘焦点框：与主题 ThemeIconLabel 一致——2px 圆角；浅色黑框、深色主题 WindowText 反白
    if (hasFocus()) {
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
