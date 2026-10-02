#include "test.h"

Test::Test(/* args */)
{

}

Test::~Test()
{
    
}

void Test::show(DeviceInformation &cc)
{
    // qDebug () << "show " << QThread::currentThread();
    qDebug () <<"show "<< cc;
}