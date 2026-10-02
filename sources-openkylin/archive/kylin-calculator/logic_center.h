#ifndef __LOGIC_CENTER__
#define __LOGIC_CENTER__

#include <QObject>

#include "mainwindow.h"
#include "data_warehouse.h"

class LogicCenter : public QObject
{
    Q_OBJECT

public:
    static LogicCenter *getInstance(void);
    ~LogicCenter();

private:
    MainWindow *m_mainWin;

    LogicCenter();
    void init(void);
    void establishInterrupt(void);

signals:

public slots:
    void slotTranparencyChange(void);
};

#endif
