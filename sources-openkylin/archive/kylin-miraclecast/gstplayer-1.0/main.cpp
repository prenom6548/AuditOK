#include <glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

#include <QApplication>
#include <QWidget>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QTimer>
#include <QDebug>
#include <QCommandLineParser>
#include <QDBusInterface>
#include <QProcess>
#include <QStringList>
#include "playwidget.h"
#include <QTranslator>
#include "mobiletype.h"
#include <signal.h>
#include <iostream>
using namespace std;


static WId winId;
//hardsoft decode by default
static QString decodeFlag="hard";
static bool xvFlags = true;     //部分显卡不支持xvimagesink


void my_exit(int arg)
{
    cout<<"closed"<<endl;
    qDebug()<<"player quit by signal kill term"<<endl;
    exit(0);
}
static GstBusSyncReply create_window(GstBus *bus, GstMessage *message, GstPipeline *pipeline)
{
    if (!gst_is_video_overlay_prepare_window_handle_message(message))
        return GST_BUS_PASS;

    qDebug() << "Received bus message";

    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)), winId);

    gst_message_unref(message);

    return GST_BUS_DROP;
}

static void setenv()
{
    QString LIST_VGA = "lspci";
    QString output="";


    QStringList sl={};
    QProcess proc ;
    proc.start(LIST_VGA);
    proc.waitForFinished();
    QString res=proc.readAllStandardOutput();
    if(res!=NULL&&res.contains("VGA"))
    {
        sl=res.split('\n');
        for(int i=0;i<sl.size();i++)
        {
            if(sl[i].contains("VGA"))
            {
                output+=sl[i]+'\n';
            }
        }
    }
    qDebug()<<"set env";
    qDebug()<<output;

    QString str = "Graphics";
    bool UHD = false;
    if(output.contains(str))
    {
        int pos = output.indexOf(str);
        QString str1 = output.mid(pos+9,1);
        if(str1 == "(")
        {
            UHD = true;
        }
    }
    if(output.contains("AMD"))
    {
        //remove hardsoft decode function on AMD Oland VGA
        if(output.contains("Oland"))
        {
            decodeFlag = "kylin";
        }
        else
        {
            qputenv("LIBVA_DRIVER_NAME", "radeonsi");
            qputenv("GST_VAAPI_ALL_DRIVERS", "1");
        }
    }
    else if(output.contains("Intel"))
    {
        if(UHD)
        {
            decodeFlag = "kylin";
        }else
        {
            qputenv("LIBVA_DRIVER_NAME", "i965");
        }

    }
    else
    {
        decodeFlag = "kylin";
    }

    if(output.contains("Loongson") || output.contains("Renoir") || output.contains("Zhaoxin") || output.contains("0709"))  //部分显卡不支持xvimagesink
    {
        xvFlags =false;
        qDebug()<<"current device cannot use xviamgesink";
    }
    qDebug()<<decodeFlag;
    qDebug()<<"lenove can not support"<<UHD;
}

static void  changeMobileType(QString type){
    int manufacturer;
    qDebug() << type;
    if (type == QObject::tr("huawei"))
    {
        manufacturer = 6;
    }
    else if (type == QObject::tr("xiaomi"))
    {
        manufacturer = 7;
    }
    else
    {
        manufacturer = 8;
    }
    QDBusInterface *m_pServiceInterface = new QDBusInterface("org.freedesktop.miracleagent",
                                                             "/org/freedesktop/miracleagent",
                                                             "org.freedesktop.miracleagent.op",
                                                             QDBusConnection::sessionBus());
    m_pServiceInterface->call("SetManufacturer",manufacturer);
    QString a1=QString("%1").arg(manufacturer);
    cout << a1.toLatin1().data() << endl;
}

static void showChangeMobileTypeDialog(){
    MobileType * dialog = new MobileType();
    QObject::connect(dialog, &MobileType::sendMobileType, [=](QString type){
        changeMobileType (type);
    });
    dialog->exec();
}





// Code to play rtsp stream on qt widgets
int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    sigset(SIGTERM,my_exit);
    QApplication app(argc, argv);
    QTranslator translator;
    QLocale locale;
    if (locale.language() == QLocale::Chinese){
        translator.load(":/zh_CN.qm");
        app.installTranslator(&translator);
    }
    else if (locale.language() == QLocale::English){
        translator.load(":/en_US.qm");
        app.installTranslator(&translator);
    }

    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit ()));

    setenv();


    gst_init (&argc, &argv);

    QCommandLineParser parser;
    QCommandLineOption option_audio("a","enable audio support");
    QCommandLineOption option_scale("s","scale to resolution","split with x","scale");
    QCommandLineOption option_debug("d","debug level","debug","1");
    QCommandLineOption option_port("p","port for rtsp","port","1991");
    QCommandLineOption option_r("r","resolution","split with x","1920x1080");
    QCommandLineOption option_uibc("uibc","supported resolutions masks (CEA VESA HH)");
    QCommandLineOption option_hidc("hidc","enable hidc control back");
    QCommandLineOption option_kylinpc("kylinpc","kylinpc or not");

    QCommandLineOption option_manufacturer("m","Confirm mobile type","manufacturer","5");

    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(option_audio);
    parser.addOption(option_scale);
    parser.addOption(option_port);
    parser.addOption(option_r);
    parser.addOption(option_debug);
    parser.addOption(option_uibc);
    parser.addOption(option_hidc);
    parser.addOption(option_kylinpc);

    parser.addOption(option_manufacturer);


    parser.process(app);
    int audio=0;
    int scale=0;
    int uibc=1;
    int hidc=0;
    int resolution=0;
    int arg_width=1920;
    int arg_height=1080;
    int arg_port=888;
    int kylinpc = 0;
    int manufacturer = 5;


    if (parser.isSet(option_audio))
    {
        audio=1;
    }
    if (parser.isSet(option_uibc))
    {
        uibc=1;
    }
    else
    {
        QDBusInterface interface( "org.freedesktop.Notifications",
                                  "/org/freedesktop/Notifications",
                                  "org.freedesktop.Notifications",
                                  QDBusConnection::sessionBus());
        QString appname = QObject::tr("projection");
        quint32 notify_id = 0;
        QString app_icon = "kylin-miracast";
        QString tilte = QObject::tr("Note：device can not support uibc");
        QString body = QObject::tr("Can't be controlled by keyboard/mouse.");
        qint32 timeout = -1;
        QStringList arry1;
        QVariantMap arry2;
        interface.call(QLatin1String("Notify"),appname,notify_id,app_icon,tilte,body,arry1,arry2,timeout);
    }
    if (parser.isSet(option_hidc))
    {
        hidc=1;
    }
    if (parser.isSet(option_r))
    {
        resolution=1;
        QStringList list = parser.value(option_r).split("x");
        //**************************************************************
        //**************************************************************
        //下一行要记得修改哦
        arg_width=list[0].toInt();
        arg_height=list[1].toInt();
        qDebug()<<arg_width <<" "<<arg_height;
    }
    if (parser.isSet(option_scale))
    {
        scale=1;
        QStringList list = parser.value(option_scale).split("x");
        //**************************************************************
        //**************************************************************
        //下一行要记得修改哦
        arg_width=list[0].toInt();
        arg_height=list[1].toInt();
    }
    if (parser.isSet(option_port))
    {
        arg_port=parser.value(option_port).toInt();
        qDebug()<<"port is"<<arg_port;
    }
    if (parser.isSet(option_debug))
    {
        qDebug()<<parser.value(option_debug);
    }
    if (parser.isSet(option_kylinpc))
    {
        kylinpc = 1;
    }

    if (parser.isSet(option_manufacturer))
    {
        manufacturer = parser.value(option_manufacturer).toInt();
        qDebug()<<"manufacturer is"<<manufacturer;
        if (manufacturer == 5 && kylinpc == 0)
        {
            showChangeMobileTypeDialog();
        }
        else
        {
            QString a1=QString("%1").arg(manufacturer);
            cout << a1.toLatin1().data() << endl;
        }
    }

    // prepare the ui
    playWidget window;
    if (locale.language() == QLocale::Chinese){
        window.setWindowTitle("投屏播放器");
    }
    else if (locale.language() == QLocale::English){
        window.setWindowTitle("MiracastPlayer");
    }
    window.resize(arg_width,arg_height);
    window.showFullScreen();
    window.setWidgetSize(arg_width,arg_height);
    window.setargv( audio,  scale,  uibc,  hidc,  resolution,  arg_width,  arg_height,  arg_port,decodeFlag,kylinpc,xvFlags);
    window.init_mouse();
    window.init_keyboard();
    //window.setWindowFlags(Qt::FramelessWindowHint);
    window.show();

    int ret = app.exec();
    window.hide();
    return ret;
}
