#ifndef KCHOICEEDITION_H
#define KCHOICEEDITION_H

#include "KChoiceEdition_global.h"
#include "../Reflex/reflex.h"
#include "plugindll.h"
#include "choiseeditionui.h"

class KCHOICEEDITION_EXPORT KChoiceEdition : public IPlugin
{
public:
    KChoiceEdition();
private:
   QWidget* m_PluginInterface = nullptr;//代表本插件界面
   QWidget* CreateObject(QWidget *parent);
   QWidget* getWidget();
   QString getWidgetName();

   CONSTRUCTOR(KChoiceEdition)
};
REGISTER_REFLEX(KChoiceEdition)

#endif // KCHOICEEDITION_H
