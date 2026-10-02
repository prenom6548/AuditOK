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

#include "execdialog.h"
#include "mainwindow.h"
#include "ui_execdialog.h"
#include "src/frame/xatom-helper.h"
#include "src/frame/configuration.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/common/utils.h"
#include "v11sdk.h"

#include <QPushButton>
#include <QMessageBox>


ExecDialog *ExecDialog::m_oInstance = nullptr;
QMutex      ExecDialog::m_oMutex;

ExecDialog::ExecDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExecDialog)
{
    this->setObjectName("kylin-burner_ExecDialog");
    QFont f;
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

    ui->buttonBox->setAccessibleName("kylin-burner_QDialogButtonBox_ExecDialog_buttonBox");
    ui->inputPage->setAccessibleName("kylin-burner_Input_ExecDialog_inputPage");
    ui->stackedWidget->setAccessibleName("kylin-burner_QStackedWidget_ExecDialog_stackedWidget");
    ui->tip->setAccessibleName("kylin-burner_tips_ExecDialog_tip");
    ui->title->setAccessibleName("kylin-burner_WindowTitle_ExecDialog_title");
    ui->widget->setAccessibleName("kylin-burner_QWidget_ExecDialog_widget");

    ui->title->onlyShowInDialog();
    connect(ui->title, SIGNAL(close()), this, SLOT(close()));
    connect(this, SIGNAL(windowIconChanged(QIcon)), ui->title, SLOT(onWindowIconChange(QIcon)));
    connect(this, SIGNAL(windowTitleChanged(QString)), ui->title, SLOT(onWindowTitleChange(QString)));
    connect(this, SIGNAL(content(QString)), ui->inputPage, SIGNAL(content(QString)));
    connect(this, SIGNAL(result(QString)), ui->inputPage, SIGNAL(result(QString)));
    connect(ui->tip, SIGNAL(checked(bool)), this, SLOT(slotsTipsAgain(bool)));


    f = ui->buttonBox->font();
    f.setPixelSize(14);
    ui->buttonBox->setFont(f);
	ui->buttonBox->clear();					//删除ui文件内置的两个按钮
	ui->stackedWidget->setContentsMargins(0,0,0,0);
	ui->buttonBox->setContentsMargins(0,0,11,0);
	ui->verticalLayout->setContentsMargins(0,0,0,9);
	m_oCancelBtn = ui->buttonBox->addButton(_("Cancel"), QDialogButtonBox::AcceptRole);
    m_oEnsureBtn = ui->buttonBox->addButton(_("OK"), QDialogButtonBox::RejectRole);

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &ExecDialog::slotChangeUIFontSize);
}

ExecDialog *ExecDialog::instance(QWidget *parent)
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new ExecDialog(parent);

        }
    }
    return m_oInstance;
}

int ExecDialog::information(QString title, QString content, bool next)
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->tip->information(content, next);
	ui->buttonBox->removeButton(m_oCancelBtn);
    return !ExecDialog::exec();
}

void ExecDialog::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);
    QFont font = ui->buttonBox->font();
    font.setPixelSize(14 + scaleRatio);
    ui->buttonBox->setFont(font);

}


void ExecDialog::error(QString title, QString content, bool next)
{
    m_oTipTitle = title;
    if (CFG->get("EXEC", title, false).toBool()) return;
    ui->stackedWidget->setCurrentIndex(1);
    ui->buttonBox->removeButton(m_oCancelBtn);
    ui->tip->error(content, next);
    ExecDialog::exec();
}

void ExecDialog::paintEvent(QPaintEvent *event)
{
    QPalette pal = palette();
    if ("ukui-black" == LogicMain::instance()->theme())
        pal.setBrush(QPalette::Background, QColor("#484848"));
    else
        pal.setBrush(QPalette::Background, QColor("#FFFFFF"));
    setPalette(pal);
    QDialog::paintEvent(event);
}

void ExecDialog::slotsTipsAgain(bool b)
{
    CFG->set("EXEC", m_oTipTitle, b);
}

ExecDialog::~ExecDialog()
{
    delete ui;
}

QPushButton* ExecDialog::replayEnsure(){
	return m_oEnsureBtn;
}

QPushButton* ExecDialog::replayCancel(){
	return m_oCancelBtn;
}
void ExecDialog::close(){
     this->accept();
}
void ExecDialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        this->accept();
        return;
    case  Qt::Key_Enter:
        this->reject();
        return;
    case Qt::Key_Return:
        this->reject();
        return;
    }

    QDialog::keyPressEvent(event);
}
