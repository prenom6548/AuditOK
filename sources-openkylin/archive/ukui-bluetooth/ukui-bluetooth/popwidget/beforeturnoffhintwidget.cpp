/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "beforeturnoffhintwidget.h"
#include "ui_beforeturnoffhintwidget.h"
#include "config/config.h"

BeforeTurnOffHintWidget::BeforeTurnOffHintWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::beforeTurnOffHintWidget)
{
    ui->setupUi(this);
    QIcon warningIcon = QIcon::fromTheme("kylin-dialog-warning");
    if (warningIcon.isNull())
        warningIcon = QIcon::fromTheme("dialog-warning");
    ui->iconLabel->setPixmap(warningIcon.pixmap(22,22));
    ui->cancelButton->setChecked(true);

    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        styleGSettings = new QGSettings("org.ukui.style");
        double size = styleGSettings->get("system-font-size").toDouble();
        if (size == 10)
            ui->iconLabel->move(26,16);
        connect(styleGSettings,&QGSettings::changed,this,[=](QString key) {
            if (key == "systemFontSize") {
                double size = styleGSettings->get("system-font-size").toDouble();
                if (size == 10)
                    ui->iconLabel->move(26,16);
                else
                    ui->iconLabel->move(26,19);
                this->repaint();
            }
        });
    }
    Config::setSkipTaskBar(this);
    Config::setWindowState(this);

    initConnect();
}

BeforeTurnOffHintWidget::~BeforeTurnOffHintWidget()
{
    delete ui;
}

QString BeforeTurnOffHintWidget::textDataStr()
{
    return ui->textLabel->text();
}

QString BeforeTurnOffHintWidget::cancelStr()
{
    return ui->cancelButton->text();
}

QString BeforeTurnOffHintWidget::closeStr()
{
    return ui->closeButton->text();
}

void BeforeTurnOffHintWidget::initConnect() {
    connect(ui->closeButton, &QPushButton::pressed, this, [=]() {
        emit buttonPressed(false);
    });
    connect(ui->cancelButton, &QPushButton::pressed, this, [=]() {
        emit buttonPressed(true);
    });
}
