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
#include "waiting_dialog.h"
#include "ui_waiting_dialog.h"
#include <QMessageBox>
#include <QPainter>
#include "windowmanage.hpp"
#include "common_ui.h"
#include "global_variable.h"

WaitingDialog::WaitingDialog(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui::WaitingDialog),
      m_parentWindow(parent)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    init();
}

WaitingDialog::WaitingDialog(const QString &waitingInfo, int msec, QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui::WaitingDialog),
      m_parentWindow(parent)
{
    init();
    setAutoCloseTime(msec);
    labelSetText(waitingInfo);
}

void WaitingDialog::init()
{
    m_ui->setupUi(this);
    kabase::WindowManage::removeHeader(this);
    setWindowModality(Qt::ApplicationModal);
    m_autoCloseTimer = new QTimer(this);
    connect(m_autoCloseTimer, &QTimer::timeout, this, &WaitingDialog::close);
}

WaitingDialog::~WaitingDialog()
{
    delete m_ui;
    delete m_autoCloseTimer;
}

void WaitingDialog::labelSetText(QString text)
{
    m_ui->notifycationText->setText(text);
}

void WaitingDialog::setAutoCloseTime(int msec)
{
    m_interval = msec;
}

int WaitingDialog::exec()
{
    QWidget::show();
    if (m_parentWindow != nullptr) {
        centerTheWindow(this, m_parentWindow);
    }
    if (m_interval) {
        m_autoCloseTimer->start(m_interval);
    }
    return QDialog::exec();
}

void WaitingDialog::show()
{
    if (m_parentWindow != nullptr) {
        centerTheWindow(this, m_parentWindow);
    }
    if (m_interval) {
        m_autoCloseTimer->start(m_interval);
    }
    QWidget::show();
}
