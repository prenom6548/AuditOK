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
#include "smb_auth_info_dialog.h"
#include "ui_smb_auth_info_dialog.h"
#include "waiting_dialog.h"
#include "custom_message_box.h"

SmbAuthInfoDialog::SmbAuthInfoDialog(SMBAuthData &data, QWidget *parent)
    : m_data(data),
    CustomDialog(parent, true), 
    ui(new Ui::SmbAuthInfoDialog)
{
    ui->setupUi(this->mainWidget());
    setFixedSize(420, 340);
    setWindowTitle(tr("Authentication"));
    ui->tipLabel->setText(tr("Authentication is required to connect to the printer \"%1\"").arg(m_data.host));
    ui->usernameLabel->setText(tr("Username"));
    ui->passwordLabel->setText(tr("Password"));
    ui->workgroupLabel->setText(tr("Workgroup"));
    ui->acceptButton->setText(tr("Accept"));
    ui->cancelButton->setText(tr("Cancel"));

    ui->workgroupLineEdit->setPlaceholderText("WORKGROUP");
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    connect(ui->acceptButton, &QPushButton::clicked, this, &SmbAuthInfoDialog::acceptButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &SmbAuthInfoDialog::reject);
}

void SmbAuthInfoDialog::acceptButtonClicked()
{
    m_data.username = ui->usernameLineEdit->text();
    m_data.password = ui->passwordLineEdit->text();
    m_data.workgroup = ui->workgroupLineEdit->text();

    QPoint windowCenter = QPoint(this->mapToGlobal(QPoint(0,0)).x() + this->width()/2,this->mapToGlobal(QPoint(0,0)).y() + this->height()/2);
    WaitingDialog dialog(this);
    dialog.hide();
    dialog.labelSetText(tr("Authentication in progress. Please wait..."));
    dialog.show();

    dialog.move(windowCenter - dialog.rect().center());

    SmbClientHelper *smbHelper = new SmbClientHelper(m_data);
    int smbStatus = 0;
    QString smbErrMsg{QString()};
    connect(smbHelper, &SmbClientHelper::finished, &dialog, &QDialog::close);
    connect(smbHelper, &SmbClientHelper::succeed, [=, &smbStatus, &smbErrMsg](int status, QString errorMsg) {
        smbStatus = status;
        smbErrMsg = errorMsg;
        smbHelper->finished();
    });

    smbHelper->startWorker();
    dialog.exec();

    if (smbStatus == 0) {
        accept();
    } else {
        CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), smbErrMsg, CustomMessageBox::NoButton, this);
        QPushButton *btnYes = Msg.addButton(tr("Confirm"), CustomMessageBox::YesRole);
        Msg.exec();
    }
}

SmbAuthInfoDialog::~SmbAuthInfoDialog()
{
    delete ui;
}
