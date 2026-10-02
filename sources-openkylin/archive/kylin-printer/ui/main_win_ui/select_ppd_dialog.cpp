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
#include "select_ppd_dialog.h"
#include "ui_select_ppd_dialog.h"
#include "find_and_match_ppds.h"
#include "base_notify_dialog.h"
#include <QToolTip>
#include "custom_message_box.h"
#include "size_para.h"
#include "waiting_dialog.h"


SelectPpdDialog::SelectPpdDialog(QWidget *parentWindow ,QDialog *parent)
    : CustomWidget(parentWindow, true)
    , m_ui(new Ui::SelectPpdDialog)
    , m_ppdMap(new ppdPrinterMap)
    , m_updateFailed(false)
    , m_currentChoosenPPD("")
    , m_currentChoosenModel("")
    , m_pParentWindow(parentWindow)
{
    m_ui->setupUi(this->baseBar());
    baseBar()->setFixedSize(720,600);
    layout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setWidgetName(tr("Select Driver"));

    windowButtonBar()->minimumButton()->hide();
    windowButtonBar()->maximumButton()->hide();

    m_comboBox = new QComboBox(this);
    m_searchLineEdit = new kdk::KSearchLineEdit(this);
    m_searchLineEdit->installEventFilter(this);

    connect(m_searchLineEdit, &kdk::KSearchLineEdit::returnPressed, this, [=](){
        this->searchClicked();
    });
    connect(m_searchLineEdit, &kdk::KSearchLineEdit::textChanged, this, [=](QString text) {
        m_searchLineEdit->blockSignals(true);
        m_searchLineEdit->setText(text);
        m_searchLineEdit->blockSignals(false);
    });
    connect(m_comboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &SelectPpdDialog::clearSearch);
    m_completer = new QCompleter(this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_searchLineEdit->setCompleter(m_completer);

    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated), this, [=]() {
        m_searchLineEdit->returnPressed();
    });

    QHBoxLayout* layout1 = static_cast<QHBoxLayout*>(windowButtonBar()->layout());
    layout1->insertWidget(0,m_comboBox,0, Qt::AlignBottom);
    layout1->insertSpacing(1,8);
    layout1->insertWidget(2,m_searchLineEdit, 0, Qt::AlignBottom);

    m_comboBox->setFixedSize(90, 36);
    m_comboBox->addItem(tr("driver"));
    m_comboBox->addItem(tr("vendor"));
    m_searchLineEdit->setFixedSize(274, 36);



    //厂商表
    m_vendorModal = new QStandardItemModel;
    m_vendorModal->invisibleRootItem();
    m_vendorModal->setColumnCount(1);

    m_ui->venderListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->venderListView->setModel(m_vendorModal);
    m_ui->venderListView->setAlternatingRowColors(true);
    m_ui->venderListView->setFrameShape(QFrame::StyledPanel);
    m_vendorModal->setHeaderData(0, Qt::Horizontal, tr("vendor"));

    connect(m_ui->venderListView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(venderListClicked(QModelIndex)));

    //型号表
    m_modelModal = new QStandardItemModel;
    m_modelModal->invisibleRootItem();
    m_modelModal->setColumnCount(1);

    m_ui->modelListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->modelListView->setModel(m_modelModal);
    m_ui->modelListView->setAlternatingRowColors(true);
    m_ui->modelListView->setFrameShape(QFrame::StyledPanel);
    m_modelModal->setHeaderData(0,  Qt::Horizontal, tr("driver"));

    connect(m_ui->modelListView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(modelListClicked(QModelIndex)));
    connect(m_ui->modelListView, SIGNAL(entered(QModelIndex)),
            this, SLOT(modelListShowToolTip(QModelIndex)));

    connect(m_ui->cancelBtn,SIGNAL(clicked(bool)),this,SLOT(cancelAndClose()));
    connect(m_ui->applyBtn,SIGNAL(clicked(bool)),this,SLOT(sendOutPPDName()));

    QPoint temp = QPoint(parentWindow->mapToGlobal(QPoint(0,0)).x() + parentWindow->width()/2,parentWindow->mapToGlobal(QPoint(0,0)).y() + parentWindow->height()/2);
    this->move(temp - this->rect().center());
    this->setBackgroundRole(QPalette::Base);
}
SelectPpdDialog::~SelectPpdDialog()
{
    emit userCanceled();
    delete m_ui;
    delete m_ppdMap;
}

void SelectPpdDialog::startSearch()
{
    if (m_comboBox->currentIndex() == 0) {
        m_completer->setModel(m_modelModal);
    } else if (m_comboBox->currentIndex() == 1) {
        m_completer->setModel(m_vendorModal);
    }
}

void SelectPpdDialog::clearSearch()
{
    m_searchLineEdit->clear();
}

void SelectPpdDialog::searchClicked()
{
    if (m_searchLineEdit->text().isEmpty()) {
        return ;
    }
    QStandardItemModel *model = nullptr;
    if (m_comboBox->currentIndex() == 0) {
        model = m_modelModal;
    } else {
        model = m_vendorModal;
    }

    QModelIndex index;
    bool ok = false;
    for (int i = 0; i < model->rowCount(); ++i) {
        index = model->index(i, 0);
        if (m_searchLineEdit->text() == index.data().toString()) {
            ok = true;
            break;
        }
    }

    if (ok == false) {
        for (int i = 0; i < model->rowCount(); ++i) {
            index = model->index(i, 0);
            if (m_completer->currentCompletion() == index.data().toString()) {
                ok = true;
                break;
            }
        }
    }

    if (ok == false) {
        return;
    }

    if (m_comboBox->currentIndex() == 0) {
        m_ui->modelListView->setCurrentIndex(index);
        modelListClicked(index);
    } else {
        m_ui->venderListView->setCurrentIndex(index);
        venderListClicked(index);
    }

//    clearSearch();
}

void SelectPpdDialog::onShow()
{
    QPoint temp = QPoint(m_pParentWindow->mapToGlobal(QPoint(0,0)).x() + m_pParentWindow->width()/2,m_pParentWindow->mapToGlobal(QPoint(0,0)).y() + m_pParentWindow->height()/2);
    this->move(temp - this->rect().center());
    this->setBackgroundRole(QPalette::Base);
    qDebug()<<"Set SelectPpdDialog to toplevel.";
    show();
    updateTableView();
}

void SelectPpdDialog::updateTableView()
{
    if((g_findAndMatchWorkThread->hasPPDsMap() == true) && (g_findAndMatchWorkThread->isBusy() == false))
    {
        delete m_ppdMap;
        m_ppdMap = new ppdPrinterMap();
        g_findAndMatchWorkThread->getCurrentPPDsMap(*m_ppdMap);
    }
    else
    {
        WaitingDialog *waitDialog = new WaitingDialog(m_pParentWindow);
        waitDialog->labelSetText(tr("Reading Drivers，Please Wait..."));//正在读取驱动，请稍后…

        m_updateFailed = true;
        delete m_ppdMap;
        m_ppdMap = new ppdPrinterMap();
        g_findAndMatchWorkThread->updatePPDsMap();
        connect(this, &SelectPpdDialog::ppdUpdateFinished, waitDialog, &WaitingDialog::close);
        connect(g_findAndMatchWorkThread,&FindAndMatchPPDs::updateFinished,this,[&](){
            qDebug() << "更新完毕";
            m_updateFailed = false;
            g_findAndMatchWorkThread->getCurrentPPDsMap(*m_ppdMap);
            ppdUpdateFinished();
        });
        connect(g_findAndMatchWorkThread,&FindAndMatchPPDs::updateFailed,this,[&](){
            qDebug() << "更新失败";
            m_updateFailed = true;
            ppdUpdateFinished();
        });

        waitDialog->exec();
        if(m_updateFailed)
        {
            qDebug() << "无法取得ppd的Map，退出窗口";
            return;
        }
    }

    ppdPrinterMap::iterator vendorName;

    m_vendorModal->clear();
    int i = 0;
    for(vendorName = m_ppdMap->begin();vendorName != m_ppdMap->end();vendorName++,i++)
    {
        QStandardItem *currentVendorName = new QStandardItem(QString(vendorName.key()));
        currentVendorName->setFont(m_font);
        m_vendorModal->setItem(i,0,currentVendorName);
    }
    m_vendorModal->setHeaderData(0, Qt::Horizontal, tr("vendor"));
    if (i > 0) {
        const QModelIndex &defaultVendorIndex = m_vendorModal->index(0, 0);
        m_ui->venderListView->setCurrentIndex(defaultVendorIndex);
        venderListClicked(defaultVendorIndex);
    }
}

void SelectPpdDialog::venderListClicked(const QModelIndex &index)
{
    QStandardItem *item = m_vendorModal->itemFromIndex(index);

    ppdPrinterMap::iterator vendorName;
    QString currentVendorName = item->text();

    m_currentVendor.clear();
    vendorName = m_ppdMap->find(currentVendorName);
    if(vendorName != m_ppdMap->end())
    {
        m_currentVendor = vendorName.value();
    }

    QMap<QString,PPDsAndAttr>::iterator ppdName;
    int i = 0;
    m_modelModal->clear();
    m_currentChoosenModel.clear();
    for(ppdName = m_currentVendor.begin();ppdName != m_currentVendor.end();ppdName++,i++)
    {
        QString showText = QString(ppdName.value().ppd_make_and_model);

        QStandardItem *currentModelName = new QStandardItem(showText);
        currentModelName->setFont(m_font);
        m_modelModal->setItem(i,0,currentModelName);
    }
    m_modelModal->setHeaderData(0,  Qt::Horizontal, tr("driver"));
    if (i > 0) {
        const QModelIndex &defaultModalIndex = m_modelModal->index(0, 0);
        m_ui->modelListView->setCurrentIndex(defaultModalIndex);
    }
    if (m_comboBox->currentIndex() == 0) {
        clearSearch();
    }
}

void SelectPpdDialog::modelListClicked(const QModelIndex &index)
{
    QStandardItem *item = m_modelModal->itemFromIndex(index);

    QMap<QString,PPDsAndAttr>::iterator ppdName;
    for(ppdName = m_currentVendor.begin();ppdName != m_currentVendor.end();ppdName++)
    {
        if (item->text() == QString(ppdName.value().ppd_make_and_model))
        {
            m_currentChoosenPPD.clear();
            m_currentChoosenPPD = QString(ppdName.value().ppdname);
            m_currentChoosenModel.clear();
            m_currentChoosenModel = QString(ppdName.value().ppd_make_and_model);
        }
    }
    if (m_comboBox->currentIndex() == 1) {
        clearSearch();
    }
}

void SelectPpdDialog::modelListShowToolTip(const QModelIndex &index)
{
    if (!index.isValid())
    {
        qDebug() << "Invalid index";
        return;
    }

    QToolTip::showText(QCursor::pos(), index.data().toString());
}

void SelectPpdDialog::sendOutPPDName()
{
    if(!m_currentChoosenPPD.isEmpty() && !m_currentChoosenModel.isEmpty())
    {
        qDebug() << "选中的厂商为：" << m_currentChoosenModel;
        qDebug() << "选中的型号为：" << m_currentChoosenPPD;
        emit chosenPPDName(m_currentChoosenPPD);
        this->close();
        return;
    }
    else
    {
        QString warningMsg = tr("Please Choose Model And Vender!");
        CustomMessageBox Msg(CustomMessageBox::Warning,tr("Warning"),warningMsg,CustomMessageBox::NoButton, this);
        Msg.addButton(tr("Close"), CustomMessageBox::NoRole);
        Msg.exec();
        return;
    }
}

void SelectPpdDialog::cancelAndClose()
{
    qDebug() << "用户取消";
    this->close();
}


bool SelectPpdDialog::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==m_searchLineEdit) {
        if (event->type() == QEvent::FocusIn) {
            startSearch();
        }
    }

    return CustomWidget::eventFilter(watched,event);
}
