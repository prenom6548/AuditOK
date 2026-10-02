#include "ui/schedulesettingswidget.h"

ScheduleSettingsWidget::ScheduleSettingsWidget(QWidget *parent) 
    : QWidget(parent), 
      m_formLayout(new QFormLayout(this))
{
    setLayout(m_formLayout);
    createScheduleGroup();
}

void ScheduleSettingsWidget::createScheduleGroup() {
    // Schedule section
    m_enableSchedulesCheckBox = new QCheckBox(tr("启用计划任务"), this);
    m_enableSchedulesCheckBox->setChecked(true);
    
    m_enableAutoSyncCheckBox = new QCheckBox(tr("启用自动同步"), this);
    m_enableAutoSyncCheckBox->setChecked(true);
    
    m_scheduleIntervalSpin = new QSpinBox(this);
    m_scheduleIntervalSpin->setRange(1, 1440);
    m_scheduleIntervalSpin->setValue(30);
    m_scheduleIntervalSpin->setSuffix(tr(" 分钟"));
    
    m_schedulePriorityCombo = new QComboBox(this);
    m_schedulePriorityCombo->addItems({tr("低"), tr("普通"), tr("高"), tr("紧急")});
    m_schedulePriorityCombo->setCurrentIndex(1);
    
    m_enableScheduleNotificationsCheckBox = new QCheckBox(tr("启用计划通知"), this);
    m_enableScheduleNotificationsCheckBox->setChecked(true);
    
    m_enableScheduleLoggingCheckBox = new QCheckBox(tr("启用计划日志"), this);
    m_enableScheduleLoggingCheckBox->setChecked(true);
    
    // Add to form layout
    m_formLayout->addRow(m_enableSchedulesCheckBox);
    m_formLayout->addRow(m_enableAutoSyncCheckBox);
    m_formLayout->addRow(tr("同步间隔："), m_scheduleIntervalSpin);
    m_formLayout->addRow(tr("默认优先级："), m_schedulePriorityCombo);
    m_formLayout->addRow(m_enableScheduleNotificationsCheckBox);
    m_formLayout->addRow(m_enableScheduleLoggingCheckBox);
    
    // Set default values
    m_scheduleIntervalSpin->setValue(30);
    m_schedulePriorityCombo->setCurrentIndex(1);
}

void ScheduleSettingsWidget::loadSettings() {
    // Implementation for loading schedule settings from configuration
    QSettings settings;
    m_enableSchedulesCheckBox->setChecked(settings.value("Schedule/Enabled", true).toBool());
    m_enableAutoSyncCheckBox->setChecked(settings.value("Schedule/AutoSync", true).toBool());
    m_scheduleIntervalSpin->setValue(settings.value("Schedule/Interval", 30).toInt());
    m_schedulePriorityCombo->setCurrentIndex(settings.value("Schedule/Priority", 1).toInt());
    m_enableScheduleNotificationsCheckBox->setChecked(settings.value("Schedule/Notifications", true).toBool());
    m_enableScheduleLoggingCheckBox->setChecked(settings.value("Schedule/Logging", true).toBool());
}

void ScheduleSettingsWidget::saveSettings() {
    // Implementation for saving schedule settings to configuration
    QSettings settings;
    settings.setValue("Schedule/Enabled", m_enableSchedulesCheckBox->isChecked());
    settings.setValue("Schedule/AutoSync", m_enableAutoSyncCheckBox->isChecked());
    settings.setValue("Schedule/Interval", m_scheduleIntervalSpin->value());
    settings.setValue("Schedule/Priority", m_schedulePriorityCombo->currentIndex());
    settings.setValue("Schedule/Notifications", m_enableScheduleNotificationsCheckBox->isChecked());
    settings.setValue("Schedule/Logging", m_enableScheduleLoggingCheckBox->isChecked());
    settings.sync();
}
