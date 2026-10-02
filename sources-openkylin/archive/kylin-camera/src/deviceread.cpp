#include "deviceread.h"
#include <QDebug>

DeviceRead::DeviceRead()
{
}

//等待完成设备文件的读取,更新设备列表
void DeviceRead::run(){
    //简单暂停一秒
    qDebug() << "deviceread.cpp:34";
    QThread::msleep(1000);
    Q_EMIT finish();
}

