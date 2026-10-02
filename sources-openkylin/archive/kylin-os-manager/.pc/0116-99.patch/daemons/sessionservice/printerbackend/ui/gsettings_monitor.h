#ifndef GSETTINGS_MONITOR_H
#define GSETTINGS_MONITOR_H

#include <QObject>
#include <QGSettings>
#include <QMutex>
#include <QFont>


class GsettingsMonitor : public QObject
{
    Q_OBJECT
private:
    const char *UKUI_STYLE = "org.ukui.style";
    const char *UKUI_FONT_SIZE = "systemFontSize";
    const char *UKUI_FONT = "systemFont";
    const char *UKUI_MENU_TRANSPARENCY = "menuTransparency";
    const char *UKUI_STYLE_NAME = "styleName";

    const char *KYLIN_PRINTER_SETTINGS = "kylin.os.manager.printer.settings";
    const char *KYLIN_PRINTER_IP = "ip";
    const char *KYLIN_PRINTER_PRINTERS = "printers";
    const char *KYLIN_PRINTER_SHOW_PROTOCOLS = "show-protocols";

    const char *PANEL_CONTROL_IN_CALENDAR = "org.ukui.control-center.panel.plugins";

public:
    static GsettingsMonitor* getInstance();
    bool gsettingsCheck();
    QFont getFontSize();
    int sysTransparencyGet();
    QString sysStyleNameGet();
    QString sysDateStyleGet();
    QString sysDateStyleGet(time_t time);
    QString apiServerAddressGet();
    QByteArray printerDevicesGet();
    QStringList showProtocolsGet();

    void apiServerAddressSet(const QString &address);
    void printerDevicesSet(const QByteArray &devices);
    void showProtocolsSet(const QStringList &showProtocols);

signals:
    QFont sysFontSizeUpdate(QFont currentFont);
    void sysTransparencyUpdate(int transparency);
    void sysStyleNameUpdate(const QString &styleName);
    void sysDateStyleUpdate(const QString &dateStyle);
    void printerDevicesUpdate(const QByteArray &devices);
    void apiServerAddressUpdate(const QString &host);
    void showProtocolsUpdate(const QStringList &showProtocols);
    void getDateSignal(QString);

public slots:
    QFont update();
private:
    void initGsettings();


private:
    GsettingsMonitor(/* args */);
    GsettingsMonitor(GsettingsMonitor const&) = delete;
    GsettingsMonitor& operator=(GsettingsMonitor const&) = delete;
    ~GsettingsMonitor();

    static QMutex m_mutex;

    QGSettings *m_ukuiStyle;
    QGSettings *m_kylinPrinterSettings;
    QGSettings *m_calendarSettings;

    QString m_timeStyle = QString();
    int m_menuTrans = -1;
    QString m_styleName = QString();
    QByteArray m_devices = QByteArray();
    QString m_apiServerAddress = "api.kylinos.cn";
    QStringList m_showProtocols{QStringList()};

};

#endif // GSETTINGS_MONITOR_H
