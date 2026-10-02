#include "playcontrol.h"
#include <QFileInfo>
#include <QDateTime>

PlayControl::PlayControl(QObject *parent) : QObject(parent)
{
    player = new MMediaPlayer(this);
    moveTimer = new QTimer(this); // QTimer
    clipMoveTimer = new QTimer(this);
    qDebug() << "执行1次";
}

QMediaPlayer::State PlayControl::getAudioState()
{
    m_state = player->state();
    if (m_state == MMediaPlayer::State::StoppedState){
        return QMediaPlayer::State::StoppedState;
    } else if (m_state == MMediaPlayer::State::PlayingState){
        return QMediaPlayer::State::PlayingState;
    } else if (m_state == MMediaPlayer::State::PausedState){
        return QMediaPlayer::State::PausedState;
    }
}

void PlayControl::play_Audio(QString filePath, qint64 pos)
{
    if (isPlayingPath != filePath || isPlayingPathBirthTime != QFileInfo(filePath).birthTime().toString() || player->state() == MMediaPlayer::State::StoppedState){
        isPlayingPath = filePath; //非原路径的播放
        isPlayingPathBirthTime = QFileInfo(isPlayingPath).birthTime().toString();
        play_Audio_Reload(filePath, pos);
        qDebug() << "非原路径播放:" << filePath << ", pos:" << pos;
    } else {
        player->playPause(pos);
        qDebug() << "原路径播放:" << filePath << ", pos:" << pos;
    }
}

void PlayControl::play_Audio_Reload(QString filePath, qint64 pos)
{
    PlayControl::getInstance().getPlayer()->stop();
    qInfo() << "-----play_Audio_Reload pos:" << pos;
    player->setMedia(filePath);
    player->setVolume(85);
    player->play(pos);
}

void PlayControl::pause_Audio()
{
    qDebug() << "暂停了" ;
    player->playPause();
}

void PlayControl::stop()
{
    clipMoveTimer->stop();
    player->stop();
}

void PlayControl::setPosition(qint64 position)
{
    if (qAbs(player->position() - position) > 99)
        player->setPosition(position);
    qDebug() << "数值:" << position;
}

PlayControl::~PlayControl() {}


