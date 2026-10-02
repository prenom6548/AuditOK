#ifndef TRAY_H
#define TRAY_H

#include <QMap>
#include <QDebug>
#include <QWidget>
#include <QProcess>
#include <QSettings>
#include <QSqlQuery>
#include <QTextCodec>
#include <QPushButton>
#include <QTranslator>
#include <QSqlDatabase>
#include <QNetworkReply>
#include <QSqlQueryModel>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "traydbus.h"
#include "trayicon.h"
#include "updatewidget.h"


class updatehandle : public QWidget
{
    Q_OBJECT
public:
    updatehandle(QString getnum1,QWidget *parent = nullptr);
    ~updatehandle();
    trayicon          *trayIcon;                            //托盘实例化
    QStringList       important;                            //保存重要更新包名
    QString           argnum;                               //得到命令行输入参数
    traydbusservice   *object;                              //托盘dbus服务
    bool              powerState;                           //保存电量是否可获取
    QString           powerNum = "0";                       //保存电量值
    QString           confPath = "";                        //保存配置文件位置

    bool              dbusConnectStatus=false;              //d-bus连接状态
    bool              messstate1=true;
    bool              messstate2=true;
    int               powerSize;                            //电源设定值
    float             setPackageSize;                       //包大小设定值
    QString           CheckUpgradeTime;                     //检查更新时间
    QString           CheckUpgradeTimeSlot;                 //检查更新时间段
    bool              CheckCompleted;                       //是否检查完成
    bool              RandomizedExecute;                    //是否随机执行
    bool              GeneratRandomTime;                    //是否刷新随机时间
    bool              errorStatus=false;                    //是否完整下载安装的状态值

    updatewidget      *choice_window;                       //托盘选择框
    updateTip         *tip_window;                          //托盘提示框框
    QStringList       checkCrucial;                         //保存静默升级包以及大小信息

    source_dbus       *m_sourceMutual;                      //源管理器dbus
    update_dbus       *m_updateMutual;                      //更新管理器dbus
    power_dbus        *Power_dbus;
    QTimer            *timerDownload;                       //托盘下载时间错误延时处理
    QTimer            *timerLock;                           //文件锁定时检测
    QSettings         *mqsetting;                           //读取配置文件
    QSettings         *timersetting;                        //读取timer配置文件


    void initUI(QString);                                          //初始化界面
    void initDbus();                                        //初始化dbus
    void initTranslator();                                  //初始化翻译加载
    void getTemplateList();                                 //获取源管理器文件中的可选更新与强制更新
    void processData(QMap<QString,QStringList> data);       //处理获取的更新数据
    void handleCrucial(QStringList checkCrucialAll);        //处理静默升级包
    void handleImportant();                                 //处理可选升级包
    void register_dbus();                                   //注册D-bus服务
    bool checktime();                                       //检查设定更新时间

    //数据库相关
    QSqlDatabase      db;
    QSqlQueryModel    model;

    bool m_getsql();                                        //获取更新管理器数据库信息
    void notify_send(QString arg);                          //向notification发送信息显示

    bool packageDownOrNot(QStringList arg);
    void readconf();                                        //读取配置文件
    int _secsTo(QString,QString);           //时间差
    void getpower();                                        //获取电量值
    void UpdateDectCheck(bool);                   //调用后端UpdateDetect
    bool ignoreOrDelay(bool mode);   //是否开机检查检查弹窗
    bool package_installation_status(QStringList);
signals:
    void execSignal();                                      //关闭窗口信号

protected slots:
    void onActivated(trayicon::ActivationReason reason);    //更新选择页面
    void tray_Show();                                       //启动托盘显示
    void execslots();                                       //关闭窗口槽函数
    void d_busStatus(QString,int32_t,QString);    //判断下载状态的槽函数
    void InstallFinsih(bool,QStringList,QString,QString);   //状态槽函数
    void downloadTimeout();                                 //延时检测下载状态
    void UpdateDectSlot(bool,QStringList,QString,QString);    //updatedetect回调函数
};

#endif // TRAY_H
