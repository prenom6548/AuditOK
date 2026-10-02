/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QByteArray>
#include "custom_message_box.h"
#include "config_ip_address_dialog.h"
#include "ui_config_ip_address_dialog.h"
#include "size_para.h"
#include "global_variable.h"
#include "gsettings_monitor.h"
#include "cups_debug_logging_checkbox.h"
#include "accessinfohelper.h"

#include <QObject>

ConfigIPAddressDialog::ConfigIPAddressDialog(QWidget *parent) :
    CustomDialog(parent),
    m_ui(new Ui::ConfigIPAddressDialog),
    m_isConfigStatusOK(false)
{
    // setWindowModality(Qt::ApplicationModal);
    m_ui->setupUi((QDialog *)this->mainWidget());

    connect(m_ui->ipAddress, &QLineEdit::textChanged, this, &ConfigIPAddressDialog::onSettingsChanged);
    connect(m_ui->cupsCheckBox, &CupsDebugLoggingCheckbox::stateChanged, this, &ConfigIPAddressDialog::onSettingsChanged);
    connect(m_ui->pdfCheckBox->checkBox(), &QCheckBox::stateChanged, this, &ConfigIPAddressDialog::onSettingsChanged);
    connect(m_ui->brfCheckBox->checkBox(), &QCheckBox::stateChanged, this, &ConfigIPAddressDialog::onSettingsChanged);
    connect(m_ui->serialCheckBox, &QCheckBox::stateChanged, this, &ConfigIPAddressDialog::onSettingsChanged);
    connect(m_ui->applyBtn, SIGNAL(released()), this, SLOT(onApplyBtnClicked()));
    connect(m_ui->cancelBtn,SIGNAL(released()),this,SLOT(onCancelBtnClicked()));

    kdk::KDK_ALL_INFO_FORMAT(m_ui->ipAddress,"");

    this->layout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    m_ui->ipAddress->setAcceptDrops(false);
    setWindowTitle(tr("Configure"));
    m_ui->serviceAddressLabel->setText(tr("Service address:"));
    m_ui->pdfCheckBox->checkBox()->setText(tr("Enable") + " Generic-CUPS-PDF-Printer");
    m_ui->pdfCheckBox->label()->setText(tr("This printer can be used to convert print files to PDF format"));
    m_ui->brfCheckBox->checkBox()->setText(tr("Enable") + " Generic-CUPS-BRF-Printer");
    m_ui->brfCheckBox->label()->setText(tr("This printer can be used to convert print files to BRF format"));
    m_ui->serialCheckBox->setText(tr("Displays all serial printers"));
    m_ui->cancelBtn->setText(tr("Cancel"));
    m_ui->applyBtn->setText(tr("Ok"));
    m_ui->applyBtn->setProperty("isImportant", true);
    m_cupsDebug = m_ui->cupsCheckBox->isChecked();
    if(GsettingsMonitor::getInstance()->gsettingsCheck())
    {
        qDebug() << "success";
        m_isConfigStatusOK = true;
    }
    else
    {
        qDebug() << "fail";
        //弹窗
        CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), tr("Config init failed"),
                        CustomMessageBox::NoButton, this);
        QPushButton *btnYes = Msg.addButton(tr("Confirm"), CustomMessageBox::YesRole);
        Msg.exec();
        if (Msg.clickedButton() == btnYes)
        {
            m_isConfigStatusOK = false;
            this->close();
            return;
        }
    }

    loadSettings();
    this->show();
}

ConfigIPAddressDialog::~ConfigIPAddressDialog()
{
    delete m_ui;
}

void ConfigIPAddressDialog::loadSettings()
{
    m_oldIpAddress = GsettingsMonitor::getInstance()->apiServerAddressGet();
    qDebug() << "now api web is :" << m_oldIpAddress;
    m_ui->ipAddress->setText(m_oldIpAddress);

    m_ui->pdfCheckBox->checkBox()->setChecked(GsettingsMonitor::getInstance()->showProtocolsGet().contains("pdf"));
    m_ui->brfCheckBox->checkBox()->setChecked(GsettingsMonitor::getInstance()->showProtocolsGet().contains("brf"));
    m_ui->serialCheckBox->setChecked(GsettingsMonitor::getInstance()->showProtocolsGet().contains("serial"));

    m_ui->applyBtn->setEnabled(false);
}

void ConfigIPAddressDialog::saveSettings()
{
    if (m_oldIpAddress != m_ui->ipAddress->text()) {
        GsettingsMonitor::getInstance()->apiServerAddressSet(m_ui->ipAddress->text());
        qDebug() << "更换地址为 :" << m_ui->ipAddress->text();
    }

    QStringList showProtocols;
    if (m_ui->pdfCheckBox->checkBox()->isChecked()) {
        showProtocols.append("pdf");
    }

    if (m_ui->brfCheckBox->checkBox()->isChecked()) {
        showProtocols.append("brf");
    }

    if (m_ui->serialCheckBox->isChecked()) {
        showProtocols.append("serial");
    }
    GsettingsMonitor::getInstance()->showProtocolsSet(showProtocols);
}

void ConfigIPAddressDialog::onApplyBtnClicked()
{
    qDebug() << "按下了应用键，修改服务器地址！";
    saveSettings();
    if (m_cupsDebug != m_ui->cupsCheckBox->isChecked()) {
        m_ui->cupsCheckBox->saveDebugLogging();
    }
    this->close();
}

void ConfigIPAddressDialog::onCancelBtnClicked()
{
    qDebug() << "按下了取消键，直接退出！";
    this->close();
}

bool ConfigIPAddressDialog::getConfigStatus() const
{
    return m_isConfigStatusOK;
}

void ConfigIPAddressDialog::onSettingsChanged()
{
    if (m_cupsDebug == m_ui->cupsCheckBox->isChecked()
     && m_oldIpAddress == m_ui->ipAddress->text()
     && GsettingsMonitor::getInstance()->showProtocolsGet().contains("pdf") == m_ui->pdfCheckBox->checkBox()->isChecked()
     && GsettingsMonitor::getInstance()->showProtocolsGet().contains("brf") == m_ui->brfCheckBox->checkBox()->isChecked()
     && GsettingsMonitor::getInstance()->showProtocolsGet().contains("serial") == m_ui->serialCheckBox->isChecked()
        ) {
        m_ui->applyBtn->setEnabled(false);
    } else {
        m_ui->applyBtn->setEnabled(true);
    }
}
