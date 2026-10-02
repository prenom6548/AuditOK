/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "ipweb_widget.h"
#include "ui_ipweb_widget.h"
#include <QDebug>
#include <toolutils.h>
#include <QToolButton>
#include <ktoolbutton.h>
using namespace kdk;

IPWebWidget::IPWebWidget(bool isFirst, IPWebWidgetType type, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::IPWebWidget),
      m_curType(type)
{
    ui->setupUi(this);
    ui->delBtn->setType(KToolButtonType::Background);
    ui->delBtn->setObjectName("delBtn");
    ui->delBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    ui->delBtn->setFocusPolicy(Qt::ClickFocus);
    ui->delBtn->setIconSize(QSize(16, 16));
    ui->addBtn->setObjectName("addBtn");
    ui->addBtn->setType(KToolButtonType::Background);
    ui->addBtn->setIcon(QIcon::fromTheme("list-add-symbolic"));
    ui->addBtn->setFocusPolicy(Qt::ClickFocus);
    ui->addBtn->setIconSize(QSize(16, 16));
    connect(ui->delBtn, SIGNAL(clicked()), this, SIGNAL(delPressed()));
    connect(ui->addBtn, &QToolButton::clicked, this, [=]() {
        qDebug() << "add按钮按下！";
        this->setFocus();
        emit addPressed();
    });
    connect(ui->addBtn, SIGNAL(pressed()), this, SIGNAL(userSettingsChanged()));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));

    if (isFirst) {
        if (type == IPWebWidgetType::IP_WIDGET) {
            ui->label->setText(tr("IP"));
        } else if (type == IPWebWidgetType::WEB_WIDGET) {
            ui->label->setText(tr("Website"));
        }
        ui->delBtn->hide();
    } else {
        ui->label->hide();
        ui->addBtn->hide();
    }
    this->show();
}

IPWebWidget::~IPWebWidget()
{
    delete ui;
}

void IPWebWidget::setLineText(QString str)
{
    ui->lineEdit->setText(str);
    return;
}


QString IPWebWidget::getLineText()
{
    return ui->lineEdit->text();
}

void IPWebWidget::setAddBtnEnable(bool isEnable)
{
    ui->addBtn->setEnabled(isEnable);
    return;
}
void IPWebWidget::slotTextChanged(QString text)
{
    if (m_curType == IPWebWidgetType::IP_WIDGET) {
        if (text.trimmed().isEmpty()) {
            formatStatus = true;
            ui->labelWarning->setText("");
        } else {
            if (ToolUtils::isIP(text)) {
                formatStatus = true;
                ui->labelWarning->setText("");
            } else {
                formatStatus = false;
                ui->labelWarning->setText(tr("Format error,IP is invalid"));
            }
        }
    } else {
        if (text.trimmed().isEmpty()) {
            formatStatus = true;
            ui->labelWarning->setText("");
        } else {
            if (ToolUtils::isWeb(text)) {
                formatStatus = true;
                ui->labelWarning->setText("");
            } else {
                formatStatus = false;
                ui->labelWarning->setText(tr("Format error,web is invalid"));
            }
        }
    }
    emit userSettingsChanged();
}

void IPWebWidget::Disable(bool first_item) {
    if (first_item) {
        ui->lineEdit->setDisabled(true);
    } else {
        ui->delBtn->setDisabled(true);
        ui->lineEdit->setDisabled(true);
    }
}

void IPWebWidget::SetIpWeb(const IpWeb &ip_web) {
    ip_web_ = ip_web;
}

IpWeb::Level IPWebWidget::GetLevel() {
    return ip_web_.level;
}
