#include "core.h"

core::core(QString getnum, QWidget *parent)
    : QMainWindow(parent)
{
    updateHandle = new updatehandle(getnum,this);
    connect(updateHandle,&updatehandle::execSignal,this,&core::execSlots);
    this->setWindowFlags(Qt::SubWindow);

}

core::~core()
{

}

void core::execSlots()
{
    emit execSignal();
}


