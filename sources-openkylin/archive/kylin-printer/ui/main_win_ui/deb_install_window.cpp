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
#include <QFile>
#include <QFileDialog>
#include "custom_message_box.h"

#include "base_notify_dialog.h"
#include "find_and_match_ppds.h"
#include "ukui_apt.h"
#include "select_ppd_dialog.h"
#include "choose_ppd_combo_box.h"

#include "deb_install_window.h"
#include "ui_deb_install_window.h"

DebInstallWindow::DebInstallWindow(QWidget *parent)
    : kdk::KDialog(parent)
    , m_ui(new Ui::DebInstallWindow)
    , m_parentWidget(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(460,178);
    m_ui->setupUi(this->mainWidget());
    setWindowIcon(QIcon::fromTheme("kylin-printer"));
    setWindowTitle(tr("Modify Printer Driver"));

    connect(closeButton(), &QPushButton::clicked, this, &DebInstallWindow::onCancelBtnClicked);
    connect(m_ui->cancelBtn, &QPushButton::clicked, this, &DebInstallWindow::onCancelBtnClicked);
    connect(m_ui->forwardBtn, &QPushButton::clicked, this, &DebInstallWindow::onForwardBtnClicked);
    connect(m_ui->debNameLine, &ChoosePpdComboBox::currentPpdChanged, [=](bool isDefault, const QString &ppdName) {
        if (isDefault == true) {
            m_ui->forwardBtn->setEnabled(false);
        } else if (m_oldPpdName.size() && m_oldPpdName != ppdName) {
            m_ui->forwardBtn->setEnabled(true);
        } else {
            m_ui->forwardBtn->setEnabled(false);
        }
    });
}

DebInstallWindow::~DebInstallWindow()
{
    delete m_ui;
}

void DebInstallWindow::onCancelBtnClicked()
{
    m_ui->debNameLine->clear();
    this->close();
}

void DebInstallWindow::onForwardBtnClicked()
{
    if (m_ui->debNameLine->isEmpty()) {
        //弹出一个提示窗口，然后返回(安装失败，请重试)
        CustomMessageBox Msg(CustomMessageBox::Critical, tr("Error"), tr("Modify PPD failed: no PPD selected!"),
                        CustomMessageBox::NoButton, this);
        Msg.exec();
        return ;
    }
    modifyPPDName(m_ui->debNameLine->currentText());
    close();
}

void DebInstallWindow::onShow(const QString &ppdName)
{
    m_oldPpdName = ppdName;
    QPoint temp = QPoint(m_parentWidget->mapToGlobal(QPoint(0,0)).x() + m_parentWidget->width()/2,
                         m_parentWidget->mapToGlobal(QPoint(0,0)).y() + m_parentWidget->height()/2);
    this->move(temp - this->rect().center());
    m_ui->debNameLine->addItem(ppdName);
//    this->setBackgroundRole(QPalette::Base);
    if(!isActiveWindow())
    {
        hide();
        show();
    }
    show();
    raise();
    activateWindow();
}
