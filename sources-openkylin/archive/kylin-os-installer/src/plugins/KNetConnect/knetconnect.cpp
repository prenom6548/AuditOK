#include "knetconnect.h"
#include "netframe.h"

KNetConnect::KNetConnect()
{
}

QWidget* KNetConnect::CreateObject(QWidget *parent)//新建本插件界面
{
    m_KNetConnect = new NetFrame(parent);
    return m_KNetConnect;
}

QWidget* KNetConnect::getWidget()//获取本插件界面对象
{
      return m_KNetConnect;
}

QString KNetConnect::getWidgetName()//预留接口，暂未用到
{
    QString s = QString("");
    return (QString)s;
}
