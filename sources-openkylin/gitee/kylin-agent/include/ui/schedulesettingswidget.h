#ifndef SCHEDULESETTINGSWIDGET_H
#define SCHEDULESETTINGSWIDGET_H

#include <QWidget>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QTimeEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>

class ScheduleSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ScheduleSettingsWidget(QWidget *parent = nullptr);
    
    void loadSettings();
    void saveSettings();

private:
    QFormLayout *m_formLayout;
    
    // Schedule section
    QCheckBox *m_enableSchedulesCheckBox;
    QCheckBox *m_enableAutoSyncCheckBox;
    QSpinBox *m_scheduleIntervalSpin;
    QComboBox *m_schedulePriorityCombo;
    QCheckBox *m_enableScheduleNotificationsCheckBox;
    QCheckBox *m_enableScheduleLoggingCheckBox;
    
    void createScheduleGroup();
};

#endif // SCHEDULESETTINGSWIDGET_H