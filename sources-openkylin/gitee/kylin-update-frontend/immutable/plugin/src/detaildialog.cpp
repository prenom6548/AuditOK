#include "detaildialog.h"
/*点击appwidget详情的弹窗*/
Detaildialog::Detaildialog(QWidget *parent) : QDialog(parent)
//Detaildialog::Detaildialog(QWidget *parent,AppAllMsg msg) : QDialog(parent)
{

    qRegisterMetaType<AppMsg>("AppMsg"); //注册信号槽类型
    qRegisterMetaType<AppAllMsg>("AppAllMsg"); //注册信号槽类型

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

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
        qInfo()<<"焦点在窗口之外";
        this->close();
    }
    else
    {
        qInfo()<<"焦点在窗口里面";
    }
    return QWidget::event(event);
}


Detaildialog::~Detaildialog()
{
}
