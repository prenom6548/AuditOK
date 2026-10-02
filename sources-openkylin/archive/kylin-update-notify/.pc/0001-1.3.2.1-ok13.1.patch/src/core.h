#ifndef CORE_H
#define CORE_H

#include <QMainWindow>
#include "updatehandle.h"

class core : public QMainWindow
{
    Q_OBJECT

public:
    core(QString getnum, QWidget *parent = nullptr);
    ~core();
    updatehandle *updateHandle;

signals:
    void execSignal();

protected slots:
    void execSlots();

};
#endif // CORE_H
