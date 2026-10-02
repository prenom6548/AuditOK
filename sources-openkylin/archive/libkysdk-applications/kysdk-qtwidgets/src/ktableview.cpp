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

#include "ktableview.h"
#include "ktableheaderview.h"
#include "themeController.h"
#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QToolButton>

namespace kdk
{

class Q_DECL_HIDDEN TableDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TableDelegate(QObject *parent);

    void updateState(Qt::CheckState state, const QModelIndex &index);

    QList<int> selectList();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

Q_SIGNALS:
    void checkChange(int state);

public slots:
    void checkChangeSlot(int state);
    void onHoverIndexChanged(QModelIndex index);
    //    void selectChange(int row);

private:
    QList<int> m_checkList;
    int m_hoverRow;
};

class Q_DECL_HIDDEN KTableViewPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTableView)
public:
    KTableViewPrivate(KTableView *parent);
    virtual ~KTableViewPrivate();

    void changeTheme();

private:
    KTableView *q_ptr;
    KTableHeaderView *m_pHeaderView;
    TableDelegate *m_pDelegate;
};

KTableView::KTableView(QWidget *parent)
    : QTableView(parent)
    , d_ptr(new KTableViewPrivate(this))
{
    Q_D(KTableView);
    setShowGrid(false);
    verticalHeader()->setVisible(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setMouseTracking(true);
    installEventFilter(this);

    d->m_pHeaderView = new KTableHeaderView(Qt::Horizontal, this);
    d->m_pHeaderView->installEventFilter(this);

    d->m_pDelegate = new TableDelegate(this);
    setItemDelegate(d->m_pDelegate);

    connect(d->m_pDelegate, &TableDelegate::checkChange, d->m_pHeaderView, [=](int state) {
        d->m_pHeaderView->checkStateChangeSlot(state);
    });
    connect(d->m_pHeaderView, &KTableHeaderView::checkStateChange, d->m_pDelegate, [=](int state) {
        d->m_pDelegate->checkChangeSlot(state);
        QItemSelectionModel *SelectionModel = selectionModel();
        if (SelectionModel) {
            SelectionModel->clearSelection();
        }
        emit hoverIndexChanged(QModelIndex());
    });

    connect(this, SIGNAL(hoverIndexChanged(QModelIndex)), d->m_pDelegate, SLOT(onHoverIndexChanged(QModelIndex)));
    connect(d->m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            d->changeTheme();
        }
    });
}

void KTableView::setHorizontalTitle(QStringList list)
{
    Q_D(KTableView);
    d->m_pHeaderView->setTitle(list);
    setHorizontalHeader(d->m_pHeaderView);
}

KTableHeaderView *KTableView::headerView()
{
    Q_D(KTableView);
    if (d->m_pHeaderView)
        return d->m_pHeaderView;
}

QList<int> KTableView::selectList()
{
    Q_D(KTableView);
    return d->m_pDelegate->selectList();
}

void KTableView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    emit hoverIndexChanged(index);
    QTableView::mouseMoveEvent(event);
    this->viewport()->update();
}

bool KTableView::eventFilter(QObject *object, QEvent *event)
{
    Q_D(KTableView);
    if (object == this) {
        switch (event->type()) {
        case QEvent::Leave:
            emit hoverIndexChanged(QModelIndex());
            break;
        default:
            break;
        }
    }
    if (object == d->m_pHeaderView) {
        switch (event->type()) {
        case QEvent::Enter:
            emit hoverIndexChanged(QModelIndex());
            break;
        default:
            break;
        }
    }
    this->viewport()->update();
    return QTableView::eventFilter(object, event);
}

KTableViewPrivate::KTableViewPrivate(KTableView *parent)
    : q_ptr(parent)
{
    setParent(parent);
}

KTableViewPrivate::~KTableViewPrivate()
{
}

void KTableViewPrivate::changeTheme()
{
    initThemeStyle();
}

TableDelegate::TableDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_hoverRow = -1;
}

void TableDelegate::updateState(Qt::CheckState state, const QModelIndex &index)
{
    if (state == Qt::Checked) {
        m_checkList.append(index.row());
    } else if (state == Qt::Unchecked) {

        m_checkList.removeOne(index.row());
    }
    QAbstractItemView *view = qobject_cast<QAbstractItemView *>(parent());
    int count = view->model()->rowCount();

    if (m_checkList.count() == 0)
        emit checkChange(Qt::Unchecked);
    else if (m_checkList.count() == count)
        emit checkChange(Qt::Checked);
    else
        emit checkChange(Qt::PartiallyChecked);
}

void TableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QPalette pale;
    QColor bkgColor;
    QColor hoverColor;

    if (ThemeController::themeMode() == LightTheme) {
        if (index.row() % 2 == 0)
            bkgColor = QColor(245, 245, 245);
        else
            bkgColor = ThemeController::getCustomColorFromDT("kwhite");
        hoverColor = ThemeController::getCustomColorFromDT("kblack");
        hoverColor.setAlphaF(0.05);
    } else {
        if (index.row() % 2 == 0)
            bkgColor = ThemeController::getCustomColorFromDT("kgray-17");
        else
            bkgColor = QColor(18, 18, 18);
        hoverColor = ThemeController::getCustomColorFromDT("kwhite");
        hoverColor.setAlphaF(0.10);
    }
    if (index.row() == m_hoverRow && !option.state.testFlag(QStyle::State_Selected)) {
        painter->fillRect(option.rect, hoverColor);
    } else {
        if (option.state.testFlag(QStyle::State_Selected))
            painter->fillRect(option.rect, ThemeController::getPaletteColorFromDT("highlight-active"));
        else {
            painter->fillRect(option.rect, bkgColor);
        }
        painter->setPen(Qt::NoPen);
        painter->drawRect(option.rect);
    }

    painter->setPen(ThemeController::getPaletteColorFromDT("windowtext-active"));
    if (index.column() == 0) {
        QRect rect = option.rect;
        // 绘制复选框
        QStyleOptionButton checkBoxStyle;
        checkBoxStyle.rect = QRect(rect.x() + 8, rect.y() + (rect.height() - 16) / 2, 16, 16);

        Qt::CheckState checkState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
        checkBoxStyle.state = (checkState == Qt::Checked) ? QStyle::State_On : QStyle::State_Off;
        checkBoxStyle.state |= QStyle::State_Enabled;

        QApplication::style()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &checkBoxStyle, painter, option.widget);

        painter->drawText(option.rect.adjusted(checkBoxStyle.rect.right() + 10, 0, 0, 0), index.data(Qt::DisplayRole).toString());
    } else {
        painter->drawText(option.rect, index.data(Qt::DisplayRole).toString());
    }
}

bool TableDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if ((event->type() == QEvent::MouseButtonRelease) && (index.column() == 0)) {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        if (mouse_event->button() == Qt::LeftButton) {
            QRect r(option.rect.left() + 8, option.rect.y() + (option.rect.height() - 16) / 2, 16, 16);
            bool flag = r.contains(mouse_event->pos());
            if (flag) {
                // 根据当前单元格的选中状态来在 选中/未选中 之间切换
                QVariant value = index.data(Qt::CheckStateRole);
                Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
                                            ? Qt::Unchecked
                                            : Qt::Checked);

                // 设置当前单元格的选中状态
                bool checkState = model->setData(index, state, Qt::CheckStateRole);
                updateState(state, index);
                return checkState;
            }
        }
        return false;
    } else
        return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QList<int> TableDelegate::selectList()
{
    return m_checkList;
}

void TableDelegate::checkChangeSlot(int state)
{
    m_checkList.clear();
    QAbstractItemView *view = qobject_cast<QAbstractItemView *>(parent());
    int count = view->model()->rowCount();
    Qt::CheckState checkState;

    for (int i = 0; i < count; i++) {
        if (state == 0) {
            m_checkList.append(view->model()->index(i, 0).row());
            checkState = Qt::Unchecked;
        } else {
            m_checkList.removeOne(view->model()->index(i, 0).row());
            checkState = Qt::Checked;
        }
        // 设置当前单元格的选中状态
        view->model()->setData(view->model()->index(i, 0), checkState, Qt::CheckStateRole);
        updateState(checkState, view->model()->index(i, 0));
    }
}

void TableDelegate::onHoverIndexChanged(QModelIndex index)
{
    if (index.isValid()) {
        m_hoverRow = index.row();
    } else {
        m_hoverRow = -1;
    }
}

}

#include "ktableview.moc"
#include "moc_ktableview.cpp"
