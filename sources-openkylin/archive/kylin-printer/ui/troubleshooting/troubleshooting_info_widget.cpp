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
#include <QMovie>

#include "troubleshooting_info_widget.h"
#include "ui_troubleshooting_info_widget.h"
#include "common_ui.h"

TroubleshootingInfoWidget::TroubleshootingInfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TroubleshootingInfoWidget)
{
    ui->setupUi(this);

    ui->statusIconBtn->setProperty("useIconHighlightEffect", 0x0);
    ui->statusIconBtn->setFlat(true);
    ui->statusIconBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QMovie *movieLoading = new QMovie(":/main_win_ui/loading.gif");
    movieLoading->setScaledSize(QSize(16,16));
    ui->movieBtn->setMovie(movieLoading); //初始默认图片0
    movieLoading->start();

    grayTheLabel(ui->informativeLabel);
}

TroubleshootingInfoWidget::~TroubleshootingInfoWidget()
{
    delete ui;
}

void TroubleshootingInfoWidget::setMainLabel(const QString &str)
{
    ui->mainLabel->setText(str);
}

void TroubleshootingInfoWidget::setInformativeLabel(const QString &str)
{
    ui->informativeLabel->setText(str);
}

void TroubleshootingInfoWidget::setStatus(STATUS status)
{
    switch (status)
    {
    case SUCCESS:
        ui->statusIconBtn->setIcon(QIcon::fromTheme("ukui-dialog-success"));
        ui->statusIconBtn->show();
        ui->movieBtn->hide();
        break;
    case FAIL:
        ui->statusIconBtn->setIcon(QIcon::fromTheme("dialog-error"));
        ui->statusIconBtn->show();
        ui->movieBtn->hide();
        break;
    case PENDING:
        ui->statusIconBtn->hide();
        ui->movieBtn->show();
        break;
    default:
        break;
    }
}
