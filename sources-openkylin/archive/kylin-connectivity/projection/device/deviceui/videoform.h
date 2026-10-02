#ifndef VIDEOFORM_H
#define VIDEOFORM_H

#include <QPointer>
#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QGSettings>
#include "loadanimation.h"
#include "tablemodelistener.h"

namespace Ui {
class videoForm;
}

enum WindowEdge { TOP = 12, LEFT = 21, CENTER = 22, RIGHT = 23, BUTTOMLEFT = 31, BUTTOM = 32, BUTTOMRIGHT = 33 };

struct AVFrame;
class ToolForm;
class BaseDevice;
// class FileHandler;
class QYUVOpenGLWidget;
class QLabel;
class VideoForm : public QWidget
{
    Q_OBJECT
public:
    explicit VideoForm(QWidget *parent = nullptr, int screenIndex = 0);

    ~VideoForm();

    void updateShowSize(const QSize &newSize);

    void updateRender(const AVFrame *frame);

    void setDevice(BaseDevice *device);

    QRect getGrabCursorRect();

    const QSize &frameSize();

    // 工具栏
    void showToolForm(bool show);

    void deleteUI();

public Q_SLOTS:
    void onToolFormStatusChange();

    void onModeChanged(bool isTable);

    void onModeRotationsChange(QString rotaion);

    void onScrollTrigger();

    void onGetInputState();
    void onGetInputPos(int x, int y);
    void onLostInputState();

private:
    void initUI();

    // 注册信号
    void initConnect();

    // 移动窗口位置
    void moveCenter();

    // 注册快捷键
    void installShortcut();

    void slotActivated(QSystemTrayIcon::ActivationReason reason);

    // 窗口缩放
    void windowZoom();

    void changeInputBoxPos();

    QPoint cursorPoint();
    QRect getScreenRect();
    void setScreenIndex();
    void initPanelSetting();
    void checkScreenRect(QRect &rect);

protected:
    bool eventFilter(QObject *watch, QEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    // ui
    Ui::videoForm *ui;
    QPointer<ToolForm> m_toolForm;
    QPointer<LoadAnimation> m_loadingWidget;
    QPointer<QYUVOpenGLWidget> m_videoWidget;

    // inside member
    QSize m_frameSize;
    QPoint m_dragPosition;
    float m_widthHeightRatio = 0.5f;
    bool m_resizeFlag = false;  //窗口大小重置标志
    int m_titleHeight;          // 标题栏与导航栏高度

    bool m_isVertical = false;      // 手机横竖屏
    bool m_normalVertical = false;  // 窗口状态改变之前的横竖屏状态
    bool m_isToolFormShow = true;   // 是否展示导航栏
    int m_inputX = 0;
    int m_inputY = 0;
    int m_screenIndex = 0;
    QGSettings *m_panelSetting = nullptr;
    // outside member
    QPointer<BaseDevice> m_device;
    bool m_isLoading = false;
    QTimer *m_loadTimer = nullptr;
    TableModeListener *m_tableModeListener = nullptr;
    bool m_isTable = false;
    bool m_isMax = false;
    bool m_isFull = false;
    int m_timerId = 0;
    QSize m_oldSize = QSize(0, 0);
    int m_resizeAuto = false;
    bool m_isWindeStateChanged = false;
    bool m_isNormal = true;
};

#endif  // VIDEOFORM_H
