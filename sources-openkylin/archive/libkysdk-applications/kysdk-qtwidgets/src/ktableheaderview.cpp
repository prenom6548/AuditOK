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

#include "ktableheaderview.h"
#include "themeController.h"
#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QEvent>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QToolButton>

namespace kdk
{

class Q_DECL_HIDDEN KTableHeaderViewPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTableHeaderView)
public:
    KTableHeaderViewPrivate(KTableHeaderView *PARENT);

    void changeTheme();

private:
    KTableHeaderView *q_ptr;
    QStringList m_HeaderList;
    bool m_bChecked;
    bool m_bTristate;
    bool m_bNoChange;
    bool m_bPressed;
    QMap<int, QMenu *> m_map;
};

KTableHeaderView::KTableHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
    , d_ptr(new KTableHeaderViewPrivate(this))

{
    Q_D(KTableHeaderView);
    // 设置Section可点击，若不设置则不能发出sectionClicked信号
    this->setSectionsClickable(true);
    connect(this, &KTableHeaderView::clickedHeader, this, [=](int column) {
        if (d->m_map.contains(column)) {
            d->m_map.value(column)->exec(QCursor::pos());
        }
    });
}

void KTableHeaderView::setTitle(QStringList list)
{
    Q_D(KTableHeaderView);
    d->m_HeaderList = list;
}

void KTableHeaderView::setMenu(QMenu *menu, int column)
{
    Q_D(KTableHeaderView);
    d->m_map.insert(column, menu);
}

void KTableHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    Q_D(const KTableHeaderView);
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if (logicalIndex == 0) {
        QStyleOptionButton option;

        option.initFrom(this);

        if (d->m_bTristate && d->m_bNoChange) {
            option.state |= QStyle::State_NoChange;
        } else
            option.state |= d->m_bChecked ? QStyle::State_On : QStyle::State_Off;

        option.iconSize = QSize(16, 16);
        option.rect = QRect(rect.x() + 8, rect.y() + (rect.height() - 16) / 2, 16, 16);
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter, this);

    } else {
        if (d->m_map.contains(logicalIndex)) {
            QPixmap pixmap = QIcon::fromTheme("ukui-down-symbolic").pixmap(16, 16);
            if (ThemeController::themeMode() == DarkTheme)
                pixmap = ThemeController::drawSymbolicColoredPixmap(pixmap);
            QRect iconRect(rect.right() - 24, rect.height() / 2 - 8, 16, 16);
            painter->drawPixmap(iconRect, pixmap);
        }
    }

    QRect textRect;
    if (logicalIndex > -1 || logicalIndex < d->m_HeaderList.size()) {
        if (logicalIndex == 0) {
            textRect = rect.adjusted(32, 0, 0, 0);
            if (fontMetrics().width(d->m_HeaderList.at(logicalIndex)) > textRect.width()) {
                QString str = fontMetrics().elidedText(d->m_HeaderList.at(logicalIndex), Qt::ElideRight, textRect.width());
                painter->drawText(textRect, Qt::AlignVCenter, str);
            } else
                painter->drawText(textRect, Qt::AlignVCenter, d->m_HeaderList.at(logicalIndex));
        } else {
            if (d->m_map.contains(logicalIndex))
                textRect = rect.adjusted(6, 0, -24, 0);
            else
                textRect = rect.adjusted(6, 0, -6, 0);
            if (fontMetrics().width(d->m_HeaderList.at(logicalIndex)) > textRect.width()) {
                QString str = fontMetrics().elidedText(d->m_HeaderList.at(logicalIndex), Qt::ElideRight, textRect.width());
                painter->drawText(textRect, Qt::AlignVCenter, str);
            } else
                painter->drawText(textRect, Qt::AlignVCenter, d->m_HeaderList.at(logicalIndex));
        }
    }
}

void KTableHeaderView::mousePressEvent(QMouseEvent *e)
{
    Q_D(KTableHeaderView);
    int nColumn = logicalIndexAt(e->pos());

    if ((e->buttons() & Qt::LeftButton)) {
        if (nColumn == 0)
            d->m_bPressed = true;
        else {
            // 每个标题栏的rect
            QRect tabRect(sectionPosition(nColumn), 0, sectionSize(nColumn), viewport()->rect().height());
            // 是否在图标区域被点击
            QRect sectionRect(tabRect.right() - 24, tabRect.y() + (tabRect.height() - 16) / 2, 16, 16);
            if (sectionRect.contains(e->pos()) && d->m_map.contains(nColumn))
                emit clickedHeader(nColumn);
            QHeaderView::mousePressEvent(e);
        }
    } else {
        QHeaderView::mousePressEvent(e);
    }
    viewport()->update();
}

void KTableHeaderView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(KTableHeaderView);
    if (d->m_bPressed) {
        if (d->m_bTristate && d->m_bNoChange) {
            d->m_bChecked = true;
            d->m_bNoChange = false;
        } else {
            d->m_bChecked = !d->m_bChecked;
        }
    } else {
        QHeaderView::mouseReleaseEvent(e);
    }
    this->viewport()->update();
    d->m_bPressed = false;
    if (d->m_bChecked)
        emit checkStateChange(Qt::Checked);
    else
        emit checkStateChange(Qt::Unchecked);
}

bool KTableHeaderView::event(QEvent *e)
{
    this->viewport()->update();
    return QHeaderView::event(e);
}

void KTableHeaderView::checkStateChangeSlot(int iState)
{
    Q_D(KTableHeaderView);
    if (iState == Qt::PartiallyChecked) {
        d->m_bTristate = true;
        d->m_bNoChange = true;
    } else {
        d->m_bNoChange = false;
    }

    d->m_bChecked = (iState != Qt::Unchecked);
    this->viewport()->update();
}

KTableHeaderViewPrivate::KTableHeaderViewPrivate(KTableHeaderView *parent)
    : q_ptr(parent)
    , m_bTristate(false)
    , m_bNoChange(false)
    , m_bChecked(false)
    , m_bPressed(false)
{
    setParent(parent);
}

void KTableHeaderViewPrivate::changeTheme()
{
    Q_Q(KTableHeaderView);
    initThemeStyle();
}

}

#include "ktableheaderview.moc"
#include "moc_ktableheaderview.cpp"
