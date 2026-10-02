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

    HwWidget(QString devType, QMap<QString, QString> devInfo, QWidget *parent = nullptr);

    HwWidget(QWidget *parent = nullptr);

    ~HwWidget();

    void initGui();

    void initConnect();

    QString getDevName();

    QString getDevType();

    QString getMonitorInterface();

    QString getDiskName();

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

    QStackedWidget *m_activeStackWidget = nullptr;
    QLabel *m_activateStatus = nullptr;
    QLabel *m_deactiveTips = nullptr;
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
    bool m_isWireless = true;  // 标识网卡类型
    QString m_monitorInterface;    // 显示器接口
    QString m_diskName;    // 显示器接口

private:
    QList<QMetaObject::Connection> m_connections;
    QList<QScreen *> m_screens;

    void setContent();

    void setContent(QString devType, QMap<QString, QString> devInfo);

    QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor);

    QIcon getIcon(QString iconName, QSize iconSize);

    QString getResolution(QString interface);

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

    void setBluetoothStatus();

    void setNetworkStatus();

private:
    QMap<QString, QString> m_networkCardPathMap;

};


#endif //HWWIDGET_H
