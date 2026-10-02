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
#include "base_notify_dialog.h"
#include "ui_base_notify_dialog.h"
#include <QMessageBox>
#include <QPainter>
#include "size_para.h"

BaseNotifyDialog::BaseNotifyDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::BaseNotifyDialog)
{
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);
    m_ui->setupUi(this);
    UKUI_WIN_CONTROL_PROTOOL(this);
}
void BaseNotifyDialog::showThis()
{
    QEventLoop eventloop;
    connect(this,SIGNAL(closeThis()),&eventloop,SLOT(quit()));
    this->show();
    eventloop.exec();
    this->hide();
}
BaseNotifyDialog::~BaseNotifyDialog()
{
    delete m_ui;
}

void BaseNotifyDialog::dialogSetType(BaseNotifyDialogType type)
{
    QIcon *statusIcon = new QIcon(QIcon::fromTheme("kylin-printer"));
    m_ui->pixIcon->setIcon(QIcon::fromTheme("kylin-printer"));//主题库的新建图标
    m_ui->pixIcon->setIconSize(TITLE_BTN_SIZE);
    m_ui->pixIcon->setFixedSize(TITLE_ICON_SIZE);
    m_ui->pixIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_ui->pixIcon->setFlat(true);

    switch(type){
        case (BaseNotifyDialogType::ERROR):
            delete statusIcon;
            statusIcon = new QIcon(QIcon::fromTheme("kylin-printer"));
            m_ui->pixIcon->setIcon(statusIcon->pixmap(QSize(24,24)));
            m_ui->progressBar->hide();
            break;
        case (BaseNotifyDialogType::WAITTING):
            m_ui->pixIcon->hide();
            // setMovieDialog();
            break;
        case (BaseNotifyDialogType::WARNING):
        {
            delete statusIcon;
            statusIcon = new QIcon(QIcon::fromTheme("kylin-printer"));
            m_ui->pixIcon->setIcon(statusIcon->pixmap(QSize(24,24)));
            m_ui->progressBar->hide();
            break;
        }
        case (BaseNotifyDialogType::SUCCEED):
        {
            delete statusIcon;
            statusIcon = new QIcon(QIcon::fromTheme("ukui-dialog-success.svg"));
            m_ui->pixIcon->setIcon(statusIcon->pixmap(QSize(24,24)));
            m_ui->progressBar->hide();
            break;
        }
        default:
            break;
    }
}

void BaseNotifyDialog::labelSetText(QString text)
{
    m_ui->notifycationText->setText(text);
}

void BaseNotifyDialog::setMovieDialog()
{
    m_timer = new QTimer(this);
    m_timer->start(100);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(gifPlay()));
}

void BaseNotifyDialog::gifPlay()
{
    QString picName = "";

    if (m_count > 8)
    {
        m_count = 1;
    }

    switch (m_count)
    {
        case 1:
            picName="ukui-loading-symbolic-0";
            break;
        case 2:
            picName="ukui-loading-symbolic-1";
            break;
        case 3:
            picName="ukui-loading-symbolic-2";
            break;
        case 4:
            picName="ukui-loading-symbolic-3";
            break;
        case 5:
            picName="ukui-loading-symbolic-4";
            break;
        case 6:
            picName="ukui-loading-symbolic-5";
            break;
        case 7:
            picName="ukui-loading-symbolic-6";
            break;
        case 8:
            picName="ukui-loading-symbolic-7";
            break;
        default:
            break;
    }
    ++m_count;
    m_ui->pixIcon->setIcon(QIcon::fromTheme(picName));
}
