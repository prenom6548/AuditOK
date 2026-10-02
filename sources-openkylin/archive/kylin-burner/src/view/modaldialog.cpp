/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "modaldialog.h"
#include "ui_modaldialog.h"
#include "src/frame/xatom-helper.h"
#include "src/frame/logger.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/frame/signals.h"
#include "src/view/mainwindow.h"
#include "src/view/mainwindow.h"
#include "src/common/utils.h"
#include "v11sdk.h"

#include <QMoveEvent>
#include <QPushButton>
#include <QCoreApplication>
#include <QDesktopWidget>

#include <KF5/KWindowSystem/kwindowsystem.h>
#include <windowmanager/windowmanager.h>
using namespace kdk;

ModalDialog *ModalDialog::m_oInstance = nullptr;
QMutex       ModalDialog::m_oMutex;

ModalDialog::ModalDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModalDialog)
{
    this->setObjectName("kylin-burner_ModalDialog");
    ui->setupUi(this);
    //判断是否是wayland环境
    if(SystemInfo::m_isWayland) {
        ukuiWaylandWindowStyleChange(this);

    } else {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);
    }

    ui->btnBox->setAccessibleName("kylin-burner_QDialogButtonBox_ModalDialog_btnBox");
    ui->content->setAccessibleName("kylin-burner_QWidget_ModalDialog_content");
    ui->dataSetting->setAccessibleName("kylin-burner_SettingDatas_ModalDialog_dataSetting");
    ui->diskcheck->setAccessibleName("kylin-burner_DiskCheck_ModalDialog_diskcheck");
    ui->progress->setAccessibleName("kylin-burner_Progress_ModalDialog_progress");
    ui->showPage->setAccessibleName("kylin-burner_QStackedWidget_ModalDialog_showPage");
    ui->tip->setAccessibleName("kylin-burner_tips_ModalDialog_tip");
    ui->title->setAccessibleName("kylin-burner_WindowTitle_ModalDialog_title");

    setAttribute(Qt::WA_ShowModal, true);
    ui->title->onlyShowInDialog();
    connect(ui->title, SIGNAL(close()), this, SLOT(slotsClose()));
    connect(this, SIGNAL(windowIconChanged(QIcon)), ui->title, SLOT(onWindowIconChange(QIcon)));
    connect(this, SIGNAL(windowTitleChanged(QString)), ui->title, SLOT(onWindowTitleChange(QString)));

    //图标库主题的刻录图片名换为burner
    //setWindowIcon(QIcon::fromTheme("disk-burner"));
    setWindowIcon(QIcon::fromTheme("burner"));
    connect(ui->progress, SIGNAL(title(QString)), this, SLOT(slotsTitle(QString)));
    connect(g_user_signal, &GlobalUserSignal::iconChange, [=](QIcon icon){
        setWindowIcon(icon);
    });


    ui->btnBox->clear();
    m_oCancelBtn = ui->btnBox->addButton(_("Cancel"), QDialogButtonBox::RejectRole);
    m_oEnsureBtn = ui->btnBox->addButton(_("OK"), QDialogButtonBox::AcceptRole);
    m_oCancelBtn->setAccessibleName("kylin-burner_QPushButton_ModalDialog_oCancelBtn");
    m_oEnsureBtn->setAccessibleName("kylin-burner_QPushButton_ModalDialog_oEnsureBtn");

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &ModalDialog::slotChangeUIFontSize);
}

ModalDialog *ModalDialog::instance(QWidget *parent)
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new ModalDialog(parent);
        }
    }
    return m_oInstance;
}

QWidget *ModalDialog::showPage(int page)
{
    m_bCanClose = true;
    disconnect(this, SIGNAL(rejected()), ui->busnessSetting, SLOT(slotsAccept()));
    disconnect(this, SIGNAL(rejected()), ui->diskcheck, SLOT(accept()));
    disconnect(g_user_signal, &GlobalUserSignal::sigChangeCloseEnabel, ui->title, &WindowTitle::slotsSetCloseEnabel);
    m_isprogress = false;

    switch (page)
    {
    case Progress:
        m_oCancelBtn->show();
        m_oEnsureBtn->hide();
        setFixedSize(450, 245);

        ui->verticalLayout_2->setContentsMargins(8,16,8,8);
        ui->btnBox->setContentsMargins(0,0,24,0);

        ui->title->onlyVisibleClose(true);
        connect(g_user_signal, &GlobalUserSignal::sigChangeCloseEnabel, ui->title, &WindowTitle::slotsSetCloseEnabel);
        m_isprogress = true;
        ui->progress->resetProgressBar();
        break;
    case Tip:
        setFixedSize(450, 230);
        ui->title->onlyVisibleClose(true);
        m_oEnsureBtn->show();
		m_oCancelBtn->hide();
        break;
    case BusnessSetting:
        m_oEnsureBtn->show();
        m_oCancelBtn->show();
        m_oCancelBtn->setEnabled(true);
        m_oEnsureBtn->setProperty("isImportant", true);
        setFixedSize(405, 530);
        ui->verticalLayout->setContentsMargins(0,0,0,16);
        ui->verticalLayout_2->setSpacing(22);
        ui->btnBox->setFixedHeight(36);
        ui->btnBox->setContentsMargins(0,0,24,0);
        ui->title->onlyVisibleClose(true);
        ui->title->slotsSetCloseEnabel(true);
        connect(ui->busnessSetting, SIGNAL(title(QString)), this, SLOT(slotsTitle(QString)));
        connect(this, SIGNAL(rejected()), ui->busnessSetting, SLOT(slotsAccept()));
        connect(ui->busnessSetting, SIGNAL(ensureBtnStateChanged(bool)), m_oEnsureBtn, SLOT(setEnabled(bool)));
        connect(ui->busnessSetting, SIGNAL(cancelBtnStateChanged(bool)), m_oCancelBtn, SLOT(setEnabled(bool)));
        break;
    case DataSetting:
        m_oEnsureBtn->show();
        m_oCancelBtn->show();
        m_oCancelBtn->setEnabled(true);
        m_oEnsureBtn->setEnabled(true);
        m_oEnsureBtn->setProperty("isImportant", true);
        setFixedSize(376,470);
        ui->verticalLayout->setContentsMargins(0,0,0,10);
        ui->btnBox->setContentsMargins(0,0,58,0);
        ui->title->onlyVisibleClose(true);
        ui->title->slotsSetCloseEnabel(true);
        connect(ui->dataSetting, SIGNAL(title(QString)), this, SLOT(slotsTitle(QString)));
        connect(this, SIGNAL(rejected()), ui->dataSetting, SLOT(accept()));
        ui->dataSetting->load();
        break;
    case DiskCheck:

        m_oEnsureBtn->show();
        m_oCancelBtn->hide();
        setFixedSize(450, 342);
        ui->verticalLayout_2->setContentsMargins(9,9,24,12);
        ui->btnBox->setContentsMargins(0,0,0,0);
        ui->title->onlyVisibleClose(true);
        ui->title->slotsSetCloseEnabel(true);

        slotsTitle(_("Disk Check"));
        connect(ui->diskcheck, SIGNAL(change(bool)), m_oEnsureBtn, SLOT(setEnabled(bool)));
        connect(this, SIGNAL(rejected()), ui->diskcheck, SLOT(accept()));
        ui->diskcheck->change(ui->diskcheck->check());
        break;
    }

    if (SystemInfo::m_isWayland) {

    } else {
        KWindowSystem::setState(this->winId(), NET::SkipPager | NET::SkipSwitcher | NET::SkipTaskbar);
    }
    ui->showPage->setCurrentIndex(page);
    return ui->showPage->currentWidget();
}

QWidget *ModalDialog::getPageByIndex(int index)
{
    return ui->showPage->widget(index);
}

void ModalDialog::cancelEnable(bool b)
{
    m_bCanClose = false;
    m_oCancelBtn->setEnabled(b);
    emit g_user_signal->sigChangeCloseEnabel(b);
}

void ModalDialog::updatePosition(QPoint p)
{   
    m_oMainPos = p;
}

void ModalDialog::show()
{
    QPoint position = MainWindow::Instance()->frameGeometry().topLeft();
    move(position.x()+(MainWindow::Instance()->width()-width())/2, position.y()+(MainWindow::Instance()->height()-height())/2);
    setGeometry(geometry());
    QDialog::show();
}

void ModalDialog::wexec()
{
    QPoint position = MainWindow::Instance()->frameGeometry().topLeft();
    move(position.x()+(MainWindow::Instance()->width()-width())/2, position.y()+(MainWindow::Instance()->height()-height())/2);
    setGeometry(geometry());
    QDialog::exec();
}

void ModalDialog::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);

    QFont font = ui->btnBox->font();
    font.setPixelSize(14 + scaleRatio);
    ui->btnBox->setFont(font);

    ui->title->setFont(font);

}

void ModalDialog::paintEvent(QPaintEvent *event)
{
    QPalette pal = palette();

    if ("ukui-black" == LogicMain::instance()->theme())
    {
        pal.setBrush(QPalette::Background,QColor("#1D1D1D"));
        setPalette(pal);
    }
    else
    {
        pal.setBrush(QPalette::Background, QColor("#FFFFFF"));
        setPalette(pal);
    }

    if(m_isprogress == true){
        m_oEnsureBtn->hide();
    }

    QDialog::paintEvent(event);
}

void ModalDialog::closeEvent(QCloseEvent *event)
{
    slotsClose();
    QDialog::closeEvent(event);
}

void ModalDialog::slotsTitle(QString title)
{
    setWindowTitle(_("Burner")/* + " - " + title*/);
}

void ModalDialog::slotsClose()
{
#ifndef UNIT_TEST
    disconnect(this, SIGNAL(accepted()), ui->diskcheck, SLOT(accept()));
    if (!m_bCanClose) return;
    switch (ui->showPage->currentIndex())
    {
    case Progress:
        if (m_oCancelBtn->isEnabled())
        {
            reject();
        }
        break;
    case Tip:
        reject();
        break;
    case BusnessSetting:
        hide();
        disconnect(this, SIGNAL(accepted()), ui->dataSetting, SLOT(accept()));
        break;
    case DataSetting:
        disconnect(ui->dataSetting, SIGNAL(title(QString)), this, SLOT(slotsTitle(QString)));
        disconnect(this, SIGNAL(accepted()), ui->dataSetting, SLOT(accept()));
        disconnect(ui->dataSetting, SIGNAL(change(bool)), m_oEnsureBtn, SLOT(setEnabled(bool)));
        hide();
        break;
    case DiskCheck:
        disconnect(ui->dataSetting, SIGNAL(change(bool)), m_oEnsureBtn, SLOT(setEnabled(bool)));
        disconnect(this, SIGNAL(accepted()), ui->diskcheck, SLOT(accept()));
        hide();
        break;
    }
#endif
}

ModalDialog::~ModalDialog()
{
    delete ui;
}
