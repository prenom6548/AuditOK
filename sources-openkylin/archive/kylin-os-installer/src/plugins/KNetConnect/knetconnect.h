#ifndef KNETCONNECT_H
#define KNETCONNECT_H

#include "knetconnect_global.h"
#include "../Reflex/reflex.h"
#include "plugindll.h"
#include "netframe.h"

class KNETCONNECT_EXPORT KNetConnect : public IPlugin
{
public:
    KNetConnect();
private:
   QWidget* m_KNetConnect = nullptr;//代表本插件界面
   QWidget* CreateObject(QWidget *parent);
   QWidget* getWidget();
   QString getWidgetName();

   CONSTRUCTOR(KNetConnect)//新增数据结构，用于主框架自动加载插件
};
REGISTER_REFLEX(KNetConnect)//初始化静态成员对象，用于主框架自动加载插件

#endif // KNETCONNECT_H
