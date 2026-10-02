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

#include "chat_search.h"
#include "global/utils/globalutils.h"
#include "view/common/globalsizedata.h"
#include "global/utils/global_data.h"
#include "controller/control.h"
/* 适配kySDK */
#include "windowmanage.hpp"
#include <gsettingmonitor.h>

#include <QApplication>
#include <QDesktopWidget>
#include "ukuiwindowhelper.h"

const QSize FUNCTION_BTN_MAX_SIZE = QSize(95, 48);
const QSize FUNCTION_BTN_SIZE = QSize(80, 36);
const QSize FUNCTION_IMAGE_BTN_SIZE = QSize(100, 36);
const QSize FUNCTION_IMAGE_BTN_MAX_SIZE = QSize(115, 48);
const QSize FUNCTION_MORE_BTN_SIZE = QSize(36, 36);
const QSize FUNCTION_MORE_BTN_MAX_SIZE = QSize(48, 48);
const QSize FUNCTION_DELETE_BTN_SIZE = QSize(96, 36);
const QSize FUNCTION_DELETE_BTN_MAX_SIZE = QSize(110, 48);
const int LENGTH = 25;

ChatSearch::ChatSearch(ChatMsg *p)
{
    this->m_chat = p;

    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();

    // 设置信号绑定
    setSignalConn();
    
    // 初始化并监听gsetting
    initGsetting();
}

ChatSearch::~ChatSearch()
{
    // 析构时查看界面是否处于批量删除状态
    clearCheckBoxIcon();
}

// 单例，初始化返回指针
ChatSearch *ChatSearch::getInstance(ChatMsg *chat)
{
    static ChatSearch *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new ChatSearch(chat);
            instance->setAccessibleName("kylin-ipmsg_ChatSearch_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new ChatSearch(chat);
        instance->setAccessibleName("kylin-ipmsg_ChatSearch_instance");
#endif
    }

    return instance;
}

// 初始化组件
void ChatSearch::setWidgetUi()
{

    this->setWindowTitle(tr("Chat content"));

    // 初始化组件和布局
    m_titleBar = new TitleBar(false, false, false);
    m_searchAreaWid = new QWidget(this);
    m_chooseDeleWid = new QLabel(this);
    m_searchLineEdit = new KSearchLineEdit(this);
    m_allBtn = new QPushButton(this);
    m_fileBtn = new QPushButton(this);
    m_ImageBtn = new QPushButton(this);
    m_linkBtn = new QPushButton(this);
    m_deleMenuBtn = new QToolButton(this);
    m_cancelDeleBtn = new QPushButton(this);
    m_sureDeleBtn = new QPushButton(this);
    m_listView = new QListView(this);
    m_mainLayout = new QVBoxLayout();
    m_searchAreaVLayout = new QVBoxLayout();
    m_searchAreaHLayout = new QHBoxLayout(m_searchAreaWid);
    m_funcBtnLayout = new QHBoxLayout();
    m_deleleBtnLayout = new QHBoxLayout();
    m_deleleAreaLayout = new QHBoxLayout();
    m_deleFuncMenu = new QMenu(this);
    m_funcMenu = new QMenu(this);
    //设置objectname
    m_titleBar->setAccessibleName("kylin-ipmsg_chatsearch_TitleBar_mtitleBar");
    m_searchAreaWid->setAccessibleName("kylin-ipmsg_chatsearch_QWidget_msearchAreaWid");
    m_chooseDeleWid->setAccessibleName("kylin-ipmsg_chatsearch_QLabel_mchooseDeleWid");
    m_searchLineEdit->setAccessibleName("kylin-ipmsg_chatsearch_KSearchLineEdit_msearchLineEdit");
    m_allBtn->setAccessibleName("kylin-ipmsg_chatsearch_QPushButton_mallBtn");
    m_fileBtn->setAccessibleName("kylin-ipmsg_chatsearch_QPushButton_mfileBtn");
    m_ImageBtn->setAccessibleName("kylin-ipmsg_chatsearch_QPushButton_mImageBtn");
    m_linkBtn->setAccessibleName("kylin-ipmsg_chatsearch_QPushButton_mlinkBtn");
    m_deleMenuBtn->setAccessibleName("kylin-ipmsg_chatsearch_QToolButton_mdeleMenuBtn");
    m_cancelDeleBtn->setAccessibleName("kylin-ipmsg_chatsearch_QPushButton_mcancelDeleBtn");
    m_sureDeleBtn->setAccessibleName("kylin-ipmsg_chatsearch_QPushButton_msureDeleBtn");
    m_listView->setAccessibleName("kylin-ipmsg_chatsearch_QListView_mlistView");
    m_mainLayout->setObjectName("kylin-ipmsg_chatsearch_QVBoxLayout_mmainLayout");
    m_searchAreaVLayout->setObjectName("kylin-ipmsg_chatsearch_QVBoxLayout_msearchAreaHLayout");
    m_searchAreaHLayout->setObjectName("kylin-ipmsg_chatsearch_QHBoxLayout_msearchAreaHLayout");
    m_funcBtnLayout->setObjectName("kylin-ipmsg_chatsearch_QHBoxLayout_mfuncBtnLayout");
    m_deleleBtnLayout->setObjectName("kylin-ipmsg_chatsearch_QHBoxLayout_mdeleleBtnLayout");
    m_deleleAreaLayout->setObjectName("kylin-ipmsg_chatsearch_QHBoxLayout_mdeleleAreaLayout");
    m_deleFuncMenu->setAccessibleName("kylin-ipmsg_chatsearch_QMenu_mdeleFuncMenu");
    m_funcMenu->setAccessibleName("kylin-ipmsg_chatsearch_QMenu_mfuncMenu");
    // 将组件放入布局
    this->installEventFilter(this); //安装过滤器，进行函数过滤,使图标进行移动
    m_allBtn->installEventFilter(this);
    m_fileBtn->installEventFilter(this);
    m_ImageBtn->installEventFilter(this);
    m_linkBtn->installEventFilter(this);
    m_listView->installEventFilter(this);

    // 功能按钮布局
    m_funcBtnLayout->addSpacing(0);
    m_funcBtnLayout->addWidget(m_allBtn);
    m_funcBtnLayout->addWidget(m_fileBtn);
    m_funcBtnLayout->addWidget(m_ImageBtn);
    m_funcBtnLayout->addWidget(m_linkBtn);
    m_funcBtnLayout->addStretch();

    m_funcBtnLayout->addWidget(m_cancelDeleBtn);
    m_funcBtnLayout->addSpacing(16);
    m_funcBtnLayout->addWidget(m_sureDeleBtn);
    m_funcBtnLayout->addWidget(m_deleMenuBtn);
    m_funcBtnLayout->setMargin(0);
    m_funcBtnLayout->setSpacing(1);

    // 搜索区域纵向布局
    m_searchAreaVLayout->addWidget(m_searchLineEdit);
    m_searchAreaVLayout->addSpacing(16);
    m_searchAreaVLayout->addLayout(m_funcBtnLayout);
    m_searchAreaVLayout->setMargin(0);
    m_searchAreaVLayout->setSpacing(0);

    // 搜索区域横向布局
    m_searchAreaHLayout->addStretch();
    m_searchAreaHLayout->addLayout(m_searchAreaVLayout);
    m_searchAreaHLayout->addStretch();

    //信息展示区域（删除显现）
    m_deleleAreaLayout->addWidget(m_chooseDeleWid);
    m_deleleAreaLayout->addWidget(m_listView);
    m_deleleAreaLayout->setMargin(0);
    m_deleleAreaLayout->setSpacing(0);

    // 总体布局
    m_mainLayout->addWidget(m_titleBar);
    m_mainLayout->addWidget(m_searchAreaWid);
    m_mainLayout->addLayout(m_deleleAreaLayout);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    // 删除按钮菜单
    m_funcMenu = new QMenu(this);
    m_funcMenu->setAccessibleName("kylin-ipmsg_chatsearch_QMenu_mfuncMenu");
    m_deleteMsgAction = m_funcMenu->addAction(tr("Batch delete"));
    m_clearMsgAction = m_funcMenu->addAction(tr("Clear all messages"));

    this->setLayout(m_mainLayout);

    // 设置窗口页面居中
    QScreen *screen = QGuiApplication::primaryScreen();
    this->move(screen->geometry().center() - this->rect().center());
}

// 设置组件样式
void ChatSearch::setWidgetStyle()
{
    // 主界面属性
    /* 适配kysdk的窗管 */
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(this);
    windowhelper->removeTitleBar();

    //失焦仍显示提示信息
    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    this->setFixedSize(QSize(500, 680));
    this->setMouseTracking(true);

    // 设置标题栏属性
    m_titleBar->m_pIconBtn->hide();
    m_titleBar->setTitleText(tr("Chat Content"));
    m_titleBar->setAutoFillBackground(true);
    m_titleBar->setBackgroundRole(QPalette::Base);

    //设置搜索窗口界面的大小
    m_searchAreaWid->setFixedSize(QSize(500, 120));

    // 设置搜索区域属性
    m_searchAreaWid->setAutoFillBackground(true);
    m_searchAreaWid->setBackgroundRole(QPalette::Base);
    m_searchAreaWid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont font;
    font.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);

    m_searchLineEdit->setFixedSize(468, 36);
    m_searchLineEdit->setFont(font);
    m_searchLineEdit->setClearButtonEnabled(false);
    this->setFocusPolicy(Qt::ClickFocus);
    m_searchLineEdit->setMaxLength(30);

    // 选择全部按钮
    QFont btnFont = this->font();
    btnFont.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);
    m_allBtn->setFont(btnFont);
    m_allBtn->setFixedSize(QSize(80, 36));
    m_allBtn->setText(tr("All"));
    m_allBtn->setProperty("isImportant", true);
    m_allBtn->setFocus();
    changeBtnStyle(m_allBtn, btnFont, tr("All"), 50);

    // 文件按钮
    m_fileBtn->setFont(btnFont);
    m_fileBtn->setFixedSize(QSize(80, 36));
    m_fileBtn->setText(tr("File"));
    m_fileBtn->setFlat(true);
    changeBtnStyle(m_fileBtn, btnFont, tr("File"), 50);

    // 图片/视频按钮
    m_ImageBtn->setFont(btnFont);
    m_ImageBtn->setText(tr("Image/Video"));
    QFontMetrics fontmts = QFontMetrics(m_ImageBtn->font());
    // m_ImageBtn->setFixedSize(fontmts.width(m_ImageBtn->text()) + 35, 36);
    m_ImageBtn->setFixedSize(QSize(100, 36));
    m_ImageBtn->setFlat(true);
    changeBtnStyle(m_ImageBtn, btnFont, tr("Image/Video"), 70);

    // 链接按钮
    m_linkBtn->setFont(btnFont);
    m_linkBtn->setFixedSize(QSize(80, 36));
    m_linkBtn->setText(tr("Link"));
    m_linkBtn->setFlat(true);
    changeBtnStyle(m_linkBtn, btnFont, tr("Link"), 50);

    // 删除中取消按钮
    m_cancelDeleBtn->setFont(btnFont);
    m_cancelDeleBtn->setFixedSize(QSize(96, 36));
    m_cancelDeleBtn->setText(tr("canael"));
    changeBtnStyle(m_cancelDeleBtn, btnFont, tr("canael"), 50);
    m_cancelDeleBtn->hide();

    // 删除中确定按钮
    m_sureDeleBtn->setFont(btnFont);
    m_sureDeleBtn->setText(tr("sure"));
    fontmts = QFontMetrics(m_sureDeleBtn->font());
    m_sureDeleBtn->setFixedSize(QSize(96, 36));
    // m_sureDeleBtn->setFixedSize(fontmts.width(m_sureDeleBtn->text()) + 65, 36);
    m_sureDeleBtn->setProperty("isImportant", true);
    changeBtnStyle(m_sureDeleBtn, btnFont, tr("sure"), 50);
    m_sureDeleBtn->hide();

    // 删除菜单按钮
    m_deleMenuBtn->setFixedSize(FUNCTION_MORE_BTN_SIZE);
    m_deleMenuBtn->setIconSize(QSize(16, 16));
    m_deleMenuBtn->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    m_deleMenuBtn->setToolTip(tr("DeleteMenu"));
    m_deleMenuBtn->setPopupMode(QToolButton::InstantPopup);
    m_deleMenuBtn->setProperty("isWindowButton", 0x1);
    m_deleMenuBtn->setProperty("useIconHighlightEffect", 0x2);
    m_deleMenuBtn->setAutoRaise(true);
    m_deleMenuBtn->setFocusPolicy(Qt::NoFocus);

    QList<QAction *> actions;

    //设置菜单中的按键
    m_deleteMsgAction = new QAction(m_deleFuncMenu);
    m_deleteMsgAction->setObjectName("kylin-ipmsg_chatsearch_QAction_mdeleteMsgAction");
    m_deleteMsgAction->setText(tr("Batch delete"));
    m_clearMsgAction = new QAction(m_deleFuncMenu);
    m_clearMsgAction->setObjectName("kylin-ipmsg_chatsearch_QAction_mclearMsgAction");
    m_clearMsgAction->setText(tr("Clear all messages"));
    actions << m_deleteMsgAction << m_clearMsgAction;

    m_deleFuncMenu->addActions(actions);
    m_deleMenuBtn->setMenu(m_deleFuncMenu);

    //选择删除的列表
    m_chooseDeleWid->setFixedSize(25, 520);
    m_chooseDeleWid->hide();

    m_listView->setAutoFillBackground(true);
    //设置聊天记录中的属性
    m_chatSearchDelegate = new ChatSearchDelegate(this->m_chat, this);
    m_chatSearchDelegate->setObjectName("kylin-ipmsg_chatsearch_ChatSearchDelegate_mchatSearchDelegate");
    m_listView->setItemDelegate(m_chatSearchDelegate);

    m_chatFiler = new ChatSearchFilter();
    m_chatFiler->setObjectName("kylin-ipmsg_chatsearch_ChatSearchFilter_mchatFiler");
    m_chatFiler->setSourceModel(this->m_chat->m_chatMsgModel);
    m_chatFiler->setDynamicSortFilter(true);
    m_chatFiler->sort(0);
    m_chatFiler->setFilterKeyColumn(0);
    m_listView->setModel(this->m_chatFiler);

    m_listView->setFixedHeight(520);
    // m_listView->setSpacing(7);
    m_listView->setDragEnabled(false);
    m_listView->setFrameShape(QListView::NoFrame);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listView->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_listView->scrollToBottom();
    m_listView->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    changeTheme();

    // 右键菜单
    m_funcMenu = new QMenu(this);
    m_funcMenu->setAccessibleName("kylin-ipmsg_chatsearch_QMenu_mfuncMenu");
    m_oneDeleAction = m_funcMenu->addAction(tr("Delete"));
    m_chooseDeleAction = m_funcMenu->addAction(tr("Choose Delete"));
    m_openFileAction = m_funcMenu->addAction(tr("Open"));
    m_openDirAction = m_funcMenu->addAction(tr("Open Directory"));


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
void ChatSearch::initGsetting()
{
    // 主题颜色
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        GlobalData::getInstance()->getSystemTheme();
        changeTheme();
    });
    // 字号
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=]() {
        m_listView->update();
    });
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFontSize, this, [=](){
        m_listView->update();
        
        QFont btnFont = QFont(GlobalData::getInstance()->m_fontName);
        btnFont.setPointSizeF(GlobalData::getInstance()->m_font14pxToPt);

        changeBtnStyle(m_allBtn, btnFont, tr("All"), m_allBtn->width() - LENGTH);
        changeBtnStyle(m_fileBtn, btnFont, tr("File"), m_fileBtn->width() - LENGTH);
        changeBtnStyle(m_ImageBtn, btnFont, tr("Image/Video"), m_ImageBtn->width() - LENGTH);
        changeBtnStyle(m_linkBtn, btnFont, tr("Link"), m_linkBtn->width() - LENGTH);
    });
}

//还原文本框初始状态
void ChatSearch::changeBtnStyle(QPushButton *btn1, QPushButton *btn2, QPushButton *btn3, QPushButton *btn4)
{
    btn1->setProperty("isImportant", true);
    btn2->setProperty("isImportant", false);
    btn3->setProperty("isImportant", false);
    btn4->setProperty("isImportant", false);
    btn1->setFlat(false);
    btn2->setFlat(true);
    btn3->setFlat(true);
    btn4->setFlat(true);
}

void ChatSearch::changeTheme()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
        m_chooseDeleWid->setStyleSheet("background-color:#FFFFFF;");
        m_listView->setStyleSheet("QListView{background:#FFFFFF;}");

        if (!m_chooseDeleWid->isHidden()) {
            QString listStyle = "QListWidget{background-color:#FFFFFF;}"
                                "QListWidget::item:hover{background-color:#FFFFFF;}"
                                "QListWidget::item::selected:active{background-color:#FFFFFF;border-width:0;}"
                                "QListWidget::item:selected{background-color:#FFFFFF;border-width:0;}";
            m_chooseListWid->setStyleSheet(listStyle);
        }
    } else {
        m_chooseDeleWid->setStyleSheet("background-color:#262626;");
        m_listView->setStyleSheet("QListView{background:#262626;}");

        if (!m_chooseDeleWid->isHidden()) {
            QString listStyle = "QListWidget{background-color:#262626;}"
                                "QListWidget::item:hover{background-color:#262626;}"
                                "QListWidget::item::selected:active{background-color:#262626;border-width:0;}"
                                "QListWidget::item:selected{background-color:#262626;border-width:0;}";
            m_chooseListWid->setStyleSheet(listStyle);
        }
    }
    return;
}

// 获取当前是否处于批量删除
bool ChatSearch::getDeleteState()
{
    return m_isDeleteNow;
}

//对控件接收到的事件进行过滤
bool ChatSearch::eventFilter(QObject *watch, QEvent *e)
{
    return QObject::eventFilter(watch, e);
}

void ChatSearch::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    emit sigCloseSearchWid();
    return;
}

// 设置信号绑定
void ChatSearch::setSignalConn()
{
    connect(m_titleBar->m_pCloseButton, &QPushButton::clicked, this, &ChatSearch::close);

    connect(m_allBtn, &QPushButton::clicked, this, &ChatSearch::showAllInfo);
    connect(m_fileBtn, &QPushButton::clicked, this, &ChatSearch::showFileInfo);
    connect(m_ImageBtn, &QPushButton::clicked, this, &ChatSearch::showImageInfo);
    connect(m_linkBtn, &QPushButton::clicked, this, &ChatSearch::showLinkInfo);
    connect(m_deleMenuBtn, &QToolButton::clicked, this, &ChatSearch::showDeleMenu);
    connect(m_cancelDeleBtn, &QPushButton::clicked, this, &ChatSearch::slotCancelClick);
    connect(m_sureDeleBtn, &QPushButton::clicked, this, &ChatSearch::slotSureClick);

    connect(m_listView, &QListView::customContextMenuRequested, this, &ChatSearch::showContextMenu);
    connect(m_listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotChooseMsg(const QModelIndex &)));

    connect(m_listView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onSliderMoved(int)));

    //菜单按钮中点击
    connect(m_deleteMsgAction, &QAction::triggered, this, &ChatSearch::showChooseMsg);
    connect(m_clearMsgAction, &QAction::triggered, this, &ChatSearch::slotClearMsg);

    //右键菜单中的点击
    connect(m_chooseDeleAction, &QAction::triggered, this, &ChatSearch::showChooseMsg);
    connect(m_oneDeleAction, &QAction::triggered, this, &ChatSearch::slotDeleteMsg);
    connect(m_openFileAction, &QAction::triggered, this, &ChatSearch::slotOpenFile);
    connect(m_openDirAction, &QAction::triggered, this, &ChatSearch::slotOpenDir);

    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &ChatSearch::filterFriendByReg);

    /* 监听字体变化 */
    connect(Control::getInstance(), &Control::sigFontChange, this, [=]() {
        QFont font14 = GlobalData::getInstance()->getFontSize14px();

        m_titleBar->m_pFuncLabel->setFont(font14);
        m_searchLineEdit->setFont(font14);
        m_allBtn->setFont(font14);
        m_fileBtn->setFont(font14);
        m_ImageBtn->setFont(font14);
        m_linkBtn->setFont(font14);
        m_deleMenuBtn->setFont(font14);
        m_cancelDeleBtn->setFont(font14);
        m_sureDeleBtn->setFont(font14);
    });
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFont, this, [=](){
        m_listView->update();
    });
    connect(GlobalData::getInstance(), &GlobalData::signalChangeHightColor, this, [=](){
        m_listView->update();
    });


    connect(GlobalData::getInstance(), &GlobalData::signalChangePC, this, &ChatSearch::changeSizePCMode);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFlatH, this, &ChatSearch::changeSizeHScreenMode);
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFlatV, this, &ChatSearch::changeSizeVScreenMode);

    return;
}

//控制两个list一起滚动
void ChatSearch::onSliderMoved(int pos)
{
    if (!m_chooseDeleWid->isHidden()) {
        m_chooseListWid->verticalScrollBar()->setValue(pos);
    }

    m_listView->verticalScrollBar()->setValue(pos);
}

// 滚动条是否在底端
bool ChatSearch::isScrollOnBottom()
{
    if (m_listView->verticalScrollBar()->value() == m_listView->verticalScrollBar()->maximum()) {
        if (!m_chooseDeleWid->isHidden()) {
            if (m_chooseListWid->verticalScrollBar()->value() == m_chooseListWid->verticalScrollBar()->maximum()) {
                return true;
            } else {
                return false;
            }
        }
        return true;
    }
    return false;
}

// 滚动条设为底端
void ChatSearch::setScrollToBottom()
{
    if (!m_chooseDeleWid->isHidden()) {
        m_chooseListWid->scrollToBottom();
    }

    m_listView->scrollToBottom();
}

// 根据字符串过虑聊天记录
void ChatSearch::filterFriendByReg(QString searchStr)
{
    /* SDK功能点：历史记录搜索 */
    GlobalSizeData::SDKPointHistorySearch();

    QString regStr;
    regStr.clear();

    switch (m_type) {
    case InfoType::All:
        regStr = QString("all:") + searchStr;
        break;
    case InfoType::File:
        regStr = QString("file:") + searchStr;
        break;
    case InfoType::Image:
        regStr = QString("image:") + searchStr;
        break;
    case InfoType::Link:
        regStr = QString("link:") + searchStr;
        break;
    default:
        regStr = QString("all:") + searchStr;
        break;
    }

    m_searchText = searchStr;

    m_chatFiler->setFilterRegExp(QRegExp(regStr, Qt::CaseInsensitive, QRegExp::FixedString));

    m_listView->update();

    if (!m_chooseDeleWid->isHidden()) {
        showDeleMsgCheckbox();
    }

    this->setScrollToBottom();

    return;
}

// 获取焦点
void ChatSearch::getFocus()
{
    if (this->isHidden()) {
        m_listView->scrollToBottom();
    }
}

//选择全部信息
void ChatSearch::showAllInfo()
{
    m_type = InfoType::All;
    changeBtnStyle(m_allBtn, m_fileBtn, m_ImageBtn, m_linkBtn);

    //设置聊天记录中的属性
    if (m_searchLineEdit->text() == "") {
        QString regStr;
        regStr.clear();
        regStr = QString("all:");

        m_chatFiler->setFilterRegExp(QRegExp(regStr, Qt::CaseInsensitive, QRegExp::FixedString));
    } else {
        filterFriendByReg(m_searchLineEdit->text());
    }

    //从底部开始显示
    setScrollToBottom();

    return;
}

//选择文件类型的信息
void ChatSearch::showFileInfo()
{
    m_type = InfoType::File;
    changeBtnStyle(m_fileBtn, m_allBtn, m_ImageBtn, m_linkBtn);

    if (m_searchLineEdit->text() == "") {
        QString regStr;
        regStr.clear();

        regStr = QString("file:");

        m_chatFiler->setFilterRegExp(QRegExp(regStr, Qt::CaseInsensitive, QRegExp::FixedString));

    } else {
        filterFriendByReg(m_searchLineEdit->text());
    }

    //从底部开始显示
    setScrollToBottom();

    return;
}

//选择图片/视频类型的信息
void ChatSearch::showImageInfo()
{
    m_type = InfoType::Image;
    changeBtnStyle(m_ImageBtn, m_fileBtn, m_allBtn, m_linkBtn);

    if (m_searchLineEdit->text() == "") {
        QString regStr;
        regStr.clear();

        regStr = QString("image:");

        m_chatFiler->setFilterRegExp(QRegExp(regStr, Qt::CaseInsensitive, QRegExp::FixedString));

    } else {
        filterFriendByReg(m_searchLineEdit->text());
    }

    //从底部开始显示
    setScrollToBottom();

    return;
}

//选择链接类型的信息

void ChatSearch::showLinkInfo()
{
    m_type = InfoType::Link;
    changeBtnStyle(m_linkBtn, m_ImageBtn, m_fileBtn, m_allBtn);

    if (m_searchLineEdit->text() == "") {
        QString regStr;
        regStr.clear();

        regStr = QString("link:");

        m_chatFiler->setFilterRegExp(QRegExp(regStr, Qt::CaseInsensitive, QRegExp::FixedString));

    } else {
        filterFriendByReg(m_searchLineEdit->text());
    }

    //从底部开始显示
    setScrollToBottom();

    return;
}

// 点击菜单按钮
void ChatSearch::showDeleMenu()
{
    m_deleteMsgAction->setVisible(true);
    m_clearMsgAction->setVisible(true);
}

// 显示右键菜单
void ChatSearch::showContextMenu(const QPoint &point)
{
    Q_UNUSED(point);
    if (m_chooseDeleWid->isHidden()) {
        if (!((m_listView->selectionModel()->selectedIndexes()).empty())) {

            // 将代理index转换为消息index
            QModelIndex msgIndex = m_listView->currentIndex();
            int msgType = msgIndex.data(ChatMsgModel::MsgType).toInt();

            if (msgType == ChatMsgModel::MessageType::TextMsg) {

                m_oneDeleAction->setVisible(true);
                m_chooseDeleAction->setVisible(true);
                m_openFileAction->setVisible(false);
                m_openDirAction->setVisible(false);

            } else if (msgType == ChatMsgModel::MessageType::TimeMsg) {

                m_oneDeleAction->setVisible(false);
                m_chooseDeleAction->setVisible(false);
                m_openFileAction->setVisible(false);
                m_openDirAction->setVisible(false);

            } else {
                m_oneDeleAction->setVisible(true);
                m_chooseDeleAction->setVisible(true);
                m_openFileAction->setVisible(true);
                m_openDirAction->setVisible(true);
            }

#ifndef TEST_MODE
            m_funcMenu->exec(QCursor::pos());
#endif
            m_listView->selectionModel()->clear();
            return;
        }
    } else {
        return;
    }
}

// 检查文件和文件夹是否存在
bool ChatSearch::checkFileExist(QString filePath)
{
    if (!QFileInfo::exists(filePath)) {
#ifndef TEST_MODE
        QMessageBox::critical(this, "", tr("No such file or directory!"));
#endif
        return false;
    }

    return true;
}

// 使用默认应用打开文件或文件夹
void ChatSearch::slotOpenFile()
{
    /* SDK功能点：打开文件 */
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
    QString cmd = QString("/usr/bin/xdg-open ") + filePath;
#ifndef TEST_MODE
    QProcess::startDetached(cmd);
#endif
}

// 打开所在目录
void ChatSearch::slotOpenDir()
{
    /* SDK功能点：打开文件目录 */
    GlobalSizeData::SDKPointOpenDir();

    QModelIndex msgIndex = m_listView->currentIndex();

    // 获取消息数据
    QString filePath = msgIndex.data(ChatMsgModel::FilePath).toString();

    if (!checkFileExist(filePath)) {
        return;
    }

    QString pathCmd = "\"" + filePath + "\"";
    QString cmd = QString("/usr/bin/peony --show-items " + pathCmd);

#ifndef TEST_MODE
    QProcess::startDetached(cmd);
#endif
}

// 删除消息
void ChatSearch::slotDeleteMsg()
{
    /* SDK功能点：删除记录 */
    GlobalSizeData::SDKPointDeleteRecord();

#ifndef TEST_MODE
    QMessageBox *msg = new QMessageBox(this);
    msg->setAccessibleName("kylin-ipmsg_chatsearch_QMessageBox_msg");
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
    //获取当前选择的索引
    QModelIndex searchIndex = m_listView->currentIndex();

    // 获取消息ID号
    int searchMsgId = searchIndex.data(ChatMsgModel::MsgId).toInt();

    //循环找到在model中的索引
    for (int i = 0; i < this->m_chat->m_chatMsgModel->rowCount(); i++) {
        QModelIndex msgIndex = this->m_chat->m_chatMsgModel->index(i, 0);
        int mId = msgIndex.data(ChatMsgModel::MsgId).toInt();

        if (mId == searchMsgId) {
            this->m_chat->m_chatMsgModel->delChatMsg(searchMsgId, msgIndex.row());
            break;
        }
    }

    return;
}

// 清空消息
void ChatSearch::slotClearMsg()
{
    /* SDK功能点：清空聊天记录 */
    GlobalSizeData::SDKPointClearChatRecord();

#ifndef TEST_MODE
    QMessageBox *msg = new QMessageBox(this);
    msg->setAccessibleName("kylin-ipmsg_chatsearch_QMessageBox_msg");
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
    this->m_chat->m_chatMsgModel->clearChatMsg();
    return;
}

// 批量删除消息
void ChatSearch::showChooseMsg()
{
    /* SDK功能点：批量删除 */
    GlobalSizeData::SDKPointBatchDelete();

    // 处于批量删除界面，更新listview
    m_isDeleteNow = true;
    m_listView->update();

    // m_chooseDeleWid->show();
    m_sureDeleBtn->show();
    m_cancelDeleBtn->show();
    m_allBtn->hide();
    m_fileBtn->hide();
    m_ImageBtn->hide();
    m_linkBtn->hide();
    m_deleMenuBtn->hide();

    if (m_chooseMsgHash.size() == 0) {
        m_sureDeleBtn->setEnabled(false);
    } else {
        m_sureDeleBtn->setEnabled(true);
    }

    // showDeleMsgCheckbox();
    //从底部开始显示
    // setScrollToBottom();
}

// checkbox选择要删除的信息
void ChatSearch::slotChooseMsg(const QModelIndex &)
{
    if (m_isDeleteNow == false) {
        return;
    } else {
        QModelIndex modelRow;
        modelRow = m_listView->selectionModel()->currentIndex(); //获取listview最近行号
        // 获取行号
        int itemRow = modelRow.row();
        // 获取消息ID号
        int searchMsgId = modelRow.data(ChatMsgModel::MsgId).toInt();

        clickMsgToChangeState(searchMsgId);
        // 将对应的id + 行号记录到hash表中
        recordChooseDeleteMsg(searchMsgId, itemRow);
    }
    return;
}

//创建删除时复选框
void ChatSearch::showDeleMsgCheckbox()
{
    if (m_chooseListWid != nullptr) {
        delete m_chooseListWid;
        m_chooseListWid = nullptr;
    }
    m_chooseListWid = new QListWidget(m_chooseDeleWid);
    m_chooseListWid->setAccessibleName("kylin-ipmsg_chatsearch_QListWidget_mchooseListWid");
    m_chooseListWid->show();

    int count = m_listView->model()->rowCount();
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(16, 80));
        item->setFlags(Qt::ItemIsEnabled);

        QString listStyle;
        if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
            listStyle = "QListWidget{background-color:#FFFFFF;}"
                        "QListWidget::item:hover{background-color:#FFFFFF;}"
                        "QListWidget::item::selected:active{background-color:#FFFFFF;border-width:0;}"
                        "QListWidget::item:selected{background-color:#FFFFFF;border-width:0;}";
            m_chooseListWid->setStyleSheet(listStyle);
        } else {
            listStyle = "QListWidget{background-color:#262626;}"
                        "QListWidget::item:hover{background-color:#262626;}"
                        "QListWidget::item::selected:active{background-color:#262626;border-width:0;}"
                        "QListWidget::item:selected{background-color:#262626;border-width:0;}";
            m_chooseListWid->setStyleSheet(listStyle);
        }


        QWidget *m_checkWidget = new QWidget();
        QCheckBox *m_chooseDeleBox = new QCheckBox(m_checkWidget);
        m_checkWidget->setAccessibleName("kylin-ipmsg_chatsearch_QWidget_mcheckWidget");
        m_chooseDeleBox->setAccessibleName("kylin-ipmsg_chatsearch_QCheckBox_mchooseDeleBox");
        m_chooseDeleBox->setFixedSize(QSize(16, 16));

        //设置项中的复选框位置
        QHBoxLayout *hLayout;
        hLayout = new QHBoxLayout();
        hLayout->setObjectName("kylin-ipmsg_chatsearch_QHBoxLayout_hLayout");
        hLayout->addStretch();
        hLayout->addWidget(m_chooseDeleBox);
        hLayout->addStretch();
        hLayout->setMargin(0);
        QVBoxLayout *vLayout;
        vLayout = new QVBoxLayout();
        vLayout->setObjectName("kylin-ipmsg_chatsearch_QVBoxLayout_vLayout");
        vLayout->addStretch();
        vLayout->addLayout(hLayout);
        vLayout->addStretch();
        vLayout->setSpacing(7);
        vLayout->setMargin(0);
        m_checkWidget->setLayout(vLayout);

        m_chooseListWid->addItem(item);                      //在ListWidget中添加一个条目
        m_chooseListWid->setItemWidget(item, m_checkWidget); //在这个条目中放置CheckBox

        connect(m_chooseDeleBox, &QCheckBox::clicked, [=]() {
            // i = 行号
            // recordChooseDeleteMsg(i);
        });
    }
    //选择删除的列表
    m_chooseListWid->setFixedHeight(520);
    m_chooseListWid->setFixedWidth(25);
    m_chooseListWid->setDragEnabled(false);
    m_chooseListWid->setFrameShape(QListView::NoFrame);
    m_chooseListWid->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chooseListWid->setContextMenuPolicy(Qt::CustomContextMenu);
    m_chooseListWid->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_chooseListWid->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_chooseListWid->scrollToBottom();
    m_chooseListWid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //    m_chooseListWid->hide();

    connect(m_chooseListWid->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onSliderMoved(int)));

    return;
}

void ChatSearch::recordChooseDeleteMsg(int id, int row)
{
    // 记录选择删除的信息
    // 使用hash表进行记录，
    if (m_chooseMsgHash.contains(id)) {
        m_chooseMsgHash.remove(id);
    } else {
        m_chooseMsgHash.insert(id, row);
    }

    if (m_chooseMsgHash.size() == 0) {
        m_sureDeleBtn->setEnabled(false);
    } else {
        m_sureDeleBtn->setEnabled(true);
    }
}

// 点击消息改变消息状态
void ChatSearch::clickMsgToChangeState(int msgId)
{
    if (!this->m_chat || !this->m_chat->m_chatMsgModel) {
        return;
    }
    ChatMsgInfo msgInfo;
    msgInfo.m_msgId = msgId;

    this->m_chat->m_chatMsgModel->setMsgChooseState(&msgInfo);

    return;
}

// 取消删除消息
void ChatSearch::slotCancelClick()
{
    // 清除批量删除的图标 + hash表
    clearCheckBoxIcon();

    if (m_chooseListWid != nullptr) {
        delete m_chooseListWid;
        m_chooseListWid = nullptr;
    }

    // m_chooseDeleWid->hide();
    m_sureDeleBtn->hide();
    m_cancelDeleBtn->hide();
    m_allBtn->show();
    m_fileBtn->show();
    m_ImageBtn->show();
    m_linkBtn->show();
    m_deleMenuBtn->show();
}

// 确定删除消息
void ChatSearch::slotSureClick()
{
#ifndef TEST_MODE
    QMessageBox *msg = new QMessageBox(this);
    msg->setAccessibleName("kylin-ipmsg_chatsearch_QMessageBox_msg");
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

    // 清除批量删除的图标 + hash表
    // 判断hash表是否有数据
    if (m_chooseMsgHash.size() != 0) {
        QHash<int, int>::iterator iter = m_chooseMsgHash.begin();
        while (iter != m_chooseMsgHash.end()) {
            // 获取消息ID号
            int searchMsgId = iter.key();
            clickMsgToChangeState(searchMsgId);
            this->m_chat->m_chatMsgModel->delSearchMsg(searchMsgId);
            iter++;
        }
        m_chooseMsgHash.clear();
    }

    m_isDeleteNow = false;

    slotCancelClick();
    return;
}

void ChatSearch::clearCheckBoxIcon()
{
    // 判断hash表是否有数据
    if (m_chooseMsgHash.size() != 0) {
        QHash<int, int>::iterator iter = m_chooseMsgHash.begin();
        while (iter != m_chooseMsgHash.end()) {
            // 获取消息ID号
            int searchMsgId = iter.key();
            clickMsgToChangeState(searchMsgId);
            iter++;
        }
        m_chooseMsgHash.clear();
    }

    m_isDeleteNow = false;
    return;
}

void ChatSearch::changeBtnStyle(QPushButton* btn, QFont font, QString btnText, int width)
{
    btn->setFont(font);

    QFontMetrics fontmts = QFontMetrics(btn->font());
    QString displayText = btnText;

    int dif = fontmts.width(btnText) - width;
    if (dif > 0) {
        btn->setToolTip(btnText);
        displayText = fontmts.elidedText(btnText, Qt::ElideRight, width);
    }
    btn->setText(displayText);
    return;
}

void ChatSearch::changeSizeHScreenMode()
{
    int w = 794;
    int h = GlobalData::getInstance()->m_currentSize.second;
    this->setFixedSize(QSize(w, h));
    m_allBtn->setFixedSize(FUNCTION_BTN_MAX_SIZE);
    m_fileBtn->setFixedSize(FUNCTION_BTN_MAX_SIZE);
    m_ImageBtn->setFixedSize(FUNCTION_IMAGE_BTN_MAX_SIZE);
    m_linkBtn->setFixedSize(FUNCTION_BTN_MAX_SIZE);
    m_deleMenuBtn->setFixedSize(FUNCTION_MORE_BTN_MAX_SIZE);
    m_sureDeleBtn->setFixedSize(FUNCTION_DELETE_BTN_MAX_SIZE);
    m_cancelDeleBtn->setFixedSize(FUNCTION_DELETE_BTN_MAX_SIZE);
    m_titleBar->slotChangeHScreenMode();
    return;
}

void ChatSearch::changeSizeVScreenMode()
{
    int w = GlobalData::getInstance()->m_currentSize.first;
    int h = GlobalData::getInstance()->m_currentSize.second;
    this->setFixedSize(QSize(w, h));
    m_allBtn->setFixedSize(FUNCTION_BTN_MAX_SIZE);
    m_fileBtn->setFixedSize(FUNCTION_BTN_MAX_SIZE);
    m_ImageBtn->setFixedSize(FUNCTION_IMAGE_BTN_MAX_SIZE);
    m_linkBtn->setFixedSize(FUNCTION_BTN_MAX_SIZE);
    m_deleMenuBtn->setFixedSize(FUNCTION_MORE_BTN_MAX_SIZE);
    m_sureDeleBtn->setFixedSize(FUNCTION_DELETE_BTN_MAX_SIZE);
    m_cancelDeleBtn->setFixedSize(FUNCTION_DELETE_BTN_MAX_SIZE);
    m_titleBar->slotChangeVScreenMode();
    return;
}

void ChatSearch::changeSizePCMode()
{
    this->setFixedSize(QSize(500, 680));
    m_allBtn->setFixedSize(FUNCTION_BTN_SIZE);
    m_fileBtn->setFixedSize(FUNCTION_BTN_SIZE);
    m_ImageBtn->setFixedSize(FUNCTION_IMAGE_BTN_SIZE);
    m_linkBtn->setFixedSize(FUNCTION_BTN_SIZE);
    m_deleMenuBtn->setFixedSize(FUNCTION_MORE_BTN_SIZE);
    m_sureDeleBtn->setFixedSize(FUNCTION_DELETE_BTN_SIZE);
    m_cancelDeleBtn->setFixedSize(FUNCTION_DELETE_BTN_SIZE);
    m_titleBar->slotChangePCMode();
    return;
}
void ChatSearch::keyPressEvent(QKeyEvent *event)
{

    if(event->modifiers() == Qt::ControlModifier&&event->key()==Qt::Key_E){
        qDebug()<<__LINE__<<__func__;
        m_searchLineEdit->setFocus();
    }
    return QWidget::keyPressEvent(event);

}
