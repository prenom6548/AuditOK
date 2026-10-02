#ifndef RECORDMODULE_H
#define RECORDMODULE_H


#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QPainterPath>
#include "defaultgetset.h"

class RecordModule: public QWidget
{
    Q_OBJECT
public:
    explicit RecordModule(QWidget *parent = nullptr);
    QTimer *recorderTime = nullptr;//计时器
    ~RecordModule();
    QPushButton *videoStopBtn = nullptr;    //停止录像
    QPushButton *videoContinueBtn = nullptr;  //开始录像
    QPushButton *videoPauseBtn = nullptr; //暂停录像
    int hour = 0;
    int min = 0;
    int sec = 0;
private:

    DefaultGetSet gsettingItem;
    QLabel *recorderTimeLabel = nullptr;//录像计时的容器
    QHBoxLayout *recordModuleLayout = nullptr;
    QWidget *recordModuleContainer = nullptr;
    QHBoxLayout *layout = nullptr;
public Q_SLOTS:
    void updateRecordingTime(int hourSig, int minSig, int secSig);
    void clickRecordSlot();
private Q_SLOTS:
    void startVideoPress();
    void suspendVideoPress();
    void stopVideoPress();
    void paintEvent(QPaintEvent *event);
Q_SIGNALS:
    void recordSuspendSig(bool);
    void recordStopSig();
};

#endif // RECORDMODULE_H
