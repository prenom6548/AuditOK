#ifndef HWWIDGET_H
#define HWWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "include/text_label.h"
#include "include/customlabel.h"
#include <QTreeWidget>
#include <QHeaderView>
#include <QStackedWidget>
#include <kborderlessbutton.h>
#include <QPainter>
#include <QTreeWidget>
#include <QHeaderView>
#include <QStackedWidget>
#include <QPainter>
#include <QTimer>
#include <QMap>
#include <QObject>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QClipboard>
#include <QApplication>
#include <QProcess>
#include <QGSettings>

#include "treeitem.h"
#include "hardwareinfo/deviceinfo.h"
#include "hardwareinfo/cpuinfo.h"
#include "hardwareinfo/memoryinfo.h"
#include "hardwareinfo/graphiccardinfo.h"
#include "hardwareinfo/diskinfo.h"
#include "hardwareinfo/baseboardinfo.h"
#include "hardwareinfo/monitorinfo.h"
#include "hardwareinfo/keyboardinfo.h"
#include "hardwareinfo/mouseinfo.h"
#include "hardwareinfo/cdinfo.h"
#include "hardwareinfo/camerainfo.h"
#include "hardwareinfo/faninfo.h"
#include "hardwareinfo/batteryinfo.h"
#include "hardwareinfo/networkcardinfo.h"
#include "hardwareinfo/soundcardinfo.h"
#include "hardwareinfo/bluetoothinfo.h"
#include "include/text_label.h"

class HwWidget : public QWidget {
Q_OBJECT

public:
    HwWidget(DeviceInfo *devinfo, QWidget *parent = nullptr);

    HwWidget(QWidget *parent = nullptr);

    ~HwWidget();

    void initGui();

    void initConnect();

    QString getDevName();

    QString getDevType();

    void actionExportSlot();

    void spreadIn();

    void setDisableStatus(bool status);

public:
    DeviceInfo *m_devInfo = nullptr;
    int itemCount = 0;    // 统计Item数量
    QPixmap m_pixMap;
    QLabel *m_iconLabel = nullptr;    // 图标
    QLabel *m_textLabel = nullptr;    // 设备类型
    K::TextLabel *m_modelLabel = nullptr;   // 设备型号
    QLabel *m_activateStatus = nullptr;
    QStackedWidget *m_btnStackedWidget = nullptr;
    kdk::KBorderlessButton *m_spreadOutBtn = nullptr;
    kdk::KBorderlessButton *m_spreadInBtn = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QTimer *m_timer = nullptr;

    QGSettings *m_data = nullptr;

    QList<TreeItem *> m_treeItemList;
    QList<QTreeWidgetItem *> m_treeWidgetItemList;

    QMenu *m_menu = nullptr;
    QAction *m_actionExport = nullptr;
    QAction *m_actionActive = nullptr;
    QAction *m_actionDeactive = nullptr;

    QStringList m_explorContent;
    QClipboard *m_clipboard = nullptr;
    QDBusInterface *m_interface = nullptr;

    QString m_bluetoothAddress;    // bluetooth的唯一标识

private:
    QList<QMetaObject::Connection> m_connections;
    QList<QScreen *> m_screens;

    void setContent();

    QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor);

protected:
    void mousePressEvent(QMouseEvent *mouseEvent) override;

private slots:

    void setIconColor();

    void rightClickedMenuRequest();

    void actionActiveBluetoothSlot();

    void actionDeactiveBluetoothSlot();

    void actionActiveSoundCardSlot();

    void actionDeactiveSoundCardSlot();

    void actionActiveNetworkCardSlot();

    void actionDeactiveNetworkCardSlot();

private:
    QMap<QString, QString> m_networkCardPathMap;

    QStringList m_cpuInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Cores"),
            QObject::tr("Online Cores"),
            QObject::tr("Thread Per Cores"),
            QObject::tr("MAX MHz"),
            QObject::tr("L1d Cache"),
            QObject::tr("L1i Cache"),
            QObject::tr("L2 Cache"),
            QObject::tr("L3 Cache"),
            QObject::tr("CPU Current Frequency"),
            QObject::tr("CPU Usage Rate"),
            QObject::tr("CPU Temperature")
    };
    QMap<QString, QString> m_cpuMap = {{"1",                                  "model"},
                                       {QObject::tr("Manufacturer"),          "vendor"},
                                       {QObject::tr("Cores"),                 "cores"},
                                       {QObject::tr("Online Cores"),          "onlinecores"},
                                       {QObject::tr("Thread Per Cores"),      "threadprecore"},
                                       {QObject::tr("MAX MHz"),               "maxMHz"},
                                       {QObject::tr("L1d Cache"),             "L1dcache"},
                                       {QObject::tr("L1i Cache"),             "L1icache"},
                                       {QObject::tr("L2 Cache"),              "L2cache"},
                                       {QObject::tr("L3 Cache"),              "L3cache"},
                                       {QObject::tr("CPU Current Frequency"), "Frequency"},
                                       {QObject::tr("CPU Usage Rate"),        "UsageRate"},
                                       {QObject::tr("CPU Temperature"),       "Temprature"}};

    QStringList m_memoryInfo = {
            "1",
            QObject::tr("Bank Locator"),
            QObject::tr("Total Width"),
            QObject::tr("Size"),
            QObject::tr("Serial Number"),
            QObject::tr("Manufacturer"),
            QObject::tr("Data Width"),
            QObject::tr("Type"),
            QObject::tr("Speed"),
            QObject::tr("Part Number"),
            QObject::tr("Memory Usage")
    };
    QMap<QString, QString> m_memoryMap = {{"1",                          "displayname"},
                                          {QObject::tr("Bank Locator"),  "banklocator"},
                                          {QObject::tr("Total Width"),   "totalwidth"},
                                          {QObject::tr("Size"),          "size"},
                                          {QObject::tr("Serial Number"), "serial"},
                                          {QObject::tr("Manufacturer"),  "verdor"},
                                          {QObject::tr("Data Width"),    "datawidth"},
                                          {QObject::tr("Type"),          "type"},
                                          {QObject::tr("Speed"),         "speed"},
                                          {QObject::tr("Part Number"),   "partnumber"},
                                          {QObject::tr("Memory Usage"),  "Memory Usage"}};

    QStringList m_graphicCardInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Sub System Device ID"),
            QObject::tr("Model"),
            QObject::tr("Memory"),
            QObject::tr("Bus Info"),
            QObject::tr("Clock"),
            QObject::tr("Physics ID"),
            QObject::tr("Version"),
            QObject::tr("Driver"),
            QObject::tr("Width"),
            QObject::tr("GDDR Size"),
            QObject::tr("EGL Version"),
            QObject::tr("EGL Api List"),
            QObject::tr("OpenGL Version"),
            QObject::tr("GLSL Version"),
    };
    QMap<QString, QString> m_graphicCardMap = {{"1",                                    "model"},
                                               {QObject::tr("Manufacturer"),         "vendor"},
                                               {QObject::tr("Sub System Device ID"), "subsystem"},
                                               {QObject::tr("Model"),                "model"},
                                               {QObject::tr("Memory"),               "memorysize"},
                                               {QObject::tr("Bus Info"),             "businfo"},
                                               {QObject::tr("Clock"),                "clock"},
                                               {QObject::tr("Physics ID"),           "physid"},
                                               {QObject::tr("Version"),              "version"},
                                               {QObject::tr("Driver"),               "driver_now"},
                                               {QObject::tr("Width"),                "width"},
                                               {QObject::tr("GDDR Size"),            "gddrsize"},
                                               {QObject::tr("EGL Version"),          "egl_version"},
                                               {QObject::tr("EGL Api List"),         "egl_apis"},
                                               {QObject::tr("OpenGL Version"),       "gl_version"},
                                               {QObject::tr("GLSL Version"),         "glsl_version"}};

    QStringList m_baseboardInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Version"),
            QObject::tr("Chassis Handle"),
            QObject::tr("Bios Vendor"),
            QObject::tr("Bios Version"),
            QObject::tr("Chipser"),
            QObject::tr("Serial Number"),
            QObject::tr("Publish Date")
    };
    QMap<QString, QString> m_baseboardMap = {{"1",                           "model"},
                                             {QObject::tr("Manufacturer"),   "vendor"},
                                             {QObject::tr("Version"),        "version"},
                                             {QObject::tr("Chassis Handle"), "chassishandle"},
                                             {QObject::tr("Bios Vendor"),    "biosvendor"},
                                             {QObject::tr("Bios Version"),   "biosversion"},
                                             {QObject::tr("Chipser"),        "chipset"},
                                             {QObject::tr("Serial Number"),  "serial_num"},
                                             {QObject::tr("Publish Date"),   "publish_date"}};

    QStringList m_networkCardInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Model"),
            QObject::tr("Bus"),
            QObject::tr("Driver"),
            QObject::tr("MAC Address"),
            QObject::tr("Type"),
            QObject::tr("IP Address"),
            QObject::tr("Net Musk"),
            QObject::tr("Send Bytes"),
            QObject::tr("Recved Bytes"),
            QObject::tr("MTU")
    };
    QMap<QString, QString> m_networkCardMap = {{"1",                         "name"},
                                               {QObject::tr("Manufacturer"), "vendor"},
                                               {QObject::tr("Model"),        "model"},
                                               {QObject::tr("Bus"),          "bus"},
                                               {QObject::tr("Driver"),       "driver_now"},
                                               {QObject::tr("MAC Address"),  "MAC_addr"},
                                               {QObject::tr("Type"),         "isWireless"},
                                               {QObject::tr("IP Address"),   "ip_addr"},
                                               {QObject::tr("Net Musk"),     "net_musk"},
                                               {QObject::tr("Send Bytes"),   "send_bytes"},
                                               {QObject::tr("Recved Bytes"), "recv_bytes"},
                                               {QObject::tr("MTU"),          "MTU"}
    };

    QStringList m_diskInfo = {
            "1",
            QObject::tr("Vendor"),
            QObject::tr("Model"),
            QObject::tr("Size"),
            QObject::tr("Serial Number"),
            QObject::tr("Disk Name"),
            QObject::tr("Frimwarever Version"),
            QObject::tr("Disk Type"),
            QObject::tr("Main Disk"),
            QObject::tr("Interface"),
            QObject::tr("UFS")
    };
    QMap<QString, QString> m_diskMap = {{"1",                                "name"},
                                        {QObject::tr("Model"),               "model"},
                                        {QObject::tr("Size"),                "size"},
                                        {QObject::tr("Serial Number"),       "serial"},
                                        {QObject::tr("Frimwarever Version"), "frimwareversion"},
                                        {QObject::tr("Disk Type"),           "type"},
                                        {QObject::tr("Vendor"),              "vendor"},
                                        {QObject::tr("Main Disk"),           "is_main"},
                                        {QObject::tr("Interface"),           "interface"},
                                        {QObject::tr("UFS"),                 "ufs"}
    };

    QStringList m_monitorInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Model"),
            QObject::tr("Monitor Size"),
            QObject::tr("Monitor Ratio"),
            QObject::tr("Resolution"),
            QObject::tr("Max Resolution"),
            QObject::tr("Main Monitor"),
            QObject::tr("Interface"),
            QObject::tr("Visible Area"),
            QObject::tr("Gamma")
    };
    QMap<QString, QString> m_monitorMap = {{"1",                           "name"},
                                           {QObject::tr("Manufacturer"),   "vendor"},
                                           {QObject::tr("Model"),          "model"},
                                           {QObject::tr("Monitor Size"),   "size"},
                                           {QObject::tr("Monitor Ratio"),  "ratio"},
                                           {QObject::tr("Resolution"),     "resolution"},
                                           {QObject::tr("Max Resolution"), "max_resolution"},
                                           {QObject::tr("Main Monitor"),   "is_main"},
                                           {QObject::tr("Interface"),      "interface"},
                                           {QObject::tr("Visible Area"),   "visible_area"},
                                           {QObject::tr("Gamma"),          "gamma"}};

    QStringList m_soundCardInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Bus Address"),
            QObject::tr("Clock"),
            QObject::tr("Driver"),
            QObject::tr("Width")
    };
    QMap<QString, QString> m_soundCardMap = {{"1",                         "model"},
                                             {QObject::tr("Manufacturer"), "vendor"},
                                             {QObject::tr("Bus Address"),  "bus"},
                                             {QObject::tr("Clock"),        "clock"},
                                             {QObject::tr("Driver"),       "driver_now"},
                                             {QObject::tr("Width"),        "width"}};

    QStringList m_keyboardInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Model"),
            QObject::tr("Interface"),
            QObject::tr("Driver")
    };
    QMap<QString, QString> m_keyboardMap = {{"1",                         "name"},
                                            {QObject::tr("Manufacturer"), "vendor"},
                                            {QObject::tr("Model"),        "model"},
                                            {QObject::tr("Interface"),    "interface"},
                                            {QObject::tr("Driver"),       "driver"}};

    QStringList m_mouseInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Model"),
            QObject::tr("Interface"),
            QObject::tr("Driver")
    };
    QMap<QString, QString> m_mouseMap = {{"1",                         "name"},
                                         {QObject::tr("Manufacturer"), "vendor"},
                                         {QObject::tr("Model"),        "model"},
                                         {QObject::tr("Interface"),    "interface"},
                                         {QObject::tr("Driver"),       "driver"}};

    QStringList m_bluetoothInfo = {"1",
                                   QObject::tr("Manufacturer"),
                                   QObject::tr("Driver"),
                                   QObject::tr("Driver Version"),
                                   QObject::tr("Address"),
                                   QObject::tr("Link Mode"),
                                   QObject::tr("Link Policy"),
                                   QObject::tr("SCO MTU"),
                                   QObject::tr("ACL MTU"),
                                   QObject::tr("Speed"),
                                   QObject::tr("Features"),
                                   QObject::tr("Packe Type")
    };
    QMap<QString, QString> m_bluetoothMap = {{"1",                           "model"},
                                             {QObject::tr("Manufacturer"),   "vendor"},
                                             {QObject::tr("Driver"),         "driver_now"},
                                             {QObject::tr("Driver Version"), "version"},
                                             {QObject::tr("Address"),        "address"},
                                             {QObject::tr("Link Mode"),      "link_mode"},
                                             {QObject::tr("Link Policy"),    "link_policy"},
                                             {QObject::tr("Packe Type"),     "packettype"},
                                             {QObject::tr("SCO MTU"),        "scomtu"},
                                             {QObject::tr("ACL MTU"),        "aclmtu"},
                                             {QObject::tr("Speed"),          "speed"},
                                             {QObject::tr("Features"),       "features"}};

    QStringList m_cdInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Name"),
            QObject::tr("Bus Info"),
            QObject::tr("Version"),
            QObject::tr("Driver"),
            QObject::tr("Device Number"),
            QObject::tr("Speed"),
            QObject::tr("Serial Number")
    };
    QMap<QString, QString> m_cdInfoMap = {{"1",                          "model"},
                                          {QObject::tr("Manufacturer"),  "vendor"},
                                          {QObject::tr("Name"),          "name"},
                                          {QObject::tr("Bus Info"),      "businfo"},
                                          {QObject::tr("Version"),       "version"},
                                          {QObject::tr("Driver"),        "driver"},
                                          {QObject::tr("Device Number"), "devicenum"},
                                          {QObject::tr("Speed"),         "speed"},
                                          {QObject::tr("Serial Number"), "serialnum"}};

    QStringList m_cameraInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Model"),
            QObject::tr("Interface"),
            QObject::tr("Driver"),
            QObject::tr("Version"),
            QObject::tr("Bus Info"),
            QObject::tr("Speed")
    };
    QMap<QString, QString> m_cameraInfoMap = {{"1",                         "name"},
                                              {QObject::tr("Manufacturer"), "vendor"},
                                              {QObject::tr("Model"),        "model"},
                                              {QObject::tr("Interface"),    "interface"},
                                              {QObject::tr("Driver"),       "driver"},
                                              {QObject::tr("Version"),      "version"},
                                              {QObject::tr("Bus Info"),     "businfo"},
                                              {QObject::tr("Speed"),        "speed"}};

    QStringList m_fanInfo = {
            "1",
            QObject::tr("Speed")
    };
    QMap<QString, QString> m_fanInfoMap = {{"1",                  "name"},
                                           {QObject::tr("Speed"), "speed"}};

    QStringList m_batteryInfo = {
            "1",
            QObject::tr("Manufacturer"),
            QObject::tr("Serial Number"),
            QObject::tr("State"),
            QObject::tr("Energy"),
            QObject::tr("Used Times"),
            QObject::tr("Energy Full"),
            QObject::tr("Time To Empty"),
            QObject::tr("Percentage")
    };
    QMap<QString, QString> m_batteryInfoMap = {{"1",                          "model"},
                                               {QObject::tr("Manufacturer"),  "vendor"},
                                               {QObject::tr("Serial Number"), "serailnum"},
                                               {QObject::tr("State"),         "state"},
                                               {QObject::tr("Energy"),        "energy"},
                                               {QObject::tr("Used Times"),    "used_times"},
                                               {QObject::tr("Energy Full"),   "energy_full"},
                                               {QObject::tr("Time To Empty"), "time_to_empty"},
                                               {QObject::tr("Percentage"),    "percentage"}};

};

#endif //HWWIDGET_H
