#ifndef POP_WINDOW_MANAGER_H
#define POP_WINDOW_MANAGER_H

#include <QObject>
#include "device_information.h"
#include <QQueue>


class PopWindowManager : public QObject
{
    Q_OBJECT
private:
    QQueue<DeviceInformation> m_infoList;
    QMap<QString, DeviceInformation> m_devMp;
    explicit PopWindowManager();
    ~PopWindowManager();
    void showPopWindow(const DeviceInformation &info);
    void getDevInfo(QString);
    void modifyInfoList(QString);
    void showNext();
    void addNewPrinterDevice(DeviceInformation devInfo);
    //安装状态，0：安装成功，1：安装失败，2：已经安装
    int m_installOk = -1;
    bool m_needInstall = false;
    bool m_pkgHelperFlag = false;
public:
    static PopWindowManager *getInstance();
signals:
    void installPrinter(QString);
    void manualInstallShowRequest(DeviceInformation &);
    void deviceViewShowRequest(QString &);
    void showNextRequest();
    void installEnd();
};




#endif // POP_WINDOW_MANAGER_H
