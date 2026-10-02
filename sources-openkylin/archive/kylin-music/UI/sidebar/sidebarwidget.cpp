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

#include "sidebarwidget.h"
#include "UIControl/player/player.h"
#include "UIControl/base/musicDataBase.h"
#include "UI/base/widgetstyle.h"
#include "kwindoweffects.h"
#include "UI/mainwidget.h"
#include <ukui-log4qt.h>


SideBarWidget::SideBarWidget(QWidget *parent) : LeftsiderbarWidget(parent)
{
    m_ukuiWindowHelper = new UkuiWindowHelper(this);
    m_elideFont = this->font();
    // 按钮内文本标签可用宽度: 180(按钮) - 16(左) - 16(图标) - 8(间距) - 16(右) = 124
    m_maxElideWidth = 124;
    initWidget();
    initConnect();
    sidecolor();
    getPlayListName();

    if (QGSettings::isSchemaInstalled("org.ukui.control-center.personalise")) {
        m_pTransparency = new QGSettings("org.ukui.control-center.personalise");
        tranSparency = m_pTransparency->get("transparency").toDouble();
        m_pSideColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-normal") ;

        connect(m_pTransparency, &QGSettings::changed, this, [=](QString value) {
            if (value == "transparency") {
                tranSparency = m_pTransparency->get("transparency").toDouble();
                adjustBackground();
            }
        });
    }
    adjustBackground(true);
    setAutoFillBackground(true);
    if (ThemeController::widgetTheme() != ClassicTheme) {
        this->setAttribute(Qt::WA_TranslucentBackground, true); // 透明
    }
    installEventFilter(this);
}

void SideBarWidget::initAddButton()
{
    if (myPlayListBtn == nullptr) {
        return;
    }
    if(WidgetStyle::themeColor == 1) {
        myPlayListBtn->setIconHighlight(false);
        myPlayListBtn->setIconColor(Qt::white);
        myPlayListBtn->setBackgroundColor("#1d1d1d");
    } else if(WidgetStyle::themeColor == 0) {
        myPlayListBtn->setIconHighlight(false);
        myPlayListBtn->setIconColor("#1d1d1d");
        myPlayListBtn->setBackgroundColor(Qt::white);
    }
}

void SideBarWidget::initWidget()
{
    this->resize(200, 640);
    this->setObjectName("SideBarWidget");
    this->setProperty("useSystemStyleBlur", true);
    // this->setAttribute(Qt::WA_TranslucentBackground);
    // KWindowEffects::enableBlurBehind(this->winId(), true);

    //侧边栏界面样式
    QVBoxLayout *mainLayout = new QVBoxLayout();

    //logo
    QHBoxLayout *logoLayout = new QHBoxLayout();
    logoNameLabel = new QLabel();
    logoNameLabel->setAccessibleName("kylin-music_QLabel_logoNameLabel");
    logoNameLabel->setText(tr("Music Player"));
    logoNameLabel->setProperty("isWindowLabel" , true);
    logoNameLabel->setFixedHeight(20);

    logoLabel = new QLabel(this);
    logoLabel->setAccessibleName("kylin-music_QLabel_logoLabel");
    logoLabel->setFixedSize(24, 24);
    logoLabel->setPixmap(QIcon::fromTheme("kylin-music").pixmap(logoLabel->size()));
    logoLabel->setScaledContents(true);
    logoLayout->setSpacing(0);
    logoLayout->setMargin(8);
    
    if(g_user_signal->isRTLLang){
        logoLayout->addWidget(logoLabel,Qt::AlignRight);
        logoLayout->addSpacing(8);
        logoLayout->addWidget(logoNameLabel,0,Qt::AlignRight);
        logoLayout->addStretch();
        logoLayout->setContentsMargins(0,8,8,0);
    }else{
        logoLayout->addWidget(logoLabel,Qt::AlignLeft);
        logoLayout->addSpacing(8);
        logoLayout->addWidget(logoNameLabel,0,Qt::AlignLeft);
        logoLayout->addStretch();
        logoLayout->setContentsMargins(8,8,0,0);
    }
    
    //音乐库
    QHBoxLayout *libraryLayout = new QHBoxLayout();
    libraryLabel = new QLabel();
    libraryLabel->setAccessibleName("kylin-music_QLabel_libraryLabel");
    libraryLabel->setText(tr("Library"));
    libraryLabel->setFixedHeight(28);
    libraryLayout->addWidget(libraryLabel);
    libraryLayout->setContentsMargins(38-10,0,38-10,0);

    //歌曲列表
    playListBtn = new CustomToolButton;
    playListBtn->setAccessibleName("kylin-music_sidebarwidget_CustomToolButton_playListBtn");
    playListBtn->setLableText(tr("Song List"));
    playListBtn->setToolTip(tr("Song List"));
    playListBtn->setFixedSize(180,36);
    playListBtn->setStatusTip(IS_SELECT);
    playListBtn->setChecked(true);
    playListBtn->setProperty("isImportant", true);
    playListBtn->buttonListName = ALLMUSIC;
    playListBtn->defaultStyle();

    QHBoxLayout *playListBtnLayout = new QHBoxLayout();
    playListBtnLayout->setSpacing(0);
    playListBtnLayout->addSpacing(10);
    playListBtnLayout->addWidget(playListBtn,Qt::AlignCenter);
    playListBtnLayout->addStretch();
    playListBtnLayout->setContentsMargins(0, 0, 10, 0);

    connect(playListBtn,&CustomToolButton::selectButtonChanged,this,&SideBarWidget::slotButtonClicked);
    connect(playListBtn, &CustomToolButton::sigShowMenu, this, [this]() {
        emit sigShowMenu();
    });

    //我的歌单
    QHBoxLayout *myPlayListLayout = new QHBoxLayout;
    myPlayListLabel = new QLabel(this);
    myPlayListLabel->setAccessibleName("kylin-music_QLabel_myPlayListLabel");
    myPlayListLabel->setText(tr("My PlayList"));
    myPlayListLabel->setFixedHeight(28);

    myPlayListBtn = new KPushButton(this);
    myPlayListBtn->setAccessibleName("kylin-music_sidebarwidget_QPushButton_myPlayListBtn");
    myPlayListBtn->setIcon(QIcon::fromTheme("list-add-symbolic"));
    myPlayListBtn->setFixedSize(24, 24);
    myPlayListBtn->setIconSize(QSize(20, 20));
    myPlayListBtn->setToolTip(tr("Add Playlist"));
    myPlayListBtn->setTranslucent(true);
    myPlayListBtn->setBackgroundColorHighlight(false);
    myPlayListBtn->setBackgroundColor(Qt::white);
    myPlayListBtn->setIconColor("#1d1d1d");
    myPlayListBtn->installEventFilter(this);

    myPlayListLayout->addWidget(myPlayListLabel);
    myPlayListLayout->addStretch();
    myPlayListLayout->addWidget(myPlayListBtn);
    myPlayListLayout->setContentsMargins(38-13,0,38-27,0);

    //歌单界面
    newPlayListWidget = new QWidget();
    newPlayListWidget->setAccessibleName("kylin-music_sidebarwidget_QWidget_newPlayListWidget");
    // newPlayListWidget->setAttribute(Qt::WA_TranslucentBackground);
    newPlayListLayout = new QVBoxLayout();
    newPlayListLayout->setAlignment(Qt::AlignTop);
    newPlayListWidget->setLayout(newPlayListLayout);
    newPlayListLayout->setContentsMargins(10, 0, 10, 0);

    //    mainLayout->addLayout(libraryLayout);
    //    mainLayout->addSpacing(6);
    mainLayout->addLayout(playListBtnLayout);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(myPlayListLayout);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(newPlayListWidget);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(0, 0, 0, 8);

    //新建歌单弹窗
    newSonglistPup = new PopupDialog(this);
    newSonglistPup->setAccessibleName("kylin-music_sidebarwidget_PopupDialog_newSonglistPup");
    newSonglistPup->titleLab->setText(tr("New Playlist"));
    newSonglistPup->titleLab->setObjectName("kylin-music_sidebarwidget_PopupDialog_newSonglistPup_titleLab");
    newSonglistPup->hide();

    renameSongListPup = new PopupDialog(this);
    renameSongListPup->setAccessibleName("kylin-music_sidebarwidget_PopupDialog_renameSongListPup");
    renameSongListPup->titleLab->setText(tr("Rename"));
    renameSongListPup->titleLab->setAccessibleName("kylin-music_sidebarwidget_PopupDialog_renameSongListPup_titleLab");
    renameSongListPup->hide();

    //侧边栏界面 嵌套 widget
    mainWid = new QWidget(this);
    mainWid->setAccessibleName("kylin-music_sidebarwidget_QWidget_mainWid");
    mainWid->setObjectName("mainWid");
    mainWid->setLayout(mainLayout);
    mainWid->setFixedWidth(200);

    // 设置滑动条
    scrollArea = new CustomSiderBarScrollArea(this);
    scrollArea->setAccessibleName("kylin-music_sidebarwidget_CustomSiderBarScrollArea_scrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove",false);
    scrollArea->setFrameStyle(0);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(mainWid);

    logoWid = new QWidget(this);
    logoWid->setAccessibleName("kylin-music_sidebarwidget_QWidget_logoWid");
    logoWid->setFixedHeight(40);
    logoWid->setLayout(logoLayout);
    logoWid->setObjectName("logoWid");

    // 放置活动条
    QVBoxLayout *scrollLayout = new QVBoxLayout(this);
    scrollLayout->addWidget(logoWid);
    scrollLayout->addSpacing(16);
    scrollLayout->addWidget(scrollArea);
    scrollLayout->setMargin(0);
    scrollLayout->setSpacing(0);

    this->setLayout(scrollLayout);

    this->setAutoFillBackground(true);
}

void SideBarWidget::initConnect()
{
    connect(myPlayListBtn,&QPushButton::clicked,this,&SideBarWidget::addPlayList);
    connect(playListBtn,SIGNAL(playall(QString)),this,SLOT(playAll(QString)));
    connect(newSonglistPup->confirmBtn, &QPushButton::clicked ,newSonglistPup, &PopupDialog::confirmCheck);
    connect(newSonglistPup, &PopupDialog::accessConfirmCheck,this, &SideBarWidget::addItemToSongList);
    connect(newSonglistPup->enterLineEdit,&QLineEdit::returnPressed,newSonglistPup,&PopupDialog::confirmCheck);

    connect(renameSongListPup->confirmBtn, &QPushButton::clicked ,renameSongListPup, &PopupDialog::confirmCheck);
    connect(renameSongListPup, &PopupDialog::accessConfirmCheck,this, &SideBarWidget::renamePlayList);
    connect(renameSongListPup->enterLineEdit,&QLineEdit::returnPressed,renameSongListPup,&PopupDialog::confirmCheck);
}

void SideBarWidget::sidecolor()
{
    myPlayListLabel->setStyleSheet("line-height: 14px;color:#8C8C8C;");
    libraryLabel->setStyleSheet("line-height: 14px;");

    initAddButton();

    if(WidgetStyle::themeColor == 1) {
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(38, 38, 38));
        setAutoFillBackground(true);
        setPalette(pal);

        QList<CustomToolButton *> list = this->findChildren<CustomToolButton *>();

        for(CustomToolButton *tmp : list) {
            tmp->defaultStyle();
        }
    } else if(WidgetStyle::themeColor == 0) {
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(255, 255, 255));
        setAutoFillBackground(true);
        setPalette(pal);

        QList<CustomToolButton *> list = this->findChildren<CustomToolButton *>();

        for(CustomToolButton *tmp : list) {
            tmp->defaultStyle();
        }
    }
    updateSiderColor(false);
    scrollArea->updateScrollBarStyle(WidgetStyle::themeColor);
}


/**
 * @brief SideBarWidget::getPlayListName 获取歌单名称
 */
void SideBarWidget::getPlayListName()
{
    int ret = g_db->getPlayList(playListName);

    if(ret != DB_OP_SUCC) {
        qDebug() << "获取歌单名信息失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        return;
    }

    QString listName;

    for(int i = 0; i < playListName.size();i++) {
        listName = playListName.at(i);

        if(listName == SEARCH) {
            continue;
        }

        CustomToolButton *newBtn = new CustomToolButton;
        newBtn->setAccessibleName("kylin-music_sidebarwidget_CustomToolButton_newBtn");
        newPlayListLayout->setAlignment(Qt::AlignTop);
        newPlayListLayout->addWidget(newBtn);
        newPlayListLayout->setSpacing(6);
        if(listName == ALLMUSIC) {
            newBtn->setLableText(tr("all music"));
            newBtn->setToolTip(tr("all music"));
            newBtn->buttonListName = ALLMUSIC;
            newBtn->defaultStyle();
        }
        if(listName == FAV) {
            newBtn->setLableText(tr("I Love"));
            newBtn->setToolTip(tr("I Love"));
            newBtn->buttonListName = FAV;
            newBtn->defaultStyle();
        } else {
            QString showText = elideText(listName);
            newBtn->setLableText(showText);
            newBtn->setToolTip(listName);
            newBtn->buttonListName = "";
            newBtn->defaultStyle();
        }

        connect(newBtn,SIGNAL(playall(QString)),this,SLOT(playAll(QString)));
        connect(newBtn,SIGNAL(renamePlayList(QString)),this,SLOT(rename(QString)));
        connect(newBtn,SIGNAL(removePlayList(QString)),this,SLOT(removePlayList(QString)));
        connect(newBtn,&CustomToolButton::selectButtonChanged,this,&SideBarWidget::slotButtonClicked);
        connect(newBtn, &CustomToolButton::sigShowMenu, this, [this]() {
            emit sigShowMenu();
        });

        QString name = playController::getInstance().getPlayListName();

        if(listName == name ) {
            m_lastBtn = newBtn;
        }
    }

    if(m_lastBtn != nullptr) {
        m_lastBtn->clicked();
    }

}

/**
 * @brief SideBarWidget::addPlayList 添加歌单弹窗输入框
 */
void SideBarWidget::addPlayList()
{
    newSonglistPup->enterLineEdit->clear();
    // QRect availableGeometry = this->parentWidget()->geometry();
    newSonglistPup->show();
    // newSonglistPup->move(availableGeometry.center() - newSonglistPup->rect().center());
    newSonglistPup->enterLineEdit->setFocus();
}

/**
 * @brief SideBarWidget::addItemToSongList 新建歌单
 * 添加歌单到侧边栏
 */
void SideBarWidget::addItemToSongList()
{
#ifndef WHITE_TEST
    CustomToolButton *newBtn = new CustomToolButton();
    newBtn->setAccessibleName("kylin-music_sidebarwidget_CustomToolButton_newBtn");
    newPlayListLayout->setAlignment(Qt::AlignTop);
    newPlayListLayout->setSpacing(6);

    QString text = newSonglistPup->enterLineEdit->text();
    qDebug() << "新建歌单： " << text;

    //    QString showText = fm.elidedText(text, Qt::ElideRight, 200);

    if(text != "") {
        for(int i = 0;i< playListName.size();i++) {
            if(playListName.at(i) == text) {
                newSonglistPup->hide();
                QMessageBox::warning(Widget::mutual,tr("Prompt information"),tr("Single song name already exists!!!"),QMessageBox::Ok);
                return ;
            }
        }
        QString showText = elideText(text);
        newBtn->setLableText(showText);
        newBtn->setToolTip(text);
        newBtn->buttonListName = "";
        newBtn->defaultStyle();
        newPlayListLayout->addWidget(newBtn);
        playListName.append(text);
        newSonglistPup->hide();

        g_db->createNewPlayList(text);
        Q_EMIT playListAdded(text);
    } else {
        QString listName = newPlayListName();
        newBtn->setLableText(listName);
        newBtn->buttonListName = "";
        newBtn->defaultStyle();
        newPlayListLayout->addWidget(newBtn);
        playListName.append(listName);
        newSonglistPup->hide();
        g_db->createNewPlayList(listName);
        Q_EMIT playListAdded(listName);
    }

    connect(newBtn,SIGNAL(playall(QString)),this,SLOT(playAll(QString)));
    connect(newBtn,SIGNAL(renamePlayList(QString)),this,SLOT(rename(QString)));
    connect(newBtn,SIGNAL(removePlayList(QString)),this,SLOT(removePlayList(QString)));
    connect(newBtn,&CustomToolButton::selectButtonChanged,this,&SideBarWidget::slotButtonClicked);
    connect(newBtn, &CustomToolButton::sigShowMenu, this, [this]() {
        emit sigShowMenu();
    });
#endif
}


/**
 * @brief SideBarWidget::newPlayListName 输入为空添加歌单
 * @return
 */
QString SideBarWidget::newPlayListName()
{
#ifndef WHITE_TEST
    QMap<QString, QString> exitsName;
    for (auto &name : playListName)
    {
        exitsName.insert(name, name);
    }

    QString name = tr("New Playlist");
    if(!exitsName.contains(name))
    {
        return name;
    }

    int i = 1;
    for(i = 1; i < exitsName.size() + 1; i++)
    {
        QString newName = QString("%1 %2").arg(name).arg(i);
        if(!exitsName.contains(newName))
        {
            return newName;
        }
    }

    return QString("%1 %2").arg(name).arg(i);
#endif
}
#ifndef WHITE_TEST
/**
 * @brief SideBarWidget::mousePressEvent 设置按钮点击时的样式
 * 鼠标点击侧边栏控件时，控件重绘。否则会出现菜单栏和其他的不匹配问题。
 *
 * @param event
 */
void SideBarWidget::mousePressEvent(QMouseEvent *event)
{
    this->repaint();
    QWidget::mousePressEvent(event);
}

/**
 * @brief SideBarWidget::mouseMoveEvent
 * 鼠标划过侧边栏控件时，控件重绘。否则会出现菜单栏和其他的不匹配问题。
 * @param event
 */
void SideBarWidget::mouseMoveEvent(QMouseEvent *event)
{
    this->repaint();
    QWidget::mouseMoveEvent(event);
}

//鼠标进入侧边栏重绘
void SideBarWidget::enterEvent(QEvent *event)
{
    this->repaint();
    QWidget::enterEvent(event);
}
#endif
void SideBarWidget::updateSiderColor(bool isWindowActive)
{
    if (isWindowActive) {
        m_pSideColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-normal") ;
    }else{
        m_pSideColor = ThemeController::getPaletteColorFromDT("kcontain-secondary-inactive") ;
    }
    m_ukuiWindowHelper->setBlurEffect(QRegion(), 0, isWindowActive);
    adjustBackground();
    update();
}
#ifndef WHITE_TEST
bool SideBarWidget::eventFilter(QObject *target, QEvent *event)
{
    if (target == this && (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) || event->type() == QEvent::ActivationChange ||event->type() == QEvent::WindowActivate ||event->type() == QEvent::WindowDeactivate ) {
        if ( event->type() == QEvent::WindowActivate ) {
            updateSiderColor(true);
        } else if(event->type() == QEvent::WindowDeactivate){
            updateSiderColor(false);
        }
    }
    if (target == myPlayListBtn){
        if (event->type() == QEvent::HoverMove){
            myPlayListBtn->setIconHighlight(true);
        }else if (event->type() == QEvent::HoverLeave){
            if(WidgetStyle::themeColor == 1) {
                myPlayListBtn->setIconHighlight(false);
                myPlayListBtn->setIconColor(Qt::white);
                myPlayListBtn->setBackgroundColor("#1d1d1d");
            } else if(WidgetStyle::themeColor == 0) {
                myPlayListBtn->setIconHighlight(false);
                myPlayListBtn->setIconColor("#1d1d1d");
                myPlayListBtn->setBackgroundColor(Qt::white);
            }

        }
    }
    return QWidget::eventFilter(target, event);
}
#endif
void SideBarWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), m_pSideColor);
    QWidget::paintEvent(event);
}

/**
 * @brief SideBarWidget::playAll 歌单播放
 * @param btnText
 */
#ifndef WHITE_TEST
void SideBarWidget::playAll(QString btnText)
{
    Q_EMIT signalPlayAll(btnText);
}

void SideBarWidget::rename(QString text)
{
    btnText = text;
    // renameSongListPup->enterLineEdit->clear();
    renameSongListPup->enterLineEdit->setText(text);
    QRect availableGeometry = this->parentWidget()->geometry();
    renameSongListPup->show();
    renameSongListPup->move(availableGeometry.center() - renameSongListPup->rect().center());
    renameSongListPup->enterLineEdit->setFocus();
}
#endif
//字符截断函数
QString SideBarWidget::elideText(const QString &text)
{
    QFontMetrics fm(m_elideFont);
    if (fm.horizontalAdvance(text) > m_maxElideWidth){
        return fm.elidedText(text, Qt::ElideRight, m_maxElideWidth);
    }
    return text;
}
/**
 * @brief SideBarWidget::renamePlayList 歌单重命名
 */
void SideBarWidget::renamePlayList()
{
    QList<CustomToolButton *> list = this->findChildren<CustomToolButton *>();

    for(CustomToolButton *tmp : list) {
        if(tmp->toolTip() ==  btnText) {
            QString text = renameSongListPup->enterLineEdit->text();

            if(text != "") {
                for(int i = 0;i< playListName.size();i++) {
                    if(playListName.at(i) == text) {
                        renameSongListPup->hide();
#ifndef WHITE_TEST
                        QMessageBox::warning(Widget::mutual,tr("Prompt information"),tr("Single song name already exists!!!"),QMessageBox::Ok);
#endif
                        return ;
                    }
                }
                QString showText = elideText(text);
                tmp->setLableText(showText);
                tmp->setToolTip(text);
                tmp->click();
                //当前重命名列表为播放列表时
                if (btnText == playController::getInstance().getPlayListName()){
                    playController::getInstance().setCurList(text);
                }

                for(int i = 0; i < playListName.size();i++) {
                    if(btnText == playListName[i]) {
                        playListName[i] = text;
                    }
                }
                g_db->renamePlayList(btnText,text); // 从数据库中重命名
                Q_EMIT playListRenamed(btnText,text);   //fff
                btnText = text;
                tmp->click();
                renameSongListPup->hide();
            } else {
                QString listName = btnText;
                tmp->setLableText(listName);
                playController::getInstance().setCurList(listName);
                renameSongListPup->hide();
            }
        }
    }
}


/**
 * @brief SideBarWidget::removePlayList 删除歌单
 * @param text
 */
void SideBarWidget::removePlayList(QString text)
{
    QList<CustomToolButton *> list = this->findChildren<CustomToolButton *>();
    CustomToolButton *tmp = nullptr;

    for(auto i = list.begin();i < list.end();i++) {
        if((*i)->toolTip() == text) {
            if(i+1 == list.end()) {
                tmp = *(i-1);
                tmp->click();
            } else {
                tmp = *(i+1);
                tmp->click();
            }

            //删除歌单时，删除改播放歌单的所有media
            int ret;
            QList<musicDataStruct> musicDateList;
            ret = g_db->getSongInfoListFromPlayList(musicDateList,text);

            if(ret == DB_OP_SUCC) {
                for(int j=0; j<musicDateList.size(); j++) {
                    playController::getInstance().removeSongFromCurList(text, 0);
                }
            }

            (*i)->deleteLater();
            g_db->delPlayList(text);
            playListName.removeOne(text);
        }
    }

    Q_EMIT playListRemoved(text);
}


/**
 * @brief SideBarWidget::slotListSearch 取消侧边栏所有按钮的选中状态
 */
void SideBarWidget::slotListSearch()
{
    QList<CustomToolButton *> list = this->findChildren<CustomToolButton *>();

    for(CustomToolButton *tmp : list) {
        tmp->setStatusTip("");
        //tmp->buttonListName = "";
        tmp->defaultStyle();
    }
}


/**
 * @brief SideBarWidget::slotSongListHigh 高亮歌曲列表按钮
 */
void SideBarWidget::slotSongListHigh()
{
    QList<CustomToolButton *> list = this->findChildren<CustomToolButton *>();

    for(CustomToolButton *tmp : list) {
        tmp->setStatusTip("");
        tmp->buttonListName = "";
        tmp->defaultStyle();
    }

    playListBtn->setStatusTip(IS_SELECT);
    playListBtn->setChecked(true);
    playListBtn->setProperty("isImportant", true);
    playListBtn->buttonListName = ALLMUSIC;
    playListBtn->defaultStyle();
}

void SideBarWidget::slotButtonClicked(QString listname)
{
    setButtonStyle(listname);

    emit playListBtnClicked(listname);
    emit playListClick();
    g_user_signal->lyricsButtonIsShow = false;
}

/**
 * @brief SideBarWidget::setButtonStyle 设置当前点击按钮和其他按钮的样式
 * @param listname 当前选中按钮的文本
 */
void SideBarWidget::setButtonStyle(QString listname)
{
    QList<CustomToolButton *> list = this->findChildren<CustomToolButton *>();

    for(CustomToolButton *tmp : list) {
        if (tmp->toolTip() == listname) {
            tmp->setChecked(true);
            newPlayListLayout->update();
        } else {
            tmp->setChecked(false);
            newPlayListWidget->update();
        }
    }
}

void SideBarWidget::freshLogoWid()
{
    logoWid->repaint();
}

/**
 * @brief SideBarWidget::setThemeIcon 设置侧边栏图标名称
 * @param themeIcon 用户更改的图标主题名称
 */
void SideBarWidget::setThemeIcon(QString themeIcon)
{
    logoLabel->setFixedSize(24, 24);
    logoLabel->setPixmap(QIcon::fromTheme("kylin-music").pixmap(logoLabel->size()));
    logoLabel->setScaledContents(true);
}

void SideBarWidget::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_11;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font = this->font();
    font.setPointSizeF(nowFontSize);
    logoNameLabel->setFont(font);

    // 更新省略号使用的字体并同步所有歌单按钮
    m_elideFont = font;
    QList<CustomToolButton *> list = this->findChildren<CustomToolButton *>();
    for (CustomToolButton *btn : list) {
        btn->setLabelFont(font);
    }
    refreshAllButtonElideText();
}

void SideBarWidget::refreshAllButtonElideText()
{
    QList<CustomToolButton *> list = this->findChildren<CustomToolButton *>();
    // 用第一个有效按钮的文本标签宽度更新省略上限
    for (CustomToolButton *btn : list) {
        int w = btn->textLabelWidth();
        if (w > 0) {
            m_maxElideWidth = w;
            break;
        }
    }
    for (CustomToolButton *btn : list) {
        QString originalText = btn->toolTip();
        if (originalText.isEmpty()) {
            originalText = btn->getLableText();
        }
        if (!originalText.isEmpty()) {
            btn->setLableText(elideText(originalText));
        }
    }
}

void SideBarWidget::adjustBackground(bool isFirst)
{
    QWidget *mainWindow = this->window(); // 所有子部件通过 window() 可获取顶层窗口
    setAutoFillBackground(true);
    if(mainWindow->isActiveWindow() || isFirst)
    {
        m_pSideColor.setAlphaF(tranSparency);
    } 
    this->show();
}
