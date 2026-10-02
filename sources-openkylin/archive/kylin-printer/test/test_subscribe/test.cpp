#include "test.h"
#include <QDebug>
Test::Test(/* args */)
{

}

Test::~Test()
{
    
}

void Test::show(QStringList cc)
{
    // qDebug () << "show " << QThread::currentThread();
    qDebug () <<"show "<< cc;
}