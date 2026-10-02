#include "widget.h"
#include "kcomboxwidget.h"
#include <QAction>
using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    KSelector* selector = new KSelector(this);
    selector->label()->setText("关机频率");
//    QAction* action1 =new QAction("action1");
//    QAction* action2 =new QAction("action2");
//    QAction* action3 =new QAction("action3");
//    selector->comboBox()->addAction(action1);
//    selector->comboBox()->addAction(action2);
//    selector->comboBox()->addAction(action3);
    selector->comboBox()->addItem("1");
    selector->comboBox()->addItem("2");
    selector->comboBox()->addItem("3");

    setFocusPolicy(Qt::ClickFocus);

}

Widget::~Widget()
{
}

