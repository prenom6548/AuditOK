#ifndef PERMISSIONWIDGET_H
#define PERMISSIONWIDGET_H

#include <logic/KMBuilderManifest.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>

class PermissionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PermissionWidget(QWidget *parent = nullptr);
    ~PermissionWidget() = default;

    KMBuilderManifest::Permission getPermissionData() const;
    void setPermissionData(const KMBuilderManifest::Permission &permission);

signals:
    void back();
    void next();

private:
    KMBuilderManifest::Permission m_permission;

    QCheckBox *m_xdgUserDirsCheckBox;
    QCheckBox *m_mountHomeCheckBox;
    QTextEdit *m_filesystemDirsLineEdit;

    QCheckBox *m_networkCheckBox;
    QCheckBox *m_scanningCheckBox;

    QCheckBox *m_bluetoothCheckBox;
    QCheckBox *m_microphoneCheckBox;
    QCheckBox *m_cameraCheckBox;
    QCheckBox *m_usbCheckBox;
    QCheckBox *m_serialCheckBox;

    QCheckBox *m_autostartCheckBox;

    QCheckBox *m_systemNoticeCheckBox;
    QCheckBox *m_systemTimeCheckBox;
    QCheckBox *m_systemServerCheckBox;
    QCheckBox *m_preventSleepCheckBox;
    QCheckBox *m_systemDisplayResolutionCheckBox;
    QCheckBox *m_systemDisplayBrightnessCheckBox;
    QCheckBox *m_systemAudioCheckBox;
    QCheckBox *m_systemCrontabCheckBox;
    QCheckBox *m_systemPowerModeCheckBox;
    QCheckBox *m_systemPowerPolicyCheckBox;

    void initUI();
};

#endif // PERMISSIONWIDGET_H
