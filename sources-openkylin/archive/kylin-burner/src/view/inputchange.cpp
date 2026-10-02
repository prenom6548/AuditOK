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

#include "inputchange.h"
#include "ui_inputchange.h"
#include "windowtitle.h"
#include "src/frame/xatom-helper.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/common/utils.h"
#include "v11sdk.h"

#include <QSize>
#include <QFontMetrics>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>


#define ICON_SIZE QSize(16, 16)

InputChange::InputChange(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputChange)
{
    this->setAccessibleName("kylin-burner_InputChange");
    ui->setupUi(this);
    setModal(true);

    //判断是否是wayland环境
    if(SystemInfo::m_isWayland) {
        ukuiWaylandRemoveTitleBar(this);
    } else {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);
    }

    ui->buttonBox->setAccessibleName("kylin-burner_QDialogButtonBox_InputChange_buttonBox");
    ui->labelContent->setAccessibleName("kylin-burner_QLabel_InputChange_labelContent");
    ui->lineEditResult->setAccessibleName("kylin-burner_QLineEdit_InputChange_lineEditResult");
    ui->title->setAccessibleName("kylin-burner_WindowTitle_InputChange_title");

    ui->title->onlyShowInDialog();
    connect(ui->title, SIGNAL(close()), this, SLOT(slotClose()));
    connect(this, SIGNAL(windowIconChanged(QIcon)), ui->title, SLOT(onWindowIconChange(QIcon)));
    connect(this, SIGNAL(windowTitleChanged(QString)), ui->title, SLOT(onWindowTitleChange(QString)));
    connect(ui->lineEditResult, &QLineEdit::textChanged, this, &InputChange::slotTextChanged);
    connect(ui->lineEditResult, &QLineEdit::textEdited, this, &InputChange::slotTextChanged);

    ui->lineEditResult->setFocus();
    ui->labelContent->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    ui->buttonBox->clear();					//删除ui文件内置的两个按钮
    m_oCancelBtn = ui->buttonBox->addButton(_("Cancel"), QDialogButtonBox::RejectRole);
    m_oEnsureBtn = ui->buttonBox->addButton(_("OK"), QDialogButtonBox::AcceptRole);

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &InputChange::slotChangeUIFontSize);
}

InputChange::~InputChange()
{
    delete ui;

}

QString InputChange::result()
{
    return ui->lineEditResult->text();
}
QString InputChange::content()
{
    return ui->labelContent->text();
}

bool InputChange::input(QString &inputed, QString title , QString content, QString fileName)
{
    inputed.clear();
    if(content == _("File name duplicate, need to rename:"))
    {
        m_oEnsureBtn->setEnabled(false);
    }

    if (this->result().isEmpty())
    {
        m_oEnsureBtn->setEnabled(false);
    }

    ui->labelContent->setText(content);
    ui->lineEditResult->setText(QString("%1").arg(fileName));
    this->setLineEditSelect(fileName);
    ui->lineEditResult->setFocus();
    if (exec())
    {
        inputed =this->result();
        return true;
    }
    return false;
}

void InputChange::setLineEditSelect(QString fileName)
{
    if(fileName.isEmpty()) {
        return;
    }

    if(fileName.contains(".")) {
        ui->lineEditResult->setSelection(0, indexSuffix(fileName));
    } else {
        ui->lineEditResult->selectAll();
    }
}

int InputChange::indexSuffix(QString fileName)
{
    int selectRightPos = fileName.lastIndexOf(".");
    if (fileName.contains(".tar.") && 4 == (selectRightPos - fileName.lastIndexOf(".tar."))) {
        selectRightPos -= 4;
    }
    return selectRightPos;
}

void InputChange::slotTextChanged(QString text){

    if (!text.isEmpty()) {
        m_oEnsureBtn->setEnabled(true);
        m_oEnsureBtn->setProperty("isImportant", true);
    } else {
        m_oEnsureBtn->setEnabled(false);
    }
}

void InputChange::slotClose(){
    this->reject();
}

void InputChange::slotChangeUIFontSize(int fontSize)
{
     int scaleRatio = qAbs(fontSize - 10.0);

     QFont font;
     font = ui->labelContent->font();
     font.setPixelSize(14 + scaleRatio);

     ui->labelContent->setFont(font);
     ui->lineEditResult->setFont(font);

}
void InputChange::keyPressEvent(QKeyEvent *event)
{

    switch (event->key())
    {
    case Qt::Key_Escape:
        this->reject();
        break;
    case  Qt::Key_Enter:
        this->accept();
        break;
    case Qt::Key_Return:
        this->accept();
        break;
    }
}
