#ifndef GENERALSETTINGSWIDGET_H
#define GENERALSETTINGSWIDGET_H

#include <QWidget>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>

class GeneralSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit GeneralSettingsWidget(QWidget *parent = nullptr);
    
    void loadSettings();
    void saveSettings();

private:
    QFormLayout *m_formLayout;
    
    // General section
    QComboBox *m_themeCombo;
    QCheckBox *m_enableAutoStartCheckBox;
    QCheckBox *m_startupGreetingCheckBox;
    QCheckBox *m_enableSystemTrayCheckBox;
    QCheckBox *m_enableDelegationModeCheckBox;
    QCheckBox *m_showDetailsPreviewButtonCheckBox;
    QCheckBox *m_showSessionExportButtonCheckBox;
    QCheckBox *m_showSessionTokenTotalCheckBox;
    QCheckBox *m_showSessionContextLengthCheckBox;
    QCheckBox *m_enableVoiceWakeCheckBox;
    QCheckBox *m_autoSendAfterVoiceRecognitionCheckBox;
    QComboBox *m_voiceToneCombo;
    QCheckBox *m_autoVoiceBroadcastCheckBox;
    
    void createGeneralGroup();
};

#endif // GENERALSETTINGSWIDGET_H
