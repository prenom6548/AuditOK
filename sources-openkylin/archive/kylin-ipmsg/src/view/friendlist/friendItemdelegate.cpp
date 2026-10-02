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

#include "friendItemdelegate.h"
#include "global/utils/globalutils.h"
#include "global/utils/globaldateformat.h"
#include "view/common/globalsizedata.h"
#include "global/utils/global_data.h"
#include "friendlist.h"
#include "../kyview.h"
#include "libkydate.h"

#include <QPainterPath>

FriendItemDelegate::FriendItemDelegate() {}

FriendItemDelegate::~FriendItemDelegate() {}

QSize FriendItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(350, 80);
}

void FriendItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
        QString msgContent = index.data(FriendListModel::RecentMsgContent).toString();
        QString msgTime = index.data(FriendListModel::RecentMsgTime).toString();
        int msgNum = index.data(FriendListModel::UnreadMsgNum).toInt();
        int onlineState = index.data(FriendListModel::OnlineState).toInt();
        int priority = index.data(FriendListModel::Priority).toInt();

        // 时间的显示形式跟随ukui-control-center
        QString displayTime = "";
        QDateTime messageTime;
        messageTime = QDateTime::fromString(msgTime, GlobalUtils::getTimeFormat());
        displayTime = GlobalDateFormat::getInstance()->TranslateDateFormat(messageTime);
        //qDebug() << "( FriendItemDelegate::paint ) 时间字符串： " << msgTime << " 转化为时间为：" << messageTime << "转化为字符串为:" << displayTime;

        if (displayTime == "-1:-1"){
            displayTime = " ";
        }

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
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(itemRect, 0, 0);

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->fillPath(path, QBrush(QColor("#F5F5F5")));
            } else {
                painter->fillPath(path, QBrush(QColor("#2D2D2D")));
            }
        } else if (option.state.testFlag(QStyle::State_HasFocus)) {
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(itemRect, 0, 0);

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->fillPath(path, QBrush(QColor(lightBgColor)));
            } else {
                painter->fillPath(path, QBrush(QColor(blackBgColor)));
            }
        }

        // 置顶标识
        QSize stayTopSize(4, 18);
        QPoint stayTopPoint(itemRect.left(), itemRect.top() + 30);
        QRect stayTopRect = QRect(stayTopPoint, stayTopSize);

        // 好友头像
        QPoint avatarPoint(itemRect.left() + 16, itemRect.top() + 15);
        QSize avatarSize(50, 50);
        QRectF avatarRect = QRect(avatarPoint, avatarSize);
        // 头像画圆
        QPainterPath avatarPath;
        avatarPath.setFillRule(Qt::WindingFill); //设置填充方式
        avatarPath.addRoundedRect(avatarRect, 30, 30);

        QColor highLightColor = QApplication::palette().highlight().color();

        if (onlineState == FriendListModel::OnlineType::Offline) {
            painter->fillPath(avatarPath, QBrush(QColor("#A6A6A6")));
        } else {
            // painter->fillPath(avatarPath, QBrush(QColor("#3790FA")));
            painter->fillPath(avatarPath, QBrush(highLightColor));
        }

        // 好友昵称或备注
        QPoint nicknamePoint(avatarRect.right() + 8, itemRect.top() + 11);
        QSize nicknameSize(150, 30);
        QRectF nicknameRect = QRect(nicknamePoint, nicknameSize);

        // 最近聊天内容
        // if (msgContent != "")
        // QPoint contentPoint(avatarRect.right() + 8, itemRect.bottom() - 34);
        QPoint contentPoint(avatarRect.right() + 8, nicknameRect.bottom() - 2);
        // QSize contentSize(186, 20);
        QSize contentSize(option.rect.width() - 170, 30);
        QRectF contentRect = QRect(contentPoint, contentSize);
        // contentRect.setBottom(itemRect.bottom() - 16);

        // 最近聊天时间
        // if (msgNum != "0")
        QPoint timePoint(itemRect.right() - 130, itemRect.top() + 20);
        QSize timeSize(100, 30);
        QRectF timeRect = QRect(timePoint, timeSize);
        timeRect.setRight(itemRect.right() - 20);

        // 未读消息数
        QPoint msgNumPoint(itemRect.right() - 50, timeRect.bottom() - 5);
        // QPoint msgNumPoint(itemRect.right() - 50, itemRect.bottom() - 30);
        QSize msgNumSize(30, 20);
        QRectF msgNumRect = QRect(msgNumPoint, msgNumSize);
        msgNumRect.setRight(itemRect.right() - 20);
        if (msgNum != 0) {
            // 消息数画圆
            QPainterPath numPath;
            numPath.setFillRule(Qt::WindingFill); //设置填充方式
            numPath.addRoundedRect(msgNumRect, 10, 10);
            painter->fillPath(numPath, QBrush(QColor(250, 55, 55)));
        }

        // 绘制图片
        if (priority == FriendListModel::PriorityType::PriStayTop) {
            QPixmap stayTopPixmap = changeHighLightColor(highLightColor, ":/res/staytop.svg");
            // QPixmap stayTopPixmap = QPixmap(QIcon(":/res/staytop.svg").pixmap(stayTopRect.size()));
            painter->drawPixmap(stayTopRect, stayTopPixmap);
        }

        //绘制文字
        QTextOption option;
        QFont textFont = painter->font();

        // 好友昵称或备注
        // option.setAlignment(Qt::AlignLeft | Qt::AlignTop);
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

        // 消息数文字 判断消息数
        if (msgNum != 0) {
            textFont.setPixelSize(12);
            painter->setFont(textFont);

            if (msgNum < 100) {
                painter->drawText(msgNumRect, QString::number(msgNum), option);
            } else {
                painter->drawText(msgNumRect, QString("99+"), option);
            }
        }

        // 灰色的字
        painter->setPen(QPen(QColor("#8C8C8C")));

        option.setAlignment(Qt::AlignLeft | Qt::AlignTop);
        textFont.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
        painter->setFont(textFont);

        // 聊天内容 过长时进行省略显示
        QStringList res = msgContent.split('\n');
        msgContent = res.at(0);
        QFontMetrics fontmts = QFontMetrics(textFont);
        int dif = fontmts.width(msgContent) - contentRect.width();
        if (dif > 0) {
            msgContent = fontmts.elidedText(msgContent, Qt::ElideRight, contentRect.width());
        }
        painter->drawText(contentRect, msgContent, option);

        // 聊天时间
        option.setAlignment(Qt::AlignRight | Qt::AlignTop);
        painter->setFont(QFont(painter->fontInfo().family(), GlobalData::getInstance()->m_font12pxToPt));
        painter->drawText(timeRect, displayTime, option);

        painter->restore();
    }
}

QPixmap FriendItemDelegate::changeHighLightColor(QColor color, QString path) const
{
    QImage image(path);

    QColor rgb(color);
    QColor trans(qRgba(0,0,0,1));
    int whiteColorKey = 255 - 50;
    for (int j = 0; j < image.height(); ++j) {
        for (int i = 0; i < image.width(); ++i) {
            QColor color(image.pixel(i, j));
            //不处理透明像素点,不处理白色
            if (color == trans || (color.red() > whiteColorKey 
                                  && color.green()>whiteColorKey 
                                  && color.blue() > whiteColorKey)) {
                continue;
            }
            image.setPixel(i, j, rgb.rgb());
        }
    }
    return QPixmap::fromImage(image);
}
