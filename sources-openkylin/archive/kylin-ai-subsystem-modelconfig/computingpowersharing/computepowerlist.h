#ifndef COMPUTEPOWERLIST_H
#define COMPUTEPOWERLIST_H

#include <aiconfig/modelconfig.h>
#include <ukui-log4qt.h>

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "computepowerinterface.h"
#include "ukccframe.h"

class ComputePowerList : public QWidget {
    Q_OBJECT
public:
    explicit ComputePowerList(QWidget *parent = nullptr);
    virtual ~ComputePowerList() = default;

    virtual bool enableService() = 0;
    virtual bool disableService() = 0;

    virtual void initComputingPowerFrame() = 0;
    virtual bool openService() = 0;
    virtual bool closeService() = 0;
    virtual bool resetService() = 0;
    virtual bool restartInferenceService() = 0;

    virtual void getIpList() = 0;
    virtual void clearIpList() = 0;
    virtual void updateIpList() = 0;

    QString getLocalIp();

protected:
    void initConnections();

    void showNoDeviceAccessFrame();
    void hideNoDeviceAccessFrame();
    UkccFrame *createFrame();

    void clearIpLayout();
    void showErrorMessageBox(const QString &errorStr);
    void showErrorMessageBoxByErrorCode(ComputePowerShare::ErrorCode errorCode);

protected:
    int updateTimer = 60000;

private:
    void initMainLayout();
    void initIpLayout();
    void initNoDeviceAccessFrame();

signals:
    void requestConnectionToggled(bool connected);

protected:
    QVBoxLayout *m_mainVLayout = nullptr;
    QVBoxLayout *m_ipVLayout = nullptr;
    ComputePowerInterface m_computePowerInterface;
    QString m_localIp;
    kyai::config::model::ModelConfig mc;

private:
    UkccFrame *m_noDeviceAccessFrame = nullptr;

    UkccFrame *m_serviceStartingTipFrame = nullptr;
    QLabel *m_serviceStartingLoadingLabel = nullptr;
    QTimer *m_serviceStartingLoadingTimer = nullptr;
    int m_loadingImageIndex = 0;
};

#endif  // COMPUTEPOWERLIST_H
