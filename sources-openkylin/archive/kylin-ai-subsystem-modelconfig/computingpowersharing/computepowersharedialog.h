#ifndef COMPUTEPOWERSHAREDIALOG_H
#define COMPUTEPOWERSHAREDIALOG_H

#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/kswitchbutton.h>

#include <QBoxLayout>
#include <QComboBox>
#include <QScrollArea>
#include <QVBoxLayout>

#include "../configmanager.h"
#include "computepowerinterface.h"
#include "requestcomputepowerlist.h"
#include "sharedcomputepowerlist.h"
#include "ukccframe.h"

class ComputePowerShareDialog : public kdk::KDialog {
    Q_OBJECT
public:
    explicit ComputePowerShareDialog(QWidget *parent = nullptr);

    enum RoleSelection { PleaseSelect, SharedComputingPower = 2, RequestComputingPower };

private:
    void initComponent();
    void initConnections();
    void initUIState();

    void showRequestComputePowerUI();
    void showRequestComputePowerOpenedUI();
    void setComponentsEnabled(bool enable);
    void disableRoleSelectionComboBox();
    void showSharedComputePowerUI();
    bool openSharedServiceAndSettingCode(const QString &code);
    void showSharedComputePowerOpenedUI();

    void initMainTitleLabel();
    void initEnableComputePowerShareWidget();
    void initRoleSelectionWidget();
    void initVerificationCodeWidget();
    void resetVerificationCode(const QString &code);
    bool confirmResetVerificationCode();
    void initServiceOpeningTipWidget();
    void showServiceOpeningTipWidget();
    void hideServiceOpeningTipWidget();

    void setSharedComputePowerVerificationCode(const QString &code);
    void setComboBoxIndex(RoleSelection roleSelection);
    void createTmpFile();
    bool isReboot();

    void initRequestComputePowerList();
    void initSharedComputePowerList();

    bool isSetVerificationCode();

    bool switchToPleaseSelect();
    bool switchToPleaseSelectFromRequest();
    bool switchToPleaseSelectFromShared();

    bool switchToSharedComputingPower();
    bool switchToSharedFromRequest(const QString &code);
    bool switchToUnopenedShareFromRequest(const QString &code);
    bool switchToOpenedSharedFromRequest(const QString &code);
    bool confirmSwitchRole();

    bool switchToRequestComputingPower();
    bool switchToRequestFromPleaseSelect();
    bool switchToRequestFromUnopenedShare();

    bool openRequestService();
    bool switchToRequestFromOpenedShare();

    void setLocalIpLabelText(const QString &localIp);

    void enableComputePowerSharing();
    void disableComputePowerSharing();

    bool closeRequestComputePowerService();
    bool confirmCloseRequestComputePowerService();
    bool closeSharedComputePowerService();
    bool confirmCloseSharedComputePowerService();
    bool closeService(ComputePowerList *computePowerList);

    bool showCloseServiceTipMessageBox(const QString &message);
    bool showResetServiceTipMessageBox(const QString &message);
    void showRebootTipMessageBox();
    void reboot();
    void showServiceOpeningTipMessageBox();

    void showVerificationCodeFrame();

    void resetToPleaseSelect();
    void setToggleEnableButtonState(bool enable);
    void checkLocalModelEnable();
    bool isExistLocalModel();

    void showOpenLocalModelTipMessageBox();
    void showNoLocalModelTipMessageBox();
    void showCannotOpenSharedServiceTipMessageBox();
    void showCannotOpenRequestServiceTipMessageBox();

    bool isEnableDomainControl();
    bool isEnableLocalModel();
    void openSoftwareStore();
    UkccFrame *createFrame();

signals:
    void localModelEnableStateChanged(bool enabled);
    void requestConnectionToggled(bool connected);

public slots:
    void onToggleEnableButtonStatusChanged(bool isEnable);
    void onComboBoxCurrentIndexChanged(int currentIndex);

private:
    QBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_mainVLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QVBoxLayout *m_roleMainVLayout = nullptr;
    QVBoxLayout *m_computePowerListVLayout = nullptr;

    kdk::KLabel *m_mainTitleLabel = nullptr;
    UkccFrame *m_computePowerSharingFrame = nullptr;
    kdk::KLabel *m_enableTitleLabel = nullptr;
    kdk::KLabel *m_detailLabel = nullptr;
    kdk::KSwitchButton *m_toggleEnableButton = nullptr;

    UkccFrame *m_roleSelectionFrame = nullptr;
    kdk::KLabel *m_roleSelectionLabel = nullptr;
    kdk::KLabel *m_localIpLabel = nullptr;
    QHBoxLayout *m_ipHLayout = nullptr;
    kdk::KLabel *m_localIpValueLabel = nullptr;
    QPushButton *m_tipButton = nullptr;
    QComboBox *m_roleSelectionComboBox = nullptr;

    UkccFrame *m_verificationCodeFrame = nullptr;
    kdk::KLabel *m_setVerificationCodeLabel = nullptr;
    kdk::KLabel *m_verificationCodeDetailLabel = nullptr;
    kdk::KPasswordEdit *m_verificationCodePassword = nullptr;
    QString m_verificationCodeStr;

    QWidget *m_serviceOpengingWidget = nullptr;
    kdk::KLabel *m_serviceOpeningLabel = nullptr;
    QLabel *m_serviceOpenLoadingLabel = nullptr;
    QTimer *m_serviceOpenLoadingTimer = nullptr;
    int m_loadingImageIndex = 0;

    QPushButton *dialogCloseBtn = nullptr;

    RequestComputePowerList *m_requestComputePowerList = nullptr;
    SharedComputePowerList *m_sharedComputePowerList = nullptr;

    ConfigManager m_configManager;
    ComputePowerInterface m_computePowerInterface;

    RoleSelection m_curRole;
    bool m_enableRequestComputePower = false;
    bool m_enableSharedComputePower = false;

    const QString m_pleaseSelectKey = "";
    const QString m_sharedComputingPowerKey = "share";
    const QString m_requestComputingPowerKey = "request";
    const QString m_tmpFilePath = "/tmp/kylin-ai-subsystem-modelconfig";
    QMap<RoleSelection, QString> roleKeyMap = {{RoleSelection::PleaseSelect, m_pleaseSelectKey},
                                               {RoleSelection::RequestComputingPower, m_requestComputingPowerKey},
                                               {RoleSelection::SharedComputingPower, m_sharedComputingPowerKey}};
};

#endif  // COMPUTEPOWERSHAREDIALOG_H
