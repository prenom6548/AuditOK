/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "marktool.h"
#include <QPainter>
#include <QLine>
#include <QGraphicsBlurEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
Mark::MarkTool::MarkTool() {}

void Mark::MarkTool::setPixmap(const QPixmap &pix)
{
    clear();
    m_pix = pix;
}

void Mark::MarkTool::setMarkOperatio(Mark::MarkOperatio *mo)
{
    m_operationList.append(mo);
}

void Mark::MarkTool::undo()
{
    if (m_operationList.length() == 0) {
        return;
    }
    MarkOperatio *mo = m_operationList.last();
    m_operationList.removeLast();
    if (mo) {
        delete mo;
        mo = nullptr;
    }
}

void Mark::MarkTool::clear()
{
    for (MarkOperatio *mo : m_operationList) {
        if (mo) {
            delete mo;
            mo = nullptr;
        }
    }
    m_operationList.clear();
}

bool Mark::MarkTool::save(const QString &path, bool backup)
{
    if (m_operationList.isEmpty()) {
        return false;
    }
    QString filePath = path;

    QPixmap pix = m_pix.copy();
    for (MarkOperatio *mo : m_operationList) {
        QPainter painter(&pix);
        double trueProportion = 1;
        if (!trueProportion) {
            continue;
        }
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setPen(QPen(mo->color, mo->thickness));
        switch (mo->markType) {
        case RECTANGLE: {
            MarkOperatioRectangle *mos = static_cast<MarkOperatioRectangle *>(mo);
            painter.drawRect(QRect(mos->rect.topLeft() / trueProportion, mos->rect.size() / trueProportion));
            break;
        }
        case CIRCLE: {
            MarkOperatioCircle *mos = static_cast<MarkOperatioCircle *>(mo);
            painter.drawEllipse(mos->rLeftX / trueProportion, mos->rLeftY / trueProportion, mos->rx / trueProportion,
                                mos->ry / trueProportion);
            break;
        }
        case LINE: {
            MarkOperatioLine *mos = static_cast<MarkOperatioLine *>(mo);
            painter.drawLine(mos->startPoint / trueProportion, mos->endPoint / trueProportion);
            break;
        }
        case ARROW: {
            MarkOperatioArrow *mos = static_cast<MarkOperatioArrow *>(mo);
            QPoint startPoint = mos->startPoint / trueProportion;
            QPoint endPoint = mos->endPoint / trueProportion;
            int thickness = mos->thickness / trueProportion;
            QLineF line(startPoint, endPoint);
            double cSqrt =
                pow(mos->arrowStartPoint.x() - endPoint.x(), 2) + pow(mos->arrowStartPoint.y() - endPoint.y(), 2);
            double aSqrt = pow(mos->arrowStartPoint.x() - mos->arrowEndPoint.x(), 2)
                           + pow(mos->arrowStartPoint.y() - mos->arrowEndPoint.y(), 2);
            double b = sqrt(cSqrt - aSqrt / 2);
            line.setLength(line.length() - b);
            painter.drawLine(line);
            QPainterPath path;
            path.moveTo(mos->arrowStartPoint);
            path.lineTo(endPoint);
            path.lineTo(mos->arrowEndPoint);
            path.lineTo(mos->arrowStartPoint);
            painter.fillPath(path, QBrush(mos->color));
            break;
        }
        case PENCIL: {
            MarkOperatioPencil *mos = static_cast<MarkOperatioPencil *>(mo);
            QVector<QPoint> tmp;
            for (QPoint p : mos->pointList) {
                tmp.append(p / trueProportion);
            }
            painter.drawPolyline(tmp.data(), tmp.length());
            break;
        }
        case MARKER: {
            MarkOperatioMarker *mos = static_cast<MarkOperatioMarker *>(mo);
            painter.setCompositionMode(QPainter::CompositionMode_Multiply);
            painter.setOpacity(mos->alpha);
            painter.drawLine(mos->startPoint / trueProportion, mos->endPoint / trueProportion);
            break;
        }
        case TEXT: {
            MarkOperatioText *mos = static_cast<MarkOperatioText *>(mo);
            QFont ft;
            ft.setPointSize(mos->thickness / trueProportion);
            ft.setFamily(mos->family);
            ft.setBold(mos->blod);
            ft.setItalic(mos->italic);
            ft.setUnderline(mos->underline);
            ft.setStrikeOut(mos->strikeout);
            painter.setFont(ft);
            QPoint start = mos->startPoint / trueProportion;
            painter.drawText(QRect(start, QSize(pix.width() - start.x(), pix.height() - start.y())), mos->text);
            break;
        }
        case BLUR: {
            int radius = 8;
            int tmpRadius = radius * 2;
            MarkOperatioBlur *mos = static_cast<MarkOperatioBlur *>(mo);
            QRect selectionOrig(mos->rect.topLeft() / trueProportion, mos->rect.size() / trueProportion);
            QRect selection = orderRect(selectionOrig);
            QRect tmpSelection(selection.topLeft() - QPoint(tmpRadius, tmpRadius),
                               selection.size() + QSize(tmpRadius * 2, tmpRadius * 2));
            QGraphicsBlurEffect *blur = new QGraphicsBlurEffect();
            blur->setBlurRadius(radius);
            QGraphicsPixmapItem *item = new QGraphicsPixmapItem();
            item->setPixmap(pix.copy(tmpSelection));
            item->setGraphicsEffect(blur);
            QGraphicsScene scene;
            scene.addItem(item);
            QPixmap p(tmpSelection.size());
            p.fill();
            QPainter pai(&p);
            scene.render(&pai);
            scene.clear();
            painter.drawPixmap(selection, p, QRect(QPoint(tmpRadius, tmpRadius), selection.size()));
            break;
        }
        }
    }
    return saveImage(path, pix, backup);
}

QRect Mark::MarkTool::orderRect(const QRect &orig)
{
    QRect selection = orig;
    if (orig.width() < 0) {
        selection.setX(orig.x() + orig.width());
        selection.setWidth(orig.width() * -1);
    }
    if (orig.height() < 0) {
        selection.setY(orig.y() + orig.height());
        selection.setHeight(orig.height() * -1);
    }
    QRect tmp = selection;
    if (tmp.x() < 0) {
        selection.setX(0);
        selection.setWidth(tmp.x() + tmp.width());
    }
    if (tmp.y() < 0) {
        selection.setY(0);
        selection.setHeight(tmp.y() + tmp.height());
    }
    return selection;
}

bool Mark::MarkTool::saveImage(const QString &path, QPixmap pix, bool backup)
{
    QImage image = pix.toImage().copy();
    if (image.isNull()) {
        qDebug()<<"标注失败";
        return false;
    }
    QString realPath = path;
    realPath = realPath.mid(0, realPath.lastIndexOf(".")) + ".png";
    if (!File::saveImage(image, realPath, "png", backup)) {
        qDebug() << "保存失败";
        Q_EMIT saveFinish();
        return false;
    }
    Q_EMIT saveFinish();
    this->clear();
    return true;
}

Mark::MarkOperatioRectangle::MarkOperatioRectangle(QColor c, QRect rec, double p, int t)
{
    markType = RECTANGLE;
    color = c;
    thickness = t;
    proportion = p;
    rect = rec;
}

Mark::MarkOperatioCircle::MarkOperatioCircle(QColor c, double leftX, double leftY, int x, int y, double p, int t)
{
    markType = CIRCLE;
    color = c;
    thickness = t;
    proportion = p;
    rLeftX = leftX;
    rLeftY = leftY;
    rx = x;
    ry = y;
}

Mark::MarkOperatioLine::MarkOperatioLine(QColor c, QPoint start, QPoint end, double p, int t)
{
    markType = LINE;
    color = c;
    thickness = t;
    proportion = p;
    startPoint = start;
    endPoint = end;
}

Mark::MarkOperatioArrow::MarkOperatioArrow(QColor c, QPoint start, QPoint end, double p, int t, QPoint arrowStart,
                                           QPoint arrowEnd)
{
    markType = ARROW;
    color = c;
    thickness = t;
    proportion = p;
    startPoint = start;
    endPoint = end;
    arrowStartPoint = arrowStart;
    arrowEndPoint = arrowEnd;
}

Mark::MarkOperatioPencil::MarkOperatioPencil(QColor c, QVector<QPoint> pl, double p, int t)
{
    markType = PENCIL;
    color = c;
    thickness = t;
    proportion = p;
    pointList = pl;
}

Mark::MarkOperatioMarker::MarkOperatioMarker(QColor c, QPoint start, QPoint end, double p, int t, double a)
{
    markType = MARKER;
    color = c;
    thickness = t;
    proportion = p;
    startPoint = start;
    endPoint = end;
    alpha = a;
}

Mark::MarkOperatioText::MarkOperatioText(QColor c, QString t, QPoint start, int fs, double p, QString f, bool b, bool i,
                                         bool u, bool s)
{
    markType = TEXT;
    color = c;
    thickness = fs;
    proportion = p;
    startPoint = start;
    text = t;
    family = f;
    blod = b;
    italic = i;
    underline = u;
    strikeout = s;
}

Mark::MarkOperatioBlur::MarkOperatioBlur(QColor c, QRect rec, double p, int t)
{
    markType = BLUR;
    color = c;
    thickness = t;
    proportion = p;
    rect = rec;
}
