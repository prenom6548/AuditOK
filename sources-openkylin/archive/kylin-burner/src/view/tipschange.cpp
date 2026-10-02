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

#include "tipschange.h"
#include "ui_tipschange.h"
#include "windowtitle.h"
#include "src/frame/xatom-helper.h"
#include "src/frame/configuration.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/common/utils.h"
#include "v11sdk.h"

#include <QVBoxLayout>
#include <QSize>
#include <QFontMetrics>
#include <QPushButton>
#include <QLineEdit>



#define ICON_SIZE QSize(25, 25)

TipsChange::TipsChange(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TipsChange)
{
    this->setAccessibleName("kylin-burner_TipsChange");
    ui->setupUi(this);

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
    
    ui->buttonBox->setAccessibleName("kylin-burner_QDialogButtonBox_TipsChange_buttonBox");
    ui->labelContent->setAccessibleName("kylin-burner_QLabel_TipsChange_labelContent");
    ui->labelLogo->setAccessibleName("kylin-burner_QLabel_TipsChange_labelLogo");
    ui->title->onlyShowInDialog();
    connect(ui->title, SIGNAL(close()), this, SLOT(slotsClose()));
    connect(this, SIGNAL(windowIconChanged(QIcon)), ui->title, SLOT(onWindowIconChange(QIcon)));
    connect(this, SIGNAL(windowTitleChanged(QString)), ui->title, SLOT(onWindowTitleChange(QString)));
    ui->labelContent->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    ui->buttonBox->clear();
    m_oCancelBtn = ui->buttonBox->addButton(_("Cancel"), QDialogButtonBox::RejectRole);
    m_oEnsureBtn = ui->buttonBox->addButton(_("OK"), QDialogButtonBox::AcceptRole);
    ui->labelLogo->setPixmap(QIcon::fromTheme("dialog-question").pixmap(ICON_SIZE));

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &TipsChange::slotChangeUIFontSize);
}

TipsChange::~TipsChange()
{
    delete ui;

}
bool TipsChange::question(QString title, QString content, bool next)
{
    m_oTipTitle = title;
    if (CFG->get("EXEC", title, false).toBool()) {
        return true;
    }

    setFixedSize(424,164);
    ui->labelContent->setFocus();
    ui->labelContent->setText(content);
    m_oEnsureBtn->setProperty("isImportant", true);
    m_oEnsureBtn->setFocus();
    return !TipsChange::exec();
}
void TipsChange::slotsClose(){
    this->reject();
}

void TipsChange::slotChangeUIFontSize(int fontSize)
{
     int scaleRatio = qAbs(fontSize - 10.0);

     QFont font;
     font = ui->labelContent->font();
     font.setPixelSize(16 + scaleRatio);
     ui->labelContent->setFont(font);

}

void TipsChange::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        this->accept();
        if(m_oTipTitle==_("Delete")){
            this->reject();
        }
        break;
    case Qt::Key_Enter:
        this->reject();
        if(m_oTipTitle==_("Delete")){
            this->accept();
        }
        break;
    case Qt::Key_Return:
        this->reject();
        if(m_oTipTitle==_("Delete")){
            this->accept();
        }
        break;
    }
}

void TipsChange::slotsTipsAgain(bool b)
{
    CFG->set("EXEC", m_oTipTitle, b);
}
