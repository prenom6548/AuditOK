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

#include "kywebview.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QAction>
#include <QMenu>

KyWebView::KyWebView(QWidget *parent)
    : QWebView(parent)
{
}

void KyWebView::dragEnterEvent(QDragEnterEvent *event)
{
    event->ignore();
}

void KyWebView::dropEvent(QDropEvent *event)
{
    event->ignore();
}

void KyWebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);

    QAction *copyAction = new QAction(QIcon::fromTheme("edit-copy-symbolic"), tr("Copy")+"(C)", this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, [this]() { this->page()->triggerAction(QWebPage::Copy); });
    menu->addAction(copyAction);
    menu->addSeparator(); // 添加分隔线

    QAction *copyAllAction = new QAction(tr("Select All"), this);
    copyAllAction->setShortcut(QKeySequence::SelectAll);
    connect(copyAllAction, &QAction::triggered, [this]() { this->page()->triggerAction(QWebPage::SelectAll); this->page()->triggerAction(QWebPage::Copy); });
    menu->addAction(copyAllAction);

    // 设置菜单样式
    menu->setStyleSheet(" border: 1px solid lightgray;padding: 5px;");
    menu->exec(event->globalPos());
    delete menu;
}
