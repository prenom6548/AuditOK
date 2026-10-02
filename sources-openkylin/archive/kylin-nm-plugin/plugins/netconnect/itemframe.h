#ifndef ITEMFRAME_H
#define ITEMFRAME_H
#include <QFrame>
#include <QVBoxLayout>

#include "lanitem.h"

#include "../component/DeviceFrame/deviceframe.h"
#include "../component/Divider/divider.h"
#include <kylin-nm/depend/kynetloadratethread.h>

class ItemFrame : public QFrame
{
    Q_OBJECT
public:
    ItemFrame(QString devName, QWidget *parent = nullptr);
    ~ItemFrame();
    //单设备整体layout
    QVBoxLayout * deviceLanLayout = nullptr;
    Divider * m_divider;
    //单设备名称 单设备开关
    DeviceFrame * deviceFrame = nullptr;
    //单设备列表Frame
    QFrame * lanItemFrame = nullptr;
    //单设备列表layout
    QVBoxLayout * lanItemLayout = nullptr;
    //单设备item列表 key:uuid
    QMap<QString, LanItem *> itemMap;
    //获取当前网速子线程
    KyNetLoadRateThread *m_updateNetSpeedThread;

    QString m_deviceName;

    //更新网速显示
    void startNetSpeedThread();
    void stopNetSpeedThread();
    void updateNetSpeed();

private:
    void setNetSpeed(long int rcvRate, long int txRate, LanItem *lanItem);
};

#endif // ITEMFRAME_H
