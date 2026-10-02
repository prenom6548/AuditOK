#ifndef COMPUTEPOWERSHAREWIDGET_H
#define COMPUTEPOWERSHAREWIDGET_H

#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "../configmanager.h"
#include "computepowerinterface.h"
#include "computepowersharedialog.h"

class ComputePowerShareWidget : public QWidget {
    Q_OBJECT
public:
    explicit ComputePowerShareWidget(QWidget *parent = nullptr);
    ~ComputePowerShareWidget();
    bool isSharedServiceEnabled();
    bool isRequestServiceEnabled();

    void closeSharedService();
    void closeRequestService();
    void openComputePowerShare();

signals:
    void localModelEnableStateChanged(bool enable);
    void requestConnectionToggled(bool connected);

private:
    void initComputePowerShareWidget();
    void initConnections();
    void showComputePowerShareDialog();
    void showLegalInformationDialog();
    void initLegalInformationDialog();

private:
    QVBoxLayout *m_mainVLayout = nullptr;
    QLabel *m_advancedSettingsLabel = nullptr;

    QFrame *m_computePowerShareFrame = nullptr;
    QLabel *m_computePowerShareLabel = nullptr;
    QPushButton *m_settingsBtn = nullptr;

    QFrame *m_legalInformationFrame = nullptr;
    QLabel *m_legalInformationLabel = nullptr;
    QPushButton *m_detailsBtn = nullptr;

    QDialog *m_legalInformationDailog = nullptr;
    QVBoxLayout *m_legalInformationVLayout = nullptr;

    ComputePowerShareDialog *m_computePowerShareDialog = nullptr;
    ComputePowerInterface m_computePowerInterface;
    ConfigManager m_configManager;
};

#endif  // COMPUTEPOWERSHAREWIDGET_H
