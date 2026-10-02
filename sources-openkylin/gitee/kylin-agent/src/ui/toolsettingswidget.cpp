#include "ui/toolsettingswidget.h"

ToolSettingsWidget::ToolSettingsWidget(QWidget *parent) 
    : QWidget(parent), 
      m_formLayout(new QFormLayout(this))
{
    setLayout(m_formLayout);
    createToolsGroup();
}

void ToolSettingsWidget::createToolsGroup() {
    // Tools section
    m_enableClipboardToolCheckBox = new QCheckBox(tr("启用剪贴板工具"), this);
    m_enableClipboardToolCheckBox->setChecked(true);
    
    m_enableFileManagementToolCheckBox = new QCheckBox(tr("启用文件管理工具"), this);
    m_enableFileManagementToolCheckBox->setChecked(true);
    
    m_enableSystemInfoToolCheckBox = new QCheckBox(tr("启用系统信息工具"), this);
    m_enableSystemInfoToolCheckBox->setChecked(false);
    
    m_enableNetworkToolCheckBox = new QCheckBox(tr("启用网络工具"), this);
    m_enableNetworkToolCheckBox->setChecked(true);
    
    m_enableScriptingToolCheckBox = new QCheckBox(tr("启用脚本工具"), this);
    m_enableScriptingToolCheckBox->setChecked(false);
    
    m_enableDataAnalysisToolCheckBox = new QCheckBox(tr("启用数据分析工具"), this);
    m_enableDataAnalysisToolCheckBox->setChecked(true);
    
    m_toolExecutionModeCombo = new QComboBox(this);
    m_toolExecutionModeCombo->addItems({tr("顺序"), tr("并行"), tr("后台")});
    m_toolExecutionModeCombo->setCurrentIndex(0);
    
    m_enableToolAutoUpdateCheckBox = new QCheckBox(tr("启用工具自动更新"), this);
    m_enableToolAutoUpdateCheckBox->setChecked(true);
    
    // Add to form layout
    m_formLayout->addRow(m_enableClipboardToolCheckBox);
    m_formLayout->addRow(m_enableFileManagementToolCheckBox);
    m_formLayout->addRow(m_enableSystemInfoToolCheckBox);
    m_formLayout->addRow(m_enableNetworkToolCheckBox);
    m_formLayout->addRow(m_enableScriptingToolCheckBox);
    m_formLayout->addRow(m_enableDataAnalysisToolCheckBox);
    m_formLayout->addRow(tr("执行模式："), m_toolExecutionModeCombo);
    m_formLayout->addRow(m_enableToolAutoUpdateCheckBox);
    
    // Set default values
    m_toolExecutionModeCombo->setCurrentIndex(0);
}

void ToolSettingsWidget::loadSettings() {
    // Implementation for loading tool settings from configuration
    QSettings settings;
    m_enableClipboardToolCheckBox->setChecked(settings.value("Tools/Clipboard", true).toBool());
    m_enableFileManagementToolCheckBox->setChecked(settings.value("Tools/FileManagement", true).toBool());
    m_enableSystemInfoToolCheckBox->setChecked(settings.value("Tools/SystemInfo", false).toBool());
    m_enableNetworkToolCheckBox->setChecked(settings.value("Tools/Network", true).toBool());
    m_enableScriptingToolCheckBox->setChecked(settings.value("Tools/Scripting", false).toBool());
    m_enableDataAnalysisToolCheckBox->setChecked(settings.value("Tools/DataAnalysis", false).toBool());
    m_toolExecutionModeCombo->setCurrentIndex(settings.value("Tools/ExecutionMode", 0).toInt());
    m_enableToolAutoUpdateCheckBox->setChecked(settings.value("Tools/AutoUpdate", true).toBool());
}

void ToolSettingsWidget::saveSettings() {
    // Implementation for saving tool settings to configuration
    QSettings settings;
    settings.setValue("Tools/Clipboard", m_enableClipboardToolCheckBox->isChecked());
    settings.setValue("Tools/FileManagement", m_enableFileManagementToolCheckBox->isChecked());
    settings.setValue("Tools/SystemInfo", m_enableSystemInfoToolCheckBox->isChecked());
    settings.setValue("Tools/Network", m_enableNetworkToolCheckBox->isChecked());
    settings.setValue("Tools/Scripting", m_enableScriptingToolCheckBox->isChecked());
    settings.setValue("Tools/DataAnalysis", m_enableDataAnalysisToolCheckBox->isChecked());
    settings.setValue("Tools/ExecutionMode", m_toolExecutionModeCombo->currentIndex());
    settings.setValue("Tools/AutoUpdate", m_enableToolAutoUpdateCheckBox->isChecked());
    settings.sync();
}
