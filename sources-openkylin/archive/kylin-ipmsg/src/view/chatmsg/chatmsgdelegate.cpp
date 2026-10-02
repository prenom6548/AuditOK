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

#include "chatmsgdelegate.h"
#include "view/localinfo/localinfo.h"
#include "view/common/globalsizedata.h"
#include "global/utils/global_data.h"
#include <peony-qt/file-utils.h>
#include "global/utils/globalutils.h"
#include "global/utils/globaldateformat.h"
#include "libkydate.h"

#include <QPainterPath>

int ChatMsgDelegate::itemWidth = 400;
int ChatMsgDelegate::itemHeight = 100;

QPoint ChatMsgDelegate::borderPoint = QPoint(0, 0);
QSize ChatMsgDelegate::borderSize = QSize(0, 0);

ChatMsgDelegate::ChatMsgDelegate() {}

ChatMsgDelegate::~ChatMsgDelegate() {}

QSize ChatMsgDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    QString msgContent = index.data(ChatMsgModel::MsgContent).toString();
    int msgType = index.data(ChatMsgModel::MsgType).toInt();
    QSize resSize = QSize(itemWidth, itemHeight);

    if (msgType == ChatMsgModel::MessageType::TextMsg) {
        QFont font = GlobalData::getInstance()->getFontSize14px();
        // QSize msgTextSize = ChatMsgDelegate::getMsgRectSize(msgContent, font, 294);
        int w;
        if (GlobalData::getInstance()->m_currentMode == CurrentMode::PCMode) {
            w = 294;
        } else {
            w = option.rect.width() * 0.42;
        }
        QSize msgTextSize = ChatMsgDelegate::getMsgRectSize(msgContent, font, w);
        resSize.setWidth(msgTextSize.width());
        resSize.setHeight(msgTextSize.height() + 20);

    } else if (msgType == ChatMsgModel::MessageType::TimeMsg) {
        resSize.setHeight(25);
    } else {
        resSize.setHeight(78);
    }
    // qDebug() << "sizeHint" << resSize.width() << resSize.height();
    return resSize;
}

void ChatMsgDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->setFont(GlobalData::getInstance()->m_fontName);
        painter->save();

        // 反锯齿
        painter->setRenderHint(QPainter::Antialiasing);

        // 绘制时间item
        if (index.data(ChatMsgModel::MsgType).toInt() == ChatMsgModel::MessageType::TimeMsg) {
            // qDebug() << "MessageType::TimeMsg";

            // 获取数据
            QString messageTimeStr = index.data(ChatMsgModel::MsgTime).toString().replace("T", " ");

            // qDebug() << "messageTimeStr" << messageTimeStr;

            // 定义需要使用的Rect
            QRectF itemRect;
            QRectF timeRect;

            // 初始化字体、对齐方式、背景色
            QFont textFont = painter->font();
            QTextOption textOption;
            QColor bgBlueCol(55, 144, 250);

            // 时间的显示形式跟随ukui-control-center
            QDateTime messageTime;
            messageTime = QDateTime::fromString(messageTimeStr, GlobalUtils::getTimeFormat());
            messageTimeStr = GlobalDateFormat::getInstance()->TranslateDateFormat(messageTime);
            //qDebug() << "( ChatMsgDelegate::paint ) 时间字符串： " << messageTimeStr << " 转化为时间为：" << messageTime;

            // 整体item 矩形区域
            itemRect.setX(option.rect.x());
            itemRect.setY(option.rect.y());
            itemRect.setWidth(option.rect.width() - 1);
            itemRect.setHeight(option.rect.height() - 1);

            // test : 整体填充颜色
            // QPainterPath itemPath;
            // itemPath.setFillRule(Qt::WindingFill);
            // itemPath.addRoundedRect(itemRect, 0, 0);
            // painter->fillPath(itemPath, QBrush(Qt::red));

            // 时间矩形
            QSize timeSize(itemRect.width() - 1, itemRect.height() - 1);
            QPoint timePoint(itemRect.left(), itemRect.top());
            timeRect = QRect(timePoint, timeSize);

            // qDebug() << "display messageTimeStr" << messageTimeStr;

            // 时间文字
            textFont.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
            textOption.setAlignment(Qt::AlignCenter);
            painter->setPen(QPen(Qt::gray));
            painter->setFont(textFont);
            painter->drawText(timeRect, messageTimeStr, textOption);

            painter->restore();

            return;
        }

        // 获取数据
        QString msgTime = index.data(ChatMsgModel::MsgTime).toString();
        QString msgContent = index.data(ChatMsgModel::MsgContent).toString();
        QString filePath = index.data(ChatMsgModel::FilePath).toString();
        // int msgId          = index.data(ChatMsgModel::MsgId).toInt();
        int friendId = index.data(ChatMsgModel::FriendId).toInt();
        int isSend = index.data(ChatMsgModel::IsSend).toInt();
        int msgType = index.data(ChatMsgModel::MsgType).toInt();
        int sendState = index.data(ChatMsgModel::SendState).toInt();
        // int readState      = index.data(ChatMsgModel::ReadState).toInt();
        qint64 totalSize = index.data(ChatMsgModel::TotalSize).toLongLong();
        qint64 transferSize = index.data(ChatMsgModel::TransferSize).toLongLong();

        // qDebug() << friendId << msgTime << msgContent << filePath;

        // 定义需要使用的Rect
        QRectF itemRect;
        QRectF avatarRect;
        QRectF avatarRightRect;
        QRectF triangleRect;
        QRectF triangleRightRect;
        QRectF msgBorderRect;
        QRectF msgBorderRRect;
        QRectF msgContentRect;
        QRectF msgRightRect;
        QRect fileIconRect;
        QRectF fileNameRect;
        QRectF fileSizeRect;
        QRect msgStateRect;

        QPixmap avatarLeftPixmap;
        QPixmap avatarRightPixmap;
        QPixmap fileIconPixmap;
        QPixmap msgStatePixmap;

        // 初始化字体、对齐方式、背景色
        QFont textFont = painter->font();
        QTextOption textOption;

        QColor bgBlueCol(55, 144, 250);
        QColor highLightColor = QApplication::palette().highlight().color();
        QColor bgWhiteCol(255, 255, 255);
        QColor bgBlackCol(31, 32, 34);

        QColor textWhiteCol(217, 217, 217);
        QColor textBlackCol(38, 38, 38);

        // 整体item 矩形区域
        itemRect.setX(option.rect.x());
        itemRect.setY(option.rect.y());
        itemRect.setWidth(option.rect.width() - 1);
        itemRect.setHeight(option.rect.height() - 1);

        // test : 整体填充颜色
        // QPainterPath itemPath;
        // itemPath.setFillRule(Qt::WindingFill);
        // itemPath.addRoundedRect(itemRect, 0, 0);
        // painter->fillPath(itemPath, QBrush(Qt::red));

        /********** 画矩形 默认是接收的消息 **********/
        if (isSend == ChatMsgModel::SendType::RecvMsg) { // 接收的消息

            // if (friendAvatar == "") {
            QStandardItem *item = FriendListModel::getInstance()->getFriendById(friendId);
            QString friendAvatar = item->data(FriendListModel::Avatar).toString();
            QString friendNickname = item->data(FriendListModel::Nickname).toString();

            // 头像矩形
            QPoint avatarPoint(itemRect.left() + 8, itemRect.top() + 1);
            QSize avatarSize(38, 38);
            avatarRect = QRect(avatarPoint, avatarSize);

            // 三角箭头
            QPoint trianglePoint(avatarRect.right() + 8, avatarRect.top() + 15);
            QSize triangleSize(6, 8);
            triangleRect = QRect(trianglePoint, triangleSize);

            // 消息外框矩形
            QPoint borderPoint(triangleRect.right(), itemRect.top() + 1);
            QSize borderSize(228, 76);
            msgBorderRect = QRect(borderPoint, borderSize);

            // 头像圆角 填充颜色
            QPainterPath avatarPath;
            avatarPath.setFillRule(Qt::WindingFill);
            avatarPath.addRoundedRect(avatarRect, 19, 19);
            // painter->fillPath(avatarPath, QBrush(bgBlueCol));
            painter->fillPath(avatarPath, QBrush(highLightColor));

            // 三角箭头
            QPointF points[3] = {
                QPointF(triangleRect.x(), triangleRect.y() + 4),
                QPointF(triangleRect.x() + triangleRect.width(), triangleRect.y()),
                QPointF(triangleRect.x() + triangleRect.width(), triangleRect.y() + 8),
            };

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->setPen(QPen(bgWhiteCol));
                painter->setBrush(bgWhiteCol);
            } else {
                painter->setPen(QPen(bgBlackCol));
                painter->setBrush(bgBlackCol);
            }

            painter->drawPolygon(points, 3);

            // 消息外框加圆角
            // QPainterPath borderPath;
            // borderPath.setFillRule(Qt::WindingFill);
            // borderPath.addRoundedRect(msgBorderRect, 6, 6);
            // painter->fillPath(borderPath, QBrush(bgBlueCol));

            // 头像文字
            painter->setPen(QPen(Qt::white));
            textOption.setAlignment(Qt::AlignCenter);
            textFont.setPixelSize(18);
            painter->setFont(textFont);
            painter->drawText(avatarRect, friendAvatar, textOption);

            textFont.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            textOption.setWrapMode(QTextOption::WrapAnywhere);
            painter->setPen(QPen(Qt::white));
            painter->setFont(textFont);
            painter->drawText(msgContentRect, msgContent, textOption);

        } else if (isSend == ChatMsgModel::SendType::SendMsg) { // 发送的消息

            QString mineAvatar = LocalInfo::getInstance()->getLocalAvatar();

            /********** 调整矩形的位置 **********/

            // 头像矩形
            QSize avatarSize(38, 38);
            QPoint avatarPoint(itemRect.right() - avatarSize.width() - 5, itemRect.top() + 1);
            avatarRect = QRect(avatarPoint, avatarSize);

            // 三角箭头
            QSize triangleSize(6, 8);
            QPoint trianglePoint(avatarRect.left() - 8 - triangleSize.width(), avatarRect.top() + 15);
            triangleRect = QRect(trianglePoint, triangleSize);

            // 消息外框矩形
            QSize borderSize(228, 76);
            QPoint borderPoint(triangleRect.left() - borderSize.width(), itemRect.top() + 1);
            msgBorderRect = QRect(borderPoint, borderSize);

            // 发送状态矩形
            QSize msgStateSize(16, 16);
            QPoint msgStatePoint(msgBorderRect.left() - msgStateSize.width() - 8,
                                 msgBorderRect.top() + (borderSize.height() - msgStateSize.height()) / 2);
            msgStateRect = QRect(msgStatePoint, msgStateSize);

            // 头像矩形
            QPainterPath avatarPath;
            avatarPath.setFillRule(Qt::WindingFill);
            avatarPath.addRoundedRect(avatarRect, 19, 19);
            // painter->fillPath(avatarPath, QBrush(bgBlueCol));
            painter->fillPath(avatarPath, QBrush(highLightColor));

            // 三角箭头矩形
            QPointF points[3] = {
                QPointF(triangleRect.x() + triangleRect.width(), triangleRect.y() + 4),
                QPointF(triangleRect.x(), triangleRect.y()),
                QPointF(triangleRect.x(), triangleRect.y() + 8),
            };

            if (msgType == ChatMsgModel::MessageType::TextMsg) {
                // painter->setPen(QPen(bgBlueCol));
                // painter->setBrush(bgBlueCol);
                painter->setPen(QPen(highLightColor));
                painter->setBrush(highLightColor);
            } else {
                if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                    painter->setPen(QPen(bgWhiteCol));
                    painter->setBrush(bgWhiteCol);
                } else {
                    painter->setPen(QPen(bgBlackCol));
                    painter->setBrush(bgBlackCol);
                }
            }

            painter->drawPolygon(points, 3);

            // 消息外框矩形
            // msgBorderRect.moveRight(triangleRect.left());

            // // 消息外框加圆角
            // QPainterPath borderPath;
            // borderPath.setFillRule(Qt::WindingFill);
            // borderPath.addRoundedRect(msgBorderRect, 6, 6);
            // painter->fillPath(borderPath, QBrush(bgBlueCol));

            // 头像文字
            painter->setPen(QPen(Qt::white));
            textOption.setAlignment(Qt::AlignCenter);
            textFont.setPixelSize(18);
            // textFont.setPointSizeF(GlobalData::getInstance()->m_font18pxToPt);
            painter->setFont(textFont);
            painter->drawText(avatarRect, mineAvatar, textOption);

            textFont.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            textOption.setWrapMode(QTextOption::WrapAnywhere);
            painter->setPen(QPen(Qt::white));
            painter->setFont(textFont);
            painter->drawText(msgContentRect, msgContent, textOption);
        }

        // 消息内容
        if (msgType == ChatMsgModel::MessageType::TextMsg) {
            // 文字消息

            /********** 绘制矩形 **********/

            // 文字消息外框加圆角 填充颜色
            // painter->fillPath(borderPath, QBrush(bgBlueCol));

            // int maxTextWidth = 294;
            int maxTextWidth;
            if (GlobalData::getInstance()->m_currentMode == CurrentMode::PCMode) {
                maxTextWidth = 294;
            } else {
                maxTextWidth = option.rect.width() * 0.42;
            }
            QSize msgSize(maxTextWidth, 20);
            msgSize = ChatMsgDelegate::getMsgRectSize(msgContent, painter->font(), maxTextWidth);

            // 消息外框矩形
            msgBorderRect.setWidth(msgSize.width() + 18);
            msgBorderRect.setHeight(msgSize.height() + 18);

            if (isSend == ChatMsgModel::SendType::SendMsg) {
                // 消息外框矩形移动位置
                msgBorderRect.moveRight(triangleRect.left());

                // 发送状态矩形移动位置
                msgStateRect.moveCenter(QPoint(msgBorderRect.left() - 16, msgBorderRect.center().y()));
            }

            // 消息外框加圆角
            QPainterPath borderPath;
            borderPath.setFillRule(Qt::WindingFill);
            borderPath.addRoundedRect(msgBorderRect, 6, 6);

            if (isSend == ChatMsgModel::SendType::SendMsg) {
                // painter->fillPath(borderPath, QBrush(bgBlueCol));
                painter->fillPath(borderPath, QBrush(highLightColor));
                painter->setPen(QPen(Qt::white));
            } else {
                if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                    painter->fillPath(borderPath, QBrush(Qt::white));
                    painter->setPen(QPen(textBlackCol));
                } else {
                    painter->fillPath(borderPath, QBrush(bgBlackCol));
                    painter->setPen(QPen(textWhiteCol));
                }
            }

            // 文字消息内容矩形
            QPoint msgPoint(msgBorderRect.left() + 12, msgBorderRect.top() + 9);
            msgContentRect = QRect(msgPoint, msgSize);

            // itemRect.setHeight(msgBorderRect.height() + 10);
            // itemHeight = msgBorderRect.height() + 10;
            // ChatMsgDelegate::setItemHeight(msgBorderRect.height() + 20);
            // itemRect.setX(option.rect.x());
            // itemRect.setY(option.rect.y());
            // itemRect.setWidth(400);
            // itemRect.setHeight(msgBorderRect.height() + 20);

            // qDebug() << "option.rect" << option.rect.x() << option.rect.y();
            // qDebug() << "itemRect" << itemRect.width() << itemRect.height();

            // 文字消息内容绘制
            textFont.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            textOption.setWrapMode(QTextOption::WrapAnywhere);
            painter->setFont(textFont);
            painter->drawText(msgContentRect, msgContent, textOption);

            // 发送状态
            if (sendState == ChatMsgModel::MesssageState::SendFailed) {
                msgStatePixmap = QPixmap(QIcon(":/res/error.svg").pixmap(msgStateRect.size()));

            } else if (sendState == ChatMsgModel::MesssageState::SendDefault) {
                msgStatePixmap = QPixmap(QIcon(":/res/stop.svg").pixmap(msgStateRect.size()));

            } else {
                msgStatePixmap = QPixmap();
            }
            painter->drawPixmap(msgStateRect, msgStatePixmap);

        } else if (msgType == ChatMsgModel::MessageType::FileMsg || msgType == ChatMsgModel::MessageType::DirMsg) {
            // 文件消息

            /********** 绘制矩形 **********/

            // 文件图标矩形
            QPoint fileIconPoint(msgBorderRect.left() + 8, msgBorderRect.top() + 6);
            QSize fileIconSize(64, 64);
            fileIconRect = QRect(fileIconPoint, fileIconSize);

            // 文件名矩形
            QPoint fileNamePoint(msgBorderRect.left() + 76, msgBorderRect.top() + 15);
            QSize fileNameSize(140, 20);
            fileNameRect = QRect(fileNamePoint, fileNameSize);

            // 文件大小矩形
            QPoint fileSizePoint(fileNameRect.left(), fileNameRect.bottom() + 8);
            QSize fileSizeSize(140, 18);
            fileSizeRect = QRect(fileSizePoint, fileSizeSize);

            if (isSend == ChatMsgModel::SendType::SendMsg) {
                // 发送状态矩形移动位置
                msgStateRect.moveCenter(QPoint(msgBorderRect.left() - 16, msgBorderRect.center().y()));
            }

            /********** 绘制文字和图片 **********/

            QFileInfo fileInfo(filePath);

            // 消息外框加圆角
            QPainterPath borderPath;
            borderPath.setFillRule(Qt::WindingFill);
            borderPath.addRoundedRect(msgBorderRect, 6, 6);
            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->fillPath(borderPath, QBrush(Qt::white));
            } else {
                painter->fillPath(borderPath, QBrush(bgBlackCol));
            }

            // 文件名文字
            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->setPen(textBlackCol);
            } else {
                painter->setPen(textWhiteCol);
            }
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            textFont.setPixelSize(14);
            // textFont.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
            painter->setFont(textFont);
            QString disFileName = fileInfo.fileName();
            QFontMetrics fontmts = QFontMetrics(textFont);
            int dif = fontmts.width(disFileName) - fileNameRect.width();
            if (dif > 0) {
                disFileName = fontmts.elidedText(disFileName, Qt::ElideRight, fileNameRect.width());
            }
            painter->drawText(fileNameRect, disFileName, textOption);

            // 文件大小文字
            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->setPen(QColor(140, 140, 140));
            } else {
                painter->setPen(QColor(140, 140, 140));
            }
            QString displaySizeStr = QString();
            if (transferSize == totalSize) {
                displaySizeStr = this->sizeFormat(totalSize);
            } else {
                displaySizeStr = this->sizeFormat(totalSize) + "/" + this->sizeFormat(transferSize);
            }
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            textFont.setPixelSize(12);
            // textFont.setPointSizeF(GlobalData::getInstance()->m_font12pxToPt);
            painter->setFont(textFont);
            painter->drawText(fileSizeRect, displaySizeStr, textOption);

            // 获取图标
            QIcon fileIcon = QIcon();
            QFileIconProvider provider;
            if (msgType == ChatMsgModel::MessageType::FileMsg) {
                fileIcon = QIcon::fromTheme(Peony::FileUtils::getFileIconName("file://" + filePath, false),
                                            QIcon::fromTheme("text-x-generic"));
            } else {
                fileIcon = provider.icon(QFileIconProvider::Folder);
            }
            fileIconPixmap = QPixmap(fileIcon.pixmap(fileIconRect.size()));
            painter->drawPixmap(fileIconRect, fileIconPixmap);

            // 发送状态
            if (sendState == ChatMsgModel::MesssageState::SendDefault) {
                msgStatePixmap = QPixmap(QIcon(":/res/stop.svg").pixmap(msgStateRect.size()));

            } else if (sendState == ChatMsgModel::MesssageState::SendFailed) {
                msgStatePixmap = QPixmap(QIcon(":/res/error.svg").pixmap(msgStateRect.size()));

            } else {
                msgStatePixmap = QPixmap();
            }
            painter->drawPixmap(msgStateRect, msgStatePixmap);
        }

        if (option.state.testFlag(QStyle::State_MouseOver)) {
            ChatMsgDelegate::borderPoint = msgBorderRect.topLeft().toPoint();
            ChatMsgDelegate::borderSize = msgBorderRect.size().toSize();

            // qDebug() << "ChatMsgDelegate::borderPoint" << ChatMsgDelegate::borderPoint.x() <<
            // ChatMsgDelegate::borderPoint.y();
        }

        painter->restore();
    }
}

// TODO : 算法待优化
// 根据消息内容获取消息框大小
QSize ChatMsgDelegate::getMsgRectSize(QString msgStr, QFont font, int textWidth)
{
    QFontMetricsF fm(font);
    int lineHeight = fm.lineSpacing();
    int nCount = msgStr.count("\n");
    int maxWidth = 0;
    QSize resSize = QSize();

    if (nCount == 0) { // 没有换行符

        maxWidth = fm.width(msgStr);

        if (maxWidth > textWidth) {
            maxWidth = textWidth;

            // int lineSize = textWidth / fm.width(" ");
            int lineNum = fm.width(msgStr) / textWidth;
            // int ttmp = lineNum * fm.width(" ");

            nCount += lineNum;
        }
    } else {
        QStringList msgStrList = msgStr.split("\n");

        for (int i = 0; i < msgStrList.size(); i++) {
            QString singleStr = msgStrList[i];
            maxWidth = fm.width(singleStr) > maxWidth ? fm.width(singleStr) : maxWidth;

            if (fm.width(singleStr) > textWidth) {
                maxWidth = textWidth;

                // int lineSize = textWidth / fm.width(" ");
                int lineNum = fm.width(singleStr) / textWidth;

                lineNum = ((i + lineNum) * fm.width(" ") + fm.width(singleStr)) / textWidth;
                nCount += lineNum;
            }
        }
    }

    resSize.setWidth(maxWidth + 10);
    resSize.setHeight((nCount + 1) * lineHeight + 5);

    // qDebug() << nCount << lineHeight;
    // qDebug() << "text rect size" << resSize.width() << resSize.height();

    return resSize;

    // return QSize(itemWidth, itemHeight);
}

// 根据消息内容获取消息框大小
void ChatMsgDelegate::setItemHeight(int height)
{
    ChatMsgDelegate::itemHeight = height;
}

// 将byte转换成KB或MB
QString ChatMsgDelegate::sizeFormat(qint64 size)
{
    QString sizeFormatted;

    qint64 kbSize = 1024;
    qint64 mbSize = kbSize * kbSize;
    qint64 gbSize = mbSize * kbSize;

    if (size < kbSize) {
        sizeFormatted = QString::number(size) + " B";
    } else if (size < mbSize) {
        sizeFormatted = QString::number(size * 1.0 / kbSize, 'f', 1) + " KB";
    } else if (size < gbSize) {
        sizeFormatted = QString::number(size * 1.0 / mbSize, 'f', 1) + " MB";
    } else {
        sizeFormatted = QString::number(size * 1.0 / gbSize, 'f', 1) + " GB";
    }

    return sizeFormatted;
}
