#include "mainwindow.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <frame.h>
#include <kpushbutton.h>
#include "system_health_widget.h"
#include "computer_junk_widget.h"
#include "computer_treasure_chest_widget.h"
#include "text_label.h"

namespace home_page {

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // 标题
    auto *my_computer_label = new K::TextLabel(this);
    my_computer_label->SetPixelSize(24, true);
    my_computer_label->SetText(tr("My Computer"));
    auto *system_version_label = new K::TextLabel(this);
    // ToDo: 获取真实的系统版本
    system_version_label->SetText(tr("2.0 SP2 (nile)"));
    //auto *go_to_btn = new QPushButton(this);
    //go_to_btn->setFixedSize(16, 16);
    //go_to_btn->setIcon(QIcon::fromTheme("ukui-end-symbolic"));
    auto *title_hbox_layout = new QHBoxLayout;
    title_hbox_layout->setContentsMargins(0, 0, 0, 0);
    title_hbox_layout->setSpacing(0);
    title_hbox_layout->addWidget(my_computer_label);
    title_hbox_layout->addSpacing(8);
    title_hbox_layout->addWidget(system_version_label);
    title_hbox_layout->addSpacing(4);
    //title_hbox_layout->addWidget(go_to_btn);
    title_hbox_layout->addStretch();

    // 功能区
    auto *system_health_widget = new SystemHealthWidget(this);
    auto *computer_junk_widget = new ComputerJunkWidget(this);
    auto *computer_treasure_chest_widget = new ComputerTreasureChestWidget(this);
    auto *functional_area_hbox_layout = new QHBoxLayout;
    functional_area_hbox_layout->setContentsMargins(0, 0, 0, 0);
    functional_area_hbox_layout->setSpacing(0);
    functional_area_hbox_layout->addWidget(system_health_widget);
    functional_area_hbox_layout->addSpacing(16);
    functional_area_hbox_layout->addWidget(computer_junk_widget);
    functional_area_hbox_layout->addSpacing(16);
    functional_area_hbox_layout->addWidget(computer_treasure_chest_widget);

    // FAQ区
    auto *line = new QFrame(this);
    line->setMinimumWidth(624);
    line->setFrameShape(QFrame::HLine); // 水平线
    line->setFrameShadow(QFrame::Plain); // 无阴影
    line->setLineWidth(3); // 线宽
    auto *text_label = new QLabel(this);
    text_label->setText(tr("Self service solution for faults"));
    auto *text_hbox_layout = new QHBoxLayout;
    text_hbox_layout->setContentsMargins(0, 0, 0, 0);
    text_hbox_layout->setSpacing(0);
    text_hbox_layout->addWidget(text_label);
    text_hbox_layout->addWidget(line);

    auto *btn1 = new kdk::KPushButton(this);
    btn1->setIcon(QPixmap(":/home_page/data/icons/default_application_exception.svg").scaled(20, 20, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    btn1->setFixedSize(174,56);
    btn1->setText(tr("Application exception solution"));
    connect(btn1, &QPushButton::clicked, this, []() {Frame::SwitchToFaqSpecificType((char *)"application_exception");});

    auto *btn2 = new kdk::KPushButton(this);
    btn2->setIcon(QPixmap(":/home_page/data/icons/default_network_exception.svg").scaled(20, 20, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    btn2->setFixedSize(174,56);
    btn2->setText(tr("Network exception solution"));
    connect(btn2, &QPushButton::clicked, this, []() {Frame::SwitchToFaqSpecificType((char *)"network_exception");});

    auto *btn3 = new kdk::KPushButton(this);
    btn3->setIcon(QPixmap(":/home_page/data/icons/default_hardware_exception.svg").scaled(20, 20, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    btn3->setFixedSize(174,56);
    btn3->setText(tr("Hardware exception solution"));
    connect(btn3, &QPushButton::clicked, this, []() {Frame::SwitchToFaqSpecificType((char *)"hardware_exception");});

    auto *btn4 = new kdk::KPushButton(this);
    btn4->setIcon(QPixmap(":/home_page/data/icons/default_system_exception.svg").scaled(20, 20, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    btn4->setFixedSize(174,56);
    btn4->setText(tr("System exception solution"));
    connect(btn4, &QPushButton::clicked, this, []() {Frame::SwitchToFaqSpecificType((char *)"system_exception");});

    auto *btn_hbox_layout = new QHBoxLayout;
    btn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    btn_hbox_layout->setSpacing(0);
    btn_hbox_layout->addWidget(btn1);
    btn_hbox_layout->addStretch();
    btn_hbox_layout->addWidget(btn2);
    btn_hbox_layout->addStretch();
    btn_hbox_layout->addWidget(btn3);
    btn_hbox_layout->addStretch();
    btn_hbox_layout->addWidget(btn4);

    // 主布局
    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(40, 10, 40, 0);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(title_hbox_layout);
    vbox_layout->addSpacing(20);
    vbox_layout->addLayout(functional_area_hbox_layout);
    vbox_layout->addSpacing(24);
    vbox_layout->addLayout(text_hbox_layout);
    vbox_layout->addSpacing(16);
    vbox_layout->addLayout(btn_hbox_layout);
    vbox_layout->addStretch();
    setLayout(vbox_layout);
}

MainWindow::~MainWindow() {

}

}
