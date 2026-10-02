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
#include "property_list_window.h"
#include "ui_property_list_window.h"

PropertyListWindow::PropertyListWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PropertyListWindow)
{
    ui->setupUi(this);
    ui->cancelButton->setText(tr("Cancel"));
    ui->acceptButton->setText(tr("Confirm"));
    ui->refreshButton->setText(tr("Refresh"));
    connect(ui->acceptButton, &QPushButton::clicked, this, &PropertyListWindow::saveRequest);
    connect(ui->cancelButton, &QPushButton::clicked, this, &PropertyListWindow::closeRequest);

    for (int i = 0; i < PAGE_COUNT; i++) {
        ProperyListWidget *newPage = new ProperyListWidget(this);
        ui->stackedWidget->addWidget(newPage);
    }

}

PropertyListWindow::~PropertyListWindow()
{
    delete ui;
}

QStackedWidget *PropertyListWindow::getStackedWidget()
{
    return ui->stackedWidget;
}

QLabel *PropertyListWindow::getNameLabel()
{
    return ui->label;
}

QPushButton *PropertyListWindow::getRefreshButton()
{
    return ui->refreshButton;
}

QPushButton *PropertyListWindow::getAcceptButton()
{
    return ui->acceptButton;
}

ProperyListWidget *PropertyListWindow::getListWidget(int index)
{
    if (index >= PAGE_COUNT) {
        return nullptr;
    }
    QWidget *widget = ui->stackedWidget->widget(index);
    return qobject_cast<ProperyListWidget*>(widget);
}


