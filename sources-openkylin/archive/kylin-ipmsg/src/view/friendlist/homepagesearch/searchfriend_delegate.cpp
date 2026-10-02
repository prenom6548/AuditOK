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

#include "view/localinfo/localinfo.h"
#include "global/utils/globalutils.h"
#include "view/common/globalsizedata.h"
#include "searchfriend_delegate.h"
#include "../../kyview.h"

#include <QPainterPath>

const QString LIGHT_CLICK_COLOR = "#EBEBEB";
const QString LIGHT_HOVER_COLOR = "#F5F5F5";
const QString BLACK_CLICK_COLOR = "#383838";
const QString BLACK_HOVER_COLOR = "#4D4D4D";

SearchFriendDelegate::SearchFriendDelegate() {}

SearchFriendDelegate::~SearchFriendDelegate() {}

QSize SearchFriendDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(350, 80);
}

void SearchFriendDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->setFont(GlobalData::getInstance()->m_fontName);
        painter->save();

        // 反锯齿
        painter->setRenderHint(QPainter::Antialiasing);

        // 获取数据
        QString avatar = index.data(FriendListModel::Avatar).toString();
        QString username = index.data(FriendListModel::Username).toString();
        QString nickname = index.data(FriendListModel::Nickname).toString();
        //        int onlineState    = index.data(FriendListModel::OnlineState).toInt();
        //        int priority       = index.data(FriendListModel::Priority).toInt();

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
        QPoint nicknamePoint(avatarRect.right() + 8, itemRect.top() + 27);
        QSize nicknameSize(150, 35);
        QRectF nicknameRect = QRect(nicknamePoint, nicknameSize);

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

        /* 设置关键词高亮 */
        QString searchText = LocalInfo::getInstance()->m_searchLineEdit->text();

        QTextDocument document;

        /* 设置文字边距，保证绘制文字居中 */
        document.setDocumentMargin(0);
        document.setPlainText(displayName);
        bool found = false;
        QTextCursor highlight_cursor(&document);
        QTextCursor cursor(&document);

        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        // color_format.setForeground(QColor("#3790FA")); /* 设置高亮颜色 */
        color_format.setForeground(highLightColor); /* 设置高亮颜色 */

        /* 搜索字体高亮 */
        QTextCursor testcursor(&document);
        testcursor.select(QTextCursor::LineUnderCursor);
        QTextCharFormat fmt;
        //        fmt.setForeground(QColor("#262626"));           /* 原文本颜色 */
        fmt.setFont(textFont);
        testcursor.mergeCharFormat(fmt);
        testcursor.clearSelection();
        testcursor.movePosition(QTextCursor::EndOfLine);

        while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
            highlight_cursor = document.find(searchText, highlight_cursor);
            if (!highlight_cursor.isNull()) {
                if (!found) {
                    found = true;
                }
                highlight_cursor.mergeCharFormat(color_format);
            }
        }
        cursor.endEditBlock();

        QAbstractTextDocumentLayout::PaintContext paintContext;
        painter->save();
        painter->translate(nicknameRect.topLeft());
        painter->setClipRect(nicknameRect.translated(-nicknameRect.topLeft()));
        document.documentLayout()->draw(painter, paintContext);
        painter->restore();

        // 白色的字
        painter->setPen(QPen(Qt::white));
        option.setAlignment(Qt::AlignCenter);

        // 头像文字
        textFont.setPixelSize(22);
        painter->setFont(textFont);
        painter->drawText(avatarRect, avatar, option);

        painter->restore();
    }
}
