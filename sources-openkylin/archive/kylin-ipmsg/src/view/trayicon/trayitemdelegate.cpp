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

#include <QPainterPath>

#include "trayitemdelegate.h"
#include "model/friendlistmodel.h"
#include "view/common/globalsizedata.h"

TrayItemDelegate::TrayItemDelegate(QAbstractItemDelegate *parent) : QAbstractItemDelegate(parent) {}

TrayItemDelegate::~TrayItemDelegate() {}

QSize TrayItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(280, 58);
}

void TrayItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();

        // 反锯齿
        painter->setRenderHint(QPainter::Antialiasing);

        // 获取数据
        QString avatar = index.data(FriendListModel::Avatar).toString();
        QString username = index.data(FriendListModel::Username).toString();
        QString nickname = index.data(FriendListModel::Nickname).toString();
        QString msgContent = index.data(FriendListModel::RecentMsgContent).toString();
        QString msgTime = index.data(FriendListModel::RecentMsgTime).toString();
        int msgNum = index.data(FriendListModel::UnreadMsgNum).toInt();
        int onlineState = index.data(FriendListModel::OnlineState).toInt();
        // int priority       = index.data(FriendListModel::Priority).toInt();

        // 好友昵称或备注
        QString displayName = nickname;
        if (nickname.isEmpty()) {
            displayName = username;
        }

        // 定义深浅模式下的背景色
        QString lightBgColor = "#EBEBEB";
        QString blackBgColor = "#383838";

        // 整体item 矩形区域
        QRectF itemRect;
        itemRect.setX(option.rect.x());
        itemRect.setY(option.rect.y());
        itemRect.setWidth(option.rect.width() - 1);
        itemRect.setHeight(option.rect.height() - 1);

        QPainterPath path;
        // 鼠标悬停或者选中时改变背景色
        if (option.state.testFlag(QStyle::State_MouseOver)) {
            // painter->setPen(QPen(QColor("#ebeced")));
            path.setFillRule(Qt::WindingFill); //设置填充方式
            path.addRoundedRect(itemRect, 0, 0);

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->fillPath(path, QBrush(QColor(lightBgColor)));
            } else {
                painter->fillPath(path, QBrush(QColor(blackBgColor)));
            }
        }

        // 好友头像
        QPoint avatarPoint(itemRect.left() + 16, itemRect.top() + 10);
        QSize avatarSize(38, 38);
        QRectF avatarRect = QRect(avatarPoint, avatarSize);
        // 头像画圆
        QPainterPath avatarPath;
        avatarPath.setFillRule(Qt::WindingFill);
        avatarPath.addRoundedRect(avatarRect, avatarSize.width() / 2, avatarSize.height() / 2);

        if (onlineState == FriendListModel::OnlineType::Offline) {
            painter->fillPath(avatarPath, QBrush(QColor("#A6A6A6")));
        } else {
            painter->fillPath(avatarPath, QBrush(QColor("#3790FA")));
        }

        // 好友昵称或备注
        QPoint nicknamePoint(avatarRect.right() + 8, itemRect.top() + 19);
        QSize nicknameSize(160, 20);
        QRectF nicknameRect = QRect(nicknamePoint, nicknameSize);

        // 未读消息数
        QPoint msgNumPoint(nicknameRect.right() + 16, itemRect.top() + 21);
        QSize msgNumSize(26, 16);
        QRectF msgNumRect = QRect(msgNumPoint, msgNumSize);
        // msgNumRect.setRight(itemRect.right() - 20);
        if (msgNum != 0) {
            // 消息数画圆
            QPainterPath numPath;
            numPath.setFillRule(Qt::WindingFill); //设置填充方式
            numPath.addRoundedRect(msgNumRect, 10, 10);
            painter->fillPath(numPath, QBrush(QColor(250, 55, 55)));
        }

        //绘制文字
        QTextOption option;
        QFont textFont = painter->font();

        // 好友昵称或备注
        option.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        textFont.setPixelSize(16);
        painter->setFont(textFont);

        // 好友昵称过长时省略显示
        QFontMetrics fontmts = QFontMetrics(textFont);
        int dif = fontmts.width(displayName) - nicknameRect.width();
        if (dif > 0) {
            displayName = fontmts.elidedText(displayName, Qt::ElideRight, nicknameRect.width());
        }
        painter->drawText(nicknameRect, displayName, option);

        // 白色的字
        painter->setPen(QPen(Qt::white));
        option.setAlignment(Qt::AlignCenter);

        // 头像文字
        textFont.setPixelSize(18);
        painter->setFont(textFont);
        painter->drawText(avatarRect, avatar, option);

        // 消息数文字 判断消息数
        if (msgNum != 0) {
            textFont.setPixelSize(12);
            painter->setFont(textFont);

            if (msgNum < 100) {
                painter->drawText(msgNumRect, QString::number(msgNum), option);
            } else {
                painter->drawText(msgNumRect, QString("…"), option);
            }
        }

        painter->restore();
    }
}