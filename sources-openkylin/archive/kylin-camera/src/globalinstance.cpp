#include "globalinstance.h"


GlobalInstance* GlobalInstance::instance = new GlobalInstance;
GlobalInstance::GlobalInstance(QObject *parent) : QObject(parent)
{
    QString(qgetenv("XDG_SESSION_DESKTOP"));
}
GlobalInstance *GlobalInstance::getInstance()
{
    return instance;
}


GlobalInstance::~GlobalInstance()
{

}
