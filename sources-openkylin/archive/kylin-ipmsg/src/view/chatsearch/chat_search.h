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

#ifndef __CHAT_SEARCH_H__
#define __CHAT_SEARCH_H__

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
#include <QList>
#include <QLineEdit>
#include <QMouseEvent>
#include <QListWidget>
#include <QCheckBox>
#include <QRegExp>
#include <KWindowSystem>

#include "view/titlebar/titlebar.h"
#include "view/chatmsg/chatmsg.h"
#include "global/utils/inisetting.h"
#include "chat_search_delegate.h"
#include "chat_search_filter.h"

#include "ksearchlineedit.h"
using namespace kdk;

#define MAX_SIZE 800 // 输入框最大字符数

enum InfoType {
    All = 0,
    Image,
    File,
    Link,
};

class ChatMsg;
class ChatSearchDelegate;

class ChatSearch : public QWidget
{
    Q_OBJECT
    friend class TestChatSearch;
    friend class TestChatSearchDelegate;

public:
    ChatSearch(ChatMsg *m);
    ~ChatSearch();

    // 单例，初始化返回指针
    static ChatSearch *getInstance(ChatMsg *chat = nullptr);

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    // 初始化并监听gsetting
    void initGsetting();

    // 设置信号绑定
    void setSignalConn();

    // 获取焦点
    void getFocus();

    // 滚动条是否在底端
    bool isScrollOnBottom();

    // 滚动条设为底端
    void setScrollToBottom();

    // 监听聊天窗口是否关闭
    void setListenChatMsg();

    //创建删除时复选框
    void showDeleMsgCheckbox();

    //还原文本框初始状态
    void changeBtnStyle(QPushButton *btn1, QPushButton *btn2, QPushButton *btn3, QPushButton *btn4);

    // 改变控件的样式
    void changeTheme();

    bool getDeleteState();

    ChatMsg *m_chat;

    ChatSearchFilter *m_chatFiler;

    QString m_searchText; //向代理传关键词

    InfoType m_type = InfoType::All; //当前类型

protected:
    //过滤器
    bool eventFilter(QObject *watch, QEvent *e);

    void keyPressEvent(QKeyEvent *event);

    // 窗口关闭事件
    void closeEvent(QCloseEvent *event);

private:
    // 标题栏
    TitleBar *m_titleBar;

    // 聊天记录listview
    QListView *m_listView;
    ChatSearchDelegate *m_chatSearchDelegate;

    // 搜索区域
    QWidget *m_searchAreaWid;
    // 删除区域
    QWidget *m_deleAreaWid;
    // 选择列区域
    QLabel *m_chooseDeleWid;
    //选择删除列表
    QListWidget *m_chooseListWid = nullptr;


    // sdk 搜索输入框
    KSearchLineEdit *m_searchLineEdit;

    // 选择全部按钮
    QPushButton *m_allBtn;
    // 选择文件按钮
    QPushButton *m_fileBtn;
    // 图片/视频按钮
    QPushButton *m_ImageBtn;
    // 链接按钮
    QPushButton *m_linkBtn;
    // 删除菜单按钮
    QToolButton *m_deleMenuBtn;
    // 删除界面取消按钮
    QPushButton *m_cancelDeleBtn;
    // 删除界面确定按钮
    QPushButton *m_sureDeleBtn;

    // 总体布局
    QVBoxLayout *m_mainLayout;
    // 搜索区域纵向布局
    QVBoxLayout *m_searchAreaVLayout;
    // 搜索区域横向布局
    QHBoxLayout *m_searchAreaHLayout;
    // 搜索控件布局
    QHBoxLayout *m_searchLayout;
    // 总体功能按钮布局
    QHBoxLayout *m_funcBtnLayout;
    // 删除功能按钮布局
    QHBoxLayout *m_deleleBtnLayout;
    // 删除布局
    QHBoxLayout *m_deleleAreaLayout;

    //组建的宽和高
    int w = 468;
    int h = 30;

    // 删除菜单
    QMenu *m_deleFuncMenu;
    QAction *m_deleteMsgAction;
    QAction *m_clearMsgAction;

    // 右键菜单
    QMenu *m_funcMenu;
    QAction *m_oneDeleAction;
    QAction *m_chooseDeleAction;
    QAction *m_openFileAction;
    QAction *m_openDirAction;

    // 提示弹窗
    QMessageBox *m_messageBox;

    // 判断是否选择聊天记录进行删除，key = id，value = 行号
    QHash<int, int> m_chooseMsgHash;

    bool m_isDeleteNow = false;

    // 将选择的批量删除数据放入hash表中进行存储
    void recordChooseDeleteMsg(int id,int row);

    // 点击消息改变消息状态
    void clickMsgToChangeState(int msgId);

    // 清除批量删除的图标（当取消批量删除时，在批量删除界面直接关闭当前窗口）
    void clearCheckBoxIcon();

    void changeBtnStyle(QPushButton* btn, QFont font, QString text, int width); // 改变内容是按钮内容样式

signals:
    void sigCloseSearchWid();

public slots:

    //控制两个list一起滚动
    void onSliderMoved(int pos);

    //选择全部信息
    void showAllInfo();

    //选择文件类型的信息
    void showFileInfo();

    //选择图片/视频类型的信息
    void showImageInfo();

    //选择链接类型的信息
    void showLinkInfo();

    // 根据字符串过滤好友
    void filterFriendByReg(QString regStr);

    // 显示右键菜单
    void showContextMenu(const QPoint &);

    // 检查文件和文件夹是否存在
    bool checkFileExist(QString filePath);

    // 使用默认应用打开文件或文件夹
    void slotOpenFile();

    // 打开所在目录
    void slotOpenDir();

    // 点击删除的菜单按钮
    void showDeleMenu();

    // 批量删除消息
    void showChooseMsg();

    //选择要删除的信息
    void slotChooseMsg(const QModelIndex &);

    // 删除消息
    void slotDeleteMsg();

    // 清空消息
    void slotClearMsg();

    // 取消删除消息
    void slotCancelClick();

    // 确定删除消息
    void slotSureClick();

    void changeSizeHScreenMode();
    void changeSizeVScreenMode();
    void changeSizePCMode();
};

#endif
