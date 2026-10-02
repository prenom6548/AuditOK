#include "detaildialog.h"
/*点击appwidget详情的弹窗*/
Detaildialog::Detaildialog(QWidget *parent) : QDialog(parent)
//Detaildialog::Detaildialog(QWidget *parent,AppAllMsg msg) : QDialog(parent)
{

    qRegisterMetaType<AppMsg>("AppMsg"); //注册信号槽类型
    qRegisterMetaType<AppAllMsg>("AppAllMsg"); //注册信号槽类型

    kdk::UkuiStyleHelper::self()->removeHeader(this);

    logUi();

}
void Detaildialog::logUi()
{
//    this->setWindowFlags(Qt::Popup);
//    |Qt::FramelessWindowHint|Qt::NoDropShadowWindowHint
    logContent = new QTextEdit(this);
    logContent->setReadOnly(true);
    logContent->lineWrapMode();
    installContent = new FixLabel(this);
    installContent->setFixedHeight(26);
    logMainLayout = new QVBoxLayout(this);

    this->setLayout(logMainLayout);
    logMainLayout->addWidget(logContent,Qt::AlignCenter);
    logMainLayout->addStretch(1);
    logMainLayout->addWidget(installContent,Qt::AlignLeft|Qt::AlignBottom);
}

bool Detaildialog::event(QEvent *event)
{
    if(QEvent::WindowDeactivate == event->type())
    {
        qDebug()<<"焦点在窗口之外";
        this->close();
//        return true;
    }
    else
    {
        qDebug()<<"焦点在窗口里面";
//        return false;
    }
    return QWidget::event(event);
}

//bool Detaildialog::eventFilter(QObject *obj, QEvent *event)
//{
//    if(QEvent::WindowDeactivate == event->type())
//    {
//        qDebug()<<"焦点在窗口之外";
//        this->close();
//        return true;
//    }
//    else
//    {
//        qDebug()<<"焦点在窗口里面";
//        return false;
//    }
//}

Detaildialog::~Detaildialog()
{
}
