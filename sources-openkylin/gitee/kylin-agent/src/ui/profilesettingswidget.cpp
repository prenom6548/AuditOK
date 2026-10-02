#include "ui/profilesettingswidget.h"

ProfileSettingsWidget::ProfileSettingsWidget(QWidget *parent) 
    : QWidget(parent), 
      m_formLayout(new QFormLayout(this))
{
    setLayout(m_formLayout);
    createProfileGroup();
}

void ProfileSettingsWidget::createProfileGroup() {
    // Profile section
    m_profileNameEdit = new QLineEdit(this);
    m_profileNameEdit->setPlaceholderText(tr("输入配置文件名称"));
    
    m_autoSaveCheckBox = new QCheckBox(tr("自动保存设置"), this);
    m_autoSaveCheckBox->setChecked(true);
    
    // Add to form layout
    m_formLayout->addRow(tr("配置名称："), m_profileNameEdit);
    m_formLayout->addRow(m_autoSaveCheckBox);
    
    // Set default values
    m_profileNameEdit->setText(tr("默认"));
}

void ProfileSettingsWidget::loadSettings() {
    // Implementation for loading settings from configuration
    QSettings settings;
    m_profileNameEdit->setText(settings.value("Profile/Name", tr("默认")).toString());
    m_autoSaveCheckBox->setChecked(settings.value("Profile/AutoSave", true).toBool());
}

void ProfileSettingsWidget::saveSettings() {
    // Implementation for saving settings to configuration
    QSettings settings;
    settings.setValue("Profile/Name", m_profileNameEdit->text());
    settings.setValue("Profile/AutoSave", m_autoSaveCheckBox->isChecked());
}
