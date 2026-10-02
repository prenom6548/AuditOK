#ifndef MONITORCDROM_H
#define MONITORCDROM_H

#include <QObject>
#include <QThread>
#include <QMutex>

class MonitorCDROM : public QThread
{
    Q_OBJECT
Q_SIGNALS:
    // cdrom
    void addCDROM(QString);
    void changeCDROM(QString);
    void removeCDROM(QString);
public:
    static MonitorCDROM *instance();
public:
    MonitorCDROM(const MonitorCDROM*)=delete;
    MonitorCDROM *operator =(const MonitorCDROM*)=delete;
public:
    void stop();
protected:
    void run() Q_DECL_OVERRIDE;
private:
    MonitorCDROM(QObject *parent = nullptr);
    ~MonitorCDROM();
private:
    int preception();
    void update();
    bool isCDROM(QString);
private:
    static MonitorCDROM *m_oInstance;
    static QMutex        m_oMutex;
private:
    bool                 m_bAppRunning;
    int                  m_iPreception;
    QStringList          m_oBlocks;
};
#define MONITOR MonitorCDROM::instance()
#endif // MONITORCDROM_H
