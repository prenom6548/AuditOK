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

#include <QProcess>


#include "chatmsg.h"
#include "model/friendlistmodel.h"
#include "global/utils/globalutils.h"
#include "view/common/globalsizedata.h"
#include "global/utils/global_data.h"
#include "controller/control.h"
#include "global/utils/globaldateformat.h"
#include "libkydate.h"
#include <windowmanage.hpp>
#include <gsettingmonitor.h>
#include <usermanual.h>

#include <QDir>
#include <QFileDialog>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QTreeView>
#include <QDialogButtonBox>
#include <unistd.h>

#include <QApplication>
#include <QDesktopWidget>
#include <kysdk/diagnosetest/libkydatacollect.h>
#include "ukuiwindowhelper.h"

const QSize BTN_MAX_SIZE = QSize(48, 48);
const QSize BTN_SIZE = QSize(36, 36);
const QSize ICON_SIZE = QSize(16, 16);

ChatMsg::ChatMsg(int friendId, QString friendUuid, QWidget *parent) : QWidget(parent)
{
    // 初始化数据
    setInitData(friendId, friendUuid);

    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();

    // 设置信号绑定
    setSignalConn();
}

ChatMsg::~ChatMsg() {}

// 初始化数据
void ChatMsg::setInitData(int friendId, QString friendUuid)
{
    Q_UNUSED(friendUuid);
    QStandardItem *friendItem = FriendListModel::getInstance()->getFriendById(friendId);

    // 设置好友数据
    m_friendId = friendId;
    m_friendUuid = friendItem->data(FriendListModel::Uuid).toString();
    m_friendUsername = friendItem->data(FriendListModel::Username).toString();
    m_friendNickname = friendItem->data(FriendListModel::Nickname).toString();
    m_friendAvatar = friendItem->data(FriendListModel::Avatar).toString();

    m_isConnect = true;
}

// 初始化组件
void ChatMsg::setWidgetUi()
{
    // 初始化并监听gsetting
    initGsetting();

    // 初始化组件和布局
    m_titleBar = new TitleBar(false, false, false);
    m_listView = new QListView(this);
    m_sendAreaWid = new QWidget(this);
    m_fileBtn = new QPushButton(this);
    m_folderBtn = new QPushButton(this);
    m_screenshotBtn = new QPushButton(this);
    m_historyBtn = new QPushButton(this);
    m_sendMsgBtn = new QPushButton(this);
    m_msgEdit = new QTextEdit(this);
    m_mainLayout = new QVBoxLayout(this);
    m_sendAreaLayout = new QVBoxLayout(m_sendAreaWid);
    m_funcBtnLayout = new QHBoxLayout();
    m_sendBtnLayout = new QHBoxLayout();
    m_editLayout = new QHBoxLayout();
    //设置objectname
    m_titleBar->setAccessibleName("kylin-ipmsg_chatmsg_TitleBar_mtitleBar");
    m_listView->setAccessibleName("kylin-ipmsg_chatmsg_QListView_mlistView");
    m_sendAreaWid->setAccessibleName("kylin-ipmsg_chatmsg_QWidget_msendAreaWid");
    m_fileBtn->setAccessibleName("kylin-ipmsg_chatmsg_QPushButton_mfileBtn");
    m_folderBtn->setAccessibleName("kylin-ipmsg_chatmsg_QPushButton_m_folderBtn");
    m_screenshotBtn->setAccessibleName("kylin-ipmsg_chatmsg_QPushButton_mscreenshotBtn");
    m_historyBtn->setAccessibleName("kylin-ipmsg_chatmsg_QPushButton_mhistoryBtn");
    m_sendMsgBtn->setAccessibleName("kylin-ipmsg_chatmsg_QPushButton_msendMsgBtn");
    m_msgEdit->setAccessibleName("kylin-ipmsg_chatmsg_QTextEdit_mmsgEdit");
    m_mainLayout->setObjectName("kylin-ipmsg_chatmsg_QVBoxLayout_mmainLayout");
    m_sendAreaLayout->setObjectName("kylin-ipmsg_chatmsg_QVBoxLayout_msendAreaLayout");
    m_funcBtnLayout->setObjectName("kylin-ipmsg_chatmsg_QHBoxLayout_mfuncBtnLayout");
    m_sendBtnLayout->setObjectName("kylin-ipmsg_chatmsg_QHBoxLayout_msendBtnLayout");
    m_editLayout->setObjectName("kylin-ipmsg_chatmsg_QHBoxLayout_meditLayout");

    // 将组件放入布局
    // 功能按钮布局
    m_funcBtnLayout->addSpacing(4);
    m_funcBtnLayout->addWidget(m_fileBtn);
    m_funcBtnLayout->addSpacing(4);
    m_funcBtnLayout->addWidget(m_folderBtn);
    m_funcBtnLayout->addSpacing(4);
    m_funcBtnLayout->addWidget(m_screenshotBtn);
    m_funcBtnLayout->addSpacing(4);
    m_funcBtnLayout->addWidget(m_historyBtn);
    m_funcBtnLayout->addStretch();
    m_funcBtnLayout->setMargin(0);
    m_funcBtnLayout->setSpacing(0);

    // 发送按钮布局
    m_sendBtnLayout->addWidget(m_sendMsgBtn, 0, Qt::AlignRight);
    m_sendBtnLayout->setContentsMargins(0, 0, 16, 16);
    // m_sendBtnLayout->setMargin(0);
    m_sendBtnLayout->setSpacing(0);

    // 编辑区域布局
    m_editLayout->addSpacing(10);
    m_editLayout->addWidget(m_msgEdit);
    m_editLayout->setMargin(0);
    m_editLayout->setSpacing(0);

    // 发送区域布局
    m_sendAreaLayout->addSpacing(4);
    m_sendAreaLayout->addLayout(m_funcBtnLayout);
    // m_sendAreaLayout->addWidget(m_msgEdit);
    m_sendAreaLayout->addLayout(m_editLayout);
    m_sendAreaLayout->addLayout(m_sendBtnLayout);
    m_sendAreaLayout->setMargin(0);
    m_sendAreaLayout->setSpacing(0);

    // 总体布局
    m_mainLayout->addWidget(m_titleBar);
    m_mainLayout->addWidget(m_listView, 3);
    m_mainLayout->addWidget(m_sendAreaWid, 1);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    // 右键菜单
    m_funcMenu = new QMenu(this);
    m_reSendAction = m_funcMenu->addAction(tr("Resend"));
    m_copyAction = m_funcMenu->addAction(tr("Copy"));
    m_openFileAction = m_funcMenu->addAction(tr("Open"));
    m_openDirAction = m_funcMenu->addAction(tr("Open Directory"));
    m_saveAsAction = m_funcMenu->addAction(tr("Save As"));
    m_deleteMsgAction = m_funcMenu->addAction(tr("Delete"));
    m_clearMsgAction = m_funcMenu->addAction(tr("Clear All"));
    //设置objectname
    m_funcMenu->setAccessibleName("kylin-ipmsg_chatmsg_QMenu_mfuncMenu");
    m_reSendAction->setObjectName("kylin-ipmsg_chatmsg_QAction_mreSendAction");
    m_copyAction->setObjectName("kylin-ipmsg_chatmsg_QAction_mcopyAction");
    m_openFileAction->setObjectName("kylin-ipmsg_chatmsg_QAction_mopenFileAction");
    m_openDirAction->setObjectName("kylin-ipmsg_chatmsg_QAction_mopenDirAction");
    m_saveAsAction->setObjectName("kylin-ipmsg_chatmsg_QAction_msaveAsAction");
    m_deleteMsgAction->setObjectName("kylin-ipmsg_chatmsg_QAction_mdeleteMsgAction");
    m_clearMsgAction->setObjectName("kylin-ipmsg_chatmsg_QAction_mclearMsgAction");


    // 截图
    m_clipboard = QApplication::clipboard();
    m_startShot = false;
    m_oldClipVar = QVariant();
}

// 设置组件样式
void ChatMsg::setWidgetStyle()
{
    //毛玻璃
    // this->setProperty("useSystemStyleBlur",true);
    // this->setAttribute(Qt::WA_TranslucentBackground,true);

    //失焦仍显示提示信息
    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    // 主界面属性
    /* 适配kysdk的窗管 */
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(this);
    windowhelper->removeTitleBar();

    this->setFixedSize(QSize(700, 625));
    // this->setMouseTracking(true);

    // 设置标题栏属性
    m_titleBar->m_pIconBtn->hide();
    m_titleBar->setAutoFillBackground(true);
    m_titleBar->setBackgroundRole(QPalette::Base);

    QString displayName = m_friendNickname.isEmpty() ? m_friendUsername : m_friendNickname;
    updateFriendNickname(displayName);

    // 设置聊天记录属性
    m_chatMsgModel = new ChatMsgModel(m_friendId);
    m_chatMsgModel->setObjectName("kylin-ipmsg_chatmsg_ChatMsgModel_mchatMsgModel");
    m_chatMsgModel->getModelFromDb();

    m_chatMsgDelegate = new ChatMsgDelegate();
    m_chatMsgDelegate->setObjectName("kylin-ipmsg_chatmsg_ChatMsgDelegate_mchatMsgDelegate");

    m_listView->setItemDelegate(m_chatMsgDelegate);
    m_listView->setModel(m_chatMsgModel);
    m_listView->setFixedHeight(416);
    m_listView->setSpacing(10);
    m_listView->setDragEnabled(false);
    m_listView->setFrameShape(QListView::NoFrame);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listView->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_listView->scrollToBottom();
    m_listView->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    // m_listView->setMouseTracking(true);

    changeBackground();

    // m_listView->setAutoFillBackground(true);
    // m_listView->setBackgroundRole(QPalette::Window);

    // m_listView->setProperty("useSystemStyleBlur",true);
    // m_listView->setAttribute(Qt::WA_TranslucentBackground,true);

    // 添加分割线
    m_funcMenu->insertSeparator(m_deleteMsgAction);

    // 设置发送区域属性
    m_sendAreaWid->setAutoFillBackground(true);
    m_sendAreaWid->setBackgroundRole(QPalette::Base);

    // 文件按钮
    m_fileBtn->setFixedSize(BTN_SIZE);
    m_fileBtn->setIconSize(ICON_SIZE);
    m_fileBtn->setIcon(QIcon::fromTheme("folder-documents-symbolic"));
    m_fileBtn->setProperty("useButtonPalette", true);
    m_fileBtn->setProperty("useIconHighlightEffect", 0x2);
    // m_fileBtn->setProperty("useButtonPalette", true);
    m_fileBtn->setFlat(true);
    m_fileBtn->setToolTip(tr("File"));

    // 文件夹按钮
    m_folderBtn->setFixedSize(BTN_SIZE);
    m_folderBtn->setIconSize(ICON_SIZE);
    m_folderBtn->setIcon(QIcon::fromTheme("folder-visiting-symbolic"));
    m_folderBtn->setProperty("useButtonPalette", true);
    m_folderBtn->setProperty("useIconHighlightEffect", 0x2);
    m_folderBtn->setFlat(true);
    m_folderBtn->setToolTip(tr("Folder"));

    // 截图按钮
    m_screenshotBtn->setFixedSize(BTN_SIZE);
    m_screenshotBtn->setIconSize(ICON_SIZE);
    m_screenshotBtn->setIcon(QIcon::fromTheme("image-crop-symbolic"));
    m_screenshotBtn->setProperty("useButtonPalette", true);
    m_screenshotBtn->setProperty("useIconHighlightEffect", 0x2);
    m_screenshotBtn->setFlat(true);
    m_screenshotBtn->setToolTip(tr("Screen Shot"));

    // 历史记录按钮
    m_historyBtn->setFixedSize(BTN_SIZE);
    m_historyBtn->setIconSize(ICON_SIZE);
    m_historyBtn->setIcon(QIcon::fromTheme("document-open-recent-symbolic"));
    m_historyBtn->setProperty("useButtonPalette", true);
    m_historyBtn->setProperty("useIconHighlightEffect", 0x2);
    m_historyBtn->setFlat(true);
    m_historyBtn->setToolTip(tr("History Message"));

    /* 暂时隐藏历史更新按钮 */
    // m_historyBtn->hide();

    // 消息编辑框
    m_msgEdit->setFixedHeight(62);
    m_msgEdit->setFrameShape(QTextEdit::NoFrame);
    m_msgEdit->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_msgEdit->installEventFilter(this);
    m_msgEdit->setAcceptDrops(false);

    // 发送按钮
    QFont btnFont = GlobalData::getInstance()->getFontSize14px();
    m_sendMsgBtn->setFont(btnFont);
    m_sendMsgBtn->setFixedSize(QSize(96, 36));
    m_sendMsgBtn->setText(tr("Send"));
    m_sendMsgBtn->setProperty("isImportant", true);
    m_sendMsgBtn->setEnabled(false);

    // 允许拖拽
    this->setAcceptDrops(true);

    switch (GlobalData::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
        changeSizePCMode();        
            break;
        case CurrentMode::HMode:
        changeSizeHScreenMode();
            break;
        case CurrentMode::VMode:
        changeSizeVScreenMode();
            break;
    }
}

// 初始化并监听gsetting
void ChatMsg::initGsetting()
{
    // 主题颜色
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        GlobalData::getInstance()->getSystemTheme();
        changeBackground();
    });

    // 麒麟截图
    if (QGSettings::isSchemaInstalled(SCREEN_SHOT_GSETTING_PATH)) {
        m_screenShotSetting = new QGSettings(SCREEN_SHOT_GSETTING_PATH);
        m_screenShotSetting->setObjectName("kylin-ipmsg_chatmsg_QGSettings_mscreenShotSetting");

        connect(m_screenShotSetting, &QGSettings::changed, this, [=]() {
            // qDebug() << m_startShot << m_screenShotSetting->get("isrunning").toString();
            if (m_startShot && (m_screenShotSetting->get("isrunning").toString() == "false")) {
                m_startShot = false;
                getShotImage();
            }
        });
    }
}

// 设置信号绑定
void ChatMsg::setSignalConn()
{

    connect(m_titleBar->m_pCloseButton, &QPushButton::clicked, this, &ChatMsg::hide);

    //    connect(m_titleBar->m_pCloseButton, &QPushButton::clicked, this, &ChatMsg::slotHideChatMsg);

    connect(m_listView, &QListView::customContextMenuRequested, this, &ChatMsg::showContextMenu);
    connect(m_listView, &QListView::clicked, this, &ChatMsg::msgListClicked);

    connect(m_msgEdit, &QTextEdit::textChanged, this, &ChatMsg::slotEnableSureBtn);

    connect(m_reSendAction, &QAction::triggered, this, &ChatMsg::slotResendMsg);
    connect(m_copyAction, &QAction::triggered, this, &ChatMsg::slotCopyMsg);
    connect(m_openFileAction, &QAction::triggered, this, &ChatMsg::slotOpenFile);
    connect(m_openDirAction, &QAction::triggered, this, &ChatMsg::slotOpenDir);
    connect(m_saveAsAction, &QAction::triggered, this, &ChatMsg::slotSaveAs);
    connect(m_deleteMsgAction, &QAction::triggered, this, &ChatMsg::slotDeleteMsg);
    connect(m_clearMsgAction, &QAction::triggered, this, &ChatMsg::slotClearMsg);

    connect(m_sendMsgBtn, &QPushButton::clicked, this, &ChatMsg::slotSendText);
    connect(m_fileBtn, &QPushButton::clicked, this, &ChatMsg::slotSendFile);
    connect(m_folderBtn, &QPushButton::clicked, this, &ChatMsg::slotSendDir);
    connect(m_screenshotBtn, &QPushButton::clicked, this, &ChatMsg::slotScreenShot);
    connect(m_historyBtn, &QPushButton::clicked, this, &ChatMsg::slotHistoryMsg);

    connect(FriendListModel::getInstance(), &FriendListModel::updateFriendInfo, this, &ChatMsg::slotUpdateFriendInfo);

    /* 字体改变 */
    connect(Control::getInstance(), &Control::sigFontChange, this, [=]() {
        QFont font14 = GlobalData::getInstance()->getFontSize14px();
        this->m_sendMsgBtn->setFont(font14);
        this->m_titleBar->m_pFuncLabel->setFont(font14);
        
        changeListDelegate();
    });
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFont, this, [=](){
        m_listView->update();
    });
    connect(GlobalDateFormat::getInstance(), &GlobalDateFormat::sigTimeFormatChange, this, &ChatMsg::slotUpdateTimeFormat);
    connect(GlobalData::getInstance(), &GlobalData::signalChangePC, this, &ChatMsg::changeSizePCMode);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFlatH, this, &ChatMsg::changeSizeHScreenMode);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFlatV, this, &ChatMsg::changeSizeVScreenMode);
}

// 显示右键菜单
// TODO : 右键响应区域判断
void ChatMsg::showContextMenu(const QPoint &point)
{
#ifndef TEST_MODE
    if (!((m_listView->selectionModel()->selectedIndexes()).empty())) {
#else
    if (true) { // 在测试模式下，即使没有选择项也允许运行逻辑，但跳过 exec()
#endif
        // 将代理index转换为消息index
        QModelIndex msgIndex = m_listView->currentIndex();
        int msgType = msgIndex.data(ChatMsgModel::MsgType).toInt();
        int msgState = msgIndex.data(ChatMsgModel::SendState).toInt();
        int sendType = msgIndex.data(ChatMsgModel::IsSend).toInt();

        if (msgType == ChatMsgModel::MessageType::TimeMsg) {
            m_reSendAction->setVisible(false);
            m_copyAction->setVisible(false);
            m_openFileAction->setVisible(false);
            m_openDirAction->setVisible(false);
            m_saveAsAction->setVisible(false);
            m_deleteMsgAction->setVisible(false);
            m_clearMsgAction->setVisible(true);

#ifndef TEST_MODE
            m_funcMenu->exec(QCursor::pos());
#endif
            m_listView->selectionModel()->clear();
            return;
        }

        if (sendType == ChatMsgModel::SendType::RecvMsg) {
            m_reSendAction->setVisible(false);
        } else {
            m_reSendAction->setVisible(true);
        }

        QPoint diffPoint = QPoint(0, 0);

        diffPoint.setX(point.x() - ChatMsgDelegate::borderPoint.x());
        diffPoint.setY(point.y() - ChatMsgDelegate::borderPoint.y());

        // qDebug() << "diffPoint" << diffPoint.x() << diffPoint.y();

        if (msgState == ChatMsgModel::MesssageState::SendFailed) {
            m_reSendAction->setVisible(true);
        } else {
            m_reSendAction->setVisible(false);
        }

        if (diffPoint.x() < 0 || diffPoint.x() > ChatMsgDelegate::borderSize.width()) {
            QPoint pressPoint = m_listView->mapFromGlobal(QCursor::pos());

            // 头像区域
            QPoint avatarPoint;

            if (sendType == ChatMsgModel::SendType::RecvMsg) {
                avatarPoint.setX(ChatMsgDelegate::borderPoint.x() - 52);
            } else {
                avatarPoint.setX(ChatMsgDelegate::borderPoint.x() + ChatMsgDelegate::borderSize.width() + 14);
            }
            avatarPoint.setY(ChatMsgDelegate::borderPoint.y());

            QPoint diffAvatarPoint = QPoint(pressPoint.x() - avatarPoint.x(), pressPoint.y() - avatarPoint.y());

            if (diffAvatarPoint.x() > 0 && diffAvatarPoint.x() < 38 && diffAvatarPoint.y() > 0
                && diffAvatarPoint.x() < 38) {
                m_listView->selectionModel()->clear();
                return;
            }

            // 发送状态区域
            QPoint statePoint;
            statePoint.setX(ChatMsgDelegate::borderPoint.x() - 26);
            statePoint.setY(ChatMsgDelegate::borderPoint.y() + (ChatMsgDelegate::borderSize.height() - 16) / 2);

            QPoint diffStatePoint = QPoint(pressPoint.x() - statePoint.x(), pressPoint.y() - statePoint.y());

            if (diffStatePoint.x() < 0 || diffStatePoint.x() > 16 || diffStatePoint.y() < 0
                || diffStatePoint.y() > 16) {
                m_reSendAction->setVisible(false);
                m_copyAction->setVisible(false);
                m_openFileAction->setVisible(false);
                m_openDirAction->setVisible(false);
                m_saveAsAction->setVisible(false);
                m_deleteMsgAction->setVisible(false);
                m_clearMsgAction->setVisible(true);

            } else {
                m_listView->selectionModel()->clear();
                return;
            }

        } else {
            m_clearMsgAction->setVisible(false);
            m_copyAction->setVisible(true);
            m_deleteMsgAction->setVisible(true);
            if (msgType == ChatMsgModel::MessageType::TextMsg) {
                m_openFileAction->setVisible(false);
                m_openDirAction->setVisible(false);
                m_saveAsAction->setVisible(false);
            } else {
                m_openFileAction->setVisible(true);
                m_openDirAction->setVisible(true);
                m_saveAsAction->setVisible(true);
            }
        }

#ifndef TEST_MODE
        m_funcMenu->exec(QCursor::pos());
#endif
        m_listView->selectionModel()->clear();
    } else {
        m_reSendAction->setVisible(false);
        m_copyAction->setVisible(false);
        m_openFileAction->setVisible(false);
        m_openDirAction->setVisible(false);
        m_saveAsAction->setVisible(false);
        m_deleteMsgAction->setVisible(false);
        m_clearMsgAction->setVisible(true);

#ifndef TEST_MODE
        m_funcMenu->exec(QCursor::pos());
#endif
        m_listView->selectionModel()->clear();
    }
}

// 获取聊天消息model
ChatMsgModel *ChatMsg::getchatMsgModel()
{
    return m_chatMsgModel;
}

// 更新好友昵称
void ChatMsg::updateFriendNickname(QString displayName)
{
    this->m_titleBar->setTitleText(displayName);

    this->setWindowTitle(displayName);
}

// 输入框文字变化
void ChatMsg::slotTextChanged()
{
    // QString textContent = m_msgEdit->toPlainText();

    // int length    = textContent.count();
    // int maxLength = MAX_SIZE;

    // 判断字符数
    // if (length > maxLength) {
    //     int position = m_msgEdit->textCursor().position();
    //     QTextCursor textCursor = m_msgEdit->textCursor();

    //     textContent.remove(position - (length - maxLength), length - maxLength);
    //     textCursor.setPosition(position - (length - maxLength));

    //     m_msgEdit->setText(textContent);
    //     m_msgEdit->setTextCursor(textCursor);
    // }
}

// 获取焦点
void ChatMsg::getFocus()
{
    if (this->isHidden()) {
        m_listView->scrollToBottom();
    }

    m_msgEdit->setFocus();

    FriendListModel::getInstance()->clearUnreadMsgNum(m_friendId);
}

// 消息发送失败
void ChatMsg::sendMsgFailed()
{
    if (m_isConnect) {
        m_isConnect = false;
#ifndef TEST_MODE
        QMessageBox::critical(this, "", tr("Message send failed, please check whether IP connection is successful!"));
#endif
    }
}

// 消息发送成功
void ChatMsg::sendMsgSuccess()
{
    m_isConnect = true;
}

// 消息接收成功
void ChatMsg::recvMsgSuccess()
{
    m_isConnect = true;

    // if (this->isActiveWindow()) {
    //     m_listView->scrollToBottom();
    // }
}

// 滚动条是否在底端
bool ChatMsg::isScrollOnBottom()
{
    if (m_listView->verticalScrollBar()->value() == m_listView->verticalScrollBar()->maximum()) {
        return true;
    }

    return false;
}

// 滚动条设为底端
void ChatMsg::setScrollToBottom()
{
    m_listView->scrollToBottom();
}

// 更新"日期短格式 时间", 保证与ukui-control-center保持一致
void ChatMsg::slotUpdateTimeFormat(QString str)
{
    m_chatMsgModel->updateDateTimeFormat(str);
}

// 隐藏窗体
void ChatMsg::slotHideChatMsg()
{
    //    this->hide();
    //    if(m_openChatSearch)
    //    {
    //        m_chartSearch->close();
    //        m_openChatSearch = false;
    //    }
}

// 更新好友信息
void ChatMsg::slotUpdateFriendInfo(int friendId)
{
    if (m_friendId == friendId) {
        // 获取好友信息
        QStandardItem *item = FriendListModel::getInstance()->getFriendById(m_friendId);
        QString usernameStr = item->data(FriendListModel::Username).toString();
        QString nicknameStr = item->data(FriendListModel::Nickname).toString();

        QString displayName = nicknameStr.isEmpty() ? usernameStr : nicknameStr;
        this->updateFriendNickname(displayName);

        m_listView->update();
    }
}

// 发送消息并添加记录
void ChatMsg::sendChatMsg(ChatMsgInfo *chatMsgInfo)
{
    chatMsgInfo->m_friendId = m_friendId;
    chatMsgInfo->m_friendUuid = m_friendUuid;
    chatMsgInfo->m_isSend = ChatMsgModel::SendType::SendMsg;
    chatMsgInfo->m_msgTime = QDateTime::currentDateTime().toString(GlobalUtils::getTimeFormat());

    if (chatMsgInfo->m_msgType == ChatMsgModel::MessageType::DirMsg) {
        chatMsgInfo->m_totalSize = GlobalUtils::getDirSize(chatMsgInfo->m_filePath);
    } else if (chatMsgInfo->m_msgType == ChatMsgModel::MessageType::FileMsg) {
        chatMsgInfo->m_totalSize = QFileInfo(chatMsgInfo->m_filePath).size();
    }

    if (chatMsgInfo->m_msgContent.isEmpty()) {
        chatMsgInfo->m_msgContent = chatMsgInfo->m_filePath.split("/").back();
    }

    // 消息内容加密
    chatMsgInfo->m_msgContent = GlobalUtils::encryptData(chatMsgInfo->m_msgContent);

    if (m_chatMsgModel->addChatMsg(chatMsgInfo)) {
        emit sigSendMsg(chatMsgInfo);
    }

    m_listView->scrollToBottom();
    m_msgEdit->setFocus();
}

// 发送文字消息
void ChatMsg::slotSendText()
{
    if (m_msgEdit->toPlainText().isEmpty()) {
        m_msgEdit->setFocus();
        return;
    }
    // 初始化埋点数据，传入应用类型，桌面应用，时间类型，点击事件
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    // 传入事件描述“对话框——发送消息”，所在界面“主界面”，上传事件
    kdk_dia_upload_default(node, "send_text_click_event", "chatWindow");
    // 释放内存
    kdk_dia_data_free(node);

    ChatMsgInfo *chatMsgInfo = new ChatMsgInfo();

    chatMsgInfo->m_msgContent = m_msgEdit->toPlainText();
    chatMsgInfo->m_msgType = ChatMsgModel::MessageType::TextMsg;

    m_msgEdit->clear();

    if (chatMsgInfo->m_msgContent.size() > 800) {
        QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + QString(".txt");
        QString filePath = GlobalUtils::getDefaultPath(true) + fileName;

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            QMessageBox::critical(this, NULL, tr("Can not write file"), QMessageBox::Yes);
        }

        QTextStream stream(&file);
        stream << chatMsgInfo->m_msgContent;
        file.close();

        chatMsgInfo->m_msgContent = fileName;
        chatMsgInfo->m_filePath = filePath;
        chatMsgInfo->m_msgType = ChatMsgModel::MessageType::FileMsg;
    }

    sendChatMsg(chatMsgInfo);
}

// 键盘响应事件
void ChatMsg::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        // 用户手册
        kdk::UserManual userManual;
        if (!userManual.callUserManual("messages")) {
            qCritical() << "user manual call fail!";
        }
    }
    // qDebug() << "keyPressEvent";
    // if (event->key() == Qt::Key_Return && ((event->modifiers() & Qt::ControlModifier) || (event->modifiers() &
    // Qt::AltModifier))) {
    //     m_msgEdit->textCursor().insertBlock();

    // } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    // 	slotSendText();
    // }

    return QWidget::keyPressEvent(event);
}

// 绘制事件
void ChatMsg::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // 反锯齿;

    m_titleBar->update();
}

// 事件过滤器
bool ChatMsg::eventFilter(QObject *watch, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
        if (kEvent->key() == Qt::Key_T && (kEvent->modifiers() & Qt::AltModifier)) {
            m_screenshotBtn->click();
            return true;
        }
    }
    if (watch == m_msgEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *kEvent = static_cast<QKeyEvent *>(event);
            if (kEvent->key() == Qt::Key_Return
                && ((kEvent->modifiers() & Qt::ControlModifier) || (kEvent->modifiers() & Qt::AltModifier))) {
                m_msgEdit->textCursor().insertBlock();
                m_msgEdit->moveCursor(QTextCursor::NoMove);

            } else if (kEvent->key() == Qt::Key_Enter || kEvent->key() == Qt::Key_Return) {
                slotSendText();
                return true;
            }
        }
    }
    if (event->type() == QEvent::FocusIn) {
        // qDebug() << "get focus";
        this->getFocus();
    }

    return QObject::eventFilter(watch, event);
}

// 鼠标拖拽事件
void ChatMsg::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

// 鼠标放下事件
void ChatMsg::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (!mimeData->hasUrls()) {
        return;
    }

    QList<QUrl> urlList = mimeData->urls();

    for (int i = 0; i < urlList.size(); i++) {
        QString dropFileName = urlList.at(i).toLocalFile();
        if (dropFileName.isEmpty()) {
            continue;
        }
        QFileInfo fileInfo(dropFileName);

        if (fileInfo.exists()) {
            if (fileInfo.isFile()) {
                ChatMsgInfo *chatMsgInfo = new ChatMsgInfo();

                chatMsgInfo->m_msgContent = (dropFileName.split("/")).back();
                chatMsgInfo->m_filePath = dropFileName;
                chatMsgInfo->m_msgType = ChatMsgModel::MessageType::FileMsg;

                sendChatMsg(chatMsgInfo);

                usleep(50000);
            } else {
                ChatMsgInfo *chatMsgInfo = new ChatMsgInfo();

                chatMsgInfo->m_msgContent = (dropFileName.split("/")).back();
                chatMsgInfo->m_filePath = dropFileName;
                chatMsgInfo->m_msgType = ChatMsgModel::MessageType::DirMsg;

                sendChatMsg(chatMsgInfo);

                usleep(50000);
            }
        }
    }
}

// 窗口关闭事件
void ChatMsg::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    this->setAttribute(Qt::WA_QuitOnClose);
    event->ignore();
    this->hide();

    return;
}

// 列表左键点击
void ChatMsg::msgListClicked()
{
    QPoint pressPoint = m_listView->mapFromGlobal(QCursor::pos());

    QPoint statePoint;
    statePoint.setX(ChatMsgDelegate::borderPoint.x() - 26);
    statePoint.setY(ChatMsgDelegate::borderPoint.y() + (ChatMsgDelegate::borderSize.height() - 16) / 2);

    // qDebug() << "pressPoint" << pressPoint.x() << pressPoint.y();
    // qDebug() << "statePoint" << statePoint.x() << statePoint.y();
    // qDebug() << "ChatMsgDelegate::borderPoint" << ChatMsgDelegate::borderPoint.x() <<
    // ChatMsgDelegate::borderPoint.y();

    QPoint diffPoint = QPoint(pressPoint.x() - statePoint.x(), pressPoint.y() - statePoint.y());

    // 点击区域在发送状态内
    if (diffPoint.x() > 0 && diffPoint.x() < 16 && diffPoint.y() > 0 && diffPoint.y() < 16) {

        // 获取消息数据
        QModelIndex msgIndex = m_listView->currentIndex();
        int msgId = msgIndex.data(ChatMsgModel::MsgId).toInt();
        int msgType = msgIndex.data(ChatMsgModel::MsgType).toInt();
        int msgState = msgIndex.data(ChatMsgModel::SendState).toInt();

        if (msgState == ChatMsgModel::MesssageState::SendFailed) {
            slotResendMsg();

        } else if (msgState == ChatMsgModel::MesssageState::SendDefault) {
            if (msgType == ChatMsgModel::MessageType::FileMsg || msgType == ChatMsgModel::MessageType::DirMsg) {
                qDebug() << "cancel send msg";

                emit sigCancelMsg(m_friendUuid, msgId);

                ChatMsgInfo *updateMsgInfo = new ChatMsgInfo();
                updateMsgInfo->m_msgId = msgId;
                updateMsgInfo->m_sendState = ChatMsgModel::MesssageState::SendFailed;

                if (m_chatMsgModel->updateMsgState(updateMsgInfo)) {
                    delete updateMsgInfo;
                }
            }
        }
    }

    this->getFocus();
}

// 检查文件和文件夹是否存在
bool ChatMsg::checkFileExist(QString filePath)
{
    if (!QFileInfo::exists(filePath)) {
#ifndef TEST_MODE
        QMessageBox::critical(this, "", tr("No such file or directory!"));
#endif
        return false;
    }

    return true;
}

// 重新发送
void ChatMsg::slotResendMsg()
{
    /* sdk功能点：重新发送 */
    GlobalSizeData::SDKPointResend();

    QModelIndex msgIndex = m_listView->currentIndex();

    // 获取消息数据
    QString msgContent = msgIndex.data(ChatMsgModel::MsgContent).toString();
    QString filePath = msgIndex.data(ChatMsgModel::FilePath).toString();
    int msgType = msgIndex.data(ChatMsgModel::MsgType).toInt();
    int msgId = msgIndex.data(ChatMsgModel::MsgId).toInt();


    if (msgType == ChatMsgModel::MessageType::FileMsg || msgType == ChatMsgModel::MessageType::DirMsg) {

        if (!checkFileExist(filePath)) {
            return;
        }
    }

    ChatMsgInfo *chatMsgInfo = new ChatMsgInfo();

    chatMsgInfo->m_msgContent = msgContent;
    chatMsgInfo->m_filePath = filePath;
    chatMsgInfo->m_msgType = msgType;

    m_chatMsgModel->delChatMsg(msgId, msgIndex.row());

    sendChatMsg(chatMsgInfo);
}

// 复制消息
void ChatMsg::slotCopyMsg()
{
    /* sdk功能点：复制 */
    GlobalSizeData::SDKPointCopy();
    QModelIndex msgIndex = m_listView->currentIndex();

    // 获取消息数据
    QString msgContent = msgIndex.data(ChatMsgModel::MsgContent).toString();
    QString filePath = msgIndex.data(ChatMsgModel::FilePath).toString();
    int msgType = msgIndex.data(ChatMsgModel::MsgType).toInt();

    if (msgType == ChatMsgModel::MessageType::TextMsg) {
        m_clipboard->setText(msgContent);

    } else if (msgType == ChatMsgModel::MessageType::FileMsg || msgType == ChatMsgModel::MessageType::DirMsg) {

        if (!checkFileExist(filePath)) {
            return;
        }

        QList<QUrl> copyFileList;
        QUrl fileUrl = QUrl::fromLocalFile(filePath);

        if (fileUrl.isValid()) {
            copyFileList.push_back(fileUrl);
        } else {
            return;
        }

        QMimeData *fileData = new QMimeData();
        fileData->setObjectName("kylin-ipmsg_chatmsg_QMimeData_fileData");
        fileData->setUrls(copyFileList);

        m_clipboard->setMimeData(fileData);
    }
}

// 使用默认应用打开文件或文件夹
void ChatMsg::slotOpenFile()
{
    /* sdk功能点：打开 */
    GlobalSizeData::SDKPointOpen();

    QModelIndex msgIndex = m_listView->currentIndex();

    // 获取消息数据
    QString filePath = msgIndex.data(ChatMsgModel::FilePath).toString();

    if (!checkFileExist(filePath)) {
        return;
    }

    filePath = QString("file://") + filePath;
//    bool is_open = QDesktopServices::openUrl(QUrl(filePath, QUrl::TolerantMode));

    qDebug() << "open by xdg";
    QString cmd = QString("/usr/bin/xdg-open \"%1\"").arg(filePath);
    qDebug() << "xdg open cmd : " << cmd;
    QProcess::startDetached(cmd);
}

// 打开所在目录
void ChatMsg::slotOpenDir()
{
    /* sdk功能点：打开目录 */
    GlobalSizeData::SDKPointOpenDir();

    QModelIndex msgIndex = m_listView->currentIndex();

    // 获取消息数据
    QString filePath = msgIndex.data(ChatMsgModel::FilePath).toString();

    if (!checkFileExist(filePath)) {
        return;
    }

    QString pathCmd = "\"" + filePath + "\"";
    QString cmd = QString("/usr/bin/peony --show-items " + pathCmd);

    QProcess::startDetached(cmd);
}

#include <peony-qt/file-operation-utils.h>

// 另存为
void ChatMsg::slotSaveAs()
{
    /* sdk功能点：另存为 */
    GlobalSizeData::SDKPointSaveAs();

    QModelIndex msgIndex = m_listView->currentIndex();

    // 获取消息数据
    QString filePath = msgIndex.data(ChatMsgModel::FilePath).toString();
    QString msgContent = msgIndex.data(ChatMsgModel::MsgContent).toString();

    if (!checkFileExist(filePath)) {
        return;
    }

    if (msgContent.isEmpty()) {
        msgContent = (filePath.split("/")).back();
    }

#ifndef TEST_MODE
    QString savePath = QDir::toNativeSeparators(
        QFileDialog::getExistingDirectory(this, tr("Save As"), IniSettings::getInstance()->getFilePath()));
    if (savePath.isEmpty()) {
        return;
    }

    // 调用peony接口
    QStringList filePathList = QStringList(QString("file://" + filePath));
    QString destPathUri = QString("file://" + savePath);

    Peony::FileOperationUtils::copy(filePathList, destPathUri, false);
#endif
}

// 删除消息
void ChatMsg::slotDeleteMsg()
{
    /* SDK功能点：删除记录 */
    GlobalSizeData::SDKPointDeleteRecord();

#ifndef TEST_MODE
    QMessageBox *msg = new QMessageBox(this);
    msg->setAccessibleName("kylin-ipmsg_chatmsg_QMessageBox_msg");
    msg->setWindowTitle(tr(""));
    msg->setText(tr("Delete the currently selected message?"));
    msg->setIcon(QMessageBox::Question);
    msg->addButton(tr("No"), QMessageBox::RejectRole);
    QPushButton *sureBtn = msg->addButton(tr("Yes"), QMessageBox::AcceptRole);
    msg->setDefaultButton(sureBtn);
    msg->exec();

    if ((QPushButton *)msg->clickedButton() != sureBtn) {
        return;
    }
#endif

    QModelIndex msgIndex = m_listView->currentIndex();

    // 获取消息数据
    int msgId = msgIndex.data(ChatMsgModel::MsgId).toInt();

    m_chatMsgModel->delChatMsg(msgId, msgIndex.row());
    m_msgEdit->setFocus();
    return;
}

// 清空消息
void ChatMsg::slotClearMsg()
{
    /* SDK功能点：清空聊天记录 */
    GlobalSizeData::SDKPointClearChatRecord();

#ifndef TEST_MODE
    QMessageBox *msg = new QMessageBox(this);
    msg->setAccessibleName("kylin-ipmsg_chatmsg_QMessageBox_msg");
    msg->setWindowTitle(tr(""));
    msg->setText(tr("Clear all current messages?"));
    msg->setIcon(QMessageBox::Question);
    msg->addButton(tr("No"), QMessageBox::RejectRole);
    QPushButton *sureBtn = msg->addButton(tr("Yes"), QMessageBox::AcceptRole);
    msg->setDefaultButton(sureBtn);
    msg->exec();

    if ((QPushButton *)msg->clickedButton() != sureBtn) {
        return;
    }
#endif
    m_chatMsgModel->clearChatMsg();
    m_msgEdit->setFocus();
    return;
}

// 发送文件
void ChatMsg::slotSendFile()
{
    /* SDK功能点：发送文件 */
    GlobalSizeData::SDKPointSendFiles();

#ifndef TEST_MODE
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setAccessibleName("kylin-ipmsg_chatmsg_QFileDialog_fileDialog");

    fileDialog->setWindowTitle(tr("Send Files"));
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    if (fileDialog->exec() == QFileDialog::Accepted) {
        QStringList selectFileList = fileDialog->selectedFiles();

        for (int i = 0; i < selectFileList.size(); i++) {
            ChatMsgInfo *chatMsgInfo = new ChatMsgInfo();

            chatMsgInfo->m_msgContent = (selectFileList[i].split("/")).back();
            chatMsgInfo->m_filePath = selectFileList[i];
            chatMsgInfo->m_msgType = ChatMsgModel::MessageType::FileMsg;

            sendChatMsg(chatMsgInfo);

            usleep(50000);
        }
    }
#endif
}

// 发送文件夹
void ChatMsg::slotSendDir()
{
    /* SDK功能点：发送目录 */
    GlobalSizeData::SDKPointSendDir();

#ifndef TEST_MODE
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setAccessibleName("kylin-ipmsg_chatmsg_QFileDialog_fileDialog");

    fileDialog->setWindowTitle(tr("Send Folders"));
    fileDialog->setFileMode(QFileDialog::Directory);
    // fileDialog->setFilter(QDir::Dirs);
    fileDialog->setNameFilter(tr("folder"));

    QListView *listview = fileDialog->findChild<QListView *>("listView");
    if (listview) {
        listview->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    QTreeView *treeview = fileDialog->findChild<QTreeView *>();
    if (treeview) {
        treeview->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

    if (fileDialog->exec() == QFileDialog::Accepted) {
        QStringList selectFileList = fileDialog->selectedFiles();

        for (int i = 0; i < selectFileList.size(); i++) {
            ChatMsgInfo *chatMsgInfo = new ChatMsgInfo();

            chatMsgInfo->m_msgContent = (selectFileList[i].split("/")).back();
            chatMsgInfo->m_filePath = selectFileList[i];
            chatMsgInfo->m_msgType = ChatMsgModel::MessageType::DirMsg;

            sendChatMsg(chatMsgInfo);

            usleep(50000);
        }
    }
#endif
}

// 截图并发送
void ChatMsg::slotScreenShot()
{
    /* SDK功能点：发送截屏 */
    GlobalSizeData::SDKPointSendScreenshot();
    
#ifndef TEST_MODE
    const QMimeData *oldClipData = m_clipboard->mimeData();

    if (oldClipData->hasImage()) {
        m_oldClipVar = oldClipData->imageData();
        m_hasOldImage = true;
    } else {
        m_hasOldImage = false;
    }

    QString cmd = "/usr/bin/kylin-screenshot gui";
    QProcess::execute(cmd);

    m_startShot = true;
#endif
}

// 获取截图
void ChatMsg::getShotImage()
{
#ifndef TEST_MODE
    const QMimeData *newData = m_clipboard->mimeData();

    if (newData->hasImage()) {
        if (m_hasOldImage && (m_oldClipVar == newData->imageData())) {
            return;
        }

        QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + QString(".png");
        QString filePath = GlobalUtils::getDefaultPath(true) + fileName;

        QImage img = qvariant_cast<QImage>(newData->imageData());
        img.save(filePath);

        ChatMsgInfo *chatMsgInfo = new ChatMsgInfo();

        chatMsgInfo->m_msgContent = fileName;
        chatMsgInfo->m_filePath = filePath;
        chatMsgInfo->m_msgType = ChatMsgModel::MessageType::FileMsg;

        sendChatMsg(chatMsgInfo);
    }
#endif
}

/* 查看聊天记录 */
void ChatMsg::slotHistoryMsg()
{
    if (this->m_chartSearch == nullptr) {
        m_chartSearch = new ChatSearch(this);
        m_chartSearch->setAccessibleName("kylin-ipmsg_chatmsg_ChatSearch_mchartSearch");
        m_chartSearch->show();
        kabase::WindowManage::setMiddleOfScreen(m_chartSearch);
        m_chartSearch->raise();
        m_chartSearch->activateWindow();
        m_chartSearch->getFocus();
    } else {
        m_chartSearch->show();
        kabase::WindowManage::setMiddleOfScreen(m_chartSearch);
        m_chartSearch->raise();
        m_chartSearch->activateWindow();
        m_chartSearch->getFocus();
    }

    connect(this->m_chartSearch, &ChatSearch::sigCloseSearchWid, this, &ChatMsg::slotDeleSearch);
    return;
}

void ChatMsg::slotDeleSearch()
{
    if (this->m_chartSearch != nullptr) {
        delete this->m_chartSearch;
        this->m_chartSearch = nullptr;
    }

    return;
}

void ChatMsg::changeBackground()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
        m_listView->setStyleSheet("QListView{background:#F5F5F5;}");
    } else {
        m_listView->setStyleSheet("QListView{background:#262626;}");
    }
    return;
}

void ChatMsg::changeSizeHScreenMode()
{
    int w = GlobalData::getInstance()->m_currentSize.first;
    int h = GlobalData::getInstance()->m_currentSize.second;
    this->setFixedSize(QSize(w, h));
    m_fileBtn->setFixedSize(BTN_MAX_SIZE);
    m_folderBtn->setFixedSize(BTN_MAX_SIZE);
    m_historyBtn->setFixedSize(BTN_MAX_SIZE);
    m_screenshotBtn->setFixedSize(BTN_MAX_SIZE);
    m_titleBar->slotChangeHScreenMode();
    
    changeListDelegate();

    return;
}

void ChatMsg::changeSizeVScreenMode()
{
    int w = GlobalData::getInstance()->m_currentSize.first;
    int h = GlobalData::getInstance()->m_currentSize.second;
    this->setFixedSize(QSize(w, h));
    m_fileBtn->setFixedSize(BTN_MAX_SIZE);
    m_folderBtn->setFixedSize(BTN_MAX_SIZE);
    m_historyBtn->setFixedSize(BTN_MAX_SIZE);
    m_screenshotBtn->setFixedSize(BTN_MAX_SIZE);
    m_titleBar->slotChangeVScreenMode();
    
    changeListDelegate();

    return;
}

void ChatMsg::changeSizePCMode()
{
    this->setFixedSize(QSize(700, 625));
    m_fileBtn->setFixedSize(BTN_SIZE);
    m_folderBtn->setFixedSize(BTN_SIZE);
    m_historyBtn->setFixedSize(BTN_SIZE);
    m_screenshotBtn->setFixedSize(BTN_SIZE);
    m_titleBar->slotChangePCMode();

    changeListDelegate();
    
    return;
}

void ChatMsg::changeListDelegate()
{
    delete m_chatMsgDelegate;
    m_chatMsgDelegate = new ChatMsgDelegate();
    m_listView->setItemDelegate(m_chatMsgDelegate);
    return;
}

void ChatMsg::slotEnableSureBtn()
{
    QString content = m_msgEdit->toPlainText();
    QString trimmedContent = content.trimmed();

    m_sendMsgBtn->setEnabled(!trimmedContent.isEmpty());
}

