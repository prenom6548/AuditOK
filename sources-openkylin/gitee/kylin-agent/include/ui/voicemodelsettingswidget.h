#ifndef VOICEMODELSETTINGSWIDGET_H
#define VOICEMODELSETTINGSWIDGET_H

#include <QWidget>

class QLabel;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;

class VoiceModelSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VoiceModelSettingsWidget(QWidget *parent = nullptr);

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void resetDefaults();
    void showSavedStatus();
    QString defaultStatusText() const;

    QComboBox *m_platformCombo;
    QComboBox *m_modelCombo;
    QLineEdit *m_apiKeyEdit;
    QCheckBox *m_autoStopCheck;
    QSpinBox *m_autoStopSecondsSpin;
    QCheckBox *m_autoStopSendCheck;
    QLabel *m_statusLabel;
};

#endif // VOICEMODELSETTINGSWIDGET_H
