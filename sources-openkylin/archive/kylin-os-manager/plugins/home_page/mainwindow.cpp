#include "mainwindow.h"
#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <libkysysinfo.h>
#include <kpushbutton.h>
#include "frame.h"
#include "text_label.h"
#include "system_health_widget.h"
#include "computer_junk_widget.h"
#include "computer_treasure_chest_widget.h"
#include <gsettingmonitor.h>
#include <QTimer>
#include "kom-ukui-gsettings.h"

namespace home_page {

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    // 标题
    auto *my_computer_label = new K::TextLabel(this);
    my_computer_label->SetPixelSize(24, true);
    my_computer_label->SetAlignment(Qt::AlignBottom);
    my_computer_label->SetText(tr("My Computer"));
    auto *system_version_label = new K::TextLabel(this);
    system_version_label->SetAlignment(Qt::AlignBottom);
    auto *system_version = kdk_system_get_version(0);
    system_version_label->SetText(QString(system_version));
    free(system_version);
    //auto *go_to_btn = new QPushButton(this);
    //go_to_btn->setFixedSize(16, 16);
    //go_to_btn->setIcon(QIcon::fromTheme("ukui-end-symbolic"));
    auto *title_hbox_layout = new QHBoxLayout;
    title_hbox_layout->setContentsMargins(0, 0, 0, 0);
    title_hbox_layout->setSpacing(0);
    title_hbox_layout->addWidget(my_computer_label, 0, Qt::AlignBottom);
    title_hbox_layout->addSpacing(8);
    title_hbox_layout->addWidget(system_version_label, 0, Qt::AlignBottom);
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
    text_label->setEnabled(false);
    text_label->setText(tr("Self service solution for faults"));
    auto *text_hbox_layout = new QHBoxLayout;
    text_hbox_layout->setContentsMargins(0, 0, 0, 0);
    text_hbox_layout->setSpacing(0);
    text_hbox_layout->addWidget(text_label);
    text_hbox_layout->addWidget(line);

    auto *btn1 = new SolutionBtn(this);
    btn1->SetIcon(":/home_page/data/icons/default_application_exception.svg");
    btn1->SetText(tr("Application exception solution"));
    connect(btn1, &QPushButton::clicked, this, []() {Frame::SwitchToFaqSpecificType((char *)"application_exception");});

    auto *btn2 = new SolutionBtn(this);
    btn2->SetIcon(":/home_page/data/icons/default_network_exception.svg");
    btn2->SetText(tr("Network exception solution"));
    connect(btn2, &QPushButton::clicked, this, []() {Frame::SwitchToFaqSpecificType((char *)"network_exception");});

    auto *btn3 = new SolutionBtn(this);
    btn3->SetIcon(":/home_page/data/icons/default_hardware_exception.svg");
    btn3->SetText(tr("Hardware exception solution"));
    connect(btn3, &QPushButton::clicked, this, []() {Frame::SwitchToFaqSpecificType((char *)"hardware_exception");});

    auto *btn4 = new SolutionBtn(this);
    btn4->SetIcon(":/home_page/data/icons/default_system_exception.svg");
    btn4->SetText(tr("System exception solution"));
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

MainWindow::~MainWindow() {}

SolutionBtn::SolutionBtn(QWidget *parent)
    : kdk::KPushButton(parent),
      icon_btn_(new QPushButton(this)),
      text_label_(new kom::KomLabel(this)) {
    this->setObjectName("Kylin-os-manager_kdk::KPushButton_HomePageSolutionBtn");
    this->setAccessibleName(this->objectName());
    setMinimumSize(174,56);
    setBorderRadius(6);
    setBackgroundColor(qApp->palette().color(QPalette::Window));

    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=] {
        //适配深浅色，临时方案，待修改
        QTimer *timer = new QTimer;
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, [=] {
            setBackgroundColor(qApp->palette().color(QPalette::Window));
            timer->deleteLater();
        });
        timer->start(50);
    });

    icon_btn_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    icon_btn_->setFlat(true);
    icon_btn_->setFixedSize(24, 24);
    icon_btn_->setIconSize(QSize(24, 24));
    text_label_ = new kom::KomLabel(this);

    QHBoxLayout *hbox_layout = new QHBoxLayout(this);
    hbox_layout->setContentsMargins(24, 16, 8, 16);
    hbox_layout->setSpacing(0);
    hbox_layout->addWidget(icon_btn_);
    hbox_layout->addSpacing(8);
    hbox_layout->addWidget(text_label_);
    setLayout(hbox_layout);
}

void SolutionBtn::SetText(const QString &text) {
    text_label_->setText(text);
}

void SolutionBtn::SetIcon(const QString &path) {
    icon_btn_->setIcon(QPixmap(path));
}

}
