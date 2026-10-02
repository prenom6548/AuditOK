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
#include "troubleshooting_dialog.h"
#include "ui_troubleshooting_dialog.h"
#include "common.h"
#include "connect.h"
#include "custom_message_box.h"
#include "check_uri_connected_helper.h"
#include "information_collector.h"

#include <QListWidgetItem>

const int list_width = 370;
const int list_height = 80;


TroubleshootingDialog::TroubleshootingDialog(const UkuiUsbPrinter &printer, QWidget *parent)
    : CustomDialog(parent, true)
    , ui(new Ui::TroubleshootingDialog)
    , m_printer(printer)
{
    InformationCollector::getInstance().addMessage("Troubleshooting");
    
    ui->setupUi(this->mainWidget());
    this->setFixedSize(420, 480);
    setWindowTitle(tr("Troubleshooting"));
    ui->acceptButton->setText(tr("Accept"));
    ui->cancelButton->setText(tr("Cancel"));
    ui->fixButton->setText(tr("Repair"));

    connect(ui->cancelButton, &QPushButton::clicked, this, &TroubleshootingDialog::reject);
    connect(ui->acceptButton, &QPushButton::clicked, this, &TroubleshootingDialog::accept);
    connect(ui->fixButton, &QPushButton::clicked, this, &TroubleshootingDialog::doRepair);

    // startCheck();
}

TroubleshootingDialog::~TroubleshootingDialog()
{
    delete ui;
}

void TroubleshootingDialog::onShow()
{
    show();
    startCheck();
}

void TroubleshootingDialog::startCheck()
{
    ui->fixButton->hide();
    ui->acceptButton->hide();
    ui->cancelButton->show();

    check1();
    check2();
    check3();
    check4();

    if (checkSuccess) {
        ui->fixButton->hide();
        ui->acceptButton->show();
        ui->cancelButton->hide();
    } else {
        ui->fixButton->show();
        ui->acceptButton->hide();
        ui->cancelButton->show();
    }
}

void TroubleshootingDialog::check1()
{
    checkitem1 = new TroubleshootingInfoWidget(ui->listWidget);
    QListWidgetItem *listWidgetItem = new QListWidgetItem(ui->listWidget);
    listWidgetItem->setFlags(Qt::ItemIsSelectable);
    listWidgetItem->setSizeHint(QSize(list_width, list_height));
    checkitem1->setFixedSize(list_width, list_height-4);

    checkitem1->setMainLabel(tr("Check whether the CUPS service is enabled"));
    checkitem1->setStatus(TroubleshootingInfoWidget::PENDING);

    ui->listWidget->addItem(listWidgetItem);
    ui->listWidget->setItemWidget(listWidgetItem, checkitem1);

    bool res = processCheck("/usr/sbin/cupsd");
    delay_msec(1000);

    if (res) {
        checkitem1->setInformativeLabel(tr("CUPS service is enabled"));
        checkitem1->setStatus(TroubleshootingInfoWidget::SUCCESS);
    } else {
        checkitem1->setInformativeLabel(tr("CUPS service is not enabled"));
        checkitem1->setStatus(TroubleshootingInfoWidget::FAIL);
        checkSuccess = false;
    }
    
}

void TroubleshootingDialog::check2()
{
    checkitem2 = new TroubleshootingInfoWidget(ui->listWidget);
    QListWidgetItem *listWidgetItem = new QListWidgetItem(ui->listWidget);
    listWidgetItem->setFlags(Qt::ItemIsSelectable);
    listWidgetItem->setSizeHint(QSize(list_width, list_height));
    checkitem2->setFixedSize(list_width, list_height-4);

    checkitem2->setMainLabel(tr("Check CUPS user access rights"));
    checkitem2->setStatus(TroubleshootingInfoWidget::PENDING);

    ui->listWidget->addItem(listWidgetItem);
    ui->listWidget->setItemWidget(listWidgetItem, checkitem2);

    delay_msec(1000);
    QStringList command;
    command << "groups" << "|" << "grep" << "lpadmin";
    QString res = getRetFromCommand(command);
    if (res.contains("lpadmin")) {
        checkitem2->setInformativeLabel(tr("CUPS users can access normally"));
        checkitem2->setStatus(TroubleshootingInfoWidget::SUCCESS);
    } else {
        checkitem2->setInformativeLabel(tr("CUPS user restricted access rights"));
        checkitem2->setStatus(TroubleshootingInfoWidget::FAIL);
        checkSuccess = false;
    }
}

void TroubleshootingDialog::check3()
{
    checkitem3 = new TroubleshootingInfoWidget(ui->listWidget);
    QListWidgetItem *listWidgetItem = new QListWidgetItem(ui->listWidget);
    listWidgetItem->setFlags(Qt::ItemIsSelectable);
    listWidgetItem->setSizeHint(QSize(list_width, list_height));
    checkitem3->setFixedSize(list_width, list_height-4);

    checkitem3->setMainLabel(tr("Check whether the printer connection is normal"));
    checkitem3->setStatus(TroubleshootingInfoWidget::PENDING);

    ui->listWidget->addItem(listWidgetItem);
    ui->listWidget->setItemWidget(listWidgetItem, checkitem3);

    CheckUriConnectedHelper *h = new CheckUriConnectedHelper(m_printer.device_uri);
    QEventLoop loop;
    bool checkResult = false;
    connect(h, &CheckUriConnectedHelper::finished, &loop, &QEventLoop::quit);
    connect(h, &CheckUriConnectedHelper::result, [=, &checkResult](bool result) {
        checkResult = result;
        h->finished();
    });

    h->startWorker();
    loop.exec();

    if (checkResult) {
        checkitem3->setInformativeLabel(tr("Printer connected"));
        checkitem3->setStatus(TroubleshootingInfoWidget::SUCCESS);
    } else {
        checkitem3->setInformativeLabel(tr("Printer disconnected"));
        checkitem3->setStatus(TroubleshootingInfoWidget::FAIL);
        checkSuccess = false;
    }
}

bool check_scheme(const QString &uri) {
    if (uri.isEmpty()) {
        return false;
    }
    const QString uriSplit = "://";
    if(!uri.contains(uriSplit)) {
        return true;
    }

    QString uriScheme = uri.left(uri.indexOf(uriSplit));

    if (uriScheme == "ipp" || uriScheme == "ipps") {
        return checkLocalhostPort(631);
    }
    return true;
}

void TroubleshootingDialog::check4()
{
    checkitem4 = new TroubleshootingInfoWidget(ui->listWidget);
    QListWidgetItem *listWidgetItem = new QListWidgetItem(ui->listWidget);
    listWidgetItem->setFlags(Qt::ItemIsSelectable);
    listWidgetItem->setSizeHint(QSize(list_width, list_height));
    checkitem4->setFixedSize(list_width, list_height-4);

    checkitem4->setMainLabel(tr("Check whether the printer protocol port configuration is enabled"));
    checkitem4->setStatus(TroubleshootingInfoWidget::PENDING);

    ui->listWidget->addItem(listWidgetItem);
    ui->listWidget->setItemWidget(listWidgetItem, checkitem4);

    delay_msec(1000);
    if (check_scheme(m_printer.device_uri)) {
        checkitem4->setInformativeLabel(tr("Printer protocol port configuration is enabled"));
        checkitem4->setStatus(TroubleshootingInfoWidget::SUCCESS);
    } else {
        checkitem4->setInformativeLabel(tr("Printer protocol port is not configured to be enabled"));
        checkitem4->setStatus(TroubleshootingInfoWidget::FAIL);
        checkSuccess = false;
    }
}

void TroubleshootingDialog::doRepair()
{
    CustomMessageBox Msg(CustomMessageBox::Critical, tr("Error"), tr("Automatic repair failed and manual troubleshooting is required."),
                             CustomMessageBox::NoButton, this);
    QPushButton *btnYes = Msg.addButton(tr("Confirm"), CustomMessageBox::YesRole);
    Msg.exec();
}
