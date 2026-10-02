#ifndef BUTTON_H
#define BUTTON_H


#include <QPushButton>
#include <QToolButton>
#include <QKeyEvent>
#include <QLayout>
#include <QWidget>
#include <QSettings>
#include <QStackedWidget>
#include "pictureviewpage.h"
#include "QDBusInterface"
#include <QGSettings>
#include "defaultgetset.h"
#include <kpressbutton.h>
#include <kpushbutton.h>

class CameraButton : public kdk::KPressButton{
Q_OBJECT
public:
    explicit CameraButton(QWidget* parent = 0);
    ~CameraButton();
public:
    void setIgnorePress(bool ignore);
private:
    bool m_ignorePress = false;
Q_SIGNALS:
    void buttonTriggered();
private Q_SLOTS:
    void onClicked(bool);
};


class Button : public QWidget
{
  Q_OBJECT
public:
    explicit Button(QWidget *parent = 0);
    ~Button();
    void CreatButton();
    QStackedLayout *picStackWidget = nullptr;
    QPushButton *pictureVideo = nullptr;
    QLabel *picture = nullptr;
    QWidget *pictureWrap = nullptr;
    kdk::KPressButton *capture = nullptr;
    kdk::KPressButton *video = nullptr;
    kdk::KPushButton *cheese = nullptr;   //拍照 白天主题
    kdk::KPushButton *cheese_dark = nullptr; //拍照 黑夜主题
    //QPushButton *picture;

    QPushButton *scale = nullptr;
    QPushButton *voice = nullptr;
    QPushButton *Setting = nullptr;
//    QPushButton *stop = nullptr;
    QPushButton *cheese_video = nullptr; // 录像开始

    kdk::KPressButton *cheese_stop = nullptr;  // 录像结束
    QPushButton *restore = nullptr;   //恢复
    QPushButton *layout = nullptr;
    QPushButton *filter = nullptr;

    static bool delay_mode;  //延迟拍照模式
    static bool video_mode;  //录像模式
    static bool video_start; //录像开启
    static bool voice_mode;
    static bool Burst_mode;  //滤镜模式
    static bool picturepage_show; //相册显示

Q_SIGNALS:
    void winHeight();
protected:
    virtual void paintEvent(QPaintEvent *);
    virtual bool eventFilter(QObject *watched, QEvent *event);

public Q_SLOTS:
    void capture_click();
    void video_click();
    void cheese_click();
    void cheese_dark_click();
    void picture_click();
    void stop_click();
    void voice_click();
    void cheese_video_click();
    void cheese_stop_click();
    void restore_click();
    void dealSystemGsettingChange(const QString);
    void albumRightMenu();
    void openDir();
    void openVideoDir();
    void openFile();
    void rmEventFilter();
    void installEventFilter();

private:
    QHBoxLayout *switchLayout = nullptr;
    QHBoxLayout *btnLayout = nullptr;
    QStackedLayout *btnContainerLayout = nullptr;
    QHBoxLayout *btnLayout1 = nullptr;
    QWidget *btnWidget1 = nullptr;
    QHBoxLayout *btnLayout2 = nullptr;
    QWidget *btnWidget2 = nullptr;

    QVBoxLayout *vLayout = nullptr;
    QSettings *set = new QSettings;
    QString nowThemeStyle;
    DefaultGetSet *gsettingItem = nullptr;
    QHBoxLayout *picAndButton = nullptr;

    bool m_canPlayVideo = true;
    const uint32_t m_playVideoInterval = 1500;
    QDBusInterface *m_dbus_interface_peony; // 通过D-Bus打开文管

    void initConnect();
    void initThemeApt();
public:
    CameraButton *m_videoDelay = nullptr;
    CameraButton *m_captureDelay = nullptr;
    CameraButton *m_captureBurst = nullptr;  //连拍
private:
    CameraButton *m_captureMirror = nullptr;
    CameraButton *m_videoGrid = nullptr;
    CameraButton *m_captureGrid = nullptr;
private Q_SLOTS:
    void onVideoGridClicked();
    void onMirrorClicked();
    void onBurstClicked();
    void onDelayClicked();
Q_SIGNALS:
    void showGirdLine(bool isShown);
    void mirror(bool isMirror);
    void delay(bool);
    void burst(bool);
public:
    void setBurstCount(int);
    void enableButtons(bool);
    bool getMirror(){
        return m_captureMirror->isChecked();
    }

};

#endif // BUTTON_H
