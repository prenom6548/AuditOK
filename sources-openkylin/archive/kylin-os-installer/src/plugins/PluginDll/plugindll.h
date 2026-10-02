#ifndef PLUGINDLL_H
#define PLUGINDLL_H

#include "plugindll_global.h"

#include <QString>
#include "../Reflex/reflex.h"
#include <QtWidgets/QWidget>


class PLUGINDLLSHARED_EXPORT IPlugin : public ReflexBase
{
Q_OBJECT
public:
    IPlugin();
    virtual ~IPlugin();
    virtual QWidget* CreateObject(QWidget *parent) = 0;
    virtual QWidget* getWidget() = 0;
    virtual QString getWidgetName() = 0;
    virtual void next(){}
    virtual void back(){}
    virtual void onActive(){}
    virtual void onLeave() {}

signals:
    void signalPageChanged(int flag);


};

#endif // PLUGINDLL_H
