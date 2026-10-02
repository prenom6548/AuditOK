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

#ifndef CHATMSG_H
#define CHATMSG_H

#define MAX_SIZE 800 // 输入框最大字符数
#define SCREEN_SHOT_GSETTING_PATH "org.ukui.screenshot"

#include <QObject>
#include <QWidget>
#include <QListView>
#include <QScrollBar>
#include <QDebug>
#include <QCursor>
#include <QEvent>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QGSettings>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QClipboard>
#include <QList>
#include <QUrl>
#include <QMimeData>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>

#include "view/titlebar/titlebar.h"
#include "global/utils/inisetting.h"
#include "view/chatsearch/chat_search.h"
#include "chatmsgdelegate.h"
#include "view/chatsearch/chat_search_filter.h"
class ChatSearch;
class TestChatMsg;

class ChatMsg : public QWidget
{
    Q_OBJECT
    friend class TestChatMsg;

public:
    explicit ChatMsg(int friendId, QString friendUuid, QWidget *parent = nullptr);
    ~ChatMsg();

    // 初始化数据
    void setInitData(int friendId, QString friendUuid);

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    // 初始化并监听gsetting
    void initGsetting();

    // 设置信号绑定
    void setSignalConn();

    // 获取聊天消息model
    ChatMsgModel *getchatMsgModel();

    // 更新好友昵称
    void updateFriendNickname(QString displayName);

    // 获取焦点
    void getFocus();

    // 消息发送失败
    void sendMsgFailed();

    // 消息发送成功
    void sendMsgSuccess();

    // 消息接收成功
    void recvMsgSuccess();

    // 滚动条是否在底端
    bool isScrollOnBottom();

    // 滚动条设为底端
    void setScrollToBottom();

    // 好友用户名
    QString m_friendUsername;

    // 好友昵称
    QString m_friendNickname;

    // 好友头像
    QString m_friendAvatar;

    ChatMsgModel *m_chatMsgModel;

    ChatSearch *m_chartSearch = nullptr;

    void changeSizeHScreenMode();
    void changeSizeVScreenMode();
    void changeSizePCMode();

signals:
    void sigSendMsg(ChatMsgInfo *chatMsgInfo);
    void sigCancelMsg(QString friendUuid, int msgId);

private slots:
    void slotUpdateTimeFormat(QString);

protected:
    // 键盘响应事件
    void keyPressEvent(QKeyEvent *event);

    // 绘制事件
    void paintEvent(QPaintEvent *event);

    // 事件过滤器
    bool eventFilter(QObject *watch, QEvent *event);

    // 鼠标拖拽事件
    void dragEnterEvent(QDragEnterEvent *event);

    // 鼠标放下事件
    void dropEvent(QDropEvent *event);

    // 窗口关闭事件
    void closeEvent(QCloseEvent *event);

private:
    // 麒麟截图
    QGSettings *m_screenShotSetting = nullptr;

    bool m_startShot;
    bool m_hasOldImage;
    QVariant m_oldClipVar;

    // 标题栏
    TitleBar *m_titleBar;

    // 聊天记录listview
    QListView *m_listView;
    ChatMsgDelegate *m_chatMsgDelegate;

    // 发送区域
    QWidget *m_sendAreaWid;

    // 选择文件按钮
    QPushButton *m_fileBtn;
    // 选择文件夹按钮
    QPushButton *m_folderBtn;
    // 截图按钮
    QPushButton *m_screenshotBtn;
    // 历史记录按钮
    QPushButton *m_historyBtn;
    // 发送按钮
    QPushButton *m_sendMsgBtn;
    // 消息编辑框
    QTextEdit *m_msgEdit;

    // 总体布局
    QVBoxLayout *m_mainLayout;
    // 发送区域布局
    QVBoxLayout *m_sendAreaLayout;
    // 功能按钮布局
    QHBoxLayout *m_funcBtnLayout;
    // 发送按钮布局
    QHBoxLayout *m_sendBtnLayout;
    // 编辑区域布局
    QHBoxLayout *m_editLayout;

    // 好友ID
    int m_friendId;

    // 好友UUID
    QString m_friendUuid;


    // 右键菜单
    QMenu *m_funcMenu;
    QAction *m_reSendAction;
    QAction *m_copyAction;
    QAction *m_openFileAction;
    QAction *m_openDirAction;
    QAction *m_saveAsAction;
    QAction *m_deleteMsgAction;
    QAction *m_clearMsgAction;

    // 剪切板
    QClipboard *m_clipboard;

    // 是否连接成功
    bool m_isConnect;

    // 提示弹窗
    QMessageBox *m_messageBox;

    // 隐藏窗体
    void slotHideChatMsg();

    // 更新好友信息
    void slotUpdateFriendInfo(int friendId);

    // 输入框文字变化
    void slotTextChanged();

    // 发送消息并添加记录
    void sendChatMsg(ChatMsgInfo *chatMsgInfo);

    // 发送文字消息
    void slotSendText();

    // 显示右键菜单
    void showContextMenu(const QPoint &);

    // 列表左键点击
    void msgListClicked();

    // 检查文件和文件夹是否存在
    bool checkFileExist(QString filePath);

    // 重新发送
    void slotResendMsg();

    // 复制消息
    void slotCopyMsg();

    // 使用默认应用打开文件或文件夹
    void slotOpenFile();

    // 打开所在目录
    void slotOpenDir();

    // 另存为
    void slotSaveAs();

    // 删除消息
    void slotDeleteMsg();

    // 清空消息
    void slotClearMsg();

    // 发送文件或文件夹
    void slotSendFile();
    void slotSendDir();

    // 截图并发送
    void slotScreenShot();

    // 获取截图
    void getShotImage();

    // 查看聊天记录
    void slotHistoryMsg();

    void slotDeleSearch();
    // 改变主题
    void changeBackground();

    void changeListDelegate();

    //确认按钮激活
    void slotEnableSureBtn();
};


#endif // CHATMSG_H
