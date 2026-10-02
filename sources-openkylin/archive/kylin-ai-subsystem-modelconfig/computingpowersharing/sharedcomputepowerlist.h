#ifndef SHAREDCOMPUTEPOWERLIST_H
#define SHAREDCOMPUTEPOWERLIST_H

#include <kysdk/applications/kborderlessbutton.h>
#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/kpasswordedit.h>

#include <QWidget>

#include "computepowerlist.h"
#include "ipitemwidget.h"

class SharedComputePowerList : public ComputePowerList {
    Q_OBJECT
public:
    explicit SharedComputePowerList(QWidget *parent = nullptr);
    ~SharedComputePowerList();

    bool enableService() override;
    bool disableService() override;
    bool restartInferenceService() override;

    bool openService() override;
    bool closeService() override;
    bool resetService() override;
    void getIpList() override;
    void clearIpList() override;
    void updateIpList() override;

signals:

private:
    void initConnections();
    void updateLoadingImage();
    void autoRefreshList();
    void updateIpListUi(const QList<QString> &newConnectedIpList, const QList<QString> &newDisconnectedIpList);
    void updateIpWidget(IpItemWidget *ipWidget, const QString &newIp, bool connectionState);
    void initComputingPowerFrame() override;

    UkccFrame *createIpWidget(const QString &ipString, bool isConnected);
    void setIpWidgetSharedModelName(IpItemWidget *ipWidget, const QString &modelName);
    QString getSharedModelName();
    void showVerificationDialog(const QString &newIp, const QString &oldIp = "");
    void initVerificationDialog(const QString &newIp, const QString &oldIp = "");
    void showSwitchConnectionMessageBox(const QString &currentIp, const QString &newIp);
    void updateListCompleted();
    void serviceCloseCompleted();

private:
    UkccFrame *m_computingPowerFrame = nullptr;
    kdk::KBorderlessButton *m_refreshBtn = nullptr;
    QLabel *m_loadingLabel = nullptr;
    QTimer *m_loadingTimer = nullptr;
    QTimer *m_updateListTimer = nullptr;

    kdk::KDialog *m_verificationDialog = nullptr;
    kdk::KPasswordEdit *m_verificationPassword = nullptr;
    QPushButton *m_confirmBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;
    kdk::KLabel *m_errorLabel = nullptr;

    QList<QString> m_connectedProviderList;
    QList<QString> m_disconnectedProviderList;
    int m_loadingIndex = 0;
    bool m_isRefreshing = false;
};

#endif  // SHAREDCOMPUTEPOWERLIST_H
