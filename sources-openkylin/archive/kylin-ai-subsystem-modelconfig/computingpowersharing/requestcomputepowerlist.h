#ifndef REQUESTCOMPUTEPOWERLIST_H
#define REQUESTCOMPUTEPOWERLIST_H

#include <kysdk/applications/kborderlessbutton.h>

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "computepowerlist.h"
#include "ipitemwidget.h"
#include "ukccframe.h"

class RequestComputePowerList : public ComputePowerList {
    Q_OBJECT
public:
    explicit RequestComputePowerList(QWidget *parent = nullptr);
    ~RequestComputePowerList();

    bool enableService() override;
    bool disableService() override;
    bool restartInferenceService() override;

    bool openService() override;
    bool closeService() override;
    bool resetService() override;
    void getIpList() override;
    void clearIpList() override;
    void updateIpList() override;

    bool setVerificationCode(const QString &code);
    QString getVerificationCode();

private:
    void initConnections();
    void autoRefreshList();
    void updateIpListUi(const QList<QString> &newIpList);
    void updateIpWidget(IpItemWidget *ipWidget, const QString &newIp);
    void updateLoadingImage();
    void initComputingPowerFrame() override;

    UkccFrame *createIpWidget(const QString &ipString);
    void updateListCompleted();
    void serviceCloseCompleted();

private:
    UkccFrame *m_computingPowerFrame = nullptr;
    kdk::KBorderlessButton *m_refreshBtn = nullptr;
    QLabel *m_loadingLabel = nullptr;
    QTimer *m_loadingTimer = nullptr;
    QTimer *m_updateListTimer = nullptr;

    QList<QString> m_consumerList;
    int m_loadingIndex = 0;
    bool m_isRefreshing = false;
};

#endif  // REQUESTCOMPUTEPOWERLIST_H
