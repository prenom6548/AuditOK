#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

class QTimer;

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

enum PINSTATUS{
    INITIAL,
    INITIALED,
    FAIL
};

enum PLAYERSTATUS{
    INIT,
    FAILED,
    SUCCESS
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void countDownWarning();
    void updateCountDownWaring();
    // 恢复到初始状态
    void FirstPageReset();
    void secondPageReset(PINSTATUS status);
    void thirdPageReset(PLAYERSTATUS status);
    void switchToPage2(bool verified);
    // dbusx信号初始化
    void dbusSignalInit();
    // 获取dbus信号路径
    const char *getServiceName() const{
        return "org.freedesktop.miracleagent";
    }
    const char *getDbusSiganlPath() const{
        return "/org/freedesktop/miracleagent";
    }
    // 获取dbus信号Interface name
    const char *getDbusInterfaceName() const{
        return "org.freedesktop.miracleagent";
    }


private slots:
    void on_permitBtn_clicked();

    void on_forbidBtn_clicked();

    void on_exitBtn_clicked();

    void on_exitBtn3_clicked();

    void getFormationingSignal(QString deviceName);

    void getConfirmingSignal(QString deviceName);

    void getFormationOverSignal(int status, QString failedReason);

    void getRtspOverSignal(int status);

    void getdisconnectedSignal();

    void on_exitBtn4_clicked();

private:
    Ui::Widget *ui;
    QTimer *timer;
    int countTimer;
    QString connectDevice;
};
#endif // WIDGET_H
