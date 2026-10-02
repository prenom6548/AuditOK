#ifndef CAMERACONTROLVIEWPAGE_H
#define CAMERACONTROLVIEWPAGE_H


#include <QWidget>
#include "button.h"
#include "delaywdiget.h"
#include <QStackedWidget>
#include "recordmodule.h"
#include "npixcapwidget.h"
#include "camerafilterwidget.h"

class CameraControlViewPage : public QWidget
{
    Q_OBJECT

public:
    explicit CameraControlViewPage(QWidget *parent = nullptr);
    ~CameraControlViewPage();

    //倒计时
    QLabel *dead_time = nullptr;
    QTimer *time = nullptr;
    QWidget *timeLabel = nullptr;
    QLabel *recorder_time_label = nullptr;
    QTimer * recorder_time = nullptr;
    QWidget *recorder = nullptr;
    QWidget *recorderWidget = nullptr;
    QWidget *recorderWidgetWin = nullptr;
    QStackedWidget *buttom = nullptr;

    DelayWdiget *delayWidget = nullptr;
    QHBoxLayout *delayWidgetLayout = nullptr;
    QWidget *delayWidgetContainer = nullptr;

    RecordModule *recordModule = nullptr;
    QHBoxLayout *recordModuleLayout = nullptr;
    QWidget *recordModuleContainer = nullptr;

    NPixCapWidget *nPixCapWidget = nullptr;
    QHBoxLayout *nPixCapWidgetLayout = nullptr;
    QWidget *nPixCapWidgetContainer = nullptr;

    cameraFilterWidget *filterWidget = nullptr;
    QHBoxLayout *filterWidgetLayout = nullptr;
    QWidget *filterWidgetContainer = nullptr;

    QHBoxLayout *recordLayout = nullptr;
    QHBoxLayout *recorderWidgetLayout = nullptr;
    QHBoxLayout *hlayout = nullptr;
    QHBoxLayout *h2layout = nullptr;
    QVBoxLayout *vlayout = nullptr;
    QVBoxLayout *vlayout_out = nullptr;



};

#endif // CAMERACONTROLVIEWPAGE_H
