#ifndef GRAPHICDRIVERFINDER_H
#define GRAPHICDRIVERFINDER_H

#include <QObject>
#include <QList>
#include <QThread>
#include "../driverinfo/driverinfo.h"
#include "../hardwareinfo/graphiccardinfo.h"

class graphicDriverFinder : public QObject
{
    Q_OBJECT
public:
    graphicDriverFinder(QObject *parent = nullptr);
    ~graphicDriverFinder();

    QList<GraphicCardInfo> getHardWareInfo();
    QThread *m_thread = nullptr;

    void startWorker(){
        if(m_thread!=nullptr)
            m_thread->start();
    }

Q_SIGNALS:
    void finished();
    void succeed();
    void failed();

private:
    QList<DriverInfo> m_drivers;
    QList<GraphicCardInfo> m_hardWareInfo;

    void dowork();
    void convertStrToDrinfo(QStringList drivers);
    void driverMatch(QList<GraphicCardInfo> &cardsInfo, const QList<DriverInfo> &driversInfo);
};

bool fnmatch(const QString &pattern, const QString &string);


#endif // GRAPHICDRIVERFINDER_H
