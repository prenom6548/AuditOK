#ifndef PLAYCONTROL_H
#define PLAYCONTROL_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioBuffer>
#include <QTimer>

#include "mmediaplayer.h"

class PlayControl : public QObject
{
    Q_OBJECT
public:
    explicit PlayControl(QObject *parent = nullptr);
    ~PlayControl();

private:
    MMediaPlayer *player = nullptr;

    MMediaPlayer::State m_state;

public:
    static PlayControl &getInstance()
    {
        static PlayControl m_pInstance;
        return m_pInstance;
    }
    MMediaPlayer *getPlayer()
    {
        return player;
    }

    QString isPlayingPath = "";      //正在播放的路径
    QString isClipPath = "";         //正在裁剪的路径
    QString isPlayingPathBirthTime = "";
    QTimer *moveTimer = nullptr;     //单位时间:n毫秒
    QTimer *clipMoveTimer = nullptr; //剪裁时的单位时间:n毫秒
    void play_Audio(QString filePath, qint64 pos = 0);
    void play_Audio_Reload(QString filePath, qint64 pos = 0);
    void pause_Audio();
    void stop();
    void setPosition(qint64 position);

    QMediaPlayer::State getAudioState(); //获取音频播放状态

    QString getPlayPath() { return isPlayingPath;}
signals:

public slots:
};

#endif // PLAYCONTROL_H
