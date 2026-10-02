#include "widget.h"
#include "ktableheaderview.h"
#include "ktablemodel.h"
#include "ktableview.h"
#include <QDebug>
#include <QMenu>
#include <QPushButton>

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    KTableModel *model = new KTableModel();
    KTableView *tableview = new KTableView(this);
    //    tableview->setShowGrid(false);
    tableview->verticalHeader()->setVisible(false);

    QStringList head_list;
    head_list << "文件项"
              << "格式"
              << "时间"
              << "大小";
    tableview->setHorizontalTitle(head_list);

    for (int i = 0; i < 5; i++) {
        QList<QStandardItem *> add_items;
        for (int index = 0; index < head_list.count(); ++index) {
            add_items << new QStandardItem(QString::number(i + index));
        }
        model->appendRow(add_items);
    }

    tableview->setModel(model);

    tableview->setFixedSize(600, 500);

    tableview->setContextMenuPolicy(Qt::CustomContextMenu); // 必须设置
    QMenu *tableviewMenu = new QMenu(tableview);
    QAction *Action1 = new QAction("tableview 1", tableview);
    QAction *Action2 = new QAction("tableview 2", tableview);
    tableviewMenu->addAction(Action1);
    tableviewMenu->addAction(Action2);

    QMenu *headmenu1 = new QMenu(tableview);
    QAction *Action3 = new QAction("headmenu1 1", tableview);
    QAction *Action4 = new QAction("headmenu1 2", tableview);
    headmenu1->addAction(Action3);
    headmenu1->addAction(Action4);

    QMenu *headmenu2 = new QMenu(tableview);
    QAction *Action5 = new QAction("headmenu2 1", tableview);
    QAction *Action6 = new QAction("headmenu2 2", tableview);
    headmenu2->addAction(Action5);
    headmenu2->addAction(Action6);

    QModelIndexList list = tableview->selectionModel()->selectedIndexes();
    connect(tableview, &KTableView::customContextMenuRequested, this, [=] {
        tableviewMenu->exec(QCursor::pos());

        for (int i = 0; i < list.count(); i++)
            qDebug() << list.at(i).data().toString();
    });
    tableview->headerView()->setMenu(headmenu1, 1);
    tableview->headerView()->setMenu(headmenu2, 2);

    //    for(int i = 0;i < tableview->model()->rowCount();i++)
    //        qDebug()<< model->data(model->index(i,1)).toString();

    QPushButton *btn = new QPushButton(this);
    btn->move(0, 300);
    btn->setText("获取选中行的索引");
    connect(btn, &QPushButton::clicked, this, [=] {
        QList<int> list1 = tableview->selectList();
        qDebug() << "list1" << list1.count();
        for (int i = 0; i < list1.count(); i++)
            qDebug() << list1.at(i);
    });
}

Widget::~Widget()
{
}
