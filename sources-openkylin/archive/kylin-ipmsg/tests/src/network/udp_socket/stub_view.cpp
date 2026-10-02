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

#include "view/friendlist/friendlist.h"
#include "view/trayicon/trayiconwid.h"
#include "view/chatmsg/chatmsg.h"

// 提供缺失函数的空实现
FriendListView* FriendListView::getInstance(QListView* parent) { return nullptr; }
ChatMsg* FriendListView::getMsgWidById(int friendId) { return nullptr; }

bool ChatMsg::isScrollOnBottom() { return true; }
void ChatMsg::setScrollToBottom() {}
ChatMsgModel* ChatMsg::getchatMsgModel() { return nullptr; }

TrayIconWid* TrayIconWid::getInstance() { return nullptr; }
void TrayIconWid::newUnreadMsg() {}
