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
#include "custom_message_box.h"
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>

#include "main_win_property_widget.h"
#include "size_para.h"
#include "job_manager_window.h"
#include "ui_main_win_property_widget.h"
#include "common.h"
#include "information_collector.h"
#include "rename_printer_dialog.h"

MainWinPropertyWidget::MainWinPropertyWidget(QWidget *main, QWidget *parent)
  : QWidget(parent)
  , m_isPpdDialogShownFlag(false)
  , m_currentPrinter()
  , m_mainWinPtr(main)
  , m_ui(new Ui::MainWinPropertyWidget)
{
    setUI();
}

void MainWinPropertyWidget::setUI()
{
    m_ui->setupUi(this);

    m_logoIcon = m_ui->logoIcon;
    m_printerShowName = m_ui->printerShowName;
    m_printerShowNameEditIcon = m_ui->printerShowNameEditIcon;
    m_printTestPageBtn = m_ui->printTestPageBtn;
    m_showJobListBtn = m_ui->showJobListBtn;
    m_showPropertyBtn = m_ui->showPropertyBtn;
    m_properytView = m_ui->properytView;

    m_printerShowName->hide();
    m_printerShowNameEditIcon->hide();

    m_logoIcon->setIcon(QIcon::fromTheme("kylin-printer"));
    m_logoIcon->setIconSize(LOGO_ICON_SIZE);
    m_logoIcon->setFixedSize(LOGO_BTN_SIZE);
    m_logoIcon->setStyleSheet(BTN_TO_LABEL_STYLE);

    m_ui->printerNameButton->setText(tr("Printer"));
    m_ui->printerNameButton->setFixedSize(QSize(400,24));
    m_printerShowName->setText(tr("Printer"));
    m_printerShowName->setFixedSize(QSize(220,24));
    m_printerShowName->setAlignment(Qt::AlignCenter);
    m_printerShowName->setStyleSheet("QLineEdit{background:transparent;}");

//TODO:设计稿难以实现，待后续修整
//    m_printerShowNameEditIcon = new QLabel(this);
//    m_printerShowNameEditIcon->setPixmap(QIcon::fromTheme("document-edit.symbolic").pixmap(QSize(14,14)));
//    m_printerShowNameEditIcon->setObjectName("myLabel");
//    m_printerShowNameEditIcon->setFixedSize(14, 14);
//    m_printerShowNameEditIcon->setProperty("useIconHighlightEffect",0x8);
//    m_printerShowNameEditIcon->setProperty("setIconHighlightEffectHoverColor", QColor(Qt::blue));
//    m_printerShowNameEditIcon->setAttribute(Qt::WA_Hover);

    m_printerShowNameEditIcon->setFixedSize(20,20);
    m_printerShowNameEditIcon->setIcon(QIcon::fromTheme("document-edit-symbolic"));
    m_printerShowNameEditIcon->setIconSize(QSize(14,14));
    m_printerShowNameEditIcon->setFlat(true);

    connect(m_ui->printerNameButton, &CustomBorderlessButton::clicked, this, &MainWinPropertyWidget::onEditPrinterNameBtnPressed);
    connect(m_printerShowNameEditIcon,SIGNAL(released()),this,SLOT(onEditPrinterNameBtnPressed()));

    m_showPropertyBtn->setObjectName("showPropertyBtn");
    m_showPropertyBtn->setFixedSize(NORMAL_BTN_SIZE);
    m_showPropertyBtn->setText(tr("Property"));
    connect(m_showPropertyBtn,&QPushButton::clicked,this,[=](){
        emit sendOutCurrentPinterInfo(m_currentPrinter);
    });

    m_showJobListBtn->setObjectName("showJobListBtn");
    m_showJobListBtn->setFixedSize(NORMAL_BTN_SIZE);
    m_showJobListBtn->setText(tr("Job List"));
    connect(m_showJobListBtn,&QPushButton::clicked,this,[=](){
        emit sendOutCurrentPinterName(m_currentPrinter.name);
    });

    m_printTestPageBtn->setObjectName("printTestPageBtn");
    m_printTestPageBtn->setFixedSize(NORMAL_BTN_SIZE);
    m_printTestPageBtn->setText(tr("PrintTest"));
    connect(m_printTestPageBtn,&QPushButton::clicked,this,[=](){
        emit currentPinterInfoPrintTest(m_currentPrinter);
    });

    m_ui->toubleshootingBtn->setText(tr("Troubleshooting"));

    connect(m_ui->toubleshootingBtn,&QPushButton::clicked,this,[=](){
        emit currentPinterInfoTroubleshooting(m_currentPrinter);
    });

    m_properytView->setFixedSize(520, 146);
    // 设置表头隐藏
    m_properytView->horizontalHeader()->setHidden(true);
    m_properytView->verticalHeader()->setHidden(true);
    // 设置垂直方向对齐方式
    m_properytView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // 隔行变色
    m_properytView->setAlternatingRowColors(true);
    // 水平拉伸最后一行
    m_properytView->horizontalHeader()->setStretchLastSection(true);
    // 不显示网格线
    m_properytView->setShowGrid(false);
    // 不显示选择特效
    m_properytView->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    // 禁用双击编辑
    m_properytView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_properytView->setColumnWidth(0, 102);
    m_properytView->setColumnWidth(1, 520 - 102);
    m_propertyManagerModel = new PropertyManagerModel();
    m_properytView->setModel(m_propertyManagerModel);
    m_propertyDriverButton = new CustomPushButton(m_properytView);
    m_propertyDriverButton->setFixedWidth(m_properytView->columnWidth(1));
    m_properytView->setIndexWidget(m_propertyManagerModel->index(3,1), m_propertyDriverButton);
    connect(m_propertyDriverButton, &CustomPushButton::clicked, [=]() {
        emit ppdDialogShowRequest(m_propertyDriverButton->fullText());
    });

    // this->setStyleSheet("border:3px solid blue");
}

void MainWinPropertyWidget::setPrinterDriver(const QString &str)
{
    m_propertyDriverButton->setText(str);
}

PropertyManagerModel::PropertyManagerModel(QStandardItemModel *parent)
    : QStandardItemModel(parent)
{
    this->setRowCount(4);
    this->setColumnCount(2);

    int i = 0;
    this->setData(index(i++, 0), tr("Model:"));
    this->setData(index(i++, 0), tr("Status:"));
    this->setData(index(i++, 0), tr("Location:"));
    this->setData(index(i++, 0), tr("Driver:"));

    for (int i = 0; i < 4; i++) {
        this->item(i,0)->setTextAlignment(Qt::AlignCenter);
        this->item(i,0)->setEnabled(false);
    }
}

void PropertyManagerModel::setPrinterModel(const QString &str)
{
    this->setData(index(0, 1), str);
    this->item(0,1)->setEnabled(false);
}
void PropertyManagerModel::setPrinterStatus(const QString &str)
{
    this->setData(index(1, 1), str);
    this->item(1,1)->setEnabled(false);
}
void PropertyManagerModel::setPrinterLocation(const QString &str)
{
    this->setData(index(2, 1), str);
    this->item(2,1)->setEnabled(false);
}


void MainWinPropertyWidget::updateText(const UkuiUsbPrinter& printer)
{
    m_currentPrinter = printer;
    //打印机图标
    if (m_currentPrinter.device_uri.contains("usb://",Qt::CaseInsensitive))
    {
        m_ui->logoIcon->setIcon(QIcon::fromTheme("printer"));
    }
    else if (m_currentPrinter.device_uri.contains("/usb",Qt::CaseInsensitive))
    {
        m_ui->logoIcon->setIcon(QIcon::fromTheme("printer"));
    }
    else
    {
        m_ui->logoIcon->setIcon(QIcon::fromTheme("printer-remote"));
    }
    //打印机名称
    m_printerShowName->setText(printer.name);
    m_ui->printerNameButton->setText(printer.name);
    m_currentPrinterNameText = printer.name;
    //打印机型号
    m_propertyManagerModel->setPrinterModel(printer.product);
    //打印机状态
    if(!printer.connect_status.compare(tr("Idle")))
    {
        m_propertyManagerModel->setPrinterStatus(tr("Idle"));
    }
    else if(!printer.connect_status.compare(tr("Printing")))
    {
        m_propertyManagerModel->setPrinterStatus(tr("Printing"));
    }
    else if(!printer.connect_status.compare(tr("Stopped")))
    {
        m_propertyManagerModel->setPrinterStatus(tr("Stopped"));
    }
    else
    {
        qWarning() << "打印机状态异常：" << printer.connect_status;
        m_propertyManagerModel->setPrinterStatus(tr("Unknown"));
    }

    //打印机位置
    m_propertyManagerModel->setPrinterLocation(printer.location);
    //打印机驱动
    setPrinterDriver(printer.make_and_model);
}
void MainWinPropertyWidget::setFont(QFont ft, QLabel *label)
{
    QFontMetrics fontWidth(ft);
    QString labelString = label->text();
    int labelWidth = label->width();

    int width = fontWidth.width(labelString);  //计算字符串宽度
    if(width >= labelWidth)
    {
        QString temp = fontWidth.elidedText(labelString,Qt::ElideRight,labelWidth);
        label->setText(temp);
    }
}

void MainWinPropertyWidget::onEditPrinterNameBtnPressed()
{
    QString warningMsg = tr("Are you sure to rename \"%1\" ?").arg(m_currentPrinter.name);
    CustomMessageBox Msg(CustomMessageBox::Warning,tr("Warning"),warningMsg,CustomMessageBox::NoButton, this->m_mainWinPtr);
    Msg.setInformativeText(tr("This action will delete the job queue too!"));
    QPushButton *btnYes = Msg.addButton(tr("Rename"), CustomMessageBox::YesRole);
    QPushButton *btnNo  = Msg.addButton(tr("Cancel"), CustomMessageBox::NoRole);

    Msg.exec();
    if((QPushButton*)Msg.clickedButton() == btnYes)
    {
        RenamePrinterDialog d(m_currentPrinterNameText, this->m_mainWinPtr);
        int res = d.exec();
        if (res == QDialog::Accepted) {
            modifyCurrentPrinterName(d.getPrinterName());
        }
    }
    else
    {
        return;
    }
}
void MainWinPropertyWidget::modifyCurrentPrinterName(const QString &printerName)
{
    InformationCollector::getInstance().addPoint(InformationCollector::Rename);
    if (printerName.isEmpty()) {
        return;
    }
    bool isDefault = m_currentPrinter.is_default;
    if (false == UkuiPrinterManager::getInstance().duplicatePrinter(printerName, m_currentPrinter.name))
    {
        qDebug() << "打印机" << m_currentPrinter.name << "重命名失败(复制失败)";
        return;
    }
    else
    {
        if (false == UkuiPrinterManager::getInstance().removePrinter(m_currentPrinter.name))
        {
            UkuiPrinterManager::getInstance().removePrinter(printerName);
            qDebug() << "打印机" << m_currentPrinter.name << "重命名失败(复制成功，删除失败)";
        }
        else
        {
            qDebug() << "打印机" << m_currentPrinter.name << "重命名为" << printerName <<"成功！";
            if(isDefault)
            {
                if(false ==
                        UkuiPrinterManager::getInstance().setDefaultPrinter(printerName))
                {
                    qDebug() << "打印机" << m_currentPrinter.name << "设为默认失败(复制成功，删除成功，设为默认失败)";
                }

            }
            updateWin(printerName);
        }
    }
}

void MainWinPropertyWidget::updateWin(QString newPrinterName)
{
    qDebug() << newPrinterName;
    m_printerShowName->setText(newPrinterName);
    m_ui->printerNameButton->setText(newPrinterName);
    emit updateWholeMainWin(newPrinterName);
}
void MainWinPropertyWidget::setQSSFontSize(QFont curFont)
{

}
void MainWinPropertyWidget::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}





PropertyManagerModel::~PropertyManagerModel()
{

}
