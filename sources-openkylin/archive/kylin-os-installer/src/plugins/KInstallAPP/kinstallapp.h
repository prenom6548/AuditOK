#ifndef KINSTALLAPP_H
#define KINSTALLAPP_H

#include <QtCore/qglobal.h>
#include "../Reflex/reflex.h"
#include "plugindll.h"
#include "appcheckframe.h"


namespace KInstaller {
class Q_DECL_EXPORT KInstallApp : public IPlugin
{
    Q_OBJECT
public:
    KInstallApp(){this->setObjectName("KInstallApp");}
    ~KInstallApp(){}

    QWidget* CreateObject(QWidget *parent){
        m_frame=new appcheckframe(parent);
        return m_frame;
    }
    QWidget* getWidget(){return m_frame;}
    QString getWidgetName(){return m_frame->objectName();}
    QWidget* m_frame = nullptr;
    CONSTRUCTOR(KInstallApp)

};
    REGISTER_REFLEX(KInstallApp)
}
#endif // GENERICPLUGIN_H
