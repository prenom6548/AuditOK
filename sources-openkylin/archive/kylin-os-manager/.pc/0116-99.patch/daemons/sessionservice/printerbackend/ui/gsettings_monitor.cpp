#include "gsettings_monitor.h"
#include <QVariant>
#include <QDebug>
#include <QFontDatabase>
#include <QFont>
#include "global_variable.h"
#include <QDateTime>
#include "libkydate.h"
#include <QDBusConnection>
#include "kom-configure.h"

const double SYSTEM_DEFAULT_FONT_SIZE = 11;
const double SYSTEM_DEFAULT_FONT_SIZE_PIXELS = 9;
QMutex GsettingsMonitor:: m_mutex;
GsettingsMonitor::GsettingsMonitor()
{
    if (gsettingsCheck()) {
        initGsettings();
    }
}
GsettingsMonitor::~GsettingsMonitor(/* args */) { qDebug() << "GsettingsMonitor destroy.."; }
GsettingsMonitor* GsettingsMonitor::getInstance()
{
    static GsettingsMonitor instance;
    return &instance;
}

bool GsettingsMonitor::gsettingsCheck()
{
    if(!QGSettings::isSchemaInstalled(UKUI_STYLE)) {
        qCritical() << QString("gsettings Error: %1 not found!").arg(UKUI_STYLE);
        return false;
    }
    if(!QGSettings::isSchemaInstalled(KYLIN_PRINTER_SETTINGS)) {
        qCritical() << QString("gsettings Error: %1 not found!").arg(KYLIN_PRINTER_SETTINGS);
        return false;
    }
    return true;
}

QFont GsettingsMonitor::update()
{
    if (gsettingsCheck()) {
        return getFontSize();
    }
    return QFont();
}

void GsettingsMonitor::initGsettings()
{
    if(QGSettings::isSchemaInstalled(UKUI_STYLE)) {
        m_ukuiStyle = new QGSettings(UKUI_STYLE);
        connect(m_ukuiStyle, &QGSettings::changed, this, [=] (const QString &key) {
            if(key == UKUI_FONT_SIZE ||
               key == UKUI_FONT ) {
                //获取字号的值
                QString fontStyle = m_ukuiStyle->get(UKUI_FONT).toString();
                double fontSizeKey = m_ukuiStyle->get(UKUI_FONT_SIZE).toString().toDouble();
                QFontDatabase db;
                //发送改变信号
                if (fontSizeKey > 0 &&
                    db.families().contains(fontStyle))
                {
                    getFontSize();
                }
            }
            else if (key == UKUI_MENU_TRANSPARENCY) {
                sysTransparencyGet();
                }
            else if (key == UKUI_STYLE_NAME) {
                sysStyleNameGet();
            }
        });
        getFontSize();
    }

    m_kylinPrinterSettings = new QGSettings(KYLIN_PRINTER_SETTINGS);
    QObject::connect(m_kylinPrinterSettings , &QGSettings::changed, [=] (const QString &key){
        if (key == KYLIN_PRINTER_IP) {
            apiServerAddressGet();
        }
        else if (key == KYLIN_PRINTER_PRINTERS) {
            printerDevicesGet();
        }
        else if (key == KYLIN_PRINTER_SHOW_PROTOCOLS) {
            showProtocolsGet();
        }
    });

    if(QGSettings::isSchemaInstalled(PANEL_CONTROL_IN_CALENDAR)) {
        m_calendarSettings = new QGSettings(PANEL_CONTROL_IN_CALENDAR);
        connect(m_calendarSettings, &QGSettings::changed, this, [=] (const QString &key){
            if(key == "hoursystem") {
                sysDateStyleGet();
            }
        });
        QDBusConnection::sessionBus().connect("",
                                              "/com/kylin/kysdk/Date",
                                              "com.kylin.kysdk.DateInterface",
                                              "ShortDateSignal",
                                              this,
                                              SIGNAL(getDateSignal(QString)));
        connect(this, &GsettingsMonitor::getDateSignal, this, [=](QString date){
            sysDateStyleGet();
        });

        sysDateStyleGet();
    }

    return ;
}

QString getSystemTimeFormat()
{
    QString format;
    char *res = kdk_system_get_shortformat();
    if (!res) {
        qDebug() << "kdk_system_get_shortformat failed!";
        format = "yyyy/MM/dd";
    }
    else {
        format = QString(res);
        free(res);
    }
    return format;
}


QString GsettingsMonitor::sysDateStyleGet()
{
    if(!QGSettings::isSchemaInstalled(PANEL_CONTROL_IN_CALENDAR)) {
        return m_timeStyle;
    }

    QString tmpDate = getSystemTimeFormat() + " dddd";

    QLocale locale = (QLocale::system().name() == "zh_CN" ? (QLocale::Chinese) : (QLocale::English));
    QString tmpTime;
    QString timemodel = m_calendarSettings->get("hoursystem").toString();
    if(timemodel == "12") {
        if(locale.language() == QLocale::English){
            tmpTime = "hh:mm:ss A";
        } else {
            tmpTime = "A hh:mm:ss";
        }
    } else {
        tmpTime = "hh:mm:ss";
    }

    QString tmpTimeStyle = tmpDate + " " + tmpTime;
    if (tmpTimeStyle != m_timeStyle) {
        m_timeStyle = tmpTimeStyle;
        sysDateStyleUpdate(m_timeStyle);
    }
    return m_timeStyle;
}

QString GsettingsMonitor::sysDateStyleGet(time_t time)
{
    if (m_timeStyle.isEmpty()) {
        return  QDateTime::fromTime_t(time).toString();
    }
    return QDateTime::fromTime_t(time).toString(m_timeStyle);
}

QFont GsettingsMonitor::getFontSize()
{
    //启动时设置字号
    double fontSizeKey = SYSTEM_DEFAULT_FONT_SIZE;
    QString fontStyle = m_ukuiStyle->get(UKUI_FONT).toString();
    if (m_ukuiStyle != nullptr) {
        double fontSizeKeyTmp = m_ukuiStyle->get(UKUI_FONT_SIZE).toString().toDouble();

        if (fontSizeKeyTmp > 0) {
            fontSizeKey = fontSizeKeyTmp;
        }
    }
    if(fontSizeKey >= 15.0)
    {
        fontSizeKey = 15.0;
    }

    double baseFontSize = SYSTEM_DEFAULT_FONT_SIZE_PIXELS;
    double nowFontSize = baseFontSize * double(fontSizeKey) / SYSTEM_DEFAULT_FONT_SIZE;
    qDebug() << nowFontSize;

    QFont font;

    font.setFamily(fontStyle);

    emit sysFontSizeUpdate(font);
    return font;
}

int GsettingsMonitor::sysTransparencyGet()
{
    if(!QGSettings::isSchemaInstalled(UKUI_STYLE)) {
        return m_menuTrans;
        }

    int tmpMenuTrans = m_ukuiStyle->get(UKUI_MENU_TRANSPARENCY).toInt();
    if (tmpMenuTrans != m_menuTrans) {
        m_menuTrans = tmpMenuTrans;
        sysTransparencyUpdate(m_menuTrans);
    }

    return m_menuTrans;
}

QString GsettingsMonitor::sysStyleNameGet()
{
    if(!QGSettings::isSchemaInstalled(UKUI_STYLE)) {
        return m_styleName;
    }

    QString tmpStyleName = m_ukuiStyle->get(UKUI_STYLE_NAME).toString();
    if (tmpStyleName != m_styleName) {
        m_styleName = tmpStyleName;
        sysStyleNameUpdate(m_styleName);
    }

    return m_styleName;
}

QString GsettingsMonitor::apiServerAddressGet()
{
    // if(!QGSettings::isSchemaInstalled(KYLIN_PRINTER_SETTINGS)) {
    //     return m_apiServerAddress;
    // }

    // QString tmpApiServerAddress = "";
    // QStringList keys = m_kylinPrinterSettings->keys();
    // for (auto key : keys)
    // {
    //     if (KYLIN_PRINTER_IP == key)
    //     {
    //         tmpApiServerAddress = m_kylinPrinterSettings->get(KYLIN_PRINTER_IP).toString();
    //         break;
    //     }
    //     else
    //     {
    //         continue;
    //     }
    // }

    // if (tmpApiServerAddress.isEmpty())
    // {
    //     tmpApiServerAddress = "api.kylinos.cn";
    // }
    // else
    // {
    //     if (tmpApiServerAddress != m_apiServerAddress)
    //     {
    //         m_apiServerAddress = tmpApiServerAddress;
    //         apiServerAddressUpdate(m_apiServerAddress);
    //     }
    //     else
    //     {
    //     }
    // }

    // return m_apiServerAddress;

    kom::Configure conf;
    QString customize = conf.value(CONFIG_PLUGIN_GLOBAL, CONFIG_PLUGIN_GLOBAL_CUSTOMIZE).toString();
    if (customize ==  "icbc") {
        return "kysoftshopclient.kylin.icbc:30234";
    } else {
        return "api.kylinos.cn";
    }
}

QByteArray GsettingsMonitor::printerDevicesGet()
{
    if(!QGSettings::isSchemaInstalled(KYLIN_PRINTER_SETTINGS)) {
        return m_devices;
    }

    QByteArray tmpDevices = m_kylinPrinterSettings->get(KYLIN_PRINTER_PRINTERS).toByteArray();
    if (tmpDevices != m_devices) {
        m_devices = tmpDevices;
        printerDevicesUpdate(m_devices);
    }

    return m_devices;
}

QStringList GsettingsMonitor::showProtocolsGet()
{
    if(!QGSettings::isSchemaInstalled(KYLIN_PRINTER_SETTINGS)) {
        return m_showProtocols;
    }

    QStringList tmpShowProtocols = m_kylinPrinterSettings->get(KYLIN_PRINTER_SHOW_PROTOCOLS).toStringList();
    if (tmpShowProtocols != m_showProtocols) {
        m_showProtocols = tmpShowProtocols;
        showProtocolsUpdate(m_showProtocols);
    }

    return m_showProtocols;
}

void GsettingsMonitor::printerDevicesSet(const QByteArray& devices)
{
    if(!QGSettings::isSchemaInstalled(KYLIN_PRINTER_SETTINGS)) {
        return ;
    }
    m_kylinPrinterSettings->set(KYLIN_PRINTER_PRINTERS, devices);
    return ;
}

void GsettingsMonitor::apiServerAddressSet(const QString &address)
{
    if(!QGSettings::isSchemaInstalled(KYLIN_PRINTER_SETTINGS)) {
        return ;
    }
    m_kylinPrinterSettings->set(KYLIN_PRINTER_IP, address);
    return ;
}

void GsettingsMonitor::showProtocolsSet(const QStringList &showProtocols)
{
    if(!QGSettings::isSchemaInstalled(KYLIN_PRINTER_SETTINGS)) {
        return ;
    }
    m_kylinPrinterSettings->set(KYLIN_PRINTER_SHOW_PROTOCOLS, showProtocols);
    return ;
}
