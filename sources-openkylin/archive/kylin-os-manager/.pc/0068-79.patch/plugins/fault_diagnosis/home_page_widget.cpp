#include <QDebug>
#include <QHBoxLayout>
#include <QApplication>
#include <gsettingmonitor.h>
#include "kom-ukui-gsettings.h"
#include "utils.h"
#include "frame.h"
#include "config_win.h"
#include "home_page_widget.h"

namespace fault_diagnosis {

const static int DETECTION_ICON_WIDTH = 36;
const static int DETECTION_ICON_HEIGHT = 36;
const static int HOME_PAGE_ICON_WIDTH = 256;
const static int HOME_PAGE_ICON_HEIGHT = 256;

HomePageWidget::HomePageWidget()
    : describe_label_(new K::TextLabel(this)),
      details_label_(new K::TextLabel(this)),
      hardware_detection_icon_label_(new K::IconLabel(this)),
      hardware_detection_label_(new K::TextLabel(this)),
      kernel_detection_icon_label_(new K::IconLabel(this)),
      kernel_detection_label_(new K::TextLabel(this)),
      system_service_detection_icon_label_(new K::IconLabel(this)),
      system_service_detection_label_(new K::TextLabel(this)),
      system_application_detection_icon_label_(new K::IconLabel(this)),
      system_application_detection_label_(new K::TextLabel(this)),
      icon_label_(new K::IconLabel(this)),
      one_click_detection_btn_(new QPushButton(this)),
      internal_network_detection_btn_(new kdk::KBorderlessButton(this))
{
    describe_label_->SetText(tr("One click troubleshooting of computer malfunctions"));
    describe_label_->SetPixelSize(36, true);
    details_label_->SetText(tr("Quick detection and repair of fault issues, safeguarding the health of your computer"));
    details_label_->SetPixelSize(16);

    QColor background;
    auto theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
    if (theme == "ukui-light" || theme == "ukui-default") {
        background = QColor(230, 230, 230);
    } else {
        background = QColor(56, 56, 56);
    }
    // 第一列检测项
    hardware_detection_icon_label_->setFixedSize(DETECTION_ICON_WIDTH, DETECTION_ICON_HEIGHT);
    hardware_detection_icon_label_->SetBackgroundColor(background);
    hardware_detection_icon_label_->SetIcon(QIcon::fromTheme("ukui-zs-hard-disk-symbolic"), 0.5);
    hardware_detection_label_->SetText(tr("Hardware detection"));
    auto *hardware_detection_hbox_layout = new QHBoxLayout;
    hardware_detection_hbox_layout->setSpacing(0);
    hardware_detection_hbox_layout->setContentsMargins(0, 0, 0, 0);
    hardware_detection_hbox_layout->addWidget(hardware_detection_icon_label_);
    hardware_detection_hbox_layout->addSpacing(12);
    hardware_detection_hbox_layout->addWidget(hardware_detection_label_);

    system_service_detection_icon_label_->setFixedSize(DETECTION_ICON_WIDTH, DETECTION_ICON_HEIGHT);
    system_service_detection_icon_label_->SetBackgroundColor(background);
    system_service_detection_icon_label_->SetIcon(QIcon::fromTheme("ukui-zs-neicun-symbolic"), 0.5);
    system_service_detection_label_->SetText(tr("System service detection"));
    auto *system_service_detection_hbox_layout_ = new QHBoxLayout;
    system_service_detection_hbox_layout_->setSpacing(0);
    system_service_detection_hbox_layout_->setContentsMargins(0, 0, 0, 0);
    system_service_detection_hbox_layout_->addWidget(system_service_detection_icon_label_);
    system_service_detection_hbox_layout_->addSpacing(12);
    system_service_detection_hbox_layout_->addWidget(system_service_detection_label_);

    auto *first_column_detection_vbox_layout = new QVBoxLayout;
    first_column_detection_vbox_layout->setSpacing(0);
    first_column_detection_vbox_layout->setContentsMargins(0, 0, 0, 0);
    first_column_detection_vbox_layout->addLayout(hardware_detection_hbox_layout);
    first_column_detection_vbox_layout->addSpacing(16);
    first_column_detection_vbox_layout->addLayout(system_service_detection_hbox_layout_);

    // 第二列检测项
    kernel_detection_icon_label_->setFixedSize(DETECTION_ICON_WIDTH, DETECTION_ICON_HEIGHT);
    kernel_detection_icon_label_->SetBackgroundColor(background);
    kernel_detection_icon_label_->SetIcon(QIcon::fromTheme("ukui-zs-kernel-version-symbolic"), 0.5);
    kernel_detection_label_->SetText(tr("Kernel detection"));
    auto *kernel_detection_hbox_layout = new QHBoxLayout;
    kernel_detection_hbox_layout->setSpacing(0);
    kernel_detection_hbox_layout->setContentsMargins(0, 0, 0, 0);
    kernel_detection_hbox_layout->addWidget(kernel_detection_icon_label_);
    kernel_detection_hbox_layout->addSpacing(12);
    kernel_detection_hbox_layout->addWidget(kernel_detection_label_);

    system_application_detection_icon_label_->setFixedSize(DETECTION_ICON_WIDTH, DETECTION_ICON_HEIGHT);
    system_application_detection_icon_label_->SetBackgroundColor(background);
    system_application_detection_icon_label_->SetIcon(QIcon::fromTheme("ukui-bf-security-symbolic"), 0.5);
    system_application_detection_label_->SetText(tr("System application detection"));
    auto *system_application_detection_hbox_layout = new QHBoxLayout;
    system_application_detection_hbox_layout->setSpacing(0);
    system_application_detection_hbox_layout->setContentsMargins(0, 0, 0, 0);
    system_application_detection_hbox_layout->addWidget(system_application_detection_icon_label_);
    system_application_detection_hbox_layout->addSpacing(12);
    system_application_detection_hbox_layout->addWidget(system_application_detection_label_);

    auto *second_column_detection_vbox_layout = new QVBoxLayout;
    second_column_detection_vbox_layout->setSpacing(0);
    second_column_detection_vbox_layout->setContentsMargins(0, 0, 0, 0);
    second_column_detection_vbox_layout->addLayout(kernel_detection_hbox_layout);
    second_column_detection_vbox_layout->addSpacing(16);
    second_column_detection_vbox_layout->addLayout(system_application_detection_hbox_layout);

    // 总体检测项
    auto *detection_hbox_layout = new QHBoxLayout;
    detection_hbox_layout->setSpacing(0);
    detection_hbox_layout->setContentsMargins(0, 0, 0, 0);
    detection_hbox_layout->addLayout(first_column_detection_vbox_layout);
    detection_hbox_layout->addSpacing(36);
    detection_hbox_layout->addLayout(second_column_detection_vbox_layout);
    detection_hbox_layout->addStretch();

    // 消息展示区域
    auto *information_vbox_layout = new QVBoxLayout;
    information_vbox_layout->setSpacing(0);
    information_vbox_layout->setContentsMargins(0, 0, 0, 0);
    information_vbox_layout->addSpacing(11);
    information_vbox_layout->addWidget(describe_label_);
    information_vbox_layout->addSpacing(8);
    information_vbox_layout->addWidget(details_label_);
    information_vbox_layout->addSpacing(32);
    information_vbox_layout->addLayout(detection_hbox_layout);
    information_vbox_layout->addStretch();
    icon_label_->setFixedSize(HOME_PAGE_ICON_WIDTH, HOME_PAGE_ICON_HEIGHT);
    icon_label_->SetPixmap(QPixmap(":/fault_diagnosis/data/icons/home_page_dark.png"));
    auto *information_hbox_layout = new QHBoxLayout;
    information_hbox_layout->setSpacing(0);
    information_hbox_layout->setContentsMargins(0, 0, 0, 0);
    information_hbox_layout->addLayout(information_vbox_layout);
    information_hbox_layout->addWidget(icon_label_);

    // 一键检测按钮
    QFont font;
    font.setPointSizeF(K::Utils::AdaptPixelSizeToPointSize(24));
    one_click_detection_btn_->setFont(font);
    one_click_detection_btn_->setText(tr("One click detection"));
    one_click_detection_btn_->setMinimumSize(180, 52);
    one_click_detection_btn_->setProperty("isImportant", true);
    connect(one_click_detection_btn_, &QPushButton::clicked, this, &HomePageWidget::sig_OneClickDetection);
    auto *one_click_detection_hbox_layout = new QHBoxLayout;
    one_click_detection_hbox_layout->setSpacing(0);
    one_click_detection_hbox_layout->setContentsMargins(0, 0, 0, 0);
    one_click_detection_hbox_layout->addWidget(one_click_detection_btn_);
    one_click_detection_hbox_layout->addStretch();

    // 内网检测
    internal_network_detection_btn_->setText(tr("Internal network detection"));
    connect(internal_network_detection_btn_, &kdk::KBorderlessButton::clicked, this, &HomePageWidget::on_InternalNetworkDetection);
    auto *internal_network_detection_hbox_layout = new QHBoxLayout;
    internal_network_detection_hbox_layout->setSpacing(0);
    internal_network_detection_hbox_layout->setContentsMargins(0, 0, 0, 0);
    internal_network_detection_hbox_layout->addStretch();
    internal_network_detection_hbox_layout->addWidget(internal_network_detection_btn_);

    // 整体布局
    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(40, 152, 40, 40);
    vbox_layout->addLayout(information_hbox_layout);
    vbox_layout->addSpacing(6);
    vbox_layout->addLayout(one_click_detection_hbox_layout);
    vbox_layout->addStretch();
    vbox_layout->addLayout(internal_network_detection_hbox_layout);

    setLayout(vbox_layout);

    connect(kom::UkuiGsettings::getInstance(), &kom::UkuiGsettings::fontSizeChange, this, [this]() {
        QFont font;
        font.setPointSizeF(K::Utils::AdaptPixelSizeToPointSize(24));
        one_click_detection_btn_->setFont(font);
    });
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [this]() {
        QColor background;
        auto theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
        if (theme == "ukui-light" || theme == "ukui-default") {
            background = QColor(230, 230, 230);
        } else {
            background = QColor(56, 56, 56);
        }

        hardware_detection_icon_label_->SetBackgroundColor(background);
        kernel_detection_icon_label_->SetBackgroundColor(background);
        system_service_detection_icon_label_->SetBackgroundColor(background);
        system_application_detection_icon_label_->SetBackgroundColor(background);
    });
}

HomePageWidget::~HomePageWidget() {

}

void HomePageWidget::on_InternalNetworkDetection() {
    ConfigWin internal_network_detection_dialog;

    auto frame_rect = Frame::geometry();
    auto x = frame_rect.x() + frame_rect.width() / 2 - internal_network_detection_dialog.width() / 2;
    auto y = frame_rect.y() + frame_rect.height() / 2 - internal_network_detection_dialog.height() / 2;
    internal_network_detection_dialog.move(x, y);
    internal_network_detection_dialog.activateWindow();
    internal_network_detection_dialog.showWin();
}

}
