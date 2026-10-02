#ifndef PLAYCONTROLLER_H
#define PLAYCONTROLLER_H

#include <QObject>
#include <QTimerEvent>
#include <QTimer>
#include <QEventLoop>
#include "mmediaplayer.h"
#include "mmediaplaylist.h"

class playController : public QObject
{
    Q_OBJECT
public:
    enum PlayState {
        STOP_STATE = 0,
        PLAY_STATE = 1,
        PAUSED_STATE = 2
    };
public:
    static playController& getInstance()
    {
        static playController instance;
        return instance;
    }
    void cleanup() {
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
    explicit playController(QObject *parent = nullptr);
    ~playController();
    bool play(QString playlist, int index);

    bool play();
    bool pause();
    bool pauseOnly();
    bool stop();
    void nextSong();
    void previousSong();
    void setSongIndex(int index);
    void setCurPlaylist(QStringList songPaths);
    QVariantMap getMetadata() const;
    void addMusicToDatebase(QStringList fileNames);
Q_SIGNALS:
    void playerStateChange(playController::PlayState newState);
    void mateDataChanged(); //媒体切换信号
    void singalChangePath(QString path);
//    void curIndexChanged(int index);
private Q_SLOTS:
    //状态改变
    void slotStateChanged(MMediaPlayer::State newState);
    void slotIndexChange(int index);
private:
    //当前播放列表名
    QString m_curList;
    //当前播放的索引
    int m_curIndex;
    MMediaPlayer* m_player  = nullptr;
    MMediaPlaylist* m_playlist  = nullptr;
};

#endif // PLAYCONTROLLER_H
