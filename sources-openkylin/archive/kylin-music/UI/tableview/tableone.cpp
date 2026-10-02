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

#include <QMimeData>
#include <QDirIterator>
#include <QApplication>
#include <QFile>
#include <QFileDevice>

#include <peony-qt/file-operation-utils.h>
#include <ukui-log4qt.h>
#include "tableone.h"
#include "UI/mainwidget.h"
#include "UI/globalsignal.h"
#include "UI/base/kaction.h"
QString TableOne::m_lastSavePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

#define PT_18 18

TableOne::TableOne(QString listName, QWidget *parent)
    : QWidget(parent),
      nowListName(listName)
{
    setObjectName("TableOne");
    setAcceptDrops(true);
    initUI();
    initRightMenu();
    initConnect();
//    initStyle();
    qDebug() << "歌曲列表初始化完成";
}

TableOne::~TableOne()
{
//    m_warningBox->deleteLater();
}

void TableOne::initStyle()
{
    listTitleLabel->setStyleSheet("font-weight: 600; line-height: 24px;");
    listTotalNumLabel->setStyleSheet("font-weight: 400;color:#8C8C8C; border:none; line-height: 14px;");

    m_view->setAlternatingRowColors(false);

    if (WidgetStyle::themeColor == 1) {
        nullPageIconLabel->setPixmap(QPixmap(":/img/default/defaultIconDark.svg").scaled(96, 96));
        this->setStyleSheet("#TableOne{background:#1d1d1d;border:none;}");
        titleWid->setStyleSheet("#titleWid{background:#1d1d1d;}");
        musicWidget->setStyleSheet("#musicWidget{background:#1d1d1d;}");
        nullPageWidget->setStyleSheet("#nullPageWidget{background:#1d1d1d;}");
        m_view->setStyleSheet("#tableView{background:#1d1d1d;border:none;}"); // 去掉treeview边框
        nullPageTextLabel->setStyleSheet("font-size: 14px; color:#8F9399; opacity: 1;");

    } else if(WidgetStyle::themeColor == 0) {
        nullPageIconLabel->setPixmap(QPixmap(":/img/default/defaultIconLight.svg").scaled(96, 96));
        this->setStyleSheet("#TableOne{background:#FFFFFF;border:none;}");
        titleWid->setStyleSheet("#titleWid{background:#FFFFFF;}");
        musicWidget->setStyleSheet("#musicWidget{background:#FFFFFF;}");
        nullPageWidget->setStyleSheet("#nullPageWidget{background:#FFFFFF;}");
        m_view->setStyleSheet("#tableView{background:#FFFFFF;border:none;}");
        nullPageTextLabel->setStyleSheet("font-size: 14px; color:#8C8C8C; opacity: 1;");

    }
}

void TableOne::initTableViewStyle()
{
    if (!m_view->header()->length()) {
        return;
    }

    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setColumnWidth(0, 312);
    m_view->setColumnWidth(1, 120);
    m_view->setColumnWidth(2, 120);
    m_view->setColumnWidth(3, 138);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_view->header()->setEnabled(false); // 设置表头不可用
//    tableView->header()->setHighlightSections(false);
    m_view->header()->setFixedHeight(36);

    // fix bug#155826, 音乐窗口最大化，专辑列和时长列间距较大
//    m_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);
//    m_view->header()->setSectionResizeMode(1, QHeaderView::Stretch);
//    m_view->header()->setSectionResizeMode(2, QHeaderView::Stretch);
//    m_view->header()->setSectionResizeMode(3, QHeaderView::Stretch);



    m_model->m_model.setHorizontalHeaderLabels(m_model->titleList);

    // 表头内容样式，满足设计图
#if 1
    if(WidgetStyle::themeColor == 0) {
        m_view->header()->setStyleSheet("QHeaderView::section,QTableCornerButton::section {padding-left: 1px; \
                                                                border: none ;border-bottom: 1px solid white;\
                                                                border-right: 1px solid white;border-bottom: 1px transparent;\
                                                                background-color:white;color:#8F9399;}");
    } else {
        m_view->header()->setStyleSheet("QHeaderView::section,QTableCornerButton::section {padding-left:1px; \
                                                        border: none;border-bottom: 1px solid #1d1d1d;\
                                                        border-right: 1px solid #1d1d1d;border-bottom: 1px transparent;\
                                                        background-color:#1d1d1d;color:#8F9399;}");
    }

    m_view->header()->setDefaultAlignment(Qt::AlignLeft);
#endif

    m_model->m_model.setHorizontalHeaderLabels(m_model->titleList);
    //列表刷新，时长右对齐需加到默认表头左对齐之后
    m_model->m_model.horizontalHeaderItem(m_model->titleList.length()-1)->setTextAlignment(Qt::AlignRight);

    m_view->verticalScrollBar()->setStyleSheet("width:15px;");
    m_view->verticalScrollBar()->setProperty("drawScrollBarGroove",false);

#if 0
    // 由于已经设置了，
    if (m_model->count() <= showScrollbarNumber) {
        m_view->verticalScrollBar()->setVisible(false);
    } else {
        m_view->verticalScrollBar()->setVisible(true);
    }
#endif
}

void TableOne::initUI()
{
    listTitleLabel = new QLabel(this);
    listTitleLabel->setAccessibleName("kylin-music_QLabel_listTitleLabel");
    listTitleLabel->setAlignment(Qt::AlignBottom);
    listTitleLabel->setMaximumWidth(200);
    listTitleLabel->setFixedHeight(45);
    listTotalNumLabel = new QLabel(this);
    listTotalNumLabel->setAccessibleName("kylin-music_QLabel_listTotalNumLabel");
    listTotalNumLabel->setAlignment(Qt::AlignBottom);
    listTotalNumLabel->setFixedHeight(42); // 与listTieleLabel高度应该保持一致，修改时请注意
    addMusicButton = new QToolButton(this);
    addMusicButton->setAccessibleName("kylin-music_tableone_QToolButton_addMusicButton");
    addMusicButton->setFixedSize(125,36);
    listTitleHBoxLayout = new QHBoxLayout();
    playAllButton = new QPushButton(this);
    playAllButton->setAccessibleName("kylin-music_QPushButton_playAllButton");
    playAllButton->setFixedSize(125,36);


    titleWid = new QWidget(this);
    titleWid->setAccessibleName("kylin-music_tableone_QWidget_titleWid");
    titleWid->setObjectName("titleWid");
    titleWid->setLayout(listTitleHBoxLayout);
    listTitleHBoxLayout->setMargin(0);
    listTitleHBoxLayout->setSpacing(0);
    listTitleHBoxLayout->setAlignment(Qt::AlignLeft);
    listTitleHBoxLayout->addWidget(listTitleLabel);
    listTitleHBoxLayout->addSpacing(16);
    listTitleHBoxLayout->addWidget(listTotalNumLabel);
    listTitleHBoxLayout->addStretch();
    listTitleHBoxLayout->addWidget(addMusicButton, 1, Qt::AlignRight);
    listTitleHBoxLayout->addSpacing(16);
    listTitleHBoxLayout->addWidget(playAllButton, 0, Qt::AlignRight);
    listTitleHBoxLayout->setContentsMargins(30,20,25,30);
    titleWid->setFixedHeight(95);

    playAllButton->setText(tr("Play All"));
    this->displayBtn(playAllButton,tr("Play All"));
    playAllButton->setProperty("isImportant", true);

    addMusicButton->setText(tr("Add Music"));
    this->displayBtn(addMusicButton,tr("Add Music"));
//    addMusicButton->setProperty("isImportant",true);
    addMusicButton->setPopupMode(QToolButton::InstantPopup);
    add_menu = new QMenu(this);
    add_menu->setAccessibleName("kylin-music_QMenu_add_menu");

    addMusicFileAction = new QAction(this);
    addMusicFileAction->setObjectName("kylin-music_tableone_QAction_addMusicFileAction");
    addMusicFileAction->setText(tr("Open File"));

    addDirMusicAction = new QAction(this);
    addDirMusicAction->setObjectName("kylin-music_tableone_QAction_addDirMusicAction");
    addDirMusicAction->setText(tr("Open Folder"));
    add_menu->addAction(addMusicFileAction);
    add_menu->addAction(addDirMusicAction);
    addMusicButton->setMenu(add_menu);


    m_view = new CustomTreeView(false);
    m_view->setAccessibleName("kylin-music_tableone_CustomTreeView_m_view");
    m_view->setObjectName("tableView"); // 可以通过 #tableView 设置样式表
    m_view->setFocusPolicy(Qt::NoFocus);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers); // 使用右键菜单，需启用该属性
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中模式为选中行
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);//设置按ctrl键选中多个
    QList<musicDataStruct> resList;
    int ret;
    ret = g_db->getSongInfoListFromDB(resList,nowListName);
    if(ret == DB_OP_SUCC) {
        m_model = new MusicListModel;
        m_model->setObjectName("kylin-music_tableone_MusicListModel_m_model");
        m_model->add(resList);
        m_model->setView(*m_view);
    }


    // 将treeview 放到一个布局中，这样可以调整间隔
    tableHBoxLayout = new QHBoxLayout();
    tableHBoxLayout->setSpacing(0);
    tableHBoxLayout->addWidget(m_view);
    tableHBoxLayout->setContentsMargins(30, 0, 30, 0);

    // 将布局放到一个widget中，然后可以设置该widget的背景，否则光一个layout会出现其他地方透明
    musicWidget = new QWidget(this);
    musicWidget->setAccessibleName("kylin-music_tableone_QWidget_musicWidget");
    musicWidget->setObjectName("musicWidget");
    musicWidget->setLayout(tableHBoxLayout);

    nullPageWidget = new QWidget(this);
    nullPageWidget->setAccessibleName("kylin-music_tableone_QWidget_nullPageWidget");
    nullPageWidget->setObjectName("nullPageWidget");

    nullPageHLayout = new QHBoxLayout();
    nullPageVLayout = new QVBoxLayout();
    nullPageIconLabel = new QLabel(this);
    nullPageIconLabel->setAccessibleName("kylin-music_QLabel_nullPageIconLabel");
    nullPageTextLabel = new QLabel(this);
    nullPageTextLabel->setAccessibleName("kylin-music_QLabel_nullPageTextLabel");
    n_addDirMusicButton = new QPushButton(this);
    n_addDirMusicButton->setAccessibleName("kylin-music_QPushButton_n_addDirMusicButton");
    n_addMusicButton = new QPushButton(this);
    n_addMusicButton->setAccessibleName("kylin-music_QPushButton_n_addMusicButton");

//    n_addDirMusicButton->setFixedSize(110,36);
//    n_addMusicButton->setFixedSize(110,36);
    nullPageTextLabel->setText(tr("There are no songs!"));
    n_addMusicButton->setText(tr("Open File"));
    n_addDirMusicButton->setText(tr("Open Folder"));

    if (WidgetStyle::themeColor == 1) {
        // black theme
        nullPageIconLabel->setPixmap(QPixmap(":/img/default/defaultIconDark.svg").scaled(96, 96));
    } else {
        nullPageIconLabel->setPixmap(QPixmap(":/img/default/defaultIconLight.svg").scaled(96, 96));
    }

    nullPageWidget->setLayout(nullPageVLayout);
    nullPageVLayout->addStretch();
    nullPageVLayout->addWidget(nullPageIconLabel);
    nullPageIconLabel->setAlignment(Qt::AlignHCenter);
    nullPageVLayout->addSpacing(0);
    nullPageVLayout->addWidget(nullPageTextLabel);
    nullPageVLayout->addSpacing(24);
    nullPageTextLabel->setAlignment(Qt::AlignHCenter);
    nullPageVLayout->addLayout(nullPageHLayout);
    nullPageVLayout->addStretch();

    nullPageHLayout->addWidget(n_addMusicButton);
    nullPageHLayout->setSpacing(16);
    nullPageHLayout->addWidget(n_addDirMusicButton);

    nullPageVLayout->addStretch();
    nullPageVLayout->setAlignment(Qt::AlignCenter);
    nullPageVLayout->setSpacing(0);
    nullPageVLayout->setMargin(0);

    m_warningBox = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("Single song name is already exists"), QMessageBox::Ok,Widget::mutual);
    initTableViewStyle();
    listTotalNumLabel->setFixedWidth(120);
//    listTitleLabel->setFixedWidth(120);
    showTitleText(nowListName);
    changeNumber();

    QVBoxLayout *tableLayout = new QVBoxLayout();
    tableLayout->addWidget(titleWid, Qt::AlignTop);
    tableLayout->addWidget(musicWidget);
    tableLayout->addWidget(nullPageWidget);
    tableLayout->setMargin(0);
    tableLayout->setSpacing(0);
    this->setLayout(tableLayout);
}

//字体的截断显示显示函数
template<typename ButtonType>
void TableOne::displayBtn(ButtonType *btn, QString btnTooltip)
{
#ifndef WHITE_TEST
    QLocale locale;
    if(locale.language()==QLocale::Tibetan)
    {
        btn->setText(btnTooltip);
        QFontMetrics  fontMetrics(btn->font());
        int fontSize = fontMetrics.width(btnTooltip);
        QString tmpStr = btnTooltip;
        if (fontSize > (btn->width() - 7)) {
            tmpStr = btnTooltip.left(btnTooltip.length() * (btn->width()) / fontSize   - 7)  + "...";
            btn->setText(tmpStr);
            btn->setToolTip(btnTooltip);
        } else {
            btn->setText(btnTooltip);
        }
    }
#endif
}

void TableOne::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_18;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    listTitleLabel->setFont(font);

    selectListChanged(listTitleLabel->text());

    if (infoDialog != nullptr) {
        infoDialog->slotLableSetFontSize(size);
    }

    //改变字体大小时 截断字体省略显示
    this->displayBtn(playAllButton,tr("Play All"));
    this->displayBtn(addMusicButton,tr("Add Music"));
}


/**
 * @brief TableOne::showTitleText 通过列表名和索引值播放相应歌曲
 * @param listName
 */
void TableOne::showTitleText(QString listName)
{
    QFontMetrics fontWidth(listTitleLabel->font());//得到每个字符的宽度
    QString elideNote = fontWidth.elidedText(listName, Qt::ElideRight, 200);//最大宽度150像素
    if(listName == ALLMUSIC) {
        listTitleLabel->setText(tr("Song List"));
    } else if(listName == FAV){
        listTitleLabel->setText(tr("I Love"));
    } else if(listName == SEARCH) {
        listTitleLabel->setText(tr("Search Result"));
    } else {
        listTitleLabel->setText(elideNote);
    }
    if(elideNote != listName) {
        listTitleLabel->setToolTip(listName);
    }
    else{
        listTitleLabel->setToolTip("");
    }
}
void TableOne::initConnect()
{
    connect(m_view,&CustomTreeView::doubleClicked,this,&TableOne::playSongs);
    connect(m_view,&CustomTreeView::doubleClickedSignal,this,&TableOne::playSongs);
    connect(m_view,&CustomTreeView::customContextMenuRequested,this,&TableOne::showRightMenu);
    connect(addMusicFileAction,&QAction::triggered,this,&TableOne::addMusicSlot);
    connect(addDirMusicAction,&QAction::triggered,this,&TableOne::addDirMusicSlot);
    connect(this,&TableOne::countChanges,this,&TableOne::changeNumber);
    connect(&playController::getInstance(),&playController::currentIndexAndCurrentList,this,&TableOne::getHightLightIndex);
    connect(n_addMusicButton,&QPushButton::clicked,this,&TableOne::addMusicSlot);
    connect(n_addDirMusicButton,&QPushButton::clicked,this,&TableOne::addDirMusicSlot);
    connect(playAllButton,&QPushButton::clicked,this,&TableOne::playAllButtonClicked);
    connect(g_user_signal, &GlobalUserSignal::sigPlayFirst, this, [&](){
        if(m_model->getPathList(nowListName).length() == 0){
            return;
        }else{
            playFirst = true;
            playSongs();
        }
    });
    connect(g_user_signal, &GlobalUserSignal::sigOpenUri, this, [&](QString songFiles){
        QStringList songFilesList;
        songFilesList.append(songFiles);
        addMusicToDatebase(songFilesList);
    });
    connect(m_view, &CustomTreeView::showVerticalScrollBar, [this]() {
        listTitleHBoxLayout->setContentsMargins(30,20,25,30);
        tableHBoxLayout->setContentsMargins(30, 0, 6, 0);

//        m_view->setColumnWidth(0, 312);
//        m_view->setColumnWidth(1, 120);
//        m_view->setColumnWidth(2, 120);
//        m_view->setColumnWidth(3, 138);
    });

    connect(m_view, &CustomTreeView::hideVerticalScrollBar, [this]() {
        listTitleHBoxLayout->setContentsMargins(30,20,25,30);
        tableHBoxLayout->setContentsMargins(30, 0, 25, 0);

//        m_view->setColumnWidth(0, 312);
//        m_view->setColumnWidth(1, 120);
//        m_view->setColumnWidth(2, 120);
//        m_view->setColumnWidth(3, 138);
    });
}

void TableOne::initRightMenu()
{
    m_menu = new QMenu(m_view);
    m_menu->setAccessibleName("kylin-music_QMenu_m_menu");

    playRow = new QAction(tr("Play"));
    playRow->setObjectName("kylin-music_tableone_QAction_playRow");
    removeRow = new QAction(tr("Delete from list"));
    removeRow->setObjectName("kylin-music_tableone_QAction_removeRow");
    removeLocalRow = new QAction(tr("Remove from local"));
    removeLocalRow->setObjectName("kylin-music_tableone_QAction_removeLocalRow");
    showInfoRow = new QAction(tr("View song information"));
    showInfoRow->setObjectName("kylin-music_tableone_QAction_showInfoRow");
    addToOtherListMenu = new QMenu(tr("Add to songlist"));
    addToOtherListMenu->setAccessibleName("kylin-music_QMenu_addToOtherListMenu");
    connect(playRow,&QAction::triggered,this,&TableOne::playSongs);
    connect(removeRow,&QAction::triggered,this,&TableOne::isDeleteSongs);
    connect(removeLocalRow,&QAction::triggered,this,&TableOne::isDeleteLocalSongs);
    connect(showInfoRow,&QAction::triggered,this,&TableOne::showInfo);
    //connect(addToOtherListMenu,&QMenu::triggered,this,&TableOne::addToOtherList);

    m_view->installEventFilter(this);
}

void TableOne::showRightMenu(const QPoint &pos)
{
#ifndef WHITE_TEST
    Q_EMIT sigShowMenu();

    QModelIndex index = m_view->indexAt(pos);
    if(index.row() < 0) {
        return;
    }

    //addToOtherListMenu 子菜单设置高度和滑动条
    QWidget *containerWidget = new QWidget();
    containerWidget->setAccessibleName("kylin-music_tableone_QWidget_containerWidget");
    QVBoxLayout *containerlayout = new QVBoxLayout(containerWidget);
    CustomSiderBarScrollArea *scrollArea = new CustomSiderBarScrollArea();
    scrollArea->setAccessibleName("kylin-music_tableone_CustomSiderBarScrollArea_scrollArea");
    scrollArea->setWidget(containerWidget);
    scrollArea->setMaximumHeight(400);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove",false);
    scrollArea->setFrameStyle(0);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidgetAction *scrollWidgetAction = new QWidgetAction(addToOtherListMenu);
    scrollWidgetAction->setObjectName("kylin-music_tableone_QWidgetAction_scrollWidgetAction");
    scrollWidgetAction->setDefaultWidget(scrollArea);
    addToOtherListMenu->clear();
//    addToOtherListMenu->addAction(scrollWidgetAction);

//    m_menu->move(0,0);
    m_menu->addAction(playRow);      //添加动作到菜单
//    addToOtherListMenu->setMaximumHeight(240); // 设置添加到歌单的菜单控件宽度
    m_menu->addMenu(addToOtherListMenu);
    m_menu->addAction(removeRow);  //添加动作到菜单
    m_menu->addAction(removeLocalRow);  //添加动作到菜单
    m_menu->addAction(showInfoRow); //添加动作到菜单
    QMap<int,QString> map1 = getSelectedTaskIdList();
//    addToOtherListMenu->clear();
    QStringList allList;
    int ret = g_db->getPlayList(allList);
    if(ret == 0) {
        for(int i = 0 ;i < allList.size() ;i++) {
            if(allList[i] == SEARCH) {
                continue;
            }

            QAction *listNameAct = new QAction;
            addToOtherListMenu->addAction(listNameAct);
//            QPushButton *listNameButton = new QPushButton();
//            listNameButton->setAccessibleName("kylin-music_QPushButton_listNameButton");
//            containerlayout->addWidget(listNameButton);

            if(allList[i] != FAV) {
                listNameAct->setText(allList[i]);
            } else {
                listNameAct->setText(tr("I Love"));
            }

//            listNameAct->setFixedWidth(220);

            connect(listNameAct,&QAction::triggered,this,[=](){ addToOtherList(listNameAct);});
        }
    } else {
        qDebug() << "获取所有歌单失败";
    }

    if(map1.size() > 1) {
        m_menu->removeAction(playRow);
        m_menu->removeAction(showInfoRow);
    }

    if(nowListName == SEARCH) {
        m_menu->removeAction(removeRow);
        m_menu->removeAction(removeLocalRow);
    }
    m_menu->exec(QCursor::pos());
#endif
}

void TableOne::tableViewDoubleClicked()
{
}


/**
 * @brief TableOne::isDeleteSongs  是否从歌单中删除歌曲
 */
void TableOne::isDeleteSongs()
{
#ifndef WHITE_TEST
    //确认将选中的歌曲从歌单中删除？
    QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr(""),tr("Confirm that the selected song will be deleted from the song list?"),QMessageBox::Ok | QMessageBox::Cancel, Widget::mutual);

    warn->setDefaultButton(QMessageBox::Ok); //解决弹窗无法删除的问题

        QRect availableGeometry = parentWidget()->geometry();


    //warn->setParent(qApp->activeWindow());
    qDebug() << " size " << availableGeometry;
//    warn->setGeometry(availableGeometry.x()+ (availableGeometry.width() - warn->size().width())/2,
//                      availableGeometry.y()+15+ (availableGeometry.height() - warn->size().height())/2,
//                      warn->width(), warn->height());
    QRect a =warn->geometry();
    qDebug() << " size " << a;
    int result = warn->exec();
    if(result == QMessageBox::Ok) {
        deleteSongs();
        qDebug() << "QMessageBox::Ok";
    } else {
        qDebug() << "QMessageBox::Cancel";
        return;
    }
#endif
}


/**
 * @brief TableOne::isDeleteLocalSongs 是否从本地删除歌曲
 */
void TableOne::isDeleteLocalSongs()
{
#ifndef WHITE_TEST
    //歌曲从本地删除后不可恢复，是否确定删除？
    QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr(""),tr("After the song is deleted from the local, it cannot be resumed. Is it sure to delete?"),QMessageBox::Ok | QMessageBox::Cancel, Widget::mutual);
//    warn->button(QMessageBox::Yes)->setText("确定");
//    warn->button(QMessageBox::No)->setText("取消");

    warn->setDefaultButton(QMessageBox::Ok);

    QRect availableGeometry =parentWidget()->geometry();
    qDebug() << " size " << availableGeometry;
//    warn->setGeometry(availableGeometry.x()+ (availableGeometry.width() - warn->size().width())/2,
//                      availableGeometry.y()+15+ (availableGeometry.height() - warn->size().height())/2,
//                      warn->width(), warn->height());
    QRect a =warn->geometry();
    qDebug() << " size " << a;
    int result = warn->exec();

    if(result == QMessageBox::Ok) {
        deleteLocalSongs();
        qDebug() << "QMessageBox::Ok";
    } else {
        qDebug() << "QMessageBox::Cancel";
        return;
    }
#endif
}


/**
 * @brief TableOne::deleteSongs  从歌单中删除歌曲
 */
void TableOne::deleteSongs()
{
#ifndef WHITE_TEST
    QMap<int,QString> map1 = getSelectedTaskIdList();

    while (!map1.empty()) {
        QMap<int,QString>::iterator iter;
        iter = map1.end();
        --iter;

        int ret;
        int index = -1;

        if(nowListName != nowPlayListName && nowListName == ALLMUSIC) {
            index = MusicFileInformation::getInstance().findIndexFromPlayList(nowPlayListName,iter.value());
        }

        if(nowListName != ALLMUSIC) {
           ret = g_db->delMusicFromPlayList(iter.value(),nowListName);
           Q_EMIT sigAddToRList(iter.value(),nowListName); //删除时加入历史列表的 removeList
        } else {
//           ret = g_db->delSongFromEveryWhere(iter.value());
           ret = g_db->delSongFromAllAndPlayList(iter.value());  //不从history删除
           Q_EMIT sigAddToRList(iter.value(),nowListName);
        }

        if(ret == 0) {
//            m_model->remove(iter.key());

            if((nowListName != nowPlayListName) && nowListName == ALLMUSIC && index != -1) {
                playController::getInstance().removeSongFromCurList(nowPlayListName,index);
            } else {//
                playController::getInstance().removeSongFromCurList(nowListName,iter.key());
            }

            if(nowListName == FAV)
            {
                Q_EMIT removeILoveFilepathSignal(iter.value());
            }

            map1.remove(iter.key());
        } else {
            qDebug() << "delete failed" << iter.value();
        }
    }

    selectListChanged(nowListName);

    Q_EMIT refreshHistoryListSignal();
#endif
}

bool TableOne::moveFileToTrashNative(const QString &filePath)
{
 #ifndef WHITE_TEST
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }
    // Linux 标准回收站目录
    QString trashRoot = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.local/share/Trash";
    QString trashFiles = trashRoot + "/files";
    QString trashInfo = trashRoot + "/info";

    // 自动创建回收站全部目录+修复权限（解决开明目录缺失/权限问题）
    QDir dir;
    dir.mkpath(trashFiles);
    dir.mkpath(trashInfo);

    // 重命名避免重名（时间戳后缀）
    QString fileName = fileInfo.fileName();
    QString timeStamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString destFile = QString("%1/%2_%3").arg(trashFiles).arg(fileName).arg(timeStamp);

    // 移动文件到回收站文件区
    bool fileMoveOk = QFile::rename(filePath, destFile);
    if(!fileMoveOk) return false;

    // 写入info元数据文件（桌面回收站识别必备：原始路径、删除时间）
    QString infoFileName = QString("%1_%2.trashinfo").arg(fileName).arg(timeStamp);
    QFile infoFile(trashInfo + "/" + infoFileName);
    if(infoFile.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&infoFile);
        out << "[Trash Info]\n";
        out << QString("Path=%1\n").arg(filePath);
        out << QString("DeletionDate=%1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss"));
        infoFile.close();
    }
    return true;
#endif
}

/**
 * @brief TableOne::deleteLocalSongs 从本地以及歌单中删除歌曲
 */
void TableOne::deleteLocalSongs()
{
#ifndef WHITE_TEST
    m_map = getSelectedTaskIdList();
    for (int key : m_map.keys()) {
        QString value = m_map.value(key);
        QFile file(value);
        if(file.exists()){
            bool trashSuccess = false;
            trashSuccess = moveFileToTrashNative(value);
            if (trashSuccess){
                int ret;
                int index = -1;
                if(nowListName != nowPlayListName && nowListName == ALLMUSIC){
                    index = MusicFileInformation::getInstance().findIndexFromPlayList(nowPlayListName,value);
                }
                if(nowListName != ALLMUSIC){
                    ret = g_db->delMusicFromPlayList(value,nowListName);
                }else{
                    ret = g_db->delSongFromEveryWhere(value);
                }
                if(ret == 0){
                    if((nowListName != nowPlayListName) && nowListName == ALLMUSIC && index != -1){
                        playController::getInstance().removeSongFromCurList(nowPlayListName,index);
                    }else{
                        playController::getInstance().removeSongFromCurList(nowListName,key);
                    }
                    if(nowListName == FAV){
                        Q_EMIT removeILoveFilepathSignal(value);
                    }
                    m_map.remove(key);
                }else{
                    qDebug() << "delete failed" << value;
                }
                selectListChanged(nowListName);
                Q_EMIT refreshHistoryListSignal();
            }else{
                qDebug() << "回收站移动失败：";
            }
        } else {
            // fix #160871 处理【音乐】从歌单删除置灰的本地歌曲，出现两次弹窗问题
            int ret;
            int index = -1;
            if(nowListName != nowPlayListName && nowListName == ALLMUSIC){
                index = MusicFileInformation::getInstance().findIndexFromPlayList(nowPlayListName,value);
            }
            if(nowListName != ALLMUSIC){
                ret = g_db->delMusicFromPlayList(value,nowListName);
            }else{
                ret = g_db->delSongFromEveryWhere(value);
            }
            if(ret == 0){
                if((nowListName != nowPlayListName) && nowListName == ALLMUSIC && index != -1){
                    playController::getInstance().removeSongFromCurList(nowPlayListName,index);
                }else{
                    playController::getInstance().removeSongFromCurList(nowListName,key);
                }
                if(nowListName == FAV) {
                    Q_EMIT removeILoveFilepathSignal(value);
                }
                m_map.remove(key);
            }else{
                qDebug() << "delete failed" << value;
            }
            selectListChanged(nowListName);
            Q_EMIT refreshHistoryListSignal();
        }
    }
#endif
}

void TableOne::playSongs()
{
    int index = m_view->currentIndex().row();

    m_model->getItem(index);
    if(playFirst){
        //当前为Stopped状态，调用了Play接口
        playMusicforIndex(nowListName,0);
        playFirst = false;
    }else{
        playMusicforIndex(nowListName,index);
    }
}

/**
 * @brief TableOne::playMusicforIndex
 * @param listName 歌单列表的名称
 * @param index 当前歌曲在歌单列表的位置
 */
void TableOne::playMusicforIndex(QString listName, int index)
{
    QStringList pathList;
    pathList = m_model->getPathList(listName);
    playController::getInstance().setCurPlaylist(listName,pathList);
    qDebug() << "开始播放点击音乐列表的歌曲： " << pathList << listName << "index = " << index;
    musicDataStruct date = m_model->getItem(index);
    QString playMusicFilePath = date.filepath;
    qDebug() << "点击音乐列表的歌曲： " << playMusicFilePath;
    playController::getInstance().play(listName,index);
    playController::getInstance().delayMsecondSetVolume();
    Q_EMIT sigRemoveFromRList( playMusicFilePath ); //播放时从 removeList 移除
}
void TableOne::showInfo()
{
 #ifndef WHITE_TEST
    int index = m_view->currentIndex().row();
    musicDataStruct date = m_model->getItem(index);
    infoDialog = new MusicInfoDialog(parentWidget(), date);
    infoDialog->setAccessibleName("kylin-music_tableone_MusicInfoDialog_infoDialog");

//    connect(infoDialog,&MusicInfoDialog::accepted,infoDialog,&MusicInfoDialog::deleteLater);
//    connect(infoDialog,&MusicInfoDialog::rejected,infoDialog,&MusicInfoDialog::deleteLater);

    //将弹窗应用居中显示
    QRect availableGeometry =qApp->activeWindow()->geometry();
    qDebug() << " size " << availableGeometry;
    infoDialog->setGeometry(availableGeometry.x()+ (availableGeometry.width() - infoDialog->size().width())/2,
                             availableGeometry.y() +15+ (availableGeometry.height() - infoDialog->size().height())/2,
                            infoDialog->width(), infoDialog->height());
    infoDialog->exec();
#endif
}

void TableOne::addToOtherList(QAction *listNameAction)
{
#ifndef WHITE_TEST
    int successCount = 0;
    int failCount = 0;
    QString listName = listNameAction->text();
    if(listName == tr("I Love")) {
        listName = FAV;
    }
    QMap<int,QString> map1 = getSelectedTaskIdList();
    int allCount = map1.size();
    QMap<int,QString>::Iterator it = map1.begin();
    while(it != map1.end())
    {
        int ret = g_db->addMusicToPlayList(it.value(),listName);
        if(ret == DB_OP_SUCC)
        {
            successCount++;
            playController::getInstance().addSongToCurList(listName,it.value());
//            if(listName == tr("I Love"))
            if(listName == FAV)
            {
                Q_EMIT addILoveFilepathSignal(it.value());
            }
        }else{
            failCount++;
        }
        it++;
    }
    importFailed(successCount, failCount, allCount);
#endif
}


/**
 * @brief TableOne::addMusicSlot  添加歌曲文件槽函数
 */
void TableOne::addMusicSlot()
{
#ifndef WHITE_TEST
    KyInfo() << "addMusicSlot";

    QFileDialog *fileDialog = new QFileDialog(Widget::mutual);
    fileDialog->setAccessibleName("kylin-music_tableone_QFileDialog_fileDialog");
//    fileDialog->setParent(Widget::mutual);
    QList<QUrl> list = fileDialog->sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        //华为990、9a0需要屏蔽最小系统挂载的目录
        if (fi.fileName() == "2691-6AB8")
             continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(fileDialog);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, fileDialog,
    [=, &sidebarNum, &mntUrlList, &list](const QString path) {
        QDir wmntDir(path);
        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList wfilist = wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
                       QFileInfo fi = wfilist.at(i);
                       //华为990、9a0需要屏蔽最小系统挂载的目录
                       if (fi.fileName() == "2691-6AB8")
                            continue;
                 mntUrlList << QUrl("file://" + fi.filePath());
             }
             fileDialog->setSidebarUrls(list + mntUrlList);

             fileDialog->update();
        });

    connect(fileDialog, &QFileDialog::finished, fileDialog, [=, &list]() {
        QList<QUrl> lists;
        QString homePath = "file://" + QDir::homePath();
        lists << QUrl("file://");
        lists << QUrl(homePath);
        fileDialog->setSidebarUrls(lists);
    });

    fileDialog->setSidebarUrls(list + mntUrlList);
    QString setFilter = tr("Audio File") + (" (%1) ");
    setFilter = setFilter.arg(MusicFileInformation::getInstance().getMusicType().join(" "));
    fileDialog->setNameFilter(setFilter);
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    fileDialog->setOption(QFileDialog::HideNameFilterDetails);

    QStringList selectDirPath  =
                QFileDialog::getOpenFileNames(Widget::mutual, tr("Select a file storage directory"), m_lastSavePath.isEmpty() ? "/" : m_lastSavePath , tr("All Files (*)"));
    if(!selectDirPath.isEmpty()) {
        m_lastSavePath = QFileInfo(selectDirPath.first()).absolutePath();
    } else {
        return;
    }

    for( int i = 0; i < selectDirPath.length(); i++ ) {
        qDebug() << "selectDirPath: " << selectDirPath.at(i) << selectDirPath.length();

    }
    addMusicToDatebase(selectDirPath);
#endif
}


/**
 * @brief TableOne::addDirMusicSlot   添加文件夹
 */
void TableOne::addDirMusicSlot()
{
#ifndef WHITE_TEST
    KyInfo() << "addDirMusicSlot";

    QFileDialog *fileDialog = new QFileDialog(Widget::mutual);
    fileDialog->setAccessibleName("kylin-music_tableone_QFileDialog_fileDialog");

    QList<QUrl> list = fileDialog->sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        //华为990、9a0需要屏蔽最小系统挂载的目录
        if (fi.fileName() == "2691-6AB8")
             continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(fileDialog);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, fileDialog,
            [=, &sidebarNum, &mntUrlList, &list](const QString path) {
        QDir wmntDir(path);
        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList wfilist = wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
                       QFileInfo fi = wfilist.at(i);
                       //华为990、9a0需要屏蔽最小系统挂载的目录
                       if (fi.fileName() == "2691-6AB8")
                            continue;
                 mntUrlList << QUrl("file://" + fi.filePath());
             }
             fileDialog->setSidebarUrls(list + mntUrlList);

             fileDialog->update();
        });

        connect(fileDialog, &QFileDialog::finished, fileDialog, [=, &list]() {
            QList<QUrl> lists;
            QString homePath = "file://" + QDir::homePath();
            lists << QUrl("file://");
            lists << QUrl(homePath);
            fileDialog->setSidebarUrls(lists);
        });

    //自己QFileDialog的用法，这里只是列子
//    fileDialog->setNameFilter(QLatin1String("*.mp3 *.ogg *.wma *.amr *.flac *.wav *.ape *.m4a *.ac3 *.aac"));

    fileDialog->setSidebarUrls(list + mntUrlList);
    fileDialog->setFileMode(QFileDialog::Directory);
    QString selectDirPath  =
                QFileDialog::getExistingDirectory(Widget::mutual, tr("Select a file storage directory"), m_lastSavePath.isEmpty() ? "/" : m_lastSavePath);
    QDir dir(selectDirPath);
    QStringList nameFilters;
    nameFilters << "*";
    dir.setNameFilters(nameFilters);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);

    QStringList m_fileNames = dir.entryList(MusicFileInformation::getInstance().getMusicType(), QDir::Files|QDir::Readable, QDir::Name);

    QStringList absolute;
    foreach (const QString& filename, m_fileNames) {
        absolute << dir.absoluteFilePath(filename);
    }
    if(!selectDirPath.isEmpty())
    {
        m_lastSavePath = selectDirPath;
    }
    else
    {
        return;
    }
//    foreach (QString dirPath, m_fileNames) {
//        QDir dir(dirPath);
//        QStringList fileNames = dir.entryList(MusicFileInformation::getInstance().getMusicType(), QDir::Files|QDir::Readable, QDir::Name);
//        for(int i = 0; i < fileNames.count(); i++)
//        {
//            fileNames[i] = QString(dirPath + "/" + fileNames[i]);
//        }

//    }

    addMusicToDatebase(absolute);
#endif
}

void TableOne::addMusicToDatebase(QStringList fileNames)
{
    //成功数量
    int successCount = 0;
    //失败数量
    int failCount = 0;

    MusicFileInformation::getInstance().addFile(fileNames);
    int allCount = MusicFileInformation::getInstance().getCount();
    int fail_count = MusicFileInformation::getInstance().getFailCount();
    QList<musicDataStruct> resList;
    resList = MusicFileInformation::getInstance().resList;

    int ret;
    foreach (const musicDataStruct date, resList)
    {
        if(nowListName == ALLMUSIC)
        {
            ret = g_db->addMusicToLocalMusic(date);
            selectListChanged(ALLMUSIC);
        } else if (nowListName == SEARCH) {
            return;
        } else {
            ret = g_db->addNewSongToPlayList(date,nowListName);   //数据库接口
        }

        if(ret == DB_OP_SUCC){
            successCount++;
            playController::getInstance().addSongToCurList(nowListName,date.filepath);
        }else if (ret == DB_OP_ADD_REPEAT){
#ifndef WHITE_TEST
            m_warningBox->show();
#endif
        }
//        else
//        {
//            failCount++;
//        }
    }

    selectListChanged(nowListName);
    importFinished(successCount, fail_count, allCount);
    playController::getInstance().delayMsecondSetVolume();

    qDebug() << "failCount = " << failCount;
}


/**
 * @brief TableOne::importFinished 成功添加多少首歌曲
 * @param successCount
 * @param m_failCount
 * @param allCount
 */
void TableOne::importFinished(int successCount, int m_failCount, int allCount)
{

    if(m_failCount > 0) {
        Widget::mutual->setCreatFinishMsg(tr("Add failed! Format error or no access permission!"));
        return;
    }

    if(successCount > 0)
    {
        if(allCount == 1) //单首歌不提示
        {
            return;
        }
        QMessageBox *warn = new QMessageBox(QMessageBox::NoIcon,tr("Prompt information"),tr("Success add %1 songs").arg(successCount),QMessageBox::Yes,Widget::mutual);
        warn->setIconPixmap(QIcon::fromTheme("ukui-dialog-success", QIcon::fromTheme("ukui-dialog-success")).pixmap(24, 24));
        warn->button(QMessageBox::Yes)->setText("确定");
        warn->exec();
    }
    else
    {
        //此处逻辑待优化，双击打开不应该提示错误
        //if(allCount == 0)
        {
            return;
        }
        QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("Add failed"),QMessageBox::Yes,Widget::mutual);
        warn->button(QMessageBox::Yes)->setText("确定");
        warn->exec();
    }
}


/**
 * @brief TableOne::importFailed 导入失败
 * @param successCount
 * @param m_failCount
 * @param allCount
 */
void TableOne::importFailed(int successCount, int m_failCount, int allCount)
{
    qDebug() << "allCount = " << allCount;


    if (successCount > 0) {
        QMessageBox *warn = new QMessageBox(QMessageBox::NoIcon,tr("Prompt information"),tr("Success add %1 songs").arg(successCount),QMessageBox::Yes,Widget::mutual);
        warn->setIconPixmap(QIcon::fromTheme("ukui-dialog-success", QIcon::fromTheme("ukui-dialog-success")).pixmap(24, 24));
        warn->button(QMessageBox::Yes)->setText("确定");
        warn->exec();
    } else {
        if (m_failCount > 0) {
            QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("Repeat add"),QMessageBox::Yes,Widget::mutual);
            warn->button(QMessageBox::Yes)->setText("确定");
            warn->exec();
        }
    }
}

QMap<int,QString> TableOne::getSelectedTaskIdList()
{
    QTreeView *m_pTableView = m_view;
    QModelIndexList selected = m_pTableView->selectionModel()->selectedRows();
//    QList<int> rows;
    QMap<int,QString> map1;
    
    // 检查m_model是否为nullptr
    if (m_model == nullptr) {
        return map1;
    }
    
    foreach( const QModelIndex & index, selected)
    {
//        rows.append( index.row() );
        musicDataStruct date = m_model->getItem(index.row());
        map1.insert(index.row(),date.filepath);
    }
    return map1;
}

/**
 * @brief TableOne::setHightLightAndSelect 高亮显示已选择的列表项
 * 采用高亮显示： QApplication::palette().highlight().color())
 */
void TableOne::setHightLightAndSelect()
{
    //列表中歌曲为空时，跳过高亮判断
    if(m_model->count() == 0 || ( heightLightIndex >= m_model->count() && nowListName == nowPlayListName)) {
        return;
    }
    QStringList pathList;
    pathList = m_model->getPathList(nowListName);
    for(int i = 0; i < pathList.size(); i++ ) {
        QFileInfo info(pathList[i]);
        if(!info.exists()) {
            for (int j=0; j<4; j++){
                m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::gray)));
                m_model->m_model.item(i,j)->setData(tr("path does not exist"),Qt::ToolTipRole);
                m_model->m_model.item(i,j)->setData(QString("nofile"),Qt::StatusTipRole);
            }
        } else {
            if (m_model->m_model.item(i,0)->data(Qt::StatusTipRole).toString() != ""  ) {
                for(int j=0; j<4; j++) {
                    if(WidgetStyle::themeColor == 0) {
                        m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::black)));
                    } else if (WidgetStyle::themeColor == 1) {
                        m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::white)));
                    }
                    m_model->m_model.item(i,j)->setData(m_model->m_model.item(i,j)->text(),Qt::ToolTipRole);
                    m_model->m_model.item(i,j)->setData(QString(),Qt::StatusTipRole);
                }
            } else {
                for(int j=0; j<4; j++) {
                    if(WidgetStyle::themeColor == 0) {
                        m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::black)));
                    } else if (WidgetStyle::themeColor == 1) {
                        m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::white)));
                    }
                }
            }
            if(heightLightIndex != -1 && nowListName == nowPlayListName) {
                for (int j=0; j<4; j++) {
                    // 高亮选择highlight
                    m_model->m_model.item(heightLightIndex,j)->setData(QBrush( QApplication::palette().highlight().color()),Qt::ForegroundRole);
                    m_model->m_model.item(heightLightIndex,j)->setData(QString("heightLight"),Qt::StatusTipRole);
                }
            }
        }
    }
}

/**
 * @brief TableOne::changeNumber 更改歌曲数字
 */
void TableOne::changeNumber()
{
    int num = m_model->count();

    if (num == 1) {
        listTotalNumLabel->setText(QString::number(num) + tr(" song"));
    } else {
        listTotalNumLabel->setText(QString::number(num) + tr(" songs"));
    }

    if(num == 0) {
        musicWidget->hide();
        nullPageWidget->show();

        //搜索结果为空界面时，添加按钮设置为hide，在这里需要重新处理
        if (n_addMusicButton->isHidden()) {
            n_addMusicButton->show();
        }
        if (n_addDirMusicButton->isHidden()) {
            n_addDirMusicButton->show();
        }
        addMusicButton->hide();
        playAllButton->hide();
    } else {
        musicWidget->show();
        nullPageWidget->hide();
        addMusicButton->show();
        playAllButton->show();
    }
}

/**
 * @brief TableOne::getMusicList
 * 从数据库中获取歌单列表并显示
 */
void TableOne::getMusicList()
{
    if(nowListName == SEARCH)
    {
        QList<musicDataStruct> resList;
        int ret;
        ret = g_db->getSongInfoListFromLocalMusicByKeyword(resList, m_text);
        if (ret == DB_OP_SUCC)
        {
            m_model->clear();
            m_model->add(resList);
            for(int i = 0; i < resList.size(); i++)
            {
                g_db->addMusicToPlayList(resList.at(i).filepath, SEARCH);
            }
            initTableViewStyle();
        }
    }
    else
    {
        QList<musicDataStruct> resList;
        int ret;
        ret = g_db->getSongInfoListFromDB(resList, nowListName);
        if(ret == DB_OP_SUCC)
        {
            m_model->clear();
            m_model->add(resList);
            initTableViewStyle();
        }
    }
}


/**
 * @brief TableOne::selectListChanged   切换歌单
 * @param listname
 */
void TableOne::selectListChanged(QString listname)
{
    qDebug() << "切换歌单了！！！";
    m_view->setNowPlayListName(listname);
    m_model->clear();
    if(listname == tr("Song List")) {
        nowListName = ALLMUSIC;
    } else if(listname == tr("I Love")) {
        nowListName = FAV;
    } else if(listname == tr("Search Result")) {
        Q_EMIT signalListSearch();
        nowListName = SEARCH;
//        return;
    } else {
        nowListName = listname;
    }

    getMusicList();
    changeNumber();
    showTitleText(listname);
    setHightLightAndSelect();
}
/**
 * @brief TableOne::playListRenamed   歌单重命名
 * @param oldName
 * @param newName
 */
void TableOne::playListRenamed(QString oldName, QString newName)
{
    if(listTitleLabel->text() == oldName)
    {
        listTitleLabel->setText(newName);
        nowListName = newName;
    }
}

/**
 * @brief TableOne::getHightLightIndex  获得正在播放的歌曲索引和歌单名
 * @param index
 * @param listName
 */
void TableOne::getHightLightIndex(int index, QString listName)
{
    nowPlayListName = listName;
    heightLightIndex = index;
    if(listName == nowListName){
        setHightLightAndSelect();
    } else {
        heightLightIndex = -1;
        setHightLightAndSelect();
        heightLightIndex = index;
    }
}


/**
 * @brief TableOne::playAll   播放全部歌曲
 * @param listName
 */
void TableOne::playAll(QString listName)
{
    if(listName == tr("Song List")) {
        listName = ALLMUSIC;
    } else if(listName == tr("I Love")) {
        listName = FAV;
    } else if(listName == tr("Search Result")) {
        listName = SEARCH;
    }

    QStringList list = m_model->getPathList(listName);
    if(list.size() == 0) {
        return;
    }

    playMusicforIndex(listName,0);
}

void TableOne::playAllButtonClicked()
{
    playAll(nowListName);
}

void TableOne::slotReturnText(QString text)
{
    int ret = g_db->checkPlayListExist(SEARCH);

    if(ret == DB_OP_SUCC) {
        m_model->clear();

        int retSongInfo;
        QList<musicDataStruct> musicDateList;
        retSongInfo = g_db->getSongInfoListFromPlayList(musicDateList, SEARCH);

//        if(retSongInfo == DB_OP_SUCC) {
//            for(int i = 0; i < musicDateList.size(); i++) {
//                playController::getInstance().removeSongFromCurList(SEARCH, 0);
//            }
//        }

        g_db->delPlayList(SEARCH);
    }

    int ref = g_db->createNewPlayList(SEARCH);
    if(ref != DB_OP_SUCC) {
        qDebug() << "创建搜索歌单失败";
        return;
    }

    m_text = text;
    getMusicList();
    changeNumber();
    showTitleText(tr("Search Result"));
    //setHightLightAndSelect();

    addMusicButton->hide();
    n_addMusicButton->hide();
    n_addDirMusicButton->hide();
    listTitleHBoxLayout->setContentsMargins(25, 20, 9, 30);
}

void TableOne::slotFilePath(QString path)
{
    Q_EMIT signalSongListHigh();

    QList<musicDataStruct> resList;
    int ret = g_db->getSongInfoListFromLocalMusic(resList);
    if (ret == DB_OP_SUCC) {
        for (int i = 0; i < resList.size(); i++) {
            if(resList[i].filepath == path) {
                playMusicforIndex(ALLMUSIC, i);
            }
        }
    }

    selectListChanged(tr("Song List"));
}

void TableOne::slotSongListBySinger(QString singer)
{
    clearSearchList();

    m_model->clear();
    //取消侧边栏所有按钮的选中状态
    Q_EMIT signalListSearch();
    nowListName = SEARCH;
//        return;

    QList<musicDataStruct> resList;
    int ret = g_db->getSongInfoListBySinger(resList, singer);
    if (ret == DB_OP_SUCC) {
        m_model->clear();
        m_model->add(resList);
        for(int i = 0; i < resList.size(); i++)
        {
            g_db->addMusicToPlayList(resList.at(i).filepath, SEARCH);
        }
        initTableViewStyle();
    }

    changeNumber();
    showTitleText(tr("Search Result"));
    setHightLightAndSelect();

    addMusicButton->hide();
    listTitleHBoxLayout->setContentsMargins(25, 20, 9, 30);
}

void TableOne::slotSongListByAlbum(QString album)
{
    clearSearchList();

    m_model->clear();
    //取消侧边栏所有按钮的选中状态
    Q_EMIT signalListSearch();
    nowListName = SEARCH;

    QList<musicDataStruct> resList;
    int ret = g_db->getSongInfoListByAlbum(resList, album);
    if (ret == DB_OP_SUCC) {
        m_model->clear();
        m_model->add(resList);
        for(int i = 0; i < resList.size(); i++)
        {
            g_db->addMusicToPlayList(resList.at(i).filepath, SEARCH);
        }
        initTableViewStyle();
    }

    changeNumber();
    showTitleText(tr("Search Result"));
    setHightLightAndSelect();

    addMusicButton->hide();
    listTitleHBoxLayout->setContentsMargins(25, 20, 9, 30);
}

void TableOne::clearSearchList()
{
    int ret = g_db->checkPlayListExist(SEARCH);

    if(ret == DB_OP_SUCC) {
        m_model->clear();

        int retSongInfo;
        QList<musicDataStruct> musicDateList;
        retSongInfo = g_db->getSongInfoListFromPlayList(musicDateList, SEARCH);

        if(retSongInfo == DB_OP_SUCC) {
            for(int i = 0; i < musicDateList.size(); i++) {
                playController::getInstance().removeSongFromCurList(SEARCH, 0);
            }
        }

        g_db->delPlayList(SEARCH);
    }

    int ref = g_db->createNewPlayList(SEARCH);
    if(ref != DB_OP_SUCC) {
        qDebug() << "创建搜索歌单失败";
        return;
    }
}
#ifndef WHITE_TEST
void TableOne::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }

    QWidget::dragEnterEvent(event);
}

void TableOne::dropEvent(QDropEvent *event)
{
    QStringList localpath;

    auto urls = event->mimeData()->urls();

    if(urls.isEmpty()) {
        return;
    }

    for(auto &url : urls) {
        localpath << url.toLocalFile();
    }

    if(!localpath.isEmpty()) {
        addMusicToDatebase(localpath);
    }

    playController::getInstance().delayMsecondSetVolume();
}

void TableOne::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = m_view->indexAt(event->pos());

    Q_EMIT hoverIndexChanged(index);
}
#endif
void TableOne::mousePressEvent(QMouseEvent *event)
{
    g_user_signal->hideSearchResult();

    QWidget::mousePressEvent(event);
}

void TableOne::resizeEvent(QResizeEvent *event)
{
    // 使用变量保存tableView一页最多可以完全显示多少条目，实时刷新，实现滑动条的显示和隐藏
    showScrollbarNumber = m_view->height()/42;
    initTableViewStyle();

    QWidget::resizeEvent(event);
}
 #ifndef WHITE_TEST
void TableOne::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space) {
        return;
    }

    QWidget::keyPressEvent(event);
}
#endif
//搜索结果关键字通过LineEdit传参得到
void TableOne::slotSearchTexts(QString text)
{
    m_view->setSearchText(text);
}

//搜索歌单通过LineEdit传参得到
void TableOne::slotSearchReturnPressed(QString listName)
{
    m_view->setSearchListName(listName);
}
