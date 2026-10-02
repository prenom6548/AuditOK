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

#include <gsettingmonitor.h>
#include "searchpage.h"

SearchPage::SearchPage(QScrollArea *parent) : QScrollArea(parent)
{
    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();

    // 初始化并监听gsetting
    initGsetting();
}

SearchPage::~SearchPage()
{
    if (m_msgView) {
        m_msgView->setParent(nullptr);
    }
}

// 单例，初始化返回指针
SearchPage *SearchPage::getInstance()
{
    static SearchPage *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            instance = new SearchPage();
            instance->setAccessibleName("kylin-ipmsg_SearchPage_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        instance = new SearchPage();
        instance->setAccessibleName("kylin-ipmsg_SearchPage_instance");
#endif
    }

    return instance;
}

// 初始化组件
void SearchPage::setWidgetUi()
{
    // 初始化组件和布局
    //    m_scrollArea        = new QScrollArea(this);
    m_widget = new QWidget(this);
    m_friendLabel = new QLabel(m_widget);
    m_msgLabel = new QLabel(m_widget);
    m_friendBtn = new QPushButton(m_widget);
    m_msgBtn = new QPushButton(m_widget);
    m_friendView = new QListView(m_widget);
    m_msgView = SearchMsgList::getInstance();
    m_myItemDelegate = new SearchFriendDelegate();
    m_myItemDelegate->setParent(m_friendView);
    m_mainLayout = new QVBoxLayout(m_widget);
    m_friendAreaVLayout = new QVBoxLayout();
    m_msgAreaVLayout = new QVBoxLayout();
    m_friendAreaHLayout = new QHBoxLayout();
    m_msgAreaHLayout = new QHBoxLayout();
    //设置objectname
    m_widget->setAccessibleName("kylin-ipmsg_searchpage_QWidget_mwidget");
    m_friendLabel->setAccessibleName("kylin-ipmsg_searchpage_QLabel_mfriendLabel");
    m_msgLabel->setAccessibleName("kylin-ipmsg_searchpage_QLabel_mmsgLabel");
    m_friendBtn->setAccessibleName("kylin-ipmsg_searchpage_QPushButton_mfriendBtn");
    m_msgBtn->setAccessibleName("kylin-ipmsg_searchpage_QPushButton_mmsgBtn");
    m_friendView->setAccessibleName("kylin-ipmsg_searchpage_QListView_mfriendView");
    m_myItemDelegate->setObjectName("kylin-ipmsg_searchpage_SearchFriendDelegate_mmyItemDelegate");
    m_mainLayout->setObjectName("kylin-ipmsg_searchpage_QVBoxLayout_mmainLayout");
    m_friendAreaVLayout->setObjectName("kylin-ipmsg_searchpage_QVBoxLayout_m_friendAreaVLayout");
    m_msgAreaVLayout->setObjectName("kylin-ipmsg_searchpage_QVBoxLayout_m_msgAreaVLayout");
    m_friendAreaHLayout->setObjectName("kylin-ipmsg_searchpage_QHBoxLayout_m_friendAreaHLayout");
    m_msgAreaHLayout->setObjectName("kylin-ipmsg_searchpage_QHBoxLayout_m_msgAreaHLayout");
    // 好友区域纵向布局
    m_friendAreaVLayout->addSpacing(8);
    m_friendAreaVLayout->addWidget(m_friendLabel, Qt::AlignBottom);
    m_friendAreaVLayout->addWidget(m_friendBtn);
    m_friendAreaVLayout->setMargin(0);
    m_friendAreaVLayout->setSpacing(0);

    // 聊天记录区域纵向布局
    m_msgAreaVLayout->addWidget(m_msgLabel, Qt::AlignBottom);
    m_msgAreaVLayout->addWidget(m_msgBtn);
    m_msgAreaVLayout->setMargin(0);
    m_msgAreaVLayout->setSpacing(0);

    // 好友区域横向布局
    m_friendAreaHLayout->addSpacing(16);
    m_friendAreaHLayout->addLayout(m_friendAreaVLayout);
    m_friendAreaHLayout->addSpacing(16);
    m_friendAreaHLayout->setMargin(0);
    m_friendAreaHLayout->setSpacing(0);

    // 聊天记录区域横向布局
    m_msgAreaHLayout->addSpacing(16);
    m_msgAreaHLayout->addLayout(m_msgAreaVLayout);
    m_msgAreaHLayout->addSpacing(16);
    m_msgAreaHLayout->setMargin(0);
    m_msgAreaHLayout->setSpacing(0);

    // 总体布局
    m_mainLayout->addLayout(m_friendAreaHLayout);
    m_mainLayout->addWidget(m_friendView);
    m_mainLayout->addLayout(m_msgAreaHLayout);
    m_mainLayout->addWidget(m_msgView);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addStretch();

    m_widget->setLayout(m_mainLayout);
    m_mainLayout->setParent(m_widget);

    // 设置窗口页面居中
    if (this->isWindow()) {
        QScreen *screen = QGuiApplication::primaryScreen();
        this->move(screen->geometry().center() - this->rect().center());
    }

    m_friendListModel = FriendListModel::getInstance();

    this->m_friendView->setModel(FriendListView::getInstance()->m_sortFilterModel);

    this->m_friendView->setSpacing(0);
    this->m_friendView->setDragEnabled(false);
    this->m_friendView->setFrameShape(QListView::NoFrame);
    this->m_friendView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->m_friendView->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    this->m_friendView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->m_friendView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this->m_friendView, &QListView::customContextMenuRequested, [=](QPoint pos) {
        FriendListView::getInstance()->showContextMenu(pos);
    });
    connect(this->m_friendView, &QListView::doubleClicked, [=]() {
        QModelIndex friendIndex =
            FriendListView::getInstance()->m_sortFilterModel->mapToSource(this->m_friendView->currentIndex());
        int friendId = friendIndex.data(FriendListModel::Id).toInt();

        // 清空好友未读消息数
        FriendListModel::getInstance()->clearUnreadMsgNum(friendId);
        FriendListView::getInstance()->slotShowMsgWidById(friendId);
    });

    // 右键菜单
    QMenu *m_funcMenu = new QMenu(this->m_friendView);
    m_funcMenu->setAccessibleName("kylin-ipmsg_searchpage_QMenu_mfuncMenu");
    QAction *m_startChatAction = m_funcMenu->addAction(tr("Start Chat"));
    QAction *m_setTopAction = m_funcMenu->addAction(tr("Set to Top"));
    QAction *m_nicknameAction = m_funcMenu->addAction(tr("Change Nickname"));
    QAction *m_viewInfoAction = m_funcMenu->addAction(tr("View Info"));
    QAction *m_delFriendAction = m_funcMenu->addAction(tr("Delete Friend"));
    m_startChatAction->setObjectName("kylin-ipmsg_searchpage_QAction_mstartChatAction");
    m_setTopAction->setObjectName("kylin-ipmsg_searchpage_QAction_msetTopAction");
    m_nicknameAction->setObjectName("kylin-ipmsg_searchpage_QAction_mnicknameAction");
    m_viewInfoAction->setObjectName("kylin-ipmsg_searchpage_QAction_mviewInfoAction");
    m_delFriendAction->setObjectName("kylin-ipmsg_searchpage_QAction_mdelFriendAction");

    m_funcMenu->insertSeparator(m_delFriendAction);

    connect(m_startChatAction, &QAction::triggered, [=]() {
        FriendListView::getInstance()->startChat();
    });
    connect(m_setTopAction, &QAction::triggered, [=]() {
        FriendListView::getInstance()->setTop();
    });
    connect(m_nicknameAction, &QAction::triggered, [=]() {
        FriendListView::getInstance()->changeNickname();
    });
    connect(m_viewInfoAction, &QAction::triggered, [=]() {
        FriendListView::getInstance()->viewInfo();
    });
    connect(m_delFriendAction, &QAction::triggered, [=]() {
        FriendListView::getInstance()->delFriend();
    });

    connect(this, &SearchPage::sigUpdatFriList, this, &SearchPage::slotCheckListView);
    connect(this, &SearchPage::sigUpdatMsgList, this, &SearchPage::slotCheckListView);
}

// 设置组件样式
void SearchPage::setWidgetStyle()
{
    // 主界面属性
    this->setMinimumSize(QSize(350, 506));
    this->setMouseTracking(true);

    if (m_friendView && m_friendView->model()) {
        int rowCount = 0;
#ifndef TEST_MODE
        try {
            rowCount = m_friendView->model()->rowCount();
        } catch (...) {
            rowCount = 0;
        }
#else
        rowCount = m_friendView->model()->rowCount();
#endif
        m_friendView->setFixedHeight(rowCount * 80);
        m_friendView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        if (m_friendView->verticalScrollBar()) {
            m_friendView->verticalScrollBar()->setDisabled(true);
        }
        m_friendView->setItemDelegate(m_myItemDelegate);
    }

    if (m_msgView && m_msgView->model()) {
        int rowCount = 0;
#ifndef TEST_MODE
        try {
            rowCount = m_msgView->model()->rowCount();
        } catch (...) {
            rowCount = 0;
        }
#else
        rowCount = m_msgView->model()->rowCount();
#endif
        m_msgView->setFixedHeight(rowCount * 80);
    }

    if (m_friendLabel) {
        m_friendLabel->setText(tr("Friend"));
        m_friendLabel->setFixedHeight(30);
        m_friendLabel->setStyleSheet("color:#8C8C8C;");
        m_friendLabel->hide();
    }

    if (m_msgLabel) {
        m_msgLabel->setText(tr("Chat Record"));
        m_msgLabel->setFixedHeight(30);
        m_msgLabel->setStyleSheet("color:#8C8C8C;");
        m_msgLabel->hide();
    }

    if (m_friendBtn) {
        m_friendBtn->setMinimumSize(318, 1);
        m_friendBtn->setMaximumHeight(1);
        m_friendBtn->setEnabled(false);
        m_friendBtn->hide();
    }

    if (m_msgBtn) {
        m_msgBtn->setMinimumSize(318, 1);
        m_msgBtn->setMaximumHeight(1);
        m_msgBtn->setEnabled(false);
        m_msgBtn->hide();
    }

    this->setWidget(m_widget);
    this->setWidgetResizable(true);
    this->setFrameShape(QFrame::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (this->verticalScrollBar()) {
        this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    }
}



// 初始化并监听gsetting
void SearchPage::initGsetting()
{
    // 主题颜色
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        GlobalData::getInstance()->getSystemTheme();
        changeTheme();
    });
    connect(GlobalData::getInstance(), &GlobalData::signalChangeFont, this, &SearchPage::slotChangeFont);
}

void SearchPage::changeTheme()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
//        this->setStyleSheet("background-color:#FFFFFF");
        QPalette palette = this->palette();
        palette.setColor(QPalette::Base,QColor(255,255,255,255));
        this->setPalette(palette);
        m_widget->setStyleSheet("background-color:#FFFFFF");
        m_msgBtn->setStyleSheet("background-color:rgba(0, 0, 0, 10%);");
        m_friendBtn->setStyleSheet("background-color:rgba(0, 0, 0, 10%);");
    } else {
//        this->setStyleSheet("background-color:#000000");
        //设置深色模式搜索页面色值为#232426,样式设置方法废弃，原因：覆盖了滚动条样式，改用palette方法
        QPalette palette = this->palette();
        palette.setColor(QPalette::Base,QColor(35,36,38,255));
        this->setPalette(palette);
        m_widget->setStyleSheet("background-color:#232426");
        m_msgBtn->setStyleSheet("background-color:rgba(255, 255, 255, 40%);");
        m_friendBtn->setStyleSheet("background-color:rgba(255, 255, 255, 40%);");
    }
}

// 好友列表的大小改变
void SearchPage::FriendListSize(int num)
{
    if (m_friendLabel && m_friendBtn && m_friendView) {
        if (num == 0) {
            m_friendLabel->hide();
            m_friendBtn->hide();
            m_friendView->hide();
        } else {
            m_friendLabel->show();
            m_friendBtn->show();
            m_friendView->show();
            m_friendView->setFixedHeight(num * 80);
        }
        this->m_friendView->update();
    }
    emit sigUpdatFriList();
    return;
}

// 好友列表的大小改变
void SearchPage::MsgListSize(int num)
{
    if (m_msgLabel && m_msgBtn && m_msgView) {
        if (num == 0) {
            m_msgLabel->hide();
            m_msgBtn->hide();
            m_msgView->hide();
        } else {
            m_msgLabel->show();
            m_msgBtn->show();
            m_msgView->show();
            m_msgView->setFixedHeight(num * 80);
        }
    }
    emit sigUpdatMsgList();
    return;
}

void SearchPage::slotCheckListView()
{
    if (m_friendView && m_msgView && m_friendView->model() && m_msgView->model()) {
        if (this->m_friendView->model()->rowCount() == 0 && this->m_msgView->model()->rowCount() == 0) {
            emit sigSearchEmpty();
        } else {
            emit sigSearchExist();
        }
    }
}

void SearchPage::slotChangeFont()
{
    m_friendLabel->setFont(GlobalData::getInstance()->m_fontName);
    m_msgLabel->setFont(GlobalData::getInstance()->m_fontName);
    return;
}
