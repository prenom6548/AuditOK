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

#include "tablehistory.h"
#include "UI/mainwidget.h"
#include "UI/globalsignal.h"
#include "ukuistylehelper/ukuistylehelper.h"
#include <QApplication>
#include <QWidgetList>
#include <ukuiwindowhelper.h>
#define PT_15 15

TableHistory::TableHistory(QWidget *parent) : QDialog(parent)
{
    //设置窗口顶置: 一直在最前面,否则窗口中qmessagebox弹窗后点击其他地方会导致该弹窗消失
//    Qt::WindowFlags m_flags = windowFlags();
//    setWindowFlags(m_flags | Qt::SubWindow);
    setModal(false);
    UkuiWindowHelper* m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->removeTitleBar();
    m_windowHelper->setSkipTaskBar(true);
    initSetModel();
    initStyle();
    initConnect();
    initRightMenu();
    initTableStyle();

    qDebug() << "播放历史列表初始化完成.";
}

void TableHistory::initSetModel()
{
    //禁用界面拖拽
    this->setProperty("useStyleWindowManager", false);
    this->setFixedWidth(320);
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
//    this->setAutoFillBackground(true);
//    this->setBackgroundRole(QPalette::Base);

    m_view = new CustomTreeView(true);
    m_view->setAccessibleName("kylin-music_tablehistory_CustomTreeView_m_view");
    m_view->setObjectName("m_tableHistory");
    m_view->setIconSize(QSize(0, 0));

    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
//    m_tableHistory->setShowGrid(false);
//    m_tableHistory->verticalHeader()->setDefaultSectionSize(40);
    m_model = new MusicListModel;
    m_model->setObjectName("kylin-music_tablehistory_MusicListModel_m_model");
    QList<musicDataStruct> resList;
    g_db->getSongInfoListFromHistoryMusic(resList);
    m_model->add(resList);
    m_model->setView(*m_view);

    historyTitileLabel = new QLabel(this);
    historyTitileLabel->setAccessibleName("kylin-music_QLabel_historyTitileLabel");
    historyTitileLabel->setText(tr("History"));
    //listCountLabel = new QLabel(this);

    deleteAllBtn = new QToolButton(this);
    deleteAllBtn->setAccessibleName("kylin-music_tablehistory_QToolButton_deleteAllBtn");
    deleteAllBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   // deleteAllBtn->setIconSize(QSize(16,16));
  //  deleteAllBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    deleteAllBtn->setText(tr("Empty"));
    this->displayBtn(deleteAllBtn,tr("Empty"));

    titleHBoxLayout = new QHBoxLayout();
    titleHBoxLayout->addWidget(historyTitileLabel,0,Qt::AlignLeft);
   // titleHBoxLayout->addWidget(listCountLabel,1,Qt::AlignLeft);
    titleHBoxLayout->addWidget(deleteAllBtn,Qt::AlignRight);
    titleHBoxLayout->setAlignment(Qt::AlignBottom);
    titleHBoxLayout->setContentsMargins(16,16,16,12);

    historyTitileWidget = new QWidget(this);
    historyTitileWidget->setAccessibleName("kylin-music_tablehistory_QWidget_historyTitileWidget");
    historyTitileWidget->setFixedHeight(64);
    historyTitileWidget->setLayout(titleHBoxLayout);


    nullPageWidget = new QWidget(this);
    nullPageWidget->setAccessibleName("kylin-music_tablehistory_QWidget_nullPageWidget");
    nullIconLabel = new QLabel(this);
    nullIconLabel->setAccessibleName("kylin-music_QLabel_nullIconLabel");
    if (WidgetStyle::themeColor == 1) {
        // black theme
        nullIconLabel->setPixmap(QPixmap(":/img/default/defaultIconDark.svg").scaled(96, 96));
    } else {
        nullIconLabel->setPixmap(QPixmap(":/img/default/defaultIconLight.svg").scaled(96, 96));
    }
    nullTextLabel = new QLabel(this);
    nullTextLabel->setAccessibleName("kylin-music_QLabel_nullTextLabel");
//    nullTextLabel->setFixedHeight(30);
    nullTextLabel->setText(tr("The playlist has no songs"));
    nullPageLayout = new QVBoxLayout();
    nullPageLayout->addStretch(1);
    nullPageLayout->addWidget(nullIconLabel);
    nullIconLabel->setAlignment(Qt::AlignHCenter);
    nullPageLayout->addWidget(nullTextLabel);
    nullTextLabel->setAlignment(Qt::AlignHCenter);
    nullPageLayout->addStretch(1);
    nullPageLayout->setSpacing(0);
    nullPageLayout->setAlignment(Qt::AlignCenter);
    nullPageWidget->setLayout(nullPageLayout);


    // 歌曲历史列表
    tableHBoxLayout = new QHBoxLayout();
    tableHBoxLayout->addWidget(m_view);
    tableHBoxLayout->setContentsMargins(16, 0, 16, 0);

    tableWidget = new QWidget(this);
    tableWidget->setAccessibleName("kylin-music_tablehistory_QWidget_tableWidget");
    tableWidget->setLayout(tableHBoxLayout);

    mainVLayout = new QVBoxLayout();
    mainVLayout->addWidget(historyTitileWidget,Qt::AlignTop);
//    mainVLayout->addStretch(0);
    mainVLayout->addWidget(tableWidget);
    mainVLayout->addWidget(nullPageWidget);
    mainVLayout->setMargin(0);
    mainVLayout->setSpacing(0);
    this->setLayout(mainVLayout);

    changeNumber();
}

//字体的截断显示显示函数
void TableHistory::displayBtn(QToolButton *btn, QString btnTooltip)
{
#ifndef WHITE_TEST
    QLocale locale;
    if(locale.language()==QLocale::Tibetan)
    {
        QFontMetrics  fontMetrics(btn->font());
        int fontSize = fontMetrics.width(btnTooltip);
        if (fontSize > (btn->width() -7) ) {
            QString tmpStr = btnTooltip.left(btnTooltip.length() * (btn->width()) / fontSize   - 7)  + "...";
            btn->setText(tmpStr);
            btn->setToolTip(btnTooltip);
        } else {
            btn->setText(btnTooltip);
            btn->setToolTip("");
        }
    }
#endif
}

void TableHistory::initStyle()
{
    m_view->setStyleSheet("#m_tableHistory{border:none;}");
    historyTitileLabel->setStyleSheet("height:18px; font-weight: 600; line-height: 18px;");
    //listCountLabel->setStyleSheet("height:36px; font-weight:400;color:#8F9399; line-height: 14px;");
   // deleteAllBtn->setStyleSheet("color:#8F9399;background:transparent;");
    deleteAllBtn->setStyleSheet("width:72px;");
    if(WidgetStyle::themeColor == 0){
        nullTextLabel-> setStyleSheet("font-size: 14px; color:#8C8C8C; opacity: 1;");
    } else if(WidgetStyle::themeColor == 1){
        nullTextLabel-> setStyleSheet("font-size: 14px; color:#8F9399; opacity: 1;");
    }
}

void TableHistory::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_15;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    historyTitileLabel->setFont(font);

    //改变字体大小时 截断字体省略显示
    this->displayBtn(deleteAllBtn,tr("Empty"));
}

void TableHistory::deleteAllClicked()
{
//    g_user_signal->deleteAllHistoryPlaylistMessageBox();
#ifndef WHITE_TEST
    QMessageBox msg(QMessageBox::Question,tr("Prompt information"),tr("Clear the playlist?"),QMessageBox::Ok|QMessageBox::Cancel, Widget::mutual);

    int ret = msg.exec();
    if(ret == QMessageBox::Ok) {
        for(int i = 0;i < m_model->count();i++) {
            playController::getInstance().removeSongFromCurList(HISTORY, 0);
        }

        int result = g_db->emptyHistoryMusic();
        if(result == DB_OP_SUCC) {
            m_model->clear();
            changeNumber();
        }

    }
    // fix bug#162535 【音乐】播放历史窗口位于整个桌面的最顶层
    Widget::mutual->movePlayHistoryWid();
    this->raise();
    this->show();
    this->setFocus();
    this->activateWindow(); // 这个重要，否则会出现“删除”窗口结束后，当前窗口不激活
#endif
}

void TableHistory::slotPlayIndexChanged(int index, QString listname)
{
    if(listname == HISTORY) {
        nowPlayListName = HISTORY;
        nowPlayIndex = index;
        noRefreshHistory();
    } else {
        nowPlayListName = listname;
    }
}

void TableHistory::slotPlayPathChanged(QString songPath)
{
    if(nowPlayListName != HISTORY && songPath != "") {
        int ret = g_db->addMusicToHistoryMusic(songPath);
        if(ret == DB_OP_SUCC) {
            nowPlayIndex = 0;
            refreshHistoryTable();
        } else {
            return;
        }
    } else if(songPath == "") {
        nowPlayIndex = -1;
        noRefreshHistory();
    } else {
        return;
    }
}

bool TableHistory::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);

//    return  QWidget::nativeEvent(eventType, message, result);

    if(eventType != "xcb_generic_event_t") {
        return false;
    }

    xcb_generic_event_t *event = (xcb_generic_event_t*)message;
    switch (event->response_type & ~0x80) {
        case XCB_FOCUS_OUT:
            // -playHistoryPosWidth (负数原因：因为计算按钮的右下角点)
            QRect rect(playHistoryPosX, playHistoryPosY, -playHistoryPosWidth, playHistoryPosHeight);

            if(rect.contains(QCursor::pos(), false)) {
                return false;
            }

            qDebug() << this->geometry() << QCursor::pos()
                     << playHistoryPosX << playHistoryPosY << playHistoryPosWidth << playHistoryPosHeight;
            if (this->geometry().contains(QCursor::pos(), false)) {
                return false;
            }

            g_user_signal->historyButtonIsShow = false;

            qDebug() << "开始隐藏当前历史界面";
            this->hide();
//            Q_EMIT signalHistoryBtnChecked(false);
            break;
    }

    return false;
}

void TableHistory::changeEvent(QEvent *event)
{
    qDebug() << "changeEvent:" << event->type();

    if (event->type() == QEvent::ActivationChange && !isActiveWindow()) {
        qDebug() << "need hide";
        g_user_signal->historyButtonIsShow = false;
        hide();
    }
    return QDialog::changeEvent(event);

}

void TableHistory::playSongsDoubleClicked(QModelIndex index)
{
    int indexRow = index.row();
    QStringList pathList;

    m_model->getItem(indexRow);
    pathList = m_model->getPathList(nowListName);

    playController::getInstance().setCurPlaylist(nowListName,pathList);
    playController::getInstance().play(nowListName,indexRow);

    Q_EMIT signalHistoryPlaying();
}

void TableHistory::initTableStyle()
{
    m_view->hideColumn(2);
    m_view->setColumnWidth(0, 146);
    m_view->setColumnWidth(1, 78);
    m_view->setColumnWidth(3, 64);
    m_view->setAutoFillBackground(true);
    m_view->setHeaderHidden(true);
}

void TableHistory::initConnect()
{
    connect(deleteAllBtn,&QToolButton::clicked,this,&TableHistory::deleteAllClicked);
    connect(m_view,&TableHistory::customContextMenuRequested,this,&TableHistory::showRightMenu);
    connect(m_view,&CustomTreeView::doubleClicked,this,&TableHistory::playSongs);
    //connect(m_view,&CustomTreeView::doubleClickedSignal,this,&TableHistory::playSongsDoubleClicked);
    connect(&playController::getInstance(),&playController::currentIndexAndCurrentList,this,&TableHistory::slotPlayIndexChanged);
    connect(&playController::getInstance(),&playController::singalChangePath,this,&TableHistory::slotPlayPathChanged);
}

void TableHistory::showHistroryPlayList()
{
    if(this->isVisible() || g_user_signal->historyButtonIsShow) {
        this->hide();
        g_user_signal->historyButtonIsShow = false;
    } else {
        g_user_signal->historyButtonIsShow = true;

//        this->setFocus();
        this->raise();
        this->show();
        Widget::mutual->movePlayHistoryWid();
        // 初始化定时器
    }
}

void TableHistory::changeNumber()
{
    int num = m_model->count();

    if (num <= 1) {
        //listCountLabel->setText(tr("total") + QString::number(num) + tr("song"));
    } else {
        //listCountLabel->setText(tr("total") + QString::number(num) + tr("songs"));
    }

    if(num == 0) {
        nullPageWidget->show();
        tableWidget->hide();
        isHightLight = false;
    } else {
        nullPageWidget->hide();
        tableWidget->show();
    }
    deleteAllBtn->setEnabled(num > 0);
}

void TableHistory::refreshHistoryTable()
{
    m_model->clear();
    QList<musicDataStruct> resList;
    g_db->getSongInfoListFromHistoryMusic(resList);
    m_model->add(resList);
    changeNumber();
    initTableStyle();
    setHighlight(nowPlayIndex);
}

void TableHistory::noRefreshHistory()
{
    changeNumber();
    initTableStyle();
    setHighlight(nowPlayIndex);
}

void TableHistory::initRightMenu()
{
    m_menu = new QMenu;
    m_menu->setAccessibleName("kylin-music_QMenu_m_menu");
    playRow = new QAction(tr("Play"));
    playRow->setObjectName("kylin-music_tablehistory_QAction_playRow");
    removeRow = new QAction(tr("Delete"));
    removeRow->setObjectName("kylin-music_tablehistory_QAction_removeRow");
    playNextRow = new QAction(tr("Play the next one"));
    playNextRow->setObjectName("kylin-music_tablehistory_QAction_playNextRow");

    m_menu->addAction(playRow);
    m_menu->addAction(removeRow);
//    m_menu->addAction(playNextRow);
    connect(playRow,&QAction::triggered,this,&TableHistory::playSongs);
    connect(removeRow,&QAction::triggered,this,&TableHistory::deleteSongs);
}

void TableHistory::showRightMenu(const QPoint &pos)
{
#ifndef WHITE_TEST
    QModelIndex index = m_view->indexAt(pos);

    if(index.row() < 0) {
        return;
    }
    m_menu->exec(QCursor::pos());
#endif
}

//加入移除列表 removeList
void TableHistory::slotAddToRList(QString path,QString owner)
{
#ifndef WHITE_TEST
    QStringList  pathList = m_model->getPathList(nowListName); //历史列表
    //判断删除的歌曲是否在历史列表
    if(pathList.contains(path)){
        if( removeList.contains(path)){
            removeList[path] = owner;
        }else{
            removeList.insert(path,owner);
        }
    }
#endif
}
//移除移除列表 removeList
void TableHistory::slotRemoveFromRList(QString path){
    if(removeList.contains(path)){
        removeList.remove(path);
    }
}

/**
 * @brief TableHistory::playSongs 播放音乐
 */
void TableHistory::playSongs()
{
#ifndef WHITE_TEST
    int index = m_view->currentIndex().row();
    QStringList pathList;
    m_model->getItem(index);
    pathList = m_model->getPathList(nowListName);

    //历史列表选择播放的歌曲的路径
    QString selectPath = m_model->getItem(index).filepath;

    //选择播放歌曲是否在 removeList
    while ( removeList.contains(selectPath) )
    {
        //选择播放歌曲是否在 对应歌单列表(删除时所在的歌单列表)
        QStringList removePathList = m_model->getPathList(removeList.value(selectPath));
        if( !removePathList.contains(selectPath)) {
            //不在 则提示、删除、播放历史列表下一首
            QMessageBox::information(Widget::mutual,tr("Prompt information"),tr("The file path does not exist or has been deleted"));
            int ret = g_db->delMusicFromHistoryMusic(selectPath);
            slotRemoveFromRList(selectPath);
            if (ret == 0){
                playController::getInstance().removeSongFromCurList(nowListName,index);
                if((index+1) < pathList.size()){
                    index = index+1;
                }else{
                    index = 0;
                }
            }

        }
        selectPath = m_model->getItem(index).filepath;
    }

    //歌曲不存在则播放歌曲列表下一首
    QString lastListName = playController::getInstance().getPlayListName();
    QStringList lastPathList = m_model->getPathList(lastListName);
    int m_lastindex = lastPathList.indexOf(selectPath,0); //已删除音乐的索引
    playController::getInstance().setCurPlaylist(lastListName,lastPathList);
    playController::getInstance().play(lastListName,m_lastindex);
    refreshHistoryTable();
    return;
#endif
}

void TableHistory::deleteSongs()
{
    int index = m_view->currentIndex().row();

//    if(index == nowPlayIndex && nowPlayListName != HISTORY) {
//        QMessageBox::warning(Widget::mutual,tr("Prompt information"),tr("歌曲列表或歌单中正在播放此歌曲，无法删除~"));
//        return;
//    }

    musicDataStruct date = m_model->getItem(index);
//    g_db->delMusicFromPlayList(date.filepath,nowListName);
    playController::getInstance().removeSongFromCurList(nowListName,index);
    g_db->delMusicFromHistoryMusic(date.filepath);

    if(index == nowPlayIndex && nowPlayListName != HISTORY) {
        nowPlayIndex = -1;
    }

    refreshHistoryTable();
}

void TableHistory::changePlayHistoryPos(int posX, int posY, int width, int height)
{
    playHistoryPosX = posX;
    playHistoryPosY = posY;
    playHistoryPosWidth = width;
    playHistoryPosHeight = height;
}

/**
 * @brief TableHistory::setHighlight 高亮显示已选择的列表项
 * 采用高亮显示： QApplication::palette().highlight().color())
 * @param index
 */
void TableHistory::setHighlight(int index)
{
    if (WidgetStyle::themeColor == 1) {
        // black theme
        nullIconLabel->setPixmap(QPixmap(":/img/default/defaultIconDark.svg").scaled(96, 96));
    } else {
        nullIconLabel->setPixmap(QPixmap(":/img/default/defaultIconLight.svg").scaled(96, 96));
    }

    //播放历史模块高亮
    if(m_model->count() == 0 ||  index >= m_model->count()) {
        return;
    }

    QColor highlightColor(QApplication::palette().highlight().color());

    if(WidgetStyle::themeColor == 0) {
        for (int m = 0; m<4 ; m++) {
            for(int k = 0; k < m_model->count() ; k++) {
                QStringList pathList;
                pathList = m_model->getPathList(nowListName);

                for(int i = 0; i < pathList.size(); i++) {
                    QFileInfo info(pathList[i]);
                    if(!info.exists()) {
                        for (int j=0; j<4; j++){
                            m_model->m_model.item(i, j)->setForeground(QBrush(QColor(Qt::gray)));
                            m_model->m_model.item(i, j)->setData(tr("path does not exist"),Qt::ToolTipRole);
                        }
                    }
               }

                m_model->m_model.item(k, m)->setForeground(QBrush(QColor(Qt::black)));
            }
        }

        if(index != -1) {
            for (int i = 0 ; i<4 ;i++ ) {
                m_model->m_model.item(index,i)->setData(QBrush(highlightColor),Qt::ForegroundRole);
            }
        }
    }

    if(WidgetStyle::themeColor == 1) {
        for (int m = 0; m < 4; m++ ) {
            for(int k = 0; k < m_model->count(); k++) {
                QStringList pathList;
                pathList = m_model->getPathList(nowListName);

                for(int i = 0; i < pathList.size(); i++ ) {
                    QFileInfo info(pathList[i]);

                    if(!info.exists()) {
                        for (int j=0; j<4; j++){
                            m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::gray)));
                            m_model->m_model.item(i,j)->setData(tr("path does not exist"),Qt::ToolTipRole);
                        }
                    }
               }

                m_model->m_model.item(k, m)->setForeground(QBrush(QColor(Qt::white)));
            }
        }

        if(index != -1) {
            for (int i = 0 ; i<4 ;i++ ) {
                m_model->m_model.item(index, i)->setData(QBrush(highlightColor),Qt::ForegroundRole);
            }
        }
    }
}
