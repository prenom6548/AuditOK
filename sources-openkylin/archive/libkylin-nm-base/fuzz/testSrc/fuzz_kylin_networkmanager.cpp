#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "kylinnetworkmanager.h"



extern "C" int LLVMFuzzerTestOneInput_KyNetworkManager(const uint8_t *data, size_t size) {

    if (size < 4) {
        return 0;
    }
    KyNetworkManager manager;
    

    // 尝试初始化（注意：实际可能需要DBus等，但模糊测试可以测试接口）
     manager.kylinNetworkManagerInit();
    
      sleep(2);

    // 获取输入参数
    QString uuid;
   //QString uuid = QString::fromLatin1(reinterpret_cast<const char*>(data), std::min(static_cast<size_t>(64), size));
   QString deviceName = QString::fromLatin1(reinterpret_cast<const char*>(data), std::min(static_cast<size_t>(36), size));
    
    //测试各种枚举类型
    KyDeviceType devType = KyDeviceType(data[1] % 3);
    uuid=data[2]? "TESTUUID110-f330-42f7-b808-86753bcb0640":"";

    
    // 测试获取方法
   QStringList devList;
   QList<KyActivateItem> activeList;
   //QString deviceName="UNKOWN_DEVICE";
    KyConnectSetting connSetting;

    //qDebug() << "mqtest" <<connType<<devType<<uuid<<deviceName;

    manager.getNetworkDeviceList(devType, devList);
    
   	if (!devList.isEmpty())
	{
        deviceName=devList.at(0);
        manager.getActiveConnectionList(deviceName, CONNECT_TYPE_WIRED, activeList);
        if (!activeList.isEmpty()){
            manager.getConnectIpInfo(activeList.at(0).m_uuid, connSetting);
        }
        manager.getDeviceManaged(deviceName);
    
        manager.getDeviceState(deviceName);
        
    }else{

        manager.getActiveConnectionList(deviceName, CONNECT_TYPE_WIRED, activeList);
        manager.getConnectIpInfo(uuid, connSetting);
    }
    

    manager.getWiredDeviceIsCarriered(deviceName);
    
    manager.getDeviceManaged(deviceName);
    
    manager.getDeviceState(deviceName);


     
    return 0;
}
