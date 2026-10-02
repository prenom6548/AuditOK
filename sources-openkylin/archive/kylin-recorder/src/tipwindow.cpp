#include "tipwindow.h"
#include "mainwindow.h"
#include <ukuistylehelper/ukuistylehelper.h>

// #include "xatom-helper.h"

TipWindow::TipWindow(QWidget *parent) : QDialog(parent)
{
    int WIDTH = 300;
    int HEIGHT = 50;
    //    mainWid = new QWidget();
    tipLabel = new QLabel(this);
    tipLabel->setAccessibleName("kylin-recorder_QLabel_tipLabel");
    hLayout = new QHBoxLayout(this);
    this->setFixedSize(WIDTH, HEIGHT);
    setTipWindow();
}

void TipWindow::setTipWindow()
{
    tipLabel->setText(tr("Analysis..."));
    tipLabel->setFixedSize(150, 30);
    tipLabel->setAlignment(Qt::AlignCenter);
    this->setWindowFlag(Qt::Tool); //此代码必须在此窗管协议前，否则此模态窗口背景不变灰
    hLayout->addWidget(tipLabel);
    this->setLayout(hLayout);
    // 添加窗管协议
    kdk::UkuiStyleHelper::self()->removeHeader(this);
    this->setAttribute(Qt::WA_ShowModal, true);
}
