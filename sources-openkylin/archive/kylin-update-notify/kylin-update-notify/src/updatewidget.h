#ifndef TRAYWIDGET_H
#define TRAYWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDebug>
#include <QPainter>
#include <QProcess>
#include <QComboBox>
#include <QDir>
#include <fcntl.h>
#include <QDateTime>
#include <QPoint>
#include <QDBusInterface>
#include <QGSettings>
#include <QScreen>
#include <QDBusReply>
#include <QSettings>
#include <sys/stat.h>
#include <unistd.h>
#include <QMouseEvent>
#include "Button/fixbutton.h"

namespace Ui {
class Form;
}

class updatewidget : public QWidget
{
    Q_OBJECT

public:
    explicit updatewidget(bool check_Tips, QWidget *parent = nullptr);
    ~updatewidget();

    enum PanelStatePosition
    {
        PanelDown = 0,
        PanelUp,
        PanelLeft,
        PanelRight
    };

    bool _check_Tips;
    QLabel *update_lab;
    QLabel *icon_lab;
    QLabel *tip_lab;
    QLabel *close_lab;

    FixButton *agreebtn;
    FixButton *delaybtn;
    QPushButton *window_close;
    QWidget *labelwidget;
    QWidget *statwidget;
    QWidget *selectWidget;
    QStringList strList;
    QPixmap *icon;

    QVBoxLayout  *m_pfirstlayout    = nullptr;
    QHBoxLayout  *m_labellayout     = nullptr;
    QHBoxLayout  *m_psecondlayout   = nullptr;
    QHBoxLayout  *m_pselectlayout   = nullptr;

    QDBusInterface*  m_pServiceInterface;                            // 获取任务栏的高度
    QGSettings       *m_pPanelSetting = nullptr;
    QComboBox        *comboBox;

    int              m_nScreenWidth;                                 // 屏幕分辨率的宽
    int              m_nScreenHeight;                                // 屏幕分辨率的高
    int              m_pPeonySite;                                   // 任务栏位置
    QSettings        *timersettings;                        //读取timer配置文件
    QRect             updateWidgetRect;
    bool bIsWayland = false;

    int              connectTaskBarDbus();
    int              getPanelSite();
    void             GetsAvailableAreaScreen();
    char             *insideTime();
    int              getTaskbarPos(QString str);
    int              getTaskbarHeight(QString str);
    int              getScreenGeometry(QString methodName);
    void             set_window_position();
    void             initset_window_position();
    void             listenPanelChange();
    void             initPanelDbusGsetting();                        // 初始化与任务栏gsetting和dbus
    bool             eventFilter(QObject *, QEvent *);	     // 添加事件过滤器声明
    bool _is_openkylin();
    void updateWidgetShow();

private:
    Ui::Form *ui;
    void paintEvent(QPaintEvent *e);

signals:
    void agreeBtnSignals();
    void disAgreeBtnSignals();

protected slots:
    void agreeBtnClicked();
    void comboBoxClicked(int);
    void delayBtnClicked();
    void onClick_close_btn();
};





class updateTip : public QWidget
{
    Q_OBJECT
public:
    explicit updateTip(QWidget *parent = nullptr);
    ~updateTip();
    QLabel       *tipLabel;
    QVBoxLayout  *m_layout    = nullptr;


private:
    void paintEvent(QPaintEvent *e);


signals:

protected slots:
};

#endif // TRAYWIDGET_H
