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

#include "tip_burnresult.h"
#include "src/view/mainwindow.h"
#include "ui_tip_burnresult.h"
#include "src/frame/xatom-helper.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/common/utils.h"

#include <QIcon>

#include "ksoundeffects.h"
#include "v11sdk.h"

using namespace kdk;

#define ICON_SIZE QSize(22, 22)
#define ICON_SIZE_CLOSE QSize(16, 16)
#define ICON_SIZE_LOGO QSize(24, 24)

Tip_BurnResult *Tip_BurnResult::m_oInstance = nullptr;
QMutex       Tip_BurnResult::m_oMutex;

Tip_BurnResult::Tip_BurnResult(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Tip_BurnResult)
{
    this->setObjectName("kylin-burner_TipBurnResult");
    ui->setupUi(this);

    ui->buttonBox->setAccessibleName("kylin-burner_QDialogButtonBox_TipBurnResult_buttonBox");
    ui->close->setAccessibleName("kylin-burner_QPushButton_TipBurnResult_close");
    ui->labelContent->setAccessibleName("kylin-burner_QLabel_TipBurnResult_labelContent");
    ui->labelLogo->setAccessibleName("kylin-burner_QLabel_TipBurnResult_labelLogo");
    ui->logo->setAccessibleName("kylin-burner_QLabel_TipBurnResult_logo");
    ui->title->setAccessibleName("kylin-burner_QLabel_TipBurnResult_title");

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

    setAttribute(Qt::WA_ShowModal, true);

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(_("OK"));
    connect(this, SIGNAL(success(QString)), this, SLOT(slotsSuccess(QString)));
    connect(this, SIGNAL(fail(QString)), this, SLOT(slotsFailed(QString)));

    //close 按钮
    ui->close->setProperty("isWindowButton", 0x2);
    ui->close->setFocusPolicy(Qt::NoFocus); // 当为子窗口时，避免与回车关闭窗口冲突
    ui->close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->close->setIconSize(ICON_SIZE_CLOSE);
    ui->close->setToolTip(_("Close"));
    connect(ui->close, SIGNAL(clicked(bool)), this, SLOT(slotsClose()));
    setFixedSize(424, 164);
    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &Tip_BurnResult::slotChangeUIFontSize);
}

Tip_BurnResult::~Tip_BurnResult()
{
    delete ui;
}

Tip_BurnResult *Tip_BurnResult::instance(QWidget *parent)
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new Tip_BurnResult(parent);
        }
    }
    return m_oInstance;
}

void Tip_BurnResult::updatePosition(QPoint p)
{
    QPoint sp;
    sp.setX(p.x()-225);
    sp.setY(p.y()-115);
    move(sp);
}

void Tip_BurnResult::slotsSuccess(QString name)
{
    // 调用SDK声效
    kdk::KSoundEffects::playSound(kdk::SoundType::COMPLETE);

    ui->labelContent->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ui->labelContent->setText(name + _(" Success"));
    ui->labelLogo->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(ICON_SIZE));
    this->raise();//控件置顶
    move(MainWindow::Instance()->frameGeometry().center() - this->rect().center());
    this->exec();//控件显示
}

void Tip_BurnResult::slotsFailed(QString name)
{

    // 调用SDK声效
    kdk::KSoundEffects::playSound(kdk::SoundType::DIALOG_ERROR);

    ui->labelContent->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ui->labelContent->setText(name + _(" Failed"));
    ui->labelContent->setFixedWidth(this->width() - 60 - 25);
    ui->labelContent->setWordWrap(true);
    ui->labelContent->adjustSize();
    ui->labelLogo->setPixmap(QIcon::fromTheme("dialog-error").pixmap(ICON_SIZE));
    this->raise();//控件置顶
    move(MainWindow::Instance()->frameGeometry().center() - this->rect().center());
    this->exec();//控件显示
}

void Tip_BurnResult::slotsUmountFailed(QString name)
{ 
    ui->labelContent->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ui->labelContent->setText(name);
    ui->labelContent->setFixedWidth(this->width() - 60 - 25);
    ui->labelContent->setWordWrap(true);
    ui->labelContent->adjustSize();
    ui->labelLogo->setPixmap(QIcon::fromTheme("dialog-error").pixmap(ICON_SIZE));
    this->raise();//控件置顶
    move(MainWindow::Instance()->frameGeometry().center() - this->rect().center());
    this->exec();//控件显示
}


void Tip_BurnResult::slotsWarn(QString name)
{
    ui->labelContent->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ui->labelContent->setText(name);
    ui->labelContent->setFixedWidth(this->width() - 60 - 25);
    ui->labelContent->setWordWrap(true);
    ui->labelContent->adjustSize();
    ui->labelContent->setToolTip(name);
    ui->labelLogo->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(ICON_SIZE));
    this->raise();//控件置顶
    move(MainWindow::Instance()->frameGeometry().center() - this->rect().center());
    this->exec();//控件显示
}

void Tip_BurnResult::slotsTitle(QString title)
{

}


void Tip_BurnResult::slotsClose()
{
    this->hide();
}

void Tip_BurnResult::slotChangeUIFontSize(int fontSize)
{
     int scaleRatio = qAbs(fontSize - 10.0);

     QFont font = ui->buttonBox->font();
     font.setPixelSize(14 + scaleRatio);
     ui->buttonBox->setFont(font);

     font.setWeight(QFont::Medium);
     font.setFamily("Noto Sans CJK SC Medium");
     ui->labelContent->setFont(font);

}

void Tip_BurnResult::paintEvent(QPaintEvent *event)
{
    QPalette pal = palette();
    setPalette(pal);
    QDialog::paintEvent(event);
}


void Tip_BurnResult::on_buttonBox_clicked(QAbstractButton *button)
{

    if(QDialogButtonBox::Ok == ui->buttonBox->standardButton(button))
    {
        this->hide();
    }
}
