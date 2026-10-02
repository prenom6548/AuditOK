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

#include "search_msg_delegate.h"
#include "view/localinfo/localinfo.h"
#include "global/utils/globalutils.h"
#include "view/common/globalsizedata.h"
#include "global/utils/global_data.h"
#include "searchmsglist.h"
//#include "../../kyview.h"

#include <QPainterPath>

const QString LIGHT_CLICK_COLOR = "#EBEBEB";
const QString LIGHT_HOVER_COLOR = "#F5F5F5";
const QString BLACK_CLICK_COLOR = "#383838";
const QString BLACK_HOVER_COLOR = "#4D4D4D";

SearchMsgDelegate::SearchMsgDelegate() {}

SearchMsgDelegate::~SearchMsgDelegate() {}

bool SearchMsgDelegate::findHash(int friendId, int msgCount) const
{
    if (m_friendIDHash.contains(friendId)) {
        if (m_friendIDHash.value(friendId) == msgCount) {
            return true;
        }
    }
    return false;
}

QSize SearchMsgDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(350, 80);
}

void SearchMsgDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->setFont(GlobalData::getInstance()->m_fontName);
        painter->save();

        // 反锯齿
        painter->setRenderHint(QPainter::Antialiasing);

        int friendId = index.data(ChatMsgModel::FriendId).toInt();

        QStandardItem *friendItem = FriendListModel::getInstance()->getFriendById(friendId);
        if (!friendItem) {
            painter->restore();
            return;
        }

        // 设置好友数据
        QString username = friendItem->data(FriendListModel::Username).toString();
        QString nickname = friendItem->data(FriendListModel::Nickname).toString();
        QString avatar = friendItem->data(FriendListModel::Avatar).toString();
        int msgCount = index.data(ChatMsgModel::MsgCount).toInt();

        QString num = QString::number(msgCount) + tr(" relevant chat records");

        QString displayName = nickname;
        if (nickname.isEmpty()) {
            displayName = username;
        }

        // 整体item 矩形区域
        QRectF itemRect;
        itemRect.setX(option.rect.x());
        itemRect.setY(option.rect.y());
        itemRect.setWidth(option.rect.width() - 1);
        itemRect.setHeight(option.rect.height() - 1);

        QPainterPath path;
        // 鼠标悬停或者选中时改变背景色
        if (option.state.testFlag(QStyle::State_MouseOver)) {
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(itemRect, 0, 0);

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->fillPath(path, QBrush(QColor(LIGHT_HOVER_COLOR)));
            } else {
                painter->fillPath(path, QBrush(QColor(BLACK_HOVER_COLOR)));
            }
        } else if (option.state.testFlag(QStyle::State_HasFocus)) {
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(itemRect, 0, 0);

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->fillPath(path, QBrush(QColor(LIGHT_CLICK_COLOR)));
            } else {
                painter->fillPath(path, QBrush(QColor(BLACK_CLICK_COLOR)));
            }
        }
        
        QColor highLightColor = QApplication::palette().highlight().color();

        // 好友头像
        QPoint avatarPoint(itemRect.left() + 16, itemRect.top() + 15);
        QSize avatarSize(50, 50);
        QRectF avatarRect = QRect(avatarPoint, avatarSize);
        // 头像画圆
        QPainterPath avatarPath;
        avatarPath.setFillRule(Qt::WindingFill); //设置填充方式
        avatarPath.addRoundedRect(avatarRect, 30, 30);
        // painter->fillPath(avatarPath, QBrush(QColor("#3790FA")));
        painter->fillPath(avatarPath, QBrush(highLightColor));

        // 好友昵称或备注
        QPoint nicknamePoint(avatarRect.right() + 8, itemRect.top() + 6);
        QSize nicknameSize(150, 35);
        QRectF nicknameRect = QRect(nicknamePoint, nicknameSize);

        // 最近聊天内容
        QPoint contentPoint(avatarRect.right() + 8, nicknameRect.bottom());
        QSize contentSize(186, 50);
        QRectF contentRect = QRect(contentPoint, contentSize);

        //绘制文字
        QTextOption option;
        QFont textFont = painter->font();

        // 好友昵称或备注
        option.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
        textFont.setPointSizeF(GlobalData::getInstance()->m_font16pxToPt);

        QFontMetrics fontmtsName = QFontMetrics(textFont);
        int difName = fontmtsName.width(displayName) - nicknameRect.width();
        if (difName > 0) {
            displayName = fontmtsName.elidedText(displayName, Qt::ElideRight, nicknameRect.width());
        }
        painter->setFont(textFont);
        painter->drawText(nicknameRect, displayName, option);

        // 白色的字
        painter->setPen(QPen(Qt::white));
        option.setAlignment(Qt::AlignCenter);

        // 头像文字
        textFont.setPixelSize(22);
        painter->setFont(textFont);
        painter->drawText(avatarRect, avatar, option);

        // 灰色的字
        painter->setPen(QPen(QColor("#8C8C8C")));

        option.setAlignment(Qt::AlignLeft | Qt::AlignTop);
        textFont.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
        painter->setFont(textFont);

        // 聊天内容 过长时进行省略显示
        QFontMetrics fontmts = QFontMetrics(textFont);
        int dif = fontmts.width(num) - contentRect.width();
        if (dif > 0) {
            num = fontmts.elidedText(num, Qt::ElideRight, contentRect.width());
        }
        painter->drawText(contentRect, num, option);

        painter->restore();
    }

    return;
}
