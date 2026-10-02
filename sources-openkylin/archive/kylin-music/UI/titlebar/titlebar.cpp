/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#include <QMenu>
#include <QFile>

#include "titlebar.h"
#include "UI/globalsignal.h"
#include "UI/mainwidget.h"
#include "UI/base/xatom-helper.h"

TitleBar::TitleBar(QWidget *parent) : QFrame(parent)
{

    m_windowHelper = new UkuiWindowHelper(parent);
    m_windowHelper->removeTitleBar();
    m_maxBtnHoverDelayTimer = new QTimer(this);
    m_maxBtnHoverDelayTimer->setInterval(500);
    m_maxBtnHoverDelayTimer->setSingleShot(true);
    connect(m_maxBtnHoverDelayTimer, &QTimer::timeout, this, [this]() {
        QRect rect = maximumBtn->geometry();
        qInfo() << "rect1:" << rect.x();
        rect.setX(rect.x() + 420);
        qInfo() << "rect2:" << rect.x();
        m_windowHelper->showTileFlyout(rect);
    });

    this->m_parent = parent;
    setMouseTracking(true);
    this->installEventFilter(this);
    this->setObjectName("TitleBar");
    initTitle();
    titlecolor();
}

void TitleBar::initTitle()
{
//    setStyleSheet(::main_style);

    menumodule = new menuModule(this);
    menumodule->setAccessibleName("kylin-music_titlebar_menuModule_menumodule");
//    menumodule->menuButton
    resize(750,56);

    titleLayout = new QHBoxLayout(this);

    LayoutLeft = new QHBoxLayout;

    leftBtn = new QPushButton;
    leftBtn->setAccessibleName("kylin-music_titlebar_QPushButton_leftBtn");
//    leftBtn->setIcon(QIcon::fromTheme("pan-start-symbolic"));

//    leftBtn->setStyleSheet("QPushButton{background:transparent;}");
    leftBtn->setStyleSheet("QPushButton{background:transparent;\
                           border-image:url(:/img/default/backoff.png);}");
    leftBtn->setFixedSize(16,16);
//    leftBtn->setStyleSheet()
//    leftBtn->setCursor(Qt::PointingHandCursor);
    leftBtn->setToolTip(tr("back"));
    leftBtn->setDisabled(true);
    leftBtn->setContentsMargins(8,0,0,0);
    leftBtn->hide();
    rightBtn = new QPushButton;
    rightBtn->setAccessibleName("kylin-music_titlebar_QPushButton_rightBtn");
//    rightBtn->setIcon(QIcon::fromTheme("pan-end-symbolic"));
    rightBtn->setStyleSheet("QPushButton{background:transparent;}");
    rightBtn->setStyleSheet("QPushButton{background:transparent;\
                            border-image:url(:/img/default/forward.png);}");
    rightBtn->setFixedSize(16,16);
//    rightBtn->setCursor(Qt::PointingHandCursor);
    rightBtn->setToolTip(tr("forward"));
    rightBtn->setDisabled(true);
    rightBtn->hide();

    searchEdit = new SearchEdit(m_parent); // fix #140170, call KSearchLineEdit without `this`
    searchEdit->setPlaceholderText(tr("Search"));
    searchEdit->setAccessibleName("kylin-music_titlebar_SearchEdit_searchEdit");
    searchEdit->setWidget(m_parent);
    searchEdit->setFixedSize(QSize(240, 36));
    searchEdit->setClearButtonEnabled(true);

    toggleButton = new QPushButton(this);
    toggleButton->setFixedSize(30, 30);
    toggleButton->move(10, 10); // 放置在左上角
    toggleButton->setIcon(QIcon(":/img/kylin-open1.svg").pixmap(QSize(40,40)));
    toggleButton->setProperty("isWindowButton", 0x1);
    toggleButton->setProperty("useIconHighlightEffect", 0x2);
    toggleButton->setFlat(true);
    toggleButton->setVisible(false);
    m_lyricsWid = new LyricsWidget();
    //搜索框暂时隐藏
    LayoutLeft->addWidget(searchEdit, 0 ,Qt::AlignHCenter);

    LayoutRight = new QHBoxLayout;

    userIconBtn = new QPushButton;
    userIconBtn->setAccessibleName("kylin-music_QPushButton_userIconBtn");
//    userIconBtn->setCursor(Qt::PointingHandCursor);
    userIconBtn->setFixedSize(20,20);
    userIconBtn->setStyleSheet("QPushButton{background:transparent;\
                               border-image:url(:/img/default/headportrait.png);}");

    loginBtn = new QPushButton;
    loginBtn->setAccessibleName("kylin-music_QPushButton_loginBtn");
//    loginBtn->setText("未登录");
    loginBtn->setText(tr("Not logged in"));
    loginBtn->setFlat(true);
    loginBtn->adjustSize();
//    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet("QPushButton{background:transparent;\
                            color:rgb(238,238,238);\
                            border:none;}");

    //for trigon list
    QMenu *loginMenu = new QMenu;
    loginMenu->setAccessibleName("kylin-music_QMenu_loginMenu");
    loginBtn->setMenu(loginMenu);

//    m_optionButton = new QToolButton(this);
//    m_optionButton->setToolTip(tr("Menu"));
//    m_optionButton->setPopupMode(QToolButton::InstantPopup);
//    m_optionButton->setFixedSize(30,30);
//    m_optionButton->setIcon(QIcon::fromTheme("application-menu"));
//    m_optionButton->setProperty("useIconHighlightEffect", 0x2);
//    m_optionButton->setProperty("isWindowButton", 0x01);
//    m_optionButton->setAutoRaise(true);


    settingMenu = new QMenu();
    settingMenu->setAccessibleName("kylin-music_QMenu_settingMenu");

    changeThemeColorMenu = new QMenu(this);
    changeThemeColorMenu->setAccessibleName("kylin-music_QMenu_changeThemeColorMenu");
    darkThemeAct = new QAction(this);
    darkThemeAct->setObjectName("kylin-music_titlebar_QAction_darkThemeAct");
    lightThemeAct = new QAction(this);
    lightThemeAct->setObjectName("kylin-music_titlebar_QAction_lightThemeAct");
    aboutAct = new QAction(this);
    aboutAct->setObjectName("kylin-music_titlebar_QAction_aboutAct");
    if(WidgetStyle::themeColor == 1)
    {
        settingMenu->setStyleSheet("QMenu{background-color:#303032;color:#F9F9F9;}"
                                   "QMenu::item:selected{background-color:#48484C;}");
        changeThemeColorMenu->setStyleSheet("QMenu{background-color:#303032;color:#F9F9F9;}"
                                            "QMenu::item:selected{background-color:#48484C;}");
    }
    else if(WidgetStyle::themeColor == 0)
    {
        settingMenu->setStyleSheet("QMenu{background-color:#FFFFFF;color:#303133;}"
                                   "QMenu::item:selected{background-color:#F7F7F7;}");
        changeThemeColorMenu->setStyleSheet("QMenu{background-color:#FFFFFF;color:#303133;}"
                                            "QMenu::item:selected{background-color:#F7F7F7;}");
    }

//    changeThemeColorMenu->setTitle("更换主题颜色");
//    changeThemeColorMenu->setTitle(tr("Change the theme color"));
//    darkThemeAct->setText("深色模式");
//    darkThemeAct->setText(tr("Dark mode"));
//    lightThemeAct->setText("浅色模式");
//    lightThemeAct->setText(tr("Light color pattern"));
//    aboutAct->setText("关于");
//    aboutAct->setText(tr("about"));

//    settingMenu->addMenu(changeThemeColorMenu);
//    changeThemeColorMenu->addAction(lightThemeAct);
//    changeThemeColorMenu->addAction(darkThemeAct);
//    settingMenu->addAction(aboutAct);

//    m_optionButton->setMenu(settingMenu);   //下拉三角图标

//    connect(aboutAct,SIGNAL(triggered()),this,SLOT(showAboutWidget()));


    nullLabel = new QLabel;
    nullLabel->setAccessibleName("kylin-music_QLabel_nullLabel");
    nullLabel->setPixmap(QPixmap(":/images/TitleBar/line.png"));

    miniBtn = new QPushButton;
    miniBtn->setAccessibleName("kylin-music_QPushButton_miniBtn");
//    miniBtn->setCursor(Qt::PointingHandCursor);
    miniBtn->setFixedSize(30,30);
    miniBtn->setToolTip(tr("mini model"));
    miniBtn->setIcon(QIcon::fromTheme("ukui-mini-symbolic"));
    miniBtn->setProperty("isWindowButton", 0x1);
    miniBtn->setProperty("useIconHighlightEffect", 0x2);
    miniBtn->setFlat(true);

    minimumBtn = new QPushButton;
    minimumBtn->setAccessibleName("kylin-music_QPushButton_minimumBtn");
//    minimumBtn->setCursor(Qt::PointingHandCursor);
    minimumBtn->setToolTip(tr("To minimize the"));
    minimumBtn->setFixedSize(30,30);
    minimumBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    minimumBtn->setProperty("isWindowButton", 0x1);
    minimumBtn->setProperty("useIconHighlightEffect", 0x2);
    minimumBtn->setFlat(true);

    maximumBtn = new QPushButton;
    maximumBtn->setAccessibleName("kylin-music_QPushButton_maximumBtn");
//    maximumBtn->setCursor(Qt::PointingHandCursor);
    maximumBtn->setFixedSize(30,30);
    maximumBtn->setToolTip(tr(""));
    maximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    maximumBtn->setProperty("isWindowButton", 0x1);
    maximumBtn->setProperty("useIconHighlightEffect", 0x2);
    maximumBtn->setFlat(true);
    maximumBtn->installEventFilter(this);

    closeBtn = new QPushButton;
    closeBtn->setAccessibleName("kylin-music_QPushButton_closeBtn");
//    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFixedSize(30,30);
    closeBtn->setToolTip(tr("close"));
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x8);
    closeBtn->setFlat(true);

    connect(toggleButton, &QPushButton::clicked, m_lyricsWid, &LyricsWidget::toggleVisibility);
    connect(searchEdit,&SearchEdit::textChanged,this,&TitleBar::searchMusic);
    connect(searchEdit,&SearchEdit::sigFoucusIn,this,&TitleBar::slotFoucusIn);


    LayoutRight->setSpacing(0);
//    LayoutRight->addWidget(userIconBtn);
//    LayoutRight->addWidget(loginBtn);
//    LayoutRight->addSpacing(20);
    LayoutRight->addWidget(menumodule->menuButton);
    LayoutRight->addSpacing(4);
    LayoutRight->addWidget(miniBtn);
    LayoutRight->addSpacing(4);
    LayoutRight->addWidget(minimumBtn);
    LayoutRight->addSpacing(4);
    LayoutRight->addWidget(maximumBtn);
    LayoutRight->addSpacing(4);
    LayoutRight->addWidget(closeBtn);

    QWidget* buttonPlaceholder = new QWidget();
    buttonPlaceholder->setFixedSize(toggleButton->sizeHint());
    QVBoxLayout* placeholderLayout = new QVBoxLayout(buttonPlaceholder);
    placeholderLayout->setContentsMargins(16, 5, 0, 0); // 去除边距
    placeholderLayout->addWidget(toggleButton);

    titleLayout->addWidget(buttonPlaceholder , 0,Qt::AlignLeft);
    titleLayout->addStretch();
    titleLayout->addLayout(LayoutLeft);
    titleLayout->addStretch();
    titleLayout->addLayout(LayoutRight);
//    titleLayout->setSpacing(0);
    if(g_user_signal->isRTLLang){
        titleLayout->setContentsMargins(4,4,0,0);
    }else{
        titleLayout->setContentsMargins(0,4,4,0);
    }
}

bool TitleBar::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent->button() == Qt::LeftButton)  {
                Widget::mutual->slotShowMaximized();
                g_user_signal->hideSearchResult();

            }
        }
        if (event->type() == QEvent::MouseButtonPress) {
            g_user_signal->hideSearchResult();
        }
    }

    if (watched == maximumBtn) {
        if (event->type() == QEvent::HoverEnter) {
            m_maxBtnHoverDelayTimer->start();
        } else if (event->type() == QEvent::HoverLeave) {
            m_maxBtnHoverDelayTimer->stop();
        }
    }

    return QWidget::eventFilter(watched,event);
}

void TitleBar::searchMusic()
{
//    m_serach = new SearchResult(this);
//    MotifWmHints hint;
//    hint.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
//    hint.functions = MWM_FUNC_ALL;
//    hint.decorations = MWM_DECOR_BORDER;
//    XAtomHelper::getInstance()->setWindowMotifHint(m_serach->winId(), hint);
//    m_serach->show();
//    m_serach->raise();
//    QString enterStr = searchEdit->text().trimmed();
//    qDebug() << "搜索" <<"输入栏" << enterStr;
//    //    searchEdit->clear();
//    if(enterStr == "")
//    {
//        searchWidget->close();
//        return;
//    }
//    searchWidget->show();
//    searchWidget->raise();

//    searchWidget->clear();

//    QSqlQuery searchQuery;
//    QString searchStr = QString("select * from LocalMusic where musicname LIKE '%%1%'").arg(enterStr);
//    qDebug() << "搜索" <<"数据库" << searchStr;
//    searchQuery.exec(searchStr);
//    while(searchQuery.next())
//    {
//        QString songName = searchQuery.value(1).toString();
//        QString Path = searchQuery.value(2).toString();
//        QString Title = searchQuery.value(3).toString();
//        QString Album = searchQuery.value(4).toString();
//        QString Time = searchQuery.value(7).toString();

//        qDebug() << "搜索" << songName << Path ;

//        QListWidgetItem *item1 = new QListWidgetItem(searchWidget);
//        item1->setText(songName+" "+Title+" "+Album);
//    }
}

void TitleBar::slotFoucusIn()
{
    searchEdit->setFocus();
}

void TitleBar::titlecolor()
{
    if (WidgetStyle::themeColor == 1)
    {
        settingMenu->setStyleSheet("QMenu{background-color:#303032;color:#F9F9F9;}"
                                   "QMenu::item:selected{background-color:#3D6BE5;}");
        changeThemeColorMenu->setStyleSheet("QMenu{background-color:#303032;color:#F9F9F9;}"
                                            "QMenu::item:selected{background-color:#3D6BE5;}");

        this->setStyleSheet("#TitleBar{background-color:#1d1d1d;}");
        // 设置深色主题下搜索框占位符文本的颜色为白色
        QPalette searchPal = searchEdit->palette();
        searchPal.setColor(QPalette::PlaceholderText, QColor(29, 29, 29));
        searchEdit->setPalette(searchPal);

        menumodule->menuButton->setStyleSheet("QPushButton{background:transparent;border-radius:4px;\
                              }"
                              "QPushButton::hover{background:#D4DBE9;opacity:0.04;}"
                              "QPushButton::pressed{background:#BEBEBE;opacity:0.04;}"
                              "QPushButton::menu-indicator{image:None;}"
                              );
    }
    else if(WidgetStyle::themeColor == 0)
    {
        settingMenu->setStyleSheet("QMenu{background-color:#FFFFFF;color:#303133;}"
                                   "QMenu::item:selected{background-color:#3D6BE5;color:#FFFFFF;}");
        changeThemeColorMenu->setStyleSheet("QMenu{background-color:#FFFFFF;color:#303133;}"
                                            "QMenu::item:selected{background-color:#3D6BE5;color:#FFFFFF;}");


        this->setStyleSheet("#TitleBar{background-color:#FFFFFF;}");
        // 设置浅色主题下搜索框占位符文本的颜色为深灰色
        QPalette searchPal = searchEdit->palette();
        searchPal.setColor(QPalette::PlaceholderText, QColor(255, 255, 255));
        searchEdit->setPalette(searchPal);

        menumodule->menuButton->setStyleSheet("QPushButton{background:transparent;border-radius:4px;\
                              }"
                              "QPushButton::hover{background:rgba(0,0,0,0.15);}"
                              "QPushButton::pressed{background:rgba(0, 0, 0, 0.2);}"
                              "QPushButton::menu-indicator{image:None;}"
                              );

    }
}
