/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "surewidget.h"
#include "ui_surewidget.h"
#include "imageutil.h"
#include "common/configuration.h"

#include <QApplication>
#include <QDebug>
Surewidget::Surewidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Surewidget),
    w_timer(nullptr),
    m_configuration(Configuration::instance())
{
    ui->setupUi(this);
    curFontSize = m_configuration->getUserConfig("fontSize").toDouble();
    m_ptToPx = m_configuration->getPtToPx();
    ui->tipLabel->setFixedWidth(this->width() - 20);
    ui->warningLabel->setFixedWidth(this->width() - 20);
    QFont sysFont = qApp->font();
    sysFont.setPointSize((16 + curFontSize) *m_ptToPx);
    ui->tipLabel->setText(tr("Multiple users are logged in at the same time.Are you sure "
                            "you want to reboot this system?"));
    ui->tipLabel->setStyleSheet("QLabel{color: #ffffff;}");
    ui->tipLabel->setFontSize(18 + curFontSize);
    ui->cancelBtn->setStyleSheet("QPushButton{color:#FFFFFF;background: rgba(255, 255, 255, 0.2);border-radius: 8px;color: white;}"
                                 "QPushButton:hover{background: rgba(255, 255, 255, 0.25);border-radius: 8px;}"
                                 "QPushButton:pressed{background: rgba(255, 255, 255, 0.25);border-radius: 8px;}");
    ui->confirmBtn->setStyleSheet("QPushButton{color:#FFFFFF;background: rgba(255, 255, 255, 0.2);border-radius: 8px;color: white;}"
                                          "QPushButton:hover{background: rgba(255, 255, 255, 0.25);border-radius: 8px;}"
                                          "QPushButton:pressed{background: rgba(255, 255, 255, 0.25);border-radius: 8px;}");
    ui->cancelBtn->setFont(sysFont);
    ui->confirmBtn->setFont(sysFont);
    ui->loadLabel->hide();
    ui->warningLabel->hide();
    connect(ui->cancelBtn, &QPushButton::clicked, this, [&]() { emit cancelButtonclicked(); });
    connect(ui->confirmBtn, &QPushButton::clicked, this, [&]() {
        if(blockStatus) {
            ui->loadLabel->show();
            onLoadingImage();
            ui->tipLabel->setText(tr("There's a program running, this process cannot be interrupted."));
            ui->warningLabel->setStyleSheet("color: #ffffff;}");
            ui->warningLabel->setFontSize(14 + curFontSize);
            ui->warningLabel->setText(tr("The system will automatically %1 after the program is executed. Do not shut down the computer.").arg(getHibited_tr(m_stateType)));
            ui->cancelBtn->hide();
            ui->confirmBtn->hide();
        } else {
            emit confirmButtonclicked();
        }
    });
}

Surewidget::~Surewidget()
{
    delete ui;


}

void Surewidget::setText(const QString tips)
{
    ui->tipLabel->setText(tips);
}

void Surewidget::setStatus(int stateType, int lockType)
{
    blockStatus = lockType;
    if(lockType) {
        ui->tipLabel->setText(tr("There's a program running, this process cannot be interrupted."));
        ui->warningLabel->show();
        ui->warningLabel->setStyleSheet("QLabel{color: #ffffff;}");
        ui->warningLabel->setFontSize(18 + curFontSize);
        ui->warningLabel->setText(tr("Click OK and the system will automatically %1 after the program is executed.").arg(getHibited_tr(stateType)));
        m_stateType = stateType;
    } else {
        ui->tipLabel->setText(tr("Multiple users are logged in at the same time.Are you sure "
                                "you want to reboot this system?"));
        ui->warningLabel->hide();
        ui->loadLabel->hide();
        ui->cancelBtn->show();
        ui->confirmBtn->show();
    }
}

void Surewidget::onLoadingImage()
{
    if(!w_timer)
    {
        w_timer = new QTimer(this);
        w_timer->setInterval(150);
        connect(w_timer, &QTimer::timeout, this, &Surewidget::updatePixmap);
    }
    QPixmap icon = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(32, 32);
    m_waitingPixmap = ImageUtil::drawSymbolicColoredPixmap(icon, "white");
    ui->loadLabel->setAlignment(Qt::AlignCenter);
    ui->loadLabel->setPixmap(m_waitingPixmap);
    w_timer->start();
}

void Surewidget::updatePixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    ui->loadLabel->setPixmap(m_waitingPixmap);
}

QString Surewidget::getHibited_tr(int type)
{
    switch (type) {
    case 0:
        return tr("Shut Down");
        break;
    case 1:
        return tr("Restart");
        break;
    case 2:
        return tr("Sleep");
        break;
    case 3:
        return tr("Hibernate");
        break;
    default:
        break;
    }
}
