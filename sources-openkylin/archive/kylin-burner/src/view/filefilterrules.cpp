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

#include "burndata.h"
#include "filefilterrules.h"
#include "mainwindow.h"
#include "ui_filefilterrules.h"
#include "src/frame/xatom-helper.h"
#include "src/frame/logger.h"
#include "src/frame/configuration.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/data/datadirectory.h"
#include "src/frame/signals.h"
#include "src/common/utils.h"
#include "src/model/filefiltermodel.h"
#include "v11sdk.h"

#include <QPushButton>
#include <QDesktopWidget>

#include "KF5/KWindowSystem/kwindowsystem.h"
#include <windowmanager/windowmanager.h>

using namespace kdk;

FileFilterRules *FileFilterRules::m_oInstance = nullptr;
QMutex           FileFilterRules::m_oMutex;

FileFilterRules::FileFilterRules(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileFilterRules)
{
    this->setObjectName("kylin-burner_FileFilterRules");
    ui->setupUi(this);

    setWindowFlags(Qt::Dialog);

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

    ui->buttonBox->setAccessibleName("kylin-burner_QDialogButtonBox_FileFilterRules_buttonBox");
    ui->checkBoxHidden->setAccessibleName("kylin-burner_QCheckBox_FileFilterRules_checkBoxHidden");
    ui->checkBoxRepleace->setAccessibleName("kylin-burner_QCheckBox_FileFilterRules_checkBoxRepleace");
    ui->checkBoxSymbolLink->setAccessibleName("kylin-burner_QCheckBox_FileFilterRules_checkBoxSymbolLink");
    ui->labelTitle->setAccessibleName("kylin-burner_QLabel_FileFilterRules_labelTitle");
    ui->title->setAccessibleName("kylin-burner_WindowTitle_FileFilterRules_title");
    ui->widget_2->setAccessibleName("kylin-burner_QWidget_FileFilterRules_widget2");
    ui->widget_3->setAccessibleName("kylin-burner_QWidget_FileFilterRules_widget3");
    ui->widget_4->setAccessibleName("kylin-burner_QWidget_FileFilterRules_widget4");
    ui->widget_5->setAccessibleName("kylin-burner_QWidget_FileFilterRules_widget5");

    ui->title->onlyShowInDialog();
    connect(ui->title, SIGNAL(close()), this, SLOT(slotclose()));
    connect(this, SIGNAL(windowIconChanged(QIcon)), ui->title, SLOT(onWindowIconChange(QIcon)));
    connect(this, SIGNAL(windowTitleChanged(QString)), ui->title, SLOT(onWindowTitleChange(QString)));

    setWindowIcon(QIcon::fromTheme("burner"));
    setWindowTitle(_("Burner")/* + " - " + _("FileFilterRules")*/);
    connect(g_user_signal, &GlobalUserSignal::iconChange, [=](QIcon icon){
        setWindowIcon(icon);
    });

    ui->labelTitle->setText(_("FileFilterRules"));

    ui->checkBoxHidden->setText(_("Filter hidden file(s)."));
    ui->checkBoxSymbolLink->setText(_("Filter broken symbol links."));
    ui->checkBoxRepleace->setText(_("Repleace symbol link file(s)."));

    m_ishidden=false;
    m_isbroken=false;
    m_isrepleace=false;

    uiAdjust();

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &FileFilterRules::slotChangeUIFontSize);
}

FileFilterRules *FileFilterRules::instance(QWidget *parent)
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new FileFilterRules(parent);
        }
    }
    return m_oInstance;
}

void FileFilterRules::init(void *data)
{
    DataDirectory *node = (DataDirectory *)data;
    if (node)
    {
        // 如果节点有特定的过滤规则，使用节点规则；否则使用全局配置
        bool isHidden = node->m_bNodeFilterHidden.isValid() ? 
                       node->m_bNodeFilterHidden.toBool() : 
                       Configuration::instance()->get("FilterFile","hidden",false).toBool();
        ui->checkBoxHidden->setChecked(isHidden);

        bool isBroken = node->m_bNodeFilterBroken.isValid() ? 
                       node->m_bNodeFilterBroken.toBool() : 
                       Configuration::instance()->get("FilterFile","broken",false).toBool();
        ui->checkBoxSymbolLink->setChecked(isBroken);

        bool isReplace = node->m_bNodeRepleace.isValid() ? 
                        node->m_bNodeRepleace.toBool() : 
                        Configuration::instance()->get("FilterFile","Repleace",false).toBool();
        ui->checkBoxRepleace->setChecked(isReplace);

        // 应用过滤规则到节点
        node->execHidden(isHidden);
        node->execBroken(isBroken);
        node->execRepleace(isReplace);
    }
}

void FileFilterRules::show()
{
    m_oEnsureBtn->setProperty("isImportant", true);
    move(m_oPosition);
    if (!SystemInfo::m_isWayland) {
        KWindowSystem::setState(this->winId(), NET::SkipPager | NET::SkipSwitcher | NET::SkipTaskbar);
        QPoint position = MainWindow::Instance()->frameGeometry().topLeft();
        move(position.x()+(MainWindow::Instance()->width()-width())/2, position.y()+(MainWindow::Instance()->height()-height())/2);
        this->setGeometry(geometry());
    }
    QDialog::exec();
}

void FileFilterRules::showDlg()
{
    m_oEnsureBtn->setProperty("isImportant", true);
    if (!SystemInfo::m_isWayland) {
        KWindowSystem::setState(this->winId(), NET::SkipPager | NET::SkipSwitcher | NET::SkipTaskbar);
    }

    QWidget* parentWidget = this->parentWidget();
    if(parentWidget){
        QPoint position = parentWidget->frameGeometry().topLeft();
        move(position.x()+(parentWidget->width()-width())/2, position.y()+(parentWidget->height()-height())/2);
    }
    this->setGeometry(geometry());
    QDialog::exec();
}

void FileFilterRules::updatePosition(QPoint p)
{
    m_oPosition.setX(p.x() - width() / 2);
    m_oPosition.setY(p.y() - height() / 2);
}

void FileFilterRules::paintEvent(QPaintEvent *event)
{
    QPalette pal = palette();
    if ("ukui-black" == LogicMain::instance()->theme())
    {
        pal.setBrush(QPalette::Background,QColor("#1D1D1D"));
    }
    else
    {
        pal.setBrush(QPalette::Background, QColor("#FFFFFF"));
    }
    setPalette(pal);
    QDialog::paintEvent(event);
}

void FileFilterRules::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
}

FileFilterRules::~FileFilterRules()
{
    delete ui;
}


void FileFilterRules::uiAdjust()
{
    //setFixedSize(376,321+32);
    //ui->title->setFixedSize(380,40);
    ui->title->setContentsMargins(8,4,0,0);
    ui->widget_3->setContentsMargins(15,0,24,0);
    ui->widget_2->setContentsMargins(6,0,0,0);
    ui->buttonBox->clear();


    m_oCancelBtn = ui->buttonBox->addButton(_("Cancel"), QDialogButtonBox::RejectRole);
    m_oEnsureBtn = ui->buttonBox->addButton(_("OK"), QDialogButtonBox::AcceptRole);

    connect(m_oCancelBtn,&QPushButton::clicked,this,&FileFilterRules::slotscancel);
    connect(m_oEnsureBtn,&QPushButton::clicked,this,&FileFilterRules::slotsensure);

    ui->checkBoxHidden->setFixedHeight(21);
    ui->checkBoxRepleace->setFixedHeight(21);
    ui->checkBoxSymbolLink->setFixedHeight(21);
}

void FileFilterRules::slotChangeUIFontSize(int fontSize)
{
     int scaleRatio = qAbs(fontSize - 10.0);
     QFont font = ui->labelTitle->font();
     font.setPixelSize(16 + scaleRatio);
     font.setBold(true);
     ui->labelTitle->setFont(font);

     font.setPixelSize(14 + scaleRatio);
     font.setBold(false);

     ui->checkBoxHidden->setFont(font);
     ui->checkBoxSymbolLink->setFont(font);
     ui->checkBoxRepleace->setFont(font);

     m_oCancelBtn->setFont(font);
     m_oEnsureBtn->setFont(font);
}

void FileFilterRules::slotscancel()
{
    close();
}
void FileFilterRules::slotclose()
{
    close();
}

void FileFilterRules::slotsensure(){

    bool is_hidden_checked =  ui->checkBoxHidden->isChecked();
    bool is_replace_checked = ui->checkBoxRepleace->isChecked();
    bool is_broken_checked = ui->checkBoxSymbolLink->isChecked();

    // 获取当前操作的节点
    DataDirectory *currentNode = nullptr;
    if (FileFilterModel::instance() && FileFilterModel::instance()->m_iIndex >= 0) {
        currentNode = static_cast<DataDirectory*>(FileFilterModel::instance()->m_oRoots.at(FileFilterModel::instance()->m_iIndex));
    }

    if (currentNode)
    {
        // 保存节点特定的过滤规则
        currentNode->setNodeFilterHidden(is_hidden_checked);
        currentNode->setNodeFilterBrokenSymbolLinks(is_broken_checked);
        currentNode->setNodeRepleaceSymbolLinks(is_replace_checked);

        // 应用过滤规则到当前节点
        currentNode->execHidden(is_hidden_checked);
        currentNode->execBroken(is_broken_checked);
        currentNode->execRepleace(is_replace_checked);
    }

    emit hidden(is_hidden_checked);
    emit broken(is_broken_checked);
    emit replace(is_replace_checked);

    close();
}

