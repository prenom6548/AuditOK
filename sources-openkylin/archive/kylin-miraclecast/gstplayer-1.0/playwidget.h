#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QTimer>
#include <QMouseEvent>
#include <QPalette>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QPushButton>
#include <QVBoxLayout>
namespace Ui {
class playWidget;
}

class playWidget : public QWidget
{
    Q_OBJECT

public:
    explicit playWidget(QWidget *parent = nullptr);
    ~playWidget();
    void init();
    void initUI();
    void setWidgetSize(int x, int y);
    void init_mouse();
    void init_keyboard();
    static WId winID;

    void setenv();
    void preparePipeline();
    bool setPlayerPipe(GstElement *pipe);
    void setargv(int audio, int scale, int uibc, int hidc, int resolution, int arg_width, int arg_height, int arg_port, QString decodeFlag, int kylinPc, bool xv);

    int kylinPc;



private:
    Ui::playWidget *ui;
    int left_time=0;
    int reso=0;
    int uibc=1;
    int hidc=0;

    const int cons_leftBtnPressed=0;
    const int cons_noBtnPressed=1;
    const int cons_wheelRoll=2;             //滚轮滑动
    const int cons_keyPressed=2;
    const int cons_keyReleased=3;
    const int cons_nullBtn=0;
    const int cons_leftBtn=1;
    const int cons_midBtn=2;
    const int cons_rightBtn=4;
    int hid_key_event=0;
    int hid_mouse_event=1;

    int window_width=0;
    int window_height=0;
    int widget_width=0;
    int widget_height=0;

    int half_area_width=0;
    int half_area_height=0;
    int half_def_width=0;
    int half_def_height=0;
    int min_hor_pos=0;
    int max_hor_pos=0;
    int min_ver_pos=0;
    int max_ver_pos=0;
    int uibc_x=-1;
    int uibc_y=-1;
    int widget_show_flag=1;
    int circle_flag=0;

    QString decodeFlag="hard";
    int audio=0;
    int scale=0;

    int resolution=0;
    int arg_width=1920;
    int arg_height=1080;
    int arg_port=888;
    const gchar*hardPip;
    const gchar*softPip;
    GstElement *softPipeline;
    GstElement *hardPipeline;
    GstBus *bus;
    GstElement*currentPipe;
    QByteArray ba1;
    QByteArray ba;
    QPushButton *pushButton_allscreen;
    QPushButton *pushButton_changeDecode;
    QPushButton *pushButton_back;
    QPushButton *pushButton_fun;
    QPushButton *pushButton_home;
    QVBoxLayout*mLayout;
    int current_width = 0;
    int current_height = 0;
    bool xvFlags = true;
    bool updateFlag =false;





public slots:
    void time_update();
    void home_clicked();
    void back_clicked();
    void fun_clicked();
    void screen_clicked();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent * event);
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);
    void circleMouse(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);
private slots:
    void on_pushButton_allscreen_2_clicked();
    void closeEvent(QCloseEvent *event);
};

#endif // PLAYWIDGET_H
