#include "playcontroller.h"
#include "dbusadapter.h"
#include "musicdatabase.h"
#include "musicfileinformation.h"

playController::playController(QObject *parent)
    : QObject{parent}
{
    m_player = new MMediaPlayer(this);
    if (m_player == nullptr) {
        qDebug() << "failed to create player ";
        return;
    }
    m_playlist = new MMediaPlaylist(m_player);
    if (m_playlist == nullptr) {
        qDebug() << "failed to create laylist";
        return;
    }
    m_player->setPlaylist(m_playlist);

    m_playlist->setPlaybackMode(MMediaPlaylist::Loop);
//    connect(m_playlist,&MMediaPlaylist::currentIndexChanged,this,&playController::slotIndexChange);
    connect(m_player,&MMediaPlayer::stateChanged,this,&playController::slotStateChanged);
}

playController::~playController()
{
    if (m_playlist != nullptr) {
        delete m_playlist;
        m_playlist = nullptr;
    }
    if (m_player != nullptr) {
        m_player->stop();
        delete m_player;
        m_player = nullptr;
    }
}

bool playController::play(QString playlist, int index)
{
    if (playlist.compare(m_curList)==0)
    {
        stop();
        setSongIndex(index);
        play();
        return true;
    }
    return false;
}

bool playController::play()
{
    if (m_player == nullptr) {
        return false;
    }
    if (m_player->isAvailable() == false) { //存疑
        return false;
    }
    if (m_player->state() == MMediaPlayer::State::PlayingState) {
        m_player->pause();
    } else {
        m_player->play();
    }

    return true;
}

bool playController::pause()
{
    if (m_player == nullptr) {
        return false;
    }
    m_player->pause();

    return true;
}

bool playController::pauseOnly()
{
    if (m_player == nullptr) {
        return false;
    }
    m_player->pauseOnly();

    return true;
}

bool playController::stop()
{
    if (m_player == nullptr) {
        return false;
    }

    m_player->stop();

    return true;
}

void playController::nextSong()
{
    if (m_playlist == nullptr || m_player == nullptr) {
        qDebug() << "m_playlist or m_player is nullptr";
        return;
    }
    m_playlist->next();
    m_player->play();
}

void playController::previousSong()
{
    if (m_playlist == nullptr || m_player == nullptr) {
        qDebug() << "m_playlist or m_player is nullptr";
        return;
    }
    m_playlist->previous();
    m_player->play();
}

QVariantMap playController::getMetadata() const
{
    QVariantMap metadata;
    MMediaContent content = m_playlist->media(m_playlist->currentIndex());
    QString path = content.canonicalUrl().toLocalFile();
    QFileInfo file(path.remove("file://"));
    if(file.exists()) {
        musicDataStruct musicStruct;
        QString filePath = path.remove("file://");
        g_db->getSongInfoFromDB(filePath, musicStruct);

        metadata["xesam:title"] = musicStruct.title;
        metadata["xesam:artist"] = musicStruct.singer;
        metadata["xesam:album"] = musicStruct.album;
        metadata["mpris:artUrl"] = "";

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

    return metadata;
}

void playController::setSongIndex(int index)
{
    if (m_playlist == nullptr) {
        qDebug() << "m_playlist is null";
        return;
    }
    if(index < 0)
    {
        return;
    }
    if (index > m_playlist->mediaCount()) {
        return;
    }
    m_playlist->setCurrentIndex(index);
}

void playController::setCurPlaylist(/*QString name, */QStringList songPaths)
{
    if (m_playlist == nullptr || m_player == nullptr) {
        return;
    }
//    m_curList = name;
    m_playlist->clear();

    for (auto path : songPaths) {
        m_playlist->addMedia(QUrl::fromLocalFile(path));
    }
    m_player->stop();
    m_player->setPlaylist(nullptr);
    m_player->setPlaylist(m_playlist);
    m_playlist->setCurrentIndex(0);
    Q_EMIT mateDataChanged();

    connect(m_playlist, &MMediaPlaylist::currentIndexChanged, this, [&,this]() {
        Q_EMIT mateDataChanged();
    });
}

void playController::addMusicToDatebase(QStringList fileNames)
{
    int res;
    res = g_db->initDataBase();
    if(res != DB_OP_SUCC)
    {
        qDebug() << "数据库加载失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    }
    qDebug() << "数据库加载成功";
    MusicFileInformation::getInstance().addFile(fileNames);
    QList<musicDataStruct> resList;
    resList = MusicFileInformation::getInstance().resList;

    int ret;
    foreach (const musicDataStruct date, resList)
    {
        ret = g_db->addMusicToLocalMusic(date);
    }
}

void playController::slotStateChanged(MMediaPlayer::State newState)
{
    if(newState == MMediaPlayer::State::PlayingState)
        Q_EMIT playerStateChange(playController::PlayState::PLAY_STATE);
    else if(newState == MMediaPlayer::State::PausedState)
        Q_EMIT playerStateChange(playController::PlayState::PAUSED_STATE);
    else if(newState == MMediaPlayer::State::StoppedState)
        Q_EMIT playerStateChange(playController::PlayState::STOP_STATE);

//    DbusAdapter *m_adapter = new DbusAdapter;
//    QVariantMap changedProperties;
//    changedProperties.insert("PlaybackStatus",QVariant::fromValue(QDBusVariant("Stopped")));
//    m_adapter->notify("org.mpris.MediaPlayer2.Player",changedProperties,QStringList());
//    delete m_adapter;
}
