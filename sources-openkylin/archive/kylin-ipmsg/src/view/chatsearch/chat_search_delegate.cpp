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

#include <QDateTime>
#include <QRectF>
#include <peony-qt/file-utils.h>
#include <QPainterPath>
#include <QHelpEvent>
#include <QToolTip>

#include "chat_search_delegate.h"
#include "global/utils/globalutils.h"
#include "view/localinfo/localinfo.h"
#include "global/utils/inisetting.h"
#include "global/utils/globaldateformat.h"
#include "libkydate.h"
#include <QStyleOptionButton>

const QString LIGHT_CLICK_COLOR = "#EBEBEB";
const QString LIGHT_HOVER_COLOR = "#F5F5F5";
const QString BLACK_CLICK_COLOR = "#383838";
const QString BLACK_HOVER_COLOR = "#4D4D4D";

ChatSearchDelegate::ChatSearchDelegate(ChatMsg *chatMsg)
{
    this->m_chatMsg = chatMsg;
    this->m_chatSearch = nullptr;
}

ChatSearchDelegate::ChatSearchDelegate(ChatMsg *chatMsg, ChatSearch *chatSearch)
{
    this->m_chatMsg = chatMsg;
    this->m_chatSearch = chatSearch;
}

ChatSearchDelegate::~ChatSearchDelegate() {}

QSize ChatSearchDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(500, 80);
}

void ChatSearchDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (this->m_chatMsg == nullptr) {
        qDebug() << "Error : chatmsg pointer is null";
        return;
    }

    if (!index.isValid()) {
        qDebug() << "Error : index is not valid";
        return;
    }

    painter->setFont(GlobalData::getInstance()->m_fontName);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    /* 获取数据 */
    QString friendName = this->m_chatMsg->m_friendUsername;
    QString friendNickname = this->m_chatMsg->m_friendNickname;

    QString chatText = index.data(ChatMsgModel::ChatMsgRoles::MsgContent).toString();
    QString msgTime = index.data(ChatMsgModel::ChatMsgRoles::MsgTime).toString();
    QString filePath = index.data(ChatMsgModel::ChatMsgRoles::FilePath).toString();
    int msgType = index.data(ChatMsgModel::ChatMsgRoles::MsgType).toInt();
    int position = index.data(ChatMsgModel::ChatMsgRoles::IsSend).toInt();
    long long int totalSize = index.data(ChatMsgModel::TotalSize).toLongLong();
    long long int transferSize = index.data(ChatMsgModel::TransferSize).toLongLong();
    int chooseMsgState = index.data(ChatMsgModel::ChatMsgRoles::ChooseStateType).toInt();

    QString head;
    if (position == ChatMsgModel::SendType::RecvMsg) {
        if (friendNickname.isEmpty()) {
            head = GlobalUtils::getAvatarByName(friendName);
        } else {
            head = GlobalUtils::getAvatarByName(friendNickname);
        }
    } else {
        if (LocalInfo::getInstance()) {
            head = LocalInfo::getInstance()->getLocalAvatar();
        }
    }

    QString name;
    if (position == ChatMsgModel::SendType::RecvMsg) {
        if (friendNickname.isEmpty()) {
            name = friendName;
        } else {
            name = friendNickname;
        }
    } else {
        if (LocalInfo::getInstance() && LocalInfo::getInstance()->m_nicknameLab != nullptr) {
            name = IniSettings::getInstance()->getLocalNickname();
        }
    }

    /* 时间的显示形式跟随ukui-control-center */
    QDateTime messageTime;

    QString time;
    messageTime = QDateTime::fromString(msgTime, GlobalUtils::getTimeFormat());
    time = GlobalDateFormat::getInstance()->TranslateDateFormat(messageTime);
    //qDebug() << "( ChatSearchDelegate::paint ) 时间字符串： " << msgTime << " 转化为时间为：" << messageTime << "转化为字符串为:" << time;

    /* 画 */
    QFont font = painter->font();
    QTextOption textOption;

    QRectF itemRect;
    itemRect.setX(option.rect.x());
    itemRect.setY(option.rect.y());
    itemRect.setWidth(option.rect.width() - 1);
    itemRect.setHeight(option.rect.height() - 1);

    /* 鼠标悬停 改变背景色 */
    QPainterPath path;
    if (option.state.testFlag(QStyle::State_MouseOver)) {
        path.setFillRule(Qt::WindingFill);
        path.addRoundedRect(itemRect, 0, 0);

        if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
            painter->fillPath(path, QBrush(QColor(LIGHT_HOVER_COLOR)));
        } else {
            painter->fillPath(path, QBrush(QColor(BLACK_HOVER_COLOR)));
        }
    } else if (option.state.testFlag(QStyle::State_HasFocus) || chooseMsgState == ChatMsgModel::ChooseMsgState::ChooseConfirm) {
        path.setFillRule(Qt::WindingFill);
        path.addRoundedRect(itemRect, 0, 0);

        if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
            painter->fillPath(path, QBrush(QColor(LIGHT_CLICK_COLOR)));
        } else {
            painter->fillPath(path, QBrush(QColor(BLACK_CLICK_COLOR)));
        }
    }

    QColor highLightColor = QApplication::palette().highlight().color();

    //判断当前是否进行批量删除
    if (m_chatSearch && m_chatSearch->getDeleteState()) {
        bool checked ;

        // 置顶标识
        QSize stayTopSize(18, 18);
        QPoint stayTopPoint(itemRect.left() + 15, itemRect.top() + 30);
        QRect stayTopRect = QRect(stayTopPoint, stayTopSize);
        if (chooseMsgState == ChatMsgModel::ChooseMsgState::ChooseConfirm) {
            checked = true;
        } else {
            checked = false;
        }       
        QStyleOptionButton checkBoxStyleOption;
        checkBoxStyleOption.state |= QStyle::State_Enabled;
        checkBoxStyleOption.state |= checked ? QStyle::State_On : QStyle::State_Off;
        checkBoxStyleOption.rect = stayTopRect;
        checkBoxStyleOption.iconSize = stayTopSize;
 
        QApplication::style()->drawControl(QStyle::CE_CheckBox,&checkBoxStyleOption,painter);

        if (msgType == ChatMsgModel::MessageType::TextMsg) {

            /* 头像 */
            QPoint headPoint(stayTopRect.right() + 12, itemRect.top() + 15);
            QSize headSize(50, 50);
            QRectF headRect = QRect(headPoint, headSize);

            QPainterPath headPath;
            headPath.setFillRule(Qt::WindingFill);
            headPath.addRoundedRect(headRect, 30, 30);
            painter->fillPath(headPath, QBrush(highLightColor));

            painter->setPen(QPen(Qt::white));
            textOption.setAlignment(Qt::AlignCenter);
            font.setPixelSize(22);
            painter->setFont(font);
            painter->drawText(headRect, head, textOption);

            /* 昵称或备注 */
            QPoint namePoint(headRect.right() + 8, itemRect.top() + 8);
            QSize nameSize(282, 35);
            QRectF nameRect = QRect(namePoint, nameSize);

            painter->setPen(QPen(QColor("#8C8C8C")));
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
            font.setPointSizeF(GlobalData::getInstance()->m_font16pxToPt);
            QFontMetrics fontmtsName = QFontMetrics(font);
            if (fontmtsName.width(name) > nameRect.width()) {
                name = fontmtsName.elidedText(name, Qt::ElideRight, nameRect.width());
            }
            painter->setFont(font);
            painter->drawText(nameRect, name, textOption);

            /* 聊天内容 */
            QPoint textPoint(headRect.right() + 8, nameRect.bottom() - 2);
            QSize textSize(262, 30);
            QRectF textRect = QRect(textPoint, textSize);

            painter->setPen(QPen(QColor("#262626")));
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            font.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
            painter->setFont(font);
            QFontMetrics fontmts = QFontMetrics(font);
            QStringList res = chatText.split('\n');
            chatText = res.at(0);
            int dif = fontmts.width(chatText) - textRect.width();
            if (dif > 0) {
                chatText = fontmts.elidedText(chatText, Qt::ElideRight, textRect.width());
            }

            /* 搜索关键词 ，关键词高亮 */
            QTextDocument document;

            /* 设置文字边距 ，保证绘制文字居中 */
            document.setDocumentMargin(0);
            document.setPlainText(chatText);
            bool found = false;
            QTextCursor highlight_cursor(&document);
            QTextCursor cursor(&document);

            /* 搜索字体高亮 */
            cursor.beginEditBlock();
            QTextCharFormat color_format(highlight_cursor.charFormat());
            color_format.setForeground(highLightColor); /*设置高亮颜色*/

            QTextCursor testcursor(&document);
            testcursor.select(QTextCursor::LineUnderCursor);
            QTextCharFormat fmt;
            fmt.setFont(font);
            testcursor.mergeCharFormat(fmt);
            testcursor.clearSelection();
            testcursor.movePosition(QTextCursor::EndOfLine);

            while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
                highlight_cursor = document.find(m_chatSearch->m_searchText, highlight_cursor);
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
            painter->translate(textRect.topLeft());
            painter->setClipRect(textRect.translated(-textRect.topLeft()));
            document.documentLayout()->draw(painter, paintContext);
            painter->restore();

            /* 最近聊天时间 */
            painter->save();
            painter->setPen(QPen(QColor("#8C8C8C")));
            QPoint timePoint(itemRect.right() - 150, itemRect.top() + 20);
            QSize timeSize(120, 23);
            QRectF timeRect = QRect(timePoint, timeSize);
            timeRect.setRight(itemRect.right() - 20);

            textOption.setAlignment(Qt::AlignRight | Qt::AlignTop);
            font.setPointSizeF(GlobalData::getInstance()->m_font12pxToPt);
            //        painter->setFont(QFont(painter->fontInfo().family() , 12));
            painter->setFont(font);
            painter->drawText(timeRect, time, textOption);

            painter->restore();

        } else if (msgType == ChatMsgModel::MessageType::FileMsg || msgType == ChatMsgModel::MessageType::DirMsg) {
            /* 文件图标 */
            QPoint fileIconPoint(stayTopRect.right() + 12, itemRect.top() + 6);
            QSize fileIconSize(64, 64);
            QRect fileIconRect = QRect(fileIconPoint, fileIconSize);

            QIcon fileIcon;
            QFileIconProvider provider;
            if (msgType == ChatMsgModel::MessageType::FileMsg) {
                fileIcon = QIcon::fromTheme(Peony::FileUtils::getFileIconName("file://" + filePath, false),
                                            QIcon::fromTheme("text-x-generic"));
            } else {
                fileIcon = provider.icon(QFileIconProvider::Folder);
            }
            QPixmap fileIconPixmap = QPixmap(fileIcon.pixmap(fileIconRect.size()));
            painter->drawPixmap(fileIconRect, fileIconPixmap);

            /* 文件名 */
            QPoint fileNamePoint(stayTopRect.right() + 80, itemRect.top() + 5);
            QSize fileNameSize(262, 25);
            QRect fileNameRect = QRect(fileNamePoint, fileNameSize);

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->setPen(QColor(38, 38, 38));
            } else {
                painter->setPen(QColor(217, 217, 217));
            }

            QFileInfo fileInfo(filePath);
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
            font.setPixelSize(14);
            painter->setFont(font);
            QString fileName = fileInfo.fileName();
            QFontMetrics fontmts = QFontMetrics(font);
            int dif = fontmts.width(fileName) - fileNameRect.width();
            if (dif > 0) {
                fileName = fontmts.elidedText(fileName, Qt::ElideRight, fileNameRect.width());
            }
            painter->drawText(fileNameRect, fileName, textOption);

            /* 文件大小 */
            QPoint fileSizePoint(fileNameRect.left(), fileNameRect.bottom());
            QSize fileSizeSize(140, 18);
            QRect fileSizeRect = QRect(fileSizePoint, fileSizeSize);

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->setPen(QColor(140, 140, 140));
            } else {
                painter->setPen(QColor(140, 140, 140));
            }

            QString fileSize;
            QString switchTotalSize;
            QString switchTranferSize;
            qint64 kbSize = 1024;
            qint64 mbSize = kbSize * kbSize;
            qint64 gbSize = mbSize * kbSize;

            if (totalSize < kbSize) {
                switchTotalSize = QString::number(totalSize) + " B";
            } else if (totalSize < mbSize) {
                switchTotalSize = QString::number(totalSize * 1.0 / kbSize, 'f', 1) + " KB";
            } else if (totalSize < gbSize) {
                switchTotalSize = QString::number(totalSize * 1.0 / mbSize, 'f', 1) + " MB";
            } else {
                switchTotalSize = QString::number(totalSize * 1.0 / gbSize, 'f', 1) + " GB";
            }

            if (transferSize < kbSize) {
                switchTranferSize = QString::number(transferSize) + " B";
            } else if (totalSize < mbSize) {
                switchTranferSize = QString::number(transferSize * 1.0 / kbSize, 'f', 1) + " KB";
            } else if (totalSize < gbSize) {
                switchTranferSize = QString::number(transferSize * 1.0 / mbSize, 'f', 1) + " MB";
            } else {
                switchTranferSize = QString::number(transferSize * 1.0 / gbSize, 'f', 1) + " GB";
            }

            if (totalSize == transferSize) {
                fileSize = switchTotalSize;
            } else {
                fileSize = switchTotalSize + "/" + switchTranferSize;
            }

            textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            font.setPixelSize(12);
            painter->setFont(font);
            painter->drawText(fileSizeRect, fileSize, textOption);

            painter->setPen(QPen(QColor("#8C8C8C")));
            QPoint timePoint(itemRect.right() - 150, itemRect.top() + 10);
            QSize timeSize(120, 23);
            QRectF timeRect = QRect(timePoint, timeSize);
            timeRect.setRight(itemRect.right() - 20);

            textOption.setAlignment(Qt::AlignRight | Qt::AlignTop);
            font.setPointSizeF(GlobalData::getInstance()->m_font12pxToPt);
            painter->setFont(font);
            painter->drawText(timeRect, time, textOption);
        }
    } else {

        if (msgType == ChatMsgModel::MessageType::TextMsg) {

            /* 头像 */
            QPoint headPoint(itemRect.left() + 16, itemRect.top() + 15);
            QSize headSize(50, 50);
            QRectF headRect = QRect(headPoint, headSize);

            QPainterPath headPath;
            headPath.setFillRule(Qt::WindingFill);
            headPath.addRoundedRect(headRect, 30, 30);
            painter->fillPath(headPath, QBrush(highLightColor));

            painter->setPen(QPen(Qt::white));
            textOption.setAlignment(Qt::AlignCenter);
            font.setPixelSize(22);
            painter->setFont(font);
            painter->drawText(headRect, head, textOption);

            /* 昵称或备注 */
            QPoint namePoint(headRect.right() + 8, itemRect.top() + 8);
            QSize nameSize(282, 35);
            QRectF nameRect = QRect(namePoint, nameSize);

            painter->setPen(QPen(QColor("#8C8C8C")));
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
            font.setPointSizeF(GlobalData::getInstance()->m_font16pxToPt);
            QFontMetrics fontmtsName = QFontMetrics(font);
            if (fontmtsName.width(name) > nameRect.width()) {
                name = fontmtsName.elidedText(name, Qt::ElideRight, nameRect.width());
            }
            painter->setFont(font);
            painter->drawText(nameRect, name, textOption);

            /* 聊天内容 */
            QPoint textPoint(headRect.right() + 8, nameRect.bottom() - 2);
            QSize textSize(282, 30);
            QRectF textRect = QRect(textPoint, textSize);

            painter->setPen(QPen(QColor("#262626")));
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            font.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
            painter->setFont(font);
            QFontMetrics fontmts = QFontMetrics(font);
            QStringList res = chatText.split('\n');
            chatText = res.at(0);
            int dif = fontmts.width(chatText) - textRect.width();
            if (dif > 0) {           
                chatText = fontmts.elidedText(chatText, Qt::ElideRight, textRect.width()-18);
            }

            /* 搜索关键词 ，关键词高亮 */
            QTextDocument document;

            /* 设置文字边距 ，保证绘制文字居中 */
            document.setDocumentMargin(0);
            document.setPlainText(chatText);
            bool found = false;
            QTextCursor highlight_cursor(&document);
            QTextCursor cursor(&document);

            /* 搜索字体高亮 */
            cursor.beginEditBlock();
            QTextCharFormat color_format(highlight_cursor.charFormat());
            color_format.setForeground(highLightColor); /*设置高亮颜色*/

            QTextCursor testcursor(&document);
            testcursor.select(QTextCursor::LineUnderCursor);
            QTextCharFormat fmt;
            fmt.setFont(font);
            testcursor.mergeCharFormat(fmt);
            testcursor.clearSelection();
            testcursor.movePosition(QTextCursor::EndOfLine);

            while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
                highlight_cursor = document.find(m_chatSearch->m_searchText, highlight_cursor);
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
            painter->translate(textRect.topLeft());
            painter->setClipRect(textRect.translated(-textRect.topLeft()));
            document.documentLayout()->draw(painter, paintContext);
            painter->restore();

            /* 最近聊天时间 */
            painter->save();
            painter->setPen(QPen(QColor("#8C8C8C")));
            QPoint timePoint(itemRect.right() - 150, itemRect.top() + 20);
            QSize timeSize(120, 23);
            QRectF timeRect = QRect(timePoint, timeSize);
            timeRect.setRight(itemRect.right() - 20);

            textOption.setAlignment(Qt::AlignRight | Qt::AlignTop);
            font.setPointSizeF(GlobalData::getInstance()->m_font12pxToPt);
            //        painter->setFont(QFont(painter->fontInfo().family() , 12));
            painter->setFont(font);
            painter->drawText(timeRect, time, textOption);

            painter->restore();

        } else if (msgType == ChatMsgModel::MessageType::FileMsg || msgType == ChatMsgModel::MessageType::DirMsg) {
            /* 文件图标 */
            QPoint fileIconPoint(itemRect.left() + 8, itemRect.top() + 6);
            QSize fileIconSize(64, 64);
            QRect fileIconRect = QRect(fileIconPoint, fileIconSize);

            QIcon fileIcon;
            QFileIconProvider provider;
            if (msgType == ChatMsgModel::MessageType::FileMsg) {
                fileIcon = QIcon::fromTheme(Peony::FileUtils::getFileIconName("file://" + filePath, false),
                                            QIcon::fromTheme("text-x-generic"));
            } else {
                fileIcon = provider.icon(QFileIconProvider::Folder);
            }
            QPixmap fileIconPixmap = QPixmap(fileIcon.pixmap(fileIconRect.size()));
            painter->drawPixmap(fileIconRect, fileIconPixmap);

            /* 文件名 */
            QPoint fileNamePoint(itemRect.left() + 76, itemRect.top() + 5);
            QSize fileNameSize(282, 25);
            QRect fileNameRect = QRect(fileNamePoint, fileNameSize);

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->setPen(QColor(38, 38, 38));
            } else {
                painter->setPen(QColor(217, 217, 217));
            }

            QFileInfo fileInfo(filePath);
            textOption.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
            font.setPixelSize(14);
            painter->setFont(font);
            QString fileName = fileInfo.fileName();
            QFontMetrics fontmts = QFontMetrics(font);
            int dif = fontmts.width(fileName) - fileNameRect.width();
            if (dif > 0) {
                fileName = fontmts.elidedText(fileName, Qt::ElideRight, fileNameRect.width());
            }
            painter->drawText(fileNameRect, fileName, textOption);

            /* 文件大小 */
            QPoint fileSizePoint(fileNameRect.left(), fileNameRect.bottom());
            QSize fileSizeSize(140, 18);
            QRect fileSizeRect = QRect(fileSizePoint, fileSizeSize);

            if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
                painter->setPen(QColor(140, 140, 140));
            } else {
                painter->setPen(QColor(140, 140, 140));
            }

            QString fileSize;
            QString switchTotalSize;
            QString switchTranferSize;
            qint64 kbSize = 1024;
            qint64 mbSize = kbSize * kbSize;
            qint64 gbSize = mbSize * kbSize;

            if (totalSize < kbSize) {
                switchTotalSize = QString::number(totalSize) + " B";
            } else if (totalSize < mbSize) {
                switchTotalSize = QString::number(totalSize * 1.0 / kbSize, 'f', 1) + " KB";
            } else if (totalSize < gbSize) {
                switchTotalSize = QString::number(totalSize * 1.0 / mbSize, 'f', 1) + " MB";
            } else {
                switchTotalSize = QString::number(totalSize * 1.0 / gbSize, 'f', 1) + " GB";
            }

            if (transferSize < kbSize) {
                switchTranferSize = QString::number(transferSize) + " B";
            } else if (totalSize < mbSize) {
                switchTranferSize = QString::number(transferSize * 1.0 / kbSize, 'f', 1) + " KB";
            } else if (totalSize < gbSize) {
                switchTranferSize = QString::number(transferSize * 1.0 / mbSize, 'f', 1) + " MB";
            } else {
                switchTranferSize = QString::number(transferSize * 1.0 / gbSize, 'f', 1) + " GB";
            }

            if (totalSize == transferSize) {
                fileSize = switchTotalSize;
            } else {
                fileSize = switchTotalSize + "/" + switchTranferSize;
            }

            textOption.setAlignment(Qt::AlignLeft | Qt::AlignTop);
            font.setPixelSize(12);
            painter->setFont(font);
            painter->drawText(fileSizeRect, fileSize, textOption);

            painter->setPen(QPen(QColor("#8C8C8C")));
            QPoint timePoint(itemRect.right() - 150, itemRect.top() + 10);
            QSize timeSize(120, 23);
            QRectF timeRect = QRect(timePoint, timeSize);
            timeRect.setRight(itemRect.right() - 20);

            textOption.setAlignment(Qt::AlignRight | Qt::AlignTop);
            font.setPointSizeF(GlobalData::getInstance()->m_font12pxToPt);
            painter->setFont(font);
            painter->drawText(timeRect, time, textOption);
        }
    }

    painter->restore();

    return;
}

bool ChatSearchDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
                                   const QStyleOptionViewItem &option, const QModelIndex &index)
{
    int msgType = index.data(ChatMsgModel::ChatMsgRoles::MsgType).toInt();
    if (msgType == ChatMsgModel::MessageType::FileMsg || msgType == ChatMsgModel::MessageType::DirMsg) {
        // 计算文件名区域，需与 paint 方法一致
        QRect fileNameRect;
        if (m_chatSearch && m_chatSearch->getDeleteState()) {
            // 批量删除时的文件名区域
            QPoint fileNamePoint(275 + 80, option.rect.top() + 10); // stayTopRect.right() + 80
            QSize fileNameSize(262, 30);
            fileNameRect = QRect(fileNamePoint, fileNameSize);
        } else {
            // 普通模式下的文件名区域
            QPoint fileNamePoint(option.rect.left() + 76, option.rect.top() + 10);
            QSize fileNameSize(282, 30);
            fileNameRect = QRect(fileNamePoint, fileNameSize);
        }
        if (fileNameRect.contains(event->pos())) {
            QString filePath = index.data(ChatMsgModel::ChatMsgRoles::FilePath).toString();
            QFileInfo fileInfo(filePath);
            QString fileName = fileInfo.fileName();
            QFont font = GlobalData::getInstance()->m_fontName;
            QFontMetrics fontmts(font);
            QString elidedFileName = fontmts.elidedText(fileName, Qt::ElideRight, fileNameRect.width());
            if (fileName != elidedFileName) {
                QToolTip::showText(event->globalPos(), fileName, view);
                return true;
            }
        }
    }
    return QAbstractItemDelegate::helpEvent(event, view, option, index);
}
