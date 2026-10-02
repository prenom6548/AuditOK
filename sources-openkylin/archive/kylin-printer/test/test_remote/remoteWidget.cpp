#include "remoteWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QMenu>

#include "lpinfo_browser.h"
#include "ukui_printer.h"
#include "global_variable.h"

RemoteWidget::RemoteWidget(QWidget *parent)
    :CustomTransWidget(parent)
{
    initUi();
    initConnect();
}

void RemoteWidget::initUi()
{
    searchButton = new CustomPushButton();
    searchButton->setText("search");

    inputIP = new QLineEdit();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(inputIP);
    layout->addWidget(searchButton);
    
    QVBoxLayout *llayout = new QVBoxLayout();
    llayout->addLayout(layout);
    
    view = new QTableView();
        // 禁用双击编辑
    this->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置右键触发customContextMenuRequested信号
    this->view->setContextMenuPolicy(Qt::CustomContextMenu);
    // 选择整行
    this->view->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 隔行变色
    this->view->setAlternatingRowColors(true);
    // 隐藏行号
    this->view->verticalHeader()->hide();
    // 最后一格拉伸到最后
    this->view->horizontalHeader()->setStretchLastSection(true);
    model = new QStandardItemModel();
    model->setRowCount(0);
    model->setColumnCount(4);
    int cnt = 0;
    model->setHeaderData(cnt++, Qt::Horizontal, "name");
    model->setHeaderData(cnt++, Qt::Horizontal, "name in remote");
    model->setHeaderData(cnt++, Qt::Horizontal, "installed");
    model->setHeaderData(cnt++, Qt::Horizontal, "uri");
    view->setModel(model);
    llayout->addWidget(view);
    setLayout(llayout);
}

void RemoteWidget::initConnect()
{
    connect(searchButton, &QPushButton::clicked, this, [=](){
        doWork();
    });
    connect(this->view, &QTableView::customContextMenuRequested, this, &RemoteWidget::ShowContextMenu);
}
RemoteWidget::~RemoteWidget()
{
    
}

void RemoteWidget::doWork()
{
    QString host = inputIP->text();
    QStringList list = LpinfoBrowser::findRemotePrinter(host, 631);
    qDebug() << list;
    QList<UkuiUsbPrinter> printers;
    UkuiPrinterManager::getInstance().enumPrinters(printers);
    for (int i = 0; i < list.size(); i++) {
        model->insertRow(0);
        
        model->setData(model->index(0, 3), list.at(i));
        for (auto printer : printers) {
            if (printer.device_uri == list.at(i)) {
                model->setData(model->index(0, 0), printer.name);
                model->setData(model->index(0, 2), "yes");
                break;
            }
        }
        model->setData(model->index(0, 1), list.at(++i));
    }
}

void RemoteWidget::ShowContextMenu(const QPoint& pos)
{
    // qDebug() << this->view->selectionModel()->selectedIndexes();
    selectList = this->view->selectionModel()->selectedRows(3);
    qDebug() << selectList;
    QMenu menu;
    QList<QAction *> actions;
    this->actionAdd = new QAction(this);
    this->actionAdd->setText(tr("add"));
    this->actionPrint = new QAction(this);
    this->actionPrint->setText(tr("print"));

    actions << this->actionAdd
            << this->actionPrint;
    menu.addActions(actions);
    connect(&menu, &QMenu::triggered, this, &RemoteWidget::triggerMenu);
    menu.exec(QCursor::pos());
}

void RemoteWidget::triggerMenu(QAction *action)
{
    QString str = action->text();
    if (!this->selectList.size()) {
        return ;
    } 
    if(tr("add") == str) {
        UkuiPrinterManager::getInstance().addPrinter(selectList.at(0).data().toString(),
                                                     UkuiPrinterManager::getInstance().generatePrinterName(),
                                                     "raw",
                                                     "");
    }
    else if(tr("print") == str) {
        QString name = this->view->selectionModel()->selectedRows(0).at(0).data().toString();
        if (name.isEmpty()) 
            qDebug() << "not install!";
        UkuiPrinterManager::getInstance().printTestPage(name);
    }
    return ;
}