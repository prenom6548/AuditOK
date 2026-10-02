/*
 *
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
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */
#include "kflowlayout.h"

#include <QStyle>
#include <QtMath>

namespace kdk
{

class KFlowLayoutPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KFlowLayout)
public:
    KFlowLayoutPrivate(KFlowLayout *parent);

private:
    KFlowLayout *q_ptr;
    QList<QLayoutItem *> m_itemList;
    int m_hSpace;
    int m_vSpace;
    bool m_home = false;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;
    int fillSpaceX(QWidget *wid) const;
};

KFlowLayoutPrivate::KFlowLayoutPrivate(KFlowLayout *parent)
    : q_ptr(parent)
{
    setParent(parent);
}

int KFlowLayoutPrivate::smartSpacing(QStyle::PixelMetric pm) const
{
    Q_Q(const KFlowLayout);
    QObject *parent = q->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, 0, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}

int KFlowLayoutPrivate::doLayout(const QRect &rect, bool testOnly) const
{
    Q_Q(const KFlowLayout);
    int left, top, right, bottom;
    q->getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    int fillX = 0;
    bool bFillX = false;

    QLayoutItem *item;
    foreach (item, m_itemList) {
        QWidget *wid = item->widget();
        int spaceX = q->horizontalSpacing();
        if (spaceX == -1) {
            if (!bFillX) {
                bFillX = true;
                fillX = fillSpaceX(wid);
            }
            spaceX = fillX;
        }

        int spaceY = q->verticalSpacing();
        if (spaceY == -1 && fillX >= 0) {
            spaceY = fillX;
        } /*else {
            spaceY = wid->style()->layoutSpacing(
                        QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        }*/
        if (m_home) {
            spaceY = 32;
        }

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int KFlowLayoutPrivate::fillSpaceX(QWidget *wid) const
{
    Q_Q(const KFlowLayout);
    int num = 0;
    int x = 0;
    int numH = 0;
    int space = 4;
    if (m_home) {
        space = 24;
    }
    int len = q->parentWidget()->width() - q->contentsMargins().left() - q->contentsMargins().right();
    while (true) {
        num++;
        if (num * (wid->width() + space) - space >= len) { // 最小间距space
            break;
        }
    }

    num = num - 1;
    if (num <= 1) {
        numH = m_itemList.size();
        return 32;
    }
    int height = wid->height();
    numH = ceil(double(m_itemList.size()) / num);
    x = len + space - num * (wid->width() + space);
    x = ceil(double(x) / (num - 1)) + space;
    x = x - 1; // 考虑边框等因素影响

    int maxY = numH * (height + x) + 32 - x;
    if (m_home) {
        maxY = numH * (height + 32);
        q->parentWidget()->parentWidget()->setFixedHeight(maxY);
    }
    q->parentWidget()->setFixedHeight(maxY);

    return x;
}

KFlowLayout::KFlowLayout(QWidget *parent, bool home, int margin, int hSpacing, int vSpacing)
    : QLayout(parent)
    , d_ptr(new KFlowLayoutPrivate(this))
{
    Q_D(KFlowLayout);
    d->m_hSpace = hSpacing;
    d->m_vSpace = vSpacing;
    d->m_home = home;
    setContentsMargins(margin, margin, margin, margin);
}

KFlowLayout::KFlowLayout(int margin, int hSpacing, int vSpacing)
    : d_ptr(new KFlowLayoutPrivate(this))
{
    Q_D(KFlowLayout);
    d->m_hSpace = hSpacing;
    d->m_vSpace = vSpacing;

    setContentsMargins(margin, margin, margin, margin);
}

KFlowLayout::~KFlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0))) {
    }
}

void KFlowLayout::addItem(QLayoutItem *item)
{
    Q_D(KFlowLayout);
    d->m_itemList.append(item);
}

int KFlowLayout::horizontalSpacing() const
{
    Q_D(const KFlowLayout);
    if (d->m_hSpace >= 0 || d->m_hSpace == -1) {
        return d->m_hSpace;
    } else {
        return d->smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int KFlowLayout::verticalSpacing() const
{
    Q_D(const KFlowLayout);
    if (d->m_vSpace >= 0 || d->m_vSpace == -1) {
        return d->m_vSpace;
    } else {
        return d->smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int KFlowLayout::count() const
{
    Q_D(const KFlowLayout);
    return d->m_itemList.size();
}

QLayoutItem *KFlowLayout::itemAt(int index) const
{
    Q_D(const KFlowLayout);
    return d->m_itemList.value(index);
}

QLayoutItem *KFlowLayout::takeAt(int index)
{
    Q_D(KFlowLayout);
    if (index >= 0 && index < d->m_itemList.size())
        return d->m_itemList.takeAt(index);
    else
        return 0;
}

Qt::Orientations KFlowLayout::expandingDirections() const
{
    Q_D(const KFlowLayout);
    return 0;
}

bool KFlowLayout::hasHeightForWidth() const
{
    Q_D(const KFlowLayout);
    return true;
}

int KFlowLayout::heightForWidth(int width) const
{
    Q_D(const KFlowLayout);
    int height = d->doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void KFlowLayout::setGeometry(const QRect &rect)
{
    Q_D(KFlowLayout);
    QLayout::setGeometry(rect);
    d->doLayout(rect, false);
}

QSize KFlowLayout::sizeHint() const
{
    Q_D(const KFlowLayout);
    return minimumSize();
}

QSize KFlowLayout::minimumSize() const
{
    Q_D(const KFlowLayout);
    QSize size;
    QLayoutItem *item;
    foreach (item, d->m_itemList)
        size = size.expandedTo(item->minimumSize());

    size += QSize(2 * margin(), 2 * margin());
    return size;
}

}

#include "kflowlayout.moc"
#include "moc_kflowlayout.cpp"
