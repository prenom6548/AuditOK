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

#include <stdio.h>
#include <QDBusMessage>
#include "mainwidget.h"
#include "UI/base/xatom-helper.h"
#include "UI/globalsignal.h"
#include "kwindoweffects.h"
#include <ukui-log4qt.h>
#include <KWindowEffects>
#include <QMessageBox>
#include <windowmanager/windowmanager.h>
#include "ukuistylehelper/ukuistylehelper.h"

#include <QTimer>

#define UKUI_FONT_SIZE "systemFontSize"
#define UKUI_THEME_COLOR "themeColor"
#define UKUI_THEME_ICON "iconThemeName"
#define VOLUME 5

Widget *Widget::mutual = nullptr;

static int lock_fd = 0;

Widget::Widget(QStringList strlist, QWidget *parent)
    : QWidget(parent),
     m_inputList(strlist)
{
    setWindowTitle(tr("Music Player"));
    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, this, [=](const kdk::WindowId& window_id){
        if (getpid() == kdk::WindowManager::getPid(window_id) && m_windowId.isEmpty())
        {
            m_windowId = window_id.toString();
            qDebug() << "the windowAdded signal has been received, windowID is:" << m_windowId;
        }
    });
    Single(m_inputList);
    initLayOut();
}
void Widget::initThemeColor()
{
    if(QGSettings::isSchemaInstalled(FITTHEMEWINDOWS)) {
        themeData = new QGSettings(FITTHEMEWINDOWS);
        if(themeData->get("style-name").toString() == "ukui-dark" || themeData->get("style-name").toString() == "ukui-black"){
            WidgetStyle::themeColor = 1;
        }else{
            WidgetStyle::themeColor = 0;
        }
    }
}

void Widget::showList()
{
    m_windowHelper = new UkuiWindowHelper(this);
    kdk::UkuiStyleHelper::self()->removeHeader(this);
    mutual = this;
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    //初始化dbus
    initDbus();

    stateMusicFile(m_inputList);

    int res;
    res = g_db->initDataBase();

    if(res != DB_OP_SUCC) {
        qDebug() << "数据库加载失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    }
    qDebug() << "数据库加载成功";

    initOpacityGSettings();
    initAllComponent();
    initGSettings();
    allConnect();

    initStyle();
    kdk_dia_upload_program_startup();
    // 如果传入的文件已经存在，会弹窗。初始化时，弹窗模态异常，所以延迟一段时间，保证应用状态一致后再处理
    QTimer::singleShot(50, [this]() {
        if(!argName.isEmpty()) {
            for(int i = 0; i < argName.size(); ++ i){
                kylin_music_play_request(argName[i]);

            }
        }
    });

    isFirstObject = false;
    m_miniWidget->setAttribute(Qt::WA_AlwaysShowToolTips);
    m_titleBar->setAttribute(Qt::WA_AlwaysShowToolTips);
    //命令行删除到回收站
    process = new QProcess;
    m_initFinish = true;
}

Widget::~Widget()
{
    if (m_gsTransOpacity) {
        delete m_gsTransOpacity;
        m_gsTransOpacity = nullptr;
    }

    if (m_volSliderWid) {
        m_volSliderWid->deleteLater();
    }

    m_miniWidget->deleteLater();
}

void Widget::Single(QStringList path)   //单例
{
    QString str;

    if(path.size() > 1) {
        str = path[1];
    }

    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString lockPath = homePath.at(0) + "/.config/kylin-music-lock";
    int fd = open(lockPath.toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(fd < 0) {
        exit(1);
    }

    if (lockf(fd, F_TLOCK, 0)) {
        QDBusInterface interface( "org.ukui.kylin_music", "/org/ukui/kylin_music","org.ukui.kylin_music.play", QDBusConnection::sessionBus());
        if(path.size() == 1) {
            interface.call( "kylin_music_play_request", str);
        } else if(path.size() == 2) {
            if(str == "-sta" || str == "-state") {
                QDBusMessage play = interface.call("getState");

                if (play.type() == QDBusMessage::ReplyMessage) {
                    QString playStr = play.arguments().takeFirst().toString();
                    printf("当前播放状态 ：%s\n", (playStr.toStdString()).data());
                    fflush(stdout);
                }
            }

            if(str == "-t" || str == "-title") {
                QDBusMessage replay = interface.call("getTitle");

                if (replay.type() == QDBusMessage::ReplyMessage) {
                    QString replayStr = replay.arguments().takeFirst().toString();
                    printf("当前播放的歌曲名 ：%s\n", (replayStr.toStdString()).data());
                    fflush(stdout);
                }
            }
            interface.call( "kylin_music_play_request", str);
        }else{
            for(int i = 1; i < path.size();++i)
                interface.call("kylin_music_play_request", path[i]);
        }

        exit(0);
    }

    lock_fd = fd;
    isFirstObject = true;//我是首个对象
    for (int i = 1; i < path.size(); i ++)
        argName << path[i];
}

void Widget::stateMusicFile(QStringList args)
{
    if(args.length() != 2) {
        return;
    }

    QString file = args.at(1);

    if(!QFileInfo::exists(file)) {
        return;
    }

    QFileInfo files(file);
    QString type = files.suffix().toLower();

    if(MusicFileInformation::getInstance().getMusicType().contains("*." + type)) {
        playController::getInstance().setPlayListName(ALLMUSIC);
    }
}

/**
 * @brief Widget::initDbus 初始化dbus
 */
void Widget::initDbus()
{
    interface = new QDBusInterface("org.gnome.SessionManager",
                                   "/org/gnome/SessionManager",
                                   "org.gnome.SessionManager",
                                   QDBusConnection::sessionBus());

    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if(sessionBus.registerService("org.ukui.kylin_music")) {
        sessionBus.registerObject("/org/ukui/kylin_music",this,
                                  QDBusConnection::ExportAllContents);
        qDebug()<<"初始化DBUS成功";
    } else {
        qDebug()<<"初始化DBUS失败";
    }

    //S3 S4策略
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForShutdown"), this,
                                         SLOT(onPrepareForShutdown(bool)));
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForSleep"), this,
                                         SLOT(onPrepareForSleep(bool)));
    // 锁屏
    QDBusConnection::sessionBus().connect(QString("org.ukui.ScreenSaver"),
                                         QString("/"),
                                         QString("org.ukui.ScreenSaver"),
                                         QString("lock"), this,
                                         SLOT(onScreenLock()));
    QDBusConnection::sessionBus().connect(QString("org.ukui.ScreenSaver"),
                                         QString("/"),
                                         QString("org.ukui.ScreenSaver"),
                                         QString("unlock"), this,
                                         SLOT(onScreenUnlock()));

    // 1. 蓝牙控
    // 2. 带键控的四段式3.5mm有线耳机的暂停和播放控制
    QDBusConnection::systemBus().connect(QString(), QString("/"), "com.monitorkey.interface", "monitorkey", this, SLOT(client_get(QString)));
    //插拔
    QDBusConnection::sessionBus().connect(QString(), QString( "/"), "org.ukui.media", "sinkPortChanged",this, SLOT(inputDevice_get(QString)));
    //切换用户
    QDBusConnection::sessionBus().connect(QString("org.gnome.SessionManager"),
                                          QString("/org/gnome/SessionManager"),
                                          QString("org.gnome.SessionManager"),
                                          QString("PrepareForSwitchuser"), this,
                                          SLOT(slotPrepareForSwitchuser()));
    m_dbsAdapter = new DbusAdapter;
}

void Widget::initStyle()
{
    //不接受焦点。解决程序初始化时，空格键控制 播放/暂停 获取不到（Qt::Key_Space）事件
    QList<QPushButton*> list = this->findChildren<QPushButton*>();

    for(QPushButton *btn :list) {
        btn->setFocusPolicy(Qt::NoFocus);
    }

    QList<QToolButton*> lists = this->findChildren<QToolButton*>();

    for(QToolButton *btn :lists) {
        btn->setFocusPolicy(Qt::NoFocus);
    }
}

void Widget::onPrepareForShutdown(bool Shutdown)
{
    // 休眠
    KyInfo()<<"onPrepareForShutdown"<<Shutdown
           << "getState = " << playController::getInstance().getState();

    if(Shutdown) {
        if(playController::getInstance().getState() == playController::PLAY_STATE) {
            KyInfo() << "before pause";
            playController::getInstance().pauseOnly();
        }
    }
}

void Widget::onPrepareForSleep(bool isSleep)
{
    KyInfo()<<"onPrepareForShutdown"<< isSleep
           << "getState = " << playController::getInstance().getState();

    // 睡眠
    if(isSleep) {
        if(playController::getInstance().getState() == playController::PLAY_STATE) {
            KyInfo() << "before pause";
            playController::getInstance().pauseOnly();
        }
    }
}

/**
 * @brief Widget::client_get
 *
 * 1. 蓝牙控;
 * 2. 带键控的四段式3.5mm有线耳机的暂停和播放控制
 *
 *
 * 相关联包：
 *      ukui-media,
 *      pulseaudio,
 *      monitormickey
 *
 * 相关联包的负责人：
 *      封邵祥，
 *      左志威
 *
 * @param str 从monitormickey包接收到的信息
 */
void Widget::client_get(QString str)
{
    QString key = str.split(":").first();
    QString s = str.split(":").last();

    qDebug() << "into client_get: " << "key: " << key << "s: " << s;

    if(s == "1") {
        if (key == "164") {
            /*
             * fix #160113 【声音】【线控耳机】【音乐】接入线控耳机后按暂停键应用只暂停一下立即恢复播放
             *
             * 当华为机器使用有线耳机暂停歌曲播放，系统会通过mpris协议调用音乐的PlayPause(播放或暂停）接口。
             * 当第一次更改播放状态后，又接收到了monitorkey传来的164信号，这个信号会再次触发音乐的播放或暂停接口,会导致异常，因此做特殊处理
             */
            return;
        }
        playSongArea->slotPlayClicked();
    } else if(s == "2" || key == "163") {
        playController::getInstance().onNextSong();
    } else if(s == "3" || key == "165") {
        playController::getInstance().onPreviousSong();
    }
}

void Widget::inputDevice_get(QString str)
{
    qDebug() << "inputDevice: " << str;

    if(playController::getInstance().getState() == playController::PLAY_STATE) {
        playController::getInstance().pause();
    }
}

void Widget::slotPrepareForSwitchuser()
{
    if(playController::getInstance().getState() == playController::PLAY_STATE) {
        playController::getInstance().pause();
    }
}

int Widget::kylin_music_play_request(QString cmd1, QString cmd2, QString cmd3)
{
    qInfo() << "kylin_music_play_request:" << cmd1 << ", " << cmd2 << ", " << cmd3;
    //无参数，单例触发
    if (m_miniWidget->isHidden() && (isHidden() || !isActiveWindow())) {
        setWindowState(windowState() & ~Qt::WindowMinimized);
        show();
        kdk::WindowManager::activateWindow(m_windowId);
        sideBarWid->repaint();
    }

    if(!m_miniWidget->isHidden() && !m_miniWidget->isActiveWindow()) {
        kdk::WindowManager::activateWindow(m_windowId);
        QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
        m_miniWidget->move(screenGeometry.width()-40-m_miniWidget->width(), 40);
    }

    // 安卓文件
    if(QFileInfo::exists(cmd1) && cmd1.startsWith("//")){
        cmd1=cmd1.mid(1);
    }

    if(isFirstObject&&!QFileInfo::exists(cmd1)) {
        if(cmd1=="-c"||cmd1=="-close") {
            //this->close();
            //exit(0);
            slotClose();
        }

        isFirstObject = false;
        return 0;
    }

    //下一首
    if(cmd1 == "-n" || cmd1 == "-next") {
        playController::getInstance().onNextSong();
        return 0;
    }

    //上一首
    if(cmd1 == "-b" || cmd1 == "-back") {
        playController::getInstance().onPreviousSong();
        return 0;
    }

    //暂停
    if(cmd1 == "-p" || cmd1 == "-pause") {
        playController::getInstance().play();
        return 0;
    }

    //播放
    if(cmd1 == "-s" || cmd1 == "-start") {
        playController::getInstance().play();
        return 0;
    }

    // 调高音量
    if(cmd1=="-i"||cmd1=="-increase") {
        this->volumeIncrease();
        return 0;
    }

    // 降低音量
    if(cmd1=="-r"||cmd1=="-reduce") {
        volumeReduce();
        return 0;
    }

    // 查看当前播放状态
    if(cmd1=="-sta"||cmd1=="-state") {
//        getState();
        return 0;
    }

    // 初始化
    if(cmd1=="-init"||cmd1=="-initialization") {
        initMusic();
        return 0;
    }

    // 当前播放歌曲名
    if(cmd1=="-t"||cmd1=="-title") {
        getTitle();
        return 0;
    }

    // 单曲循环
    if(cmd1=="-C"||cmd1=="-CurrentItemInLoop") {
        playController::getInstance().getPlaylist()->setPlaybackMode(MediaPlaylist::CurrentItemInLoop);
        return 0;
    }

    // 列表循环
    if(cmd1=="-L"||cmd1=="-Loop") {
        playController::getInstance().getPlaylist()->setPlaybackMode(MediaPlaylist::Loop);
        return 0;
    }

    // 随机播放
    if(cmd1=="-R"||cmd1=="-Random") {
        playController::getInstance().getPlaylist()->setPlaybackMode(MediaPlaylist::Random);
        return 0;
    }

    // 窗口移动
    if(cmd1=="-m"||cmd1=="-move") {
        if(cmd2!=""&&cmd3!="") {
            moveWidget(cmd2, cmd3);
            return 0;
        }
    }

    // 窗口关闭
    if(cmd1=="-c"||cmd1=="-close") {
        //this->close();
         this->slotClose();
        return 0;
    }

    QStringList list;
    list << cmd1;
    if(list.isEmpty()) {
        return 0;
    }

    // 导入音频文件
    importFile(list);
    playController::getInstance().delayMsecondSetVolume();

    return 0;
}

void Widget::slotText(QString btnText)
{
    listName = btnText;
}

void Widget::importFile(QStringList list)
{
//    MusicFileInformation::getInstance().addFile(list);
//    sideBarWid->playListBtn->click();
//    playController::getInstance().setPlayListName(ALLMUSIC);

    if (m_initFinish) {
        sideBarWid->playListBtn->click();
    }

    musicListTable->addMusicToDatebase(list);
    QList<musicDataStruct> resList;
    resList = MusicFileInformation::getInstance().resList;

    foreach (const musicDataStruct date, resList) {
//        int ret;
//        ret = g_db->addMusicToLocalMusic(date);

//        if(ret == DB_OP_SUCC) {
//            playController::getInstance().addSongToCurList(ALLMUSIC,date.filepath);
//            if(listName == ALLMUSIC) {
//                Q_EMIT signalRefreshList(ALLMUSIC);
//            }

            QStringList pathList = getPath(ALLMUSIC);
            playController::getInstance().setCurPlaylist(ALLMUSIC,pathList);
            int index = MusicFileInformation::getInstance().findIndexFromPlayList(ALLMUSIC,date.filepath);
            playController::getInstance().play(ALLMUSIC,index);
//        }
    }

    if(resList.size() > 0) {
        int ref;
        ref = g_db->getSongInfoListFromDB(resList,ALLMUSIC);

        if(ref == DB_OP_SUCC) {
            for(int i = 0;i < list.size();i++) {
                if(resList.at(i).filepath == list.at(i)) {

                    QStringList pathList = getPath(ALLMUSIC);
                    playController::getInstance().setCurPlaylist(ALLMUSIC,pathList);

                    if(listName == ALLMUSIC) {
                        Q_EMIT signalRefreshList(ALLMUSIC);
                    }

                    int index = MusicFileInformation::getInstance().findIndexFromPlayList(ALLMUSIC,resList.at(i).filepath);

                    if(index == -1) {
                        return;
                    }

                    playController::getInstance().play(ALLMUSIC,index);
                }
            }
        }
    }
}

QStringList Widget::getPath(QString playListName)
{
    QStringList path;
    int ret;
    QList<musicDataStruct> resList;

    ret = g_db->getSongInfoListFromDB(resList,playListName);
    qDebug() << "getSongInfoListFromDB ret: " << ret;

    foreach (const musicDataStruct date, resList) {
        path.append(date.filepath);
    }

    return path;
}

QString Widget::getState()
{
    QString state;

    if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE) {
        state = "播放";
    } else if(playController::getInstance().getState() == playController::PlayState::PAUSED_STATE) {
        state = "暂停";
    } else if(playController::getInstance().getState() == playController::PlayState::STOP_STATE) {
        state = "停止";
    }

    return state;
}

void Widget::slotStateChanged(playController::PlayState state)
{
    // 如果正在播放，阻止锁屏
    if(!interface->isValid()) {
        return;
    }

    qDebug() << "判断是否正在播放play_state:" << state ;

    if(state == playController::PlayState::PLAY_STATE) {
        qDebug() << "正在播放阻止睡眠 state:" << state;
        QDBusMessage reply = interface->call(QDBus::Block, "Inhibit", "kylin-music", (quint32)0, "music is playing", (quint32)8);
        m_inhibitValue = reply.arguments().takeFirst().toUInt();
    } else {
        qDebug() << "不在播放可以睡眠 state:" << state;
        interface->call("Uninhibit", m_inhibitValue);
    }
}

QString Widget::getPlayMode() const
{
    switch (playController::getInstance().mode()) {
    case playController::CurrentItemInLoop:
        return "Track";
    case playController::Loop:
        return "Playlist";
    default:
        return "";
    }
}

void Widget::setPlayMode(QString state)
{

    int mode;
    if (state == "Track") {
        mode = 1;
    } else if (state == "Playlist") {
        mode = 3;
    } else {
        return;
    }
    playSongArea->setPlayMode(mode);
}

/**
 * @brief Widget::onScreenLock 锁屏
 */
void Widget::onScreenLock()
{
    KyInfo() << "getState = " << playController::getInstance().getState();

    if(playController::getInstance().getState() == playController::PLAY_STATE) {
        KyInfo() << "screenlock pause";
        playController::getInstance().pauseOnly();
    }

}

/**
 * @brief Widget::onScreenUnlock 锁屏解锁
 */
void Widget::onScreenUnlock()
{
    KyInfo() << "getState = " << playController::getInstance().getState();

#if 0
    if(playController::getInstance().getState() == playController::STOP_STATE
            || playController::getInstance().getState() == playController::PAUSED_STATE) {
        KyInfo() << "screenunlock play";
        playController::getInstance().play();
    }
#endif

}

void Widget::initMusic()
{
    QStringList playListName;
    g_db->getPlayList(playListName);

    for(auto listName : playListName) {
        if(listName == FAV) {
            continue;
        }
        sideBarWid->removePlayList(listName);
    }

    QList<musicDataStruct> retList;
    int sef = g_db->getSongInfoListFromDB(retList,FAV);

    if(sef == DB_OP_SUCC) {
        int i = 0;
        for(i = 0; i < retList.size(); i++) {
            int delState = g_db->delMusicFromPlayList(retList.at(i).filepath,FAV);

            if(delState == DB_OP_SUCC) {
                int j = 0;
                for(j = 0; j < retList.size(); j++) {
                    playController::getInstance().removeSongFromCurList(FAV, 0);
                }
            }
        }
    }

    QList<musicDataStruct> resList;
    int ret = g_db->getSongInfoListFromDB(resList,ALLMUSIC);

    if(ret == DB_OP_SUCC) {
        int i = 0;

        for(i = 0;i < resList.size();i++) {
            int ref = g_db->delSongFromEveryWhere(resList.at(i).filepath);

            if(ref == DB_OP_SUCC) {
                int j = 0;

                for(j = 0; j < resList.size(); j++) {
                    playController::getInstance().removeSongFromCurList(ALLMUSIC, 0);
                }
            }
        }
    }

    sideBarWid->playListBtn->click();
    historyListTable->refreshHistoryTable();
}

void Widget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    QColor color = palette().color(QPalette::Base);
    color.setAlpha(m_curTransOpacity);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Base,QColor(color));
    this->setPalette(pal);
    QBrush brush =QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    return QWidget::paintEvent(event);


#if 0
    // 先前绘制情况
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(m_curTransOpacity);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);

    painter.drawPath(path);

    QWidget::paintEvent(event);
#endif

}

void Widget::moveEvent(QMoveEvent *event)
{
    if (!m_initFinish){
        return;
    }
    movePlayHistoryWid();
    return QWidget::moveEvent(event);
}

void Widget::initLayOut()
{
    setMinimumSize(960,640);
    this->resize(960,640);
    // 窗体显示在中间
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    this->move((availableGeometry.width() - this->width())/2, (availableGeometry.height() - this->height())/2);
    setMouseTracking(true);
    initThemeColor();
    m_widget = new QWidget(this);
    if (WidgetStyle::themeColor == 0){
       m_widget->setStyleSheet("background: #FFFFFF");
    }else{
         m_widget->setStyleSheet("background: #1d1d1d");
    }
    // 关键：让widget自动填充父容器,
    m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QLabel *imageLabel = new QLabel(m_widget);
    imageLabel->setAlignment(Qt::AlignCenter); // 标签内容居中
    imageLabel->setFixedSize(100, 100);
    imageLabel->setPixmap(QIcon::fromTheme("kylin-music").pixmap(imageLabel->size()));
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(imageLabel);
    m_widget->setLayout(layout);
    mainHBoxLayout = new QHBoxLayout();
    mainHBoxLayout->setSpacing(0);
    mainHBoxLayout->setMargin(0);
    mainHBoxLayout->addWidget(m_widget);
    this->setLayout(mainHBoxLayout);
    this->setAttribute(Qt::WA_TranslucentBackground);
    kdk::UkuiStyleHelper::self()->removeHeader(this);

}

void Widget::initAllComponent()
{
    this->setWindowTitle(tr("Music Player"));
    this->setObjectName("mainWidget");
    mainVBoxLayout = new QVBoxLayout();

    QStringList lists;
    QString playlistName;
    int res = g_db->getPlayList(lists);
    if(res == DB_OP_SUCC) {
        playlistName = playController::getInstance().getPlayListName();

        if(playlistName == HISTORY) {
            playlistName = ALLMUSIC;
        }

        if(playlistName == SEARCH) {
            playlistName = ALLMUSIC;
        }

        if(lists.indexOf(playlistName) == -1) {
            playlistName = ALLMUSIC;
        }
    }

    musicListTable = new TableOne(playlistName,this);
    musicListTable->setAccessibleName("kylin-music_mainwidget_TableOne_musicListTable");
    playSongArea = new PlaySongArea(this);
    playSongArea->setAccessibleName("kylin-music_mainwidget_PlaySongArea_playSongArea");
    m_titleBar = new TitleBar(this);
    m_titleBar->setAccessibleName("kylin-music_mainwidget_TitleBar_m_titleBar");

    // 去掉标题栏
    MotifWmHints hintt;
    hintt.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hintt.functions = MWM_FUNC_ALL;
    hintt.decorations = MWM_DECOR_BORDER;
    // KWindowEffects::enableBlurBehind(this->winId(), true); //毛玻璃
    m_windowHelper->setBlurEffect(QRegion(), 0, true);
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hintt);

    m_miniWidget = new MiniWidget();
    m_miniWidget->setAccessibleName("kylin-music_mainwidget_MiniWidget_m_miniWidget");
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
//    KWindowEffects::enableBlurBehind(m_miniWidget->winId(), true);
    XAtomHelper::getInstance()->setWindowMotifHint(m_miniWidget->winId(), hints);


    // 规律性出现音量界面居中和正确位置，需要平板方面处理
    m_volSliderWid = new SliderWidget((QWidget *) parent());
    m_volSliderWid->setAccessibleName("kylin-music_mainwidget_SliderWidget_m_volSliderWid");
    MotifWmHints hintv;
    hintv.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hintv.functions = MWM_FUNC_ALL;
    hintv.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(m_volSliderWid->winId(), hintv);
    moveVolumeWidget();
    m_volSliderWid->hide();


    historyListTable = new TableHistory((QWidget*) parent());
    historyListTable->setAccessibleName("kylin-music_mainwidget_TableHistory_historyListTable");
//    historyListTable = new TableHistory(this);
    MotifWmHints hint;
    hint.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hint.functions = MWM_FUNC_ALL;
    hint.decorations = MWM_DECOR_BORDER;
//    KWindowEffects::enableBlurBehind(historyListTable->winId(), true);
    XAtomHelper::getInstance()->setWindowMotifHint(historyListTable->winId(), hint);
    movePlayHistoryWid();
    historyListTable->hide();

    m_lyricsWid = new LyricsWidget();
    m_lyricsWid->setAccessibleName("kylin-music_mainwidget_LyricsWidget_m_lyricsWid");
    m_lyricsWid->hide();
    m_lyricsWid->widinitstyle();

    rightVWidget = new QWidget(this);
    rightVWidget->setAccessibleName("kylin-music_mainwidget_QWidget_rightVWidget");
    rightVWidget->setLayout(mainVBoxLayout);
    mainVBoxLayout->addWidget(m_titleBar);
    mainVBoxLayout->addWidget(m_lyricsWid);
    mainVBoxLayout->addWidget(musicListTable);
    mainVBoxLayout->addWidget(playSongArea,0,Qt::AlignBottom);
    sideBarWid = new SideBarWidget();
    sideBarWid->setAccessibleName("kylin-music_mainwidget_SideBarWidget_sideBarWid");
    sideBarWid->setFixedWidth(200); //fix154349，原始为210，改为208但缩放125%左右窗口中间有细条，不知道怎么引起的 #fix393569 【音乐】左侧列表UI问题 由原来208改为200

    QWidget* m_bufferWidget = new QWidget(this);
    QHBoxLayout *bufferHLayout = new QHBoxLayout;
    bufferHLayout->addWidget(sideBarWid);
    bufferHLayout->addWidget(rightVWidget);
    m_bufferWidget->setLayout(bufferHLayout);

    bufferHLayout->setSpacing(0);
    bufferHLayout->setMargin(0);

    QTimer::singleShot(10, [this, m_bufferWidget]() {

        m_widget->hide();
        mainHBoxLayout->removeWidget(m_widget);

        mainHBoxLayout->addWidget(m_bufferWidget);

        mainVBoxLayout->setSpacing(0);
        mainVBoxLayout->setMargin(0);

        this->setLayout(mainHBoxLayout);
    });

    this->setAutoFillBackground(true);

    m_quitWindow = new QShortcut(QKeySequence("Ctrl+Q"), this);
    m_quitWindow->setContext(Qt::WindowShortcut);
}

void Widget::allConnect()
{
    // 侧边栏区域
    connect(sideBarWid,&SideBarWidget::playListBtnClicked,musicListTable,&TableOne::selectListChanged);
    connect(sideBarWid,&SideBarWidget::playListRenamed,musicListTable,&TableOne::playListRenamed);
    connect(sideBarWid,&SideBarWidget::signalPlayAll,musicListTable,&TableOne::playAll);
    connect(sideBarWid,&SideBarWidget::playListBtnClicked,this,&Widget::slotText);
    connect(sideBarWid,&SideBarWidget::playListBtnClicked,playSongArea,&PlaySongArea::slotText);
    connect(sideBarWid,&SideBarWidget::playListBtnClicked,m_miniWidget,&MiniWidget::slotText);
    connect(sideBarWid,&SideBarWidget::playListBtnClicked,m_lyricsWid,&LyricsWidget::toggleVisibility);
    connect(sideBarWid,&SideBarWidget::sigShowMenu,this, [this]() {
        qInfo() << "sideBarWid show menu";
        if (historyListTable->isActiveWindow()) {
            historyListTable->hide();
        }
    });
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,m_lyricsWid,&LyricsWidget::toggleVisibility);

    // 歌单列表区域
    connect(this,&Widget::signalRefreshList,musicListTable,&TableOne::selectListChanged);
    connect(musicListTable,&TableOne::addILoveFilepathSignal,m_miniWidget,&MiniWidget::slotFavIsExixts);
    connect(musicListTable,&TableOne::removeILoveFilepathSignal,m_miniWidget,&MiniWidget::slotFavIsExixts);
    connect(musicListTable,&TableOne::addILoveFilepathSignal,playSongArea,&PlaySongArea::slotFavIsExixts);
    connect(musicListTable,&TableOne::removeILoveFilepathSignal,playSongArea,&PlaySongArea::slotFavIsExixts);
    connect(musicListTable,&TableOne::refreshHistoryListSignal,historyListTable,&TableHistory::refreshHistoryTable);
    connect(musicListTable,&TableOne::signalListSearch,sideBarWid,&SideBarWidget::slotListSearch);
    connect(musicListTable,&TableOne::signalSongListHigh,sideBarWid,&SideBarWidget::slotSongListHigh);
    connect(musicListTable,&TableOne::sigAddToRList,historyListTable ,&TableHistory::slotAddToRList );
    connect(musicListTable,&TableOne::sigRemoveFromRList,historyListTable ,&TableHistory::slotRemoveFromRList );
    connect(musicListTable,&TableOne::sigShowMenu,this, [this]() {
        qInfo() << "TableOne show menu";
        if (historyListTable->isActiveWindow()) {
            historyListTable->hide();
        }
    });
    // 标题栏区域
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,musicListTable,&TableOne::selectListChanged);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,playSongArea,&PlaySongArea::slotText);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,this,&Widget::slotText);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,m_miniWidget,&MiniWidget::slotText);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnText,musicListTable,&TableOne::slotReturnText);
    connect(m_titleBar->searchEdit->m_result->m_MusicView,&MusicSearchListview::signalSearchTexts,musicListTable,&TableOne::slotSearchTexts);
    connect(m_titleBar->searchEdit->m_result,&SearchResult::signalFilePath,musicListTable,&TableOne::slotFilePath);
    connect(m_titleBar->searchEdit->m_result,&SearchResult::signalSongListBySinger,musicListTable,&TableOne::slotSongListBySinger);
    connect(m_titleBar->searchEdit->m_result,&SearchResult::signalSongListByAlbum,musicListTable,&TableOne::slotSongListByAlbum);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,musicListTable,&TableOne::slotSearchReturnPressed);
    connect(m_titleBar->miniBtn,&QPushButton::clicked,this,&Widget::slotShowMiniWidget);
    connect(m_titleBar->closeBtn,&QPushButton::clicked,this,&Widget::slotClose);
    connect(g_user_signal, &GlobalUserSignal::sigExit, this, &Widget::slotClose);
    connect(g_user_signal, &GlobalUserSignal::sigDeleteAllHistoryPlaylistMessageBox, this, &Widget::slotDeleteAllHistoryPlaylistMessageBox);
    connect(m_titleBar->minimumBtn,&QPushButton::clicked,this,&Widget::slotShowMinimized);
    connect(m_titleBar->maximumBtn,&QPushButton::clicked,this,&Widget::slotShowMaximized);
    connect(this,&Widget::minisiglyricsHide,m_lyricsWid,&LyricsWidget::toggleVisibility);
    connect(this,&Widget::normalsiglyricsHide,m_lyricsWid,&LyricsWidget::normalVisibility);

    // 播放区域
//    connect(playSongArea,&PlaySongArea::signalAddFromFavButton,musicListTable,&TableOne::selectListChanged);
//    connect(playSongArea,&PlaySongArea::signalDelFromFavButton,musicListTable,&TableOne::selectListChanged);
    connect(playSongArea,&PlaySongArea::showHistoryListBtnClicked,this, [this] () {
        qInfo() << "showHistoryListBtnClicked";
        if (historyListTable->isHidden()) {
            qInfo() << "showHistoryListBtnClicked can show history list";
            historyListTable->showHistroryPlayList();
        }
    });
    connect(playSongArea,&PlaySongArea::showVolumeBtnClicked, this, &Widget::showVolumeWidget);
    connect(&playController::getInstance(),&playController::signalVolume,this,&Widget::slotVolume);
    connect(m_volSliderWid->vSlider,&QSlider::valueChanged, playSongArea, &PlaySongArea::slotVolumeChanged);
    connect(playSongArea,&PlaySongArea::signalRefreshFav,musicListTable,&TableOne::selectListChanged);
    connect(playSongArea,&PlaySongArea::signalPlayingLab,this,&Widget::slotPlayingTitle);
    connect(playSongArea,&PlaySongArea::signalFavBtnChange,m_miniWidget,&MiniWidget::slotFavBtnChange);
    connect(playSongArea,&PlaySongArea::signalPlayingLab,m_miniWidget,&MiniWidget::slotPlayingLab);
    connect(playSongArea,&PlaySongArea::signalTimeLab,m_miniWidget,&MiniWidget::slotTimeLab);
    connect(playSongArea,&PlaySongArea::showLyricsClicked,m_lyricsWid,&LyricsWidget::showLyricsWid);
    connect(playSongArea,&PlaySongArea::dlgisPlaying,m_lyricsWid,&LyricsWidget::PlayButtonClicked);
    connect(playController::getInstance().getMpvCorePointer(),&MpvCore::widpositionChanged,m_lyricsWid,&LyricsWidget::PlayButtonClicked);
    connect(playController::getInstance().getMpvCorePointer(),&MpvCore::sigPlaySong, this, [this]() {
        m_dbsAdapter->updateMetadata(getMetadata());
    });

    connect(playSongArea,&PlaySongArea::dlgisStoping,m_lyricsWid,&LyricsWidget::StopButtonClicked);
    connect(&playController::getInstance(),&playController::singalChangePath,m_lyricsWid,&LyricsWidget::loadLyricsForSongWid);

    connect(&playController::getInstance(),&playController::playerStateChange,this,&Widget::slotStateChanged);

    // 播放历史区域
    connect(historyListTable,&TableHistory::signalHistoryBtnChecked,playSongArea,&PlaySongArea::slotHistoryBtnChecked);

    // mini模式
    connect(m_miniWidget->m_recoveryWinBtn,&QPushButton::clicked,this,&Widget::slotRecoverNormalWidget);
    connect(m_miniWidget->m_closeBtn,&QPushButton::clicked,this,&Widget::slotCloseMiniWidget);
    connect(m_miniWidget->m_preBtn,&kdk::KPushButton::clicked,playSongArea,&PlaySongArea::slotPrevious);
    connect(m_miniWidget->m_playBtn,&kdk::KPushButton::clicked,playSongArea,&PlaySongArea::slotPlayClicked);
    connect(m_miniWidget->m_nextBtn,&kdk::KPushButton::clicked,playSongArea,&PlaySongArea::slotNext);
    connect(m_miniWidget,&MiniWidget::signalFavBtnChange,playSongArea,&PlaySongArea::slotFavBtnChange);
    connect(m_miniWidget,&MiniWidget::signalRefreshFav,musicListTable,&TableOne::selectListChanged);
    // mini模式快捷键
    connect(m_miniWidget, &MiniWidget::playPauseKeySignal,playSongArea,&PlaySongArea::slotPlayClicked);
    connect(m_miniWidget, &MiniWidget::previousPlayKeySignal, playSongArea,&PlaySongArea::slotPrevious);
    connect(m_miniWidget, &MiniWidget::nextPlayKeySignal, playSongArea,&PlaySongArea::slotNext);
    connect(m_miniWidget, &MiniWidget::addVolumeKeySignal, this, &Widget::VolumeUp);
    connect(m_miniWidget, &MiniWidget::downVolumeKeySignal, this, &Widget::VolumeDown);
    connect(m_miniWidget, &MiniWidget::miniCompleteSwitchKeySignal, this, &Widget::slotRecoverNormalWidget);
    connect(m_miniWidget, &MiniWidget::loveSongKeySignal, playSongArea, &PlaySongArea::slotFav);
    connect(m_miniWidget, &MiniWidget::changePlaybackMode,playSongArea,&PlaySongArea::setPlayMode);

    // 快捷键设置
    connect(m_quitWindow, &QShortcut::activated, this, &Widget::slotClose);
    connect(this,&Widget::playPauseKeySignal,playSongArea,&PlaySongArea::slotPlayClicked);
    connect(this, &Widget::previousPlayKeySignal, playSongArea,&PlaySongArea::slotPrevious);
    connect(this, &Widget::nextPlayKeySignal, playSongArea,&PlaySongArea::slotNext);
    connect(this, &Widget::addVolumeKeySignal, this, &Widget::VolumeUp);
    connect(this, &Widget::downVolumeKeySignal, this, &Widget::VolumeDown);
    connect(this, &Widget::miniCompleteSwitchKeySignal, this,&Widget::slotShowMiniWidget);
    connect(this, &Widget::loveSongKeySignal, playSongArea, &PlaySongArea::slotFav);
    connect(this, &Widget::changePlaybackMode,playSongArea,&PlaySongArea::setPlayMode);
}

void Widget::initGSettings()//初始化GSettings
{
    //只有非标准字号的控件才需要绑定
    connect(this,&Widget::signalSetFontSize,musicListTable,&TableOne::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,playSongArea,&PlaySongArea::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,m_miniWidget,&MiniWidget::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,historyListTable,&TableHistory::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,m_titleBar->menumodule,&menuModule::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,sideBarWid->newSonglistPup,&PopupDialog::slotLableSetFontSize);
    //connect(this,&Widget::signalSetFontSize,sideBarWid->newSonglistPup->enterLineEdit,&NewSonglistLineEdit::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,sideBarWid->renameSongListPup,&PopupDialog::slotLableSetFontSize);
    //connect(this,&Widget::signalSetFontSize,sideBarWid->renameSongListPup->enterLineEdit,&NewSonglistLineEdit::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,m_titleBar->searchEdit->m_result,&SearchResult::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,musicListTable->infoDialog,&MusicInfoDialog::slotLableSetFontSize);
    connect(this, &Widget::signalSetThemeColor, playSongArea, &PlaySongArea::slotSetThemeColor);
    connect(this, &Widget::signalSetThemeColor, m_miniWidget,&MiniWidget::slotSetThemeColor);
    connect(this, &Widget::signalSetThemeIcon, sideBarWid, &SideBarWidget::setThemeIcon);
    connect(this, &Widget::signalSetFontSize, sideBarWid, &SideBarWidget::slotLableSetFontSize);

    if (QGSettings::isSchemaInstalled(FITCONTROLTRANS)) {
        m_transparencyGSettings = new QGSettings(FITCONTROLTRANS);
    }


    if(QGSettings::isSchemaInstalled(FITTHEMEWINDOWS)){
        if (nullptr == themeData){
           themeData = new QGSettings(FITTHEMEWINDOWS);
        }
        if(themeData->get("style-name").toString() == "ukui-dark" || themeData->get("style-name").toString() == "ukui-black"){
            WidgetStyle::themeColor = 1;
            changeDarkTheme();
        }else{
            WidgetStyle::themeColor = 0;
            changeLightTheme();
        }

        connect(themeData,&QGSettings::changed,this,[=]() {
            qDebug() << "aaaaaaaaaaaaaa";
            if(themeData->get("style-name").toString() == "ukui-dark" || themeData->get("style-name").toString() == "ukui-black"){
                WidgetStyle::themeColor = 1;
                changeDarkTheme();
            }
            else
            {
                WidgetStyle::themeColor = 0;
                changeLightTheme();
            }
            Q_EMIT signalSetThemeColor();
        });

        connect(themeData,&QGSettings::changed,this,[=] (const QString &key) {
            qDebug() << "bbbbbbbbb";
            if(key == UKUI_FONT_SIZE) {
                //获取字号的值
                int fontSizeKey = themeData->get(UKUI_FONT_SIZE).toString().toDouble();
                qDebug() << "********** " << UKUI_FONT_SIZE << fontSizeKey;
                //发送改变信号
                if (fontSizeKey > 0) {
                    Q_EMIT signalSetFontSize(fontSizeKey);
                }
            }
        });

        connect(themeData,&QGSettings::changed,this,[=] (const QString &key) {
            if(key == UKUI_THEME_ICON) {
                //获取图标名的值
                QString themeIconKey = themeData->get(UKUI_THEME_ICON).toString();
                qDebug() << "图标名： " << themeIconKey;
                //发送改变信号
                Q_EMIT signalSetThemeIcon(themeIconKey);
            }
        });
    }

    //启动时设置字号
    int fontSizeKey = 11;//系统默认字号,魔鬼数字，自行处理
    if (themeData != nullptr) {
        int fontSizeKeyTmp = themeData->get(UKUI_FONT_SIZE).toString().toInt();
        if (fontSizeKeyTmp > 0) {
            fontSizeKey = fontSizeKeyTmp;
        }
    }

    Q_EMIT signalSetFontSize(fontSizeKey);
    qDebug()<<"初始化GSettings成功";
}

void Widget::resizeEvent(QResizeEvent *event)
{
    if (!m_initFinish){
        return;
    }
    int max_w = qApp->primaryScreen()->size().width();

    //qApp->primaryScreen()->size().height() - this->height() [计算底边栏的高度]
    int max_h = qApp->primaryScreen()->size().height() - (qApp->primaryScreen()->size().height() - this->height());

    movePlayHistoryWid();
    g_user_signal->hideSearchResult();
    if(isMaximized()) {
        m_titleBar->maximumBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        isMaximize = true;
        playSongArea->isMax = true;
        m_titleBar->maximumBtn->setToolTip(tr("reduction"));
    } else if(! this->isMaximized() || (this->width() < (max_w-20)) || this->height() < (max_h-20)) {
        isMaximize = false;
        playSongArea->isMax = false;
        m_titleBar->maximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
//        m_titleBar->maximumBtn->setToolTip(tr("maximize"));
    }

    // fix #160022, 在音乐软件打开进行打开文件等操作，软件侧边栏显示异常
    sideBarWid->repaint();
//    sideBarWid->sidecolor();

    QWidget::resizeEvent(event);
}

/**
 * @brief Widget::movePlayHistoryWid 调整历史列表位置
 */
void Widget::movePlayHistoryWid()
{
    historyListTable->setFixedSize(320,this->height() - playSongArea->height() + 8);
    if(g_user_signal->isRTLLang){
        QPoint historyPos = playSongArea->listBtn->mapToGlobal(playSongArea->listBtn->rect().topLeft());
        QSize size = historyListTable->size();
        historyPos.setX(historyPos.x() - 25);
        historyPos.setY(historyPos.y() - 18 - size.height());
        QSize historySize = playSongArea->listBtn->size();
        int newPosX = historyPos.x() + 25;
        int newPosY = historyPos.y() + 18 + size.height();
        historyListTable->changePlayHistoryPos(newPosX, newPosY, historySize.width(), historySize.height());
        historyListTable->move(historyPos);
    }else{
        QPoint historyPos = playSongArea->listBtn->mapToGlobal(playSongArea->listBtn->rect().topRight());
        QSize size = historyListTable->size();
        historyPos.setX(historyPos.x() + 26 - size.width());
        historyPos.setY(historyPos.y() - 30+12 - size.height());
        QSize historySize = playSongArea->listBtn->size();
        int newPosX = historyPos.x() - 26 + size.width();
        int newPosY = historyPos.y() + 30-8 + size.height(); // 8 关系到点击列表历史按钮是否重复显示页面
        historyListTable->changePlayHistoryPos(newPosX, newPosY, historySize.width(), historySize.height());
        historyListTable->move(historyPos);
    }
}

void Widget::moveVolumeWidget()
{
    QPoint volumePos = playSongArea->volumeBtn->mapToGlobal(playSongArea->volumeBtn->rect().center());
    qDebug() << "volumePos: " << volumePos;

    m_volSliderWid->adjustSize();
    QSize size = m_volSliderWid->size();
    volumePos.setX(volumePos.x() - size.width() / 2);
    volumePos.setY(volumePos.y() - size.height() - 25);
    QSize volumSize = playSongArea->volumeBtn->size();
    int newPosX = volumePos.x() + size.width() / 2 - volumSize.width() / 2 + 1;
    int newPosY = volumePos.y() + 25 + size.height() - volumSize.height() / 2 + 1;

    m_volSliderWid->changeVolumePos(newPosX, newPosY, volumSize.width(), volumSize.height());

    m_volSliderWid->move(volumePos);
}

/**
 * @brief Widget::showVolumeWidget 调整弹出音量控件位置
 */
void Widget::showVolumeWidget()
{
    if(m_volSliderWid->isVisible()) {
        qDebug() << "开始隐藏音量控件";
        m_volSliderWid->hide();
    } else {
        qDebug() << "开始显示音量控件";
        m_volSliderWid->show();
        m_volSliderWid->raise();

//        Qt::WindowFlags flags = Qt::Popup;
//        m_volSliderWid->setWindowFlags(flags);
        qDebug() << "windowState: " << m_volSliderWid->windowState()
                 << "windowType: " << m_volSliderWid->windowType()
                 << "windowFlags: " << m_volSliderWid->windowFlags();
        moveVolumeWidget();
    }
}

void Widget::initOpacityGSettings()
{
    return;
    const QByteArray idtrans(THEME_QT_TRANS);

    if(QGSettings::isSchemaInstalled(idtrans)) {
        m_gsTransOpacity = new QGSettings(idtrans);
    }

    if (!m_gsTransOpacity) {
        m_curTransOpacity = 1;
        return;
    }

    connect(m_gsTransOpacity, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "transparency") {
            QStringList keys = m_gsTransOpacity->keys();
            if (keys.contains("transparency")) {
                m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
                qDebug() << "透明度： " << m_curTransOpacity;
                repaint();
            }
        }
    });

    QStringList keys = m_gsTransOpacity->keys();
    if(keys.contains("transparency")) {
        m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
        qDebug() << "透明度： " << m_curTransOpacity;
    }

}

void Widget::volumeChange(int volume){
    m_volSliderWid->vSlider->setValue(volume);
}

/**
 * @brief Widget::keyPressEvent 鼠标按压事件
 *
 * 主窗口需要先获取焦点，否则按键事件（键盘按下、松开）会捕捉不到
 *
 * @param event
 */
void Widget::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "event->key() = " << event->key();
    if(event->key() == Qt::Key_F1) {
        Q_EMIT signalShowGuide();
    } else if(event->key() == Qt::Key_Space) {
        Q_EMIT playPauseKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Left) {
        qDebug() << "ctrl+left";
        Q_EMIT previousPlayKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Right) {
        qDebug() << "ctrl+right";
        Q_EMIT nextPlayKeySignal();
    }

    if (event->key() == Qt::Key_Up) {
        qDebug() << "up";
        Q_EMIT addVolumeKeySignal();
    }

    if (event->key() == Qt::Key_Down) {
        qDebug() << "down";
        Q_EMIT downVolumeKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_M) {
        qDebug() << "ctrl+m";
        Q_EMIT miniCompleteSwitchKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_L) {
        qDebug() << "ctrl+l";
        Q_EMIT loveSongKeySignal();
    }
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_T){
        qDebug() << "ctrl+t";
        if (m_playbackMode == CurrentItemInLoop){
            Q_EMIT changePlaybackMode(3);
            m_playbackMode = Loop;
        }
        else if (m_playbackMode == Loop){
            Q_EMIT changePlaybackMode(4);
            m_playbackMode = Random;
        }
        else if (m_playbackMode == Random){
            Q_EMIT changePlaybackMode(1);
            m_playbackMode = CurrentItemInLoop;
        }
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_E) {
        qDebug() << "ctrl+E";
        this->m_titleBar->slotFoucusIn();
    }

    QWidget::keyPressEvent(event);
}

Widget::PlaybackMode Widget::playbackMode() const
{
    return m_playbackMode;
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->setFocus();
    }

    sideBarWid->repaint();

    QWidget::mousePressEvent(event);
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    g_user_signal->hideSearchResult();

    //update playSongArea
    if (playSongArea)
    {
        playSongArea->updatePlayArea();
    }

    QWidget::mouseMoveEvent(event);
}

/**
 * @brief Widget::changeEvent 主窗口激活后事件
 *
 * 主窗口激活后，需要将左侧主题显示正常，主要在sidecolor()函数调用处
 *
 * @param event
 */
void Widget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

#if 0
    // 作为保留，暂不删除
    if (event->type() == QEvent::ActivationChange) {
        if (this->isActiveWindow()) {
            // fix #160022, 在音乐软件打开进行打开文件等操作，软件侧边栏显示异常
            sideBarWid->sidecolor();
            playSongArea->initStyle();
        }
    }
#endif
//    if (sideBarWid) {
//            // fix #160022, 在音乐软件打开进行打开文件等操作，软件侧边栏显示异常
//            sideBarWid->sidecolor();
//    }

//    if (playSongArea) {
//            playSongArea->initStyle();
//    }

    // 主窗口需要先获取焦点，否则按键事件（键盘按下、松开）会捕捉不到
    if (historyListTable &&  historyListTable->isHidden()) {
        this->setFocus();
    }
}

void Widget::closeEvent(QCloseEvent *event)
{
    qInfo() << "closeEvent";
    if (!historyListTable->isHidden()) {
        historyListTable->close();
    }
}

void Widget::slotShowMiniWidget()
{
    // // 添加过渡动画
    // QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    // // 当数值为100后，重复性按快捷键ctrl+m会出现界面消失，所以需要减少到50
    // animation->setDuration(50);
    // animation->setStartValue(1);
    // animation->setEndValue(0);
    // connect(animation, &QPropertyAnimation::valueChanged, [&](QVariant value){
    //     update();
    // });
    // connect(animation, &QPropertyAnimation::finished, [&](){
//        this->hide();
        // setWindowOpacity(1);
    // });

    // QPropertyAnimation *animation_mini = new QPropertyAnimation(m_miniWidget, "windowOpacity");
    // animation_mini->setDuration(50);
    // animation_mini->setStartValue(0);
    // animation_mini->setEndValue(1);
    // connect(animation_mini, &QPropertyAnimation::valueChanged, [&](QVariant value){
    //     m_miniWidget->update();
    // });

//    m_miniWidget->setWindowOpacity(0);
    if(isMinimode)
        return;
    if(isMaximize == true){
        showNormal();
        isMaximize = false;
    }

    isMinimode = true;
    this->hide();
    m_miniWidget->showNormal();
    m_miniWidget->activateWindow();
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    m_miniWidget->move(screenGeometry.width()-40-m_miniWidget->width(), 40);
    // animation->start(QAbstractAnimation::DeleteWhenStopped);
    // animation_mini->start(QAbstractAnimation::DeleteWhenStopped);
    Q_EMIT minisiglyricsHide();
}

void Widget::moveWidget(QString newWidth, QString newHeight)
{
    int newX = newWidth.toInt();
    int newY = newHeight.toInt();
    this->move(newX, newY);
}

void Widget::slotClose()
{
    interface->call("Uninhibit", m_inhibitValue);
    //exit(0);
    kdk_dia_upload_program_exit();
    close();
    lockf(lock_fd, F_ULOCK, 0);
    quick_exit(0);
}

void Widget::slotDeleteAllHistoryPlaylistMessageBox()
{
    return;
}

void Widget::slotShowMinimized()
{
    qDebug() << "slotShowMinimized";
    this->showMinimized();

    // 处理156780 【guestos】【华为云】【音乐】不能最小化窗口
//    if(isMaximize == true) {
//        qDebug() << "=> showMaximized";
//        this->showMaximized();
//    } else {
//        qDebug() << "=> showNormal";
//        this->showNormal();
//    }
}

void Widget::slotShowMaximized()
{
    if(isMaximize) {
        showNormal();
        isMaximize = false;
        m_titleBar->maximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_titleBar->maximumBtn->setToolTip(tr(""));
        m_titleBar->maximumBtn->setProperty("isWindowButton", 0x1);
        m_titleBar->maximumBtn->setProperty("useIconHighlightEffect", 0x2);
        m_titleBar->maximumBtn->setFlat(true);
//        this->adjustSize();
//        QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
//        this->move((availableGeometry.width() - this->width())/2, (availableGeometry.height() - this->height())/2);
//        mainVBoxLayout = new QVBoxLayout();
    } else {
        showMaximized();
        isMaximize = true;
        m_titleBar->maximumBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        m_titleBar->maximumBtn->setToolTip(tr("reduction"));
        m_titleBar->maximumBtn->setProperty("isWindowButton", 0x1);
        m_titleBar->maximumBtn->setProperty("useIconHighlightEffect", 0x2);
        m_titleBar->maximumBtn->setFlat(true);
    }
}

/**
 * @brief Widget::Stop
 * 如果播放已停止，则不起作用；
 * 在此之后调用播放应会导致播放从曲目的开头重新开始
 */
void Widget::Stop() const
{
    playController::getInstance().stop();
}

/**
 * @brief Widget::VolumeUp
 * 音量增加
 */
void Widget::VolumeUp() const
{
    this->volumeIncrease();
}

/**
 * @brief Widget::VolumeDown
 * 音量降低
 */
void Widget::VolumeDown() const
{
    volumeReduce();
}

void Widget::setVolume(int v) const
{
    playSongArea->slotVolumeChanged(v);
}

int Widget::getVolume() const
{
    return playController::getInstance().getVolume();
}


/**
 * @brief Widget::Next
 * 跳转到曲目列表的下一首曲目。
 * 如果没有下一首曲目（并且循环播放和列表播放都关闭），则停止播放。
 * 如果播放已暂停或停止，则保持原样。
 */
void Widget::Next() const
{
    playController::getInstance().onNextSong();
}

/**
 * 跳转到曲目列表的上一首曲目。
 * 如果没有上一首曲目（并且循环播放和列表播放都关闭），则停止播放。
 * 如果播放已暂停或停止，则保持原样。
 */
void Widget::Previous() const
{
    playController::getInstance().onPreviousSong();
}

void Widget::Play() const
{
    playController::getInstance().play();
}

/**
 * @brief Widget::Pause
 * 如果播放已暫停，则不起作用。
 *
 */
void Widget::Pause() const
{
    playController::getInstance().pauseOnly();
}

/**
 * @brief Widget::PlayPause
 * 如果播放已暫停，则恢复播放；
 * 如果播放停止，则开始播放。
 */
void Widget::PlayPause() const
{
    if (playController::getInstance().getState() == playController::PlayState::PLAY_STATE) {
        this->Pause();
    } else if (playController::getInstance().getState() == playController::PlayState::PAUSED_STATE) {
        this->Play();
    }else if(playController::getInstance().getState() == playController::PlayState::STOP_STATE){
        playSongArea->playMeta();

    }
}

void Widget::Seek(const qint64 offset) const
{
    qint64 movePos = playController::getInstance().getMpvCorePointer()->position() + offset;
    // 移动的目标位置小于0，则从0开始播放
    if (movePos < 0) {
        playController::getInstance().getMpvCorePointer()->setPosition(0);
    } else if (movePos > playController::getInstance().getMpvCorePointer()->duration()) {
        playController::getInstance().onNextSong();
    } else {
        playController::getInstance().getMpvCorePointer()->setPosition(movePos);
    }
}

QString Widget::getStates() const
{
    QString state;

    if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE) {
        state = PLAY_STATUS;
    } else if(playController::getInstance().getState() == playController::PlayState::PAUSED_STATE) {
        state = PAUSED_STATUS;
    } else if(playController::getInstance().getState() == playController::PlayState::STOP_STATE) {
        state = STOP_STATUS;
    }

    return state;
}

QVariantMap Widget::getMetadata() const
{
    QVariantMap metadata;
    QString playPath = playController::getInstance().getPath();
    if(playPath != "")
    {
        musicDataStruct musicStruct;
        QString filePath = playPath.remove("file://");
        g_db->getSongInfoFromDB(filePath, musicStruct);

        QPixmap pix = MusicFileInformation::getInstance().getCoverPhotoPixmap(filePath);
        QString savePath = "";
        if (!pix.isNull()) {
            QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
            savePath = "file://" + homePath.at(0) + "/.cache/music-arturl.png";
            pix.save(homePath.at(0) + "/.cache/music-arturl.png", "PNG");
        }

        metadata["xesam:title"] = musicStruct.title;
        metadata["xesam:artist"] = musicStruct.singer;
        metadata["xesam:album"] = musicStruct.album;
        metadata["mpris:artUrl"] = savePath;

    }
    else
    {
        metadata["xesam:title"] = "";
        metadata["xesam:artist"] = "";
        metadata["xesam:album"] = "";
        metadata["mpris:artUrl"] = "";
    }

    metadata["mpris:trackid"] = 0;
    metadata["mpris:length"] = 0;
    metadata["xesam:albumArtist"] = "";
    metadata["xesam:asText"] = "";
    metadata["xesam:audioBPM"] = 0;
    metadata["xesam:autoRating"] = 0.0;
    metadata["xesam:trackNumber"] = 0;
    metadata["xesam:url"] = "";
    metadata["xesam:useCount"] = 0;
    metadata["xesam:userRating"] = 0.0;
//    qDebug() << metadata["xesam:title"] << metadata["xesam:artist"] << metadata["xesam:album"];

    return metadata;
}

void Widget::setPlayRate(double s) const
{
    playController::getInstance().getMpvCorePointer()->setPlayRate(s);
}

double Widget::getPlayRate() const
{
    return playController::getInstance().getMpvCorePointer()->getPlayRate();
}

qint64 Widget::getPosition() const
{
    return playController::getInstance().getMpvCorePointer()->position();
}

void Widget::volumeIncrease() const
{
    int value = m_volSliderWid->vSlider->value() + VOLUME;

    if(value > 100)
        value = 100;

    m_volSliderWid->vSlider->setValue(value);
}

void Widget::volumeReduce() const
{
    int value = m_volSliderWid->vSlider->value() - VOLUME;

    if(value < 0) {
        value = 0;
    }

    m_volSliderWid->vSlider->setValue(value);
}

void Widget::slotVolume(int volume)
{
    m_volSliderWid->vSlider->setValue(volume);
}

void Widget::slotRecoverNormalWidget()
{
    if(isMaximize == true) {
        this->showNormal();
        isMaximize = false;
    } else {
        this->showNormal();
    }

    m_miniWidget->hide();
    isMinimode = false;
    this->show();

    // 从mini 模式切换到正常模式时，激活当前主窗口
    this->activateWindow();
    Q_EMIT normalsiglyricsHide();
}

void Widget::slotCloseMiniWidget()
{
    this->close();
}

void Widget::slotPlayingTitle(QString title)
{
    m_playTitle = title;
}

QString Widget::getTitle()
{
    qDebug() << "m_playTitle" << m_playTitle;
    return m_playTitle;
}

void Widget::slotReturnPressed()
{
}

void Widget::creartFinish()
{
    if (m_creatFinishEnum == MESSAGE) {
        if (m_creatFinishMsg.isEmpty()) {
            return;
        }

        QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),m_creatFinishMsg.toLocal8Bit().data(),QMessageBox::Yes,Widget::mutual);

        warn->button(QMessageBox::Yes)->setText("确定");
        warn->exec();
    }
}

void Widget::setCreatFinishMsg(QString msg)
{
    m_creatFinishEnum = MESSAGE;
    m_creatFinishMsg = msg;

    //如果界面已经显示，调用弹窗
//    if (!this->isHidden()) {
        creartFinish();
//    }
}

//切换深色主题
void Widget::changeDarkTheme()
{
    sideBarWid->newSonglistPup->dlgcolor();
    sideBarWid->renameSongListPup->dlgcolor();
    sideBarWid->sidecolor();
    playSongArea->initStyle();
    m_miniWidget->initStyle();
    m_titleBar->titlecolor();
    musicListTable->initTableViewStyle();
    musicListTable->setHightLightAndSelect();
    musicListTable->initStyle();
    musicListTable->m_view->setAlternatingRowColors(false);
//    musicListTable->tableView->setShowGrid(false);
    m_volSliderWid->initColor();
    playSongArea->m_playBackModeWid->playModecolor();
    historyListTable->initStyle();
    historyListTable->noRefreshHistory();
    m_lyricsWid->widinitstyle();
    if (playSongArea->m_lyricsDialog != nullptr){
        playSongArea->m_lyricsDialog->dlginitstyle();
    }
}

//切换浅色主题
void Widget::changeLightTheme()
{
    sideBarWid->newSonglistPup->dlgcolor();
    sideBarWid->renameSongListPup->dlgcolor();
    sideBarWid->sidecolor();
    playSongArea->initStyle();
    m_miniWidget->initStyle();
    m_titleBar->titlecolor();
    musicListTable->initTableViewStyle();
    musicListTable->setHightLightAndSelect();
    musicListTable->initStyle();
    musicListTable->m_view->setAlternatingRowColors(false);
//    musicListTable->tableView->setShowGrid(false);
    m_volSliderWid->initColor();
    playSongArea->m_playBackModeWid->playModecolor();
    historyListTable->initStyle();
    historyListTable->noRefreshHistory();
    m_lyricsWid->widinitstyle();
    if (playSongArea->m_lyricsDialog != nullptr){
        playSongArea->m_lyricsDialog->dlginitstyle();
    }
}
