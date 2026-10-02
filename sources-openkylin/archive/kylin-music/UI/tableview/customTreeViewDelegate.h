/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#ifndef CUSTOMTREEVIEWDELEGATE_H
#define CUSTOMTREEVIEWDELEGATE_H

#include <QObject>
#include <QItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QToolTip>
#include <QGSettings>

class CustomTreeViewDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CustomTreeViewDelegate(bool isHistory = false, QWidget *parent = 0);
    ~CustomTreeViewDelegate();

    void initGsettings();
    void setIsHistory(bool isHistory);
    void onHoverIndexChanged(const QModelIndex &index);
    void onHoverColorChanged(QString hoverColor);
    void onLeaveFromItemEvent();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

Q_SIGNALS:
    void open(const QModelIndex &index);
    void deleteData(const QModelIndex &index);
    void hoverIndexChanged(QModelIndex index);

private:
    bool m_isHistory = false;

    int m_nSpacing;  // 按钮之间的间距
    int m_nWidth;  // 按钮宽度
    int m_nHeight;  // 按钮高度
    int m_nType;  // 按钮状态-1：划过 2：按下
    int m_hoverrow;

    QScopedPointer<QPushButton> m_pOpenButton;
    QScopedPointer<QPushButton> m_pDeleteButton;
    QPoint m_mousePoint;  // 鼠标位置

    QGSettings *qtSettings = nullptr;
    QString m_hoverColor;
    QString clickColor;

    QStringList m_list;
};

#endif // CUSTOMTREEVIEWDELEGATE_H
