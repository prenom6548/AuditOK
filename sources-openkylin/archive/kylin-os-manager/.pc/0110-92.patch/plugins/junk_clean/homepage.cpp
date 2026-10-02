#include "homepage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QColor>
#include <QFont>
#include <QPalette>
#include <QApplication>
#include <QDebug>
#include <gsettingmonitor.h>
#include "utils.h"

namespace junk_clean {

Homepage::Homepage(QWidget *parent)
    : QWidget(parent),
      title_label_(new K::TextLabel(this)),
      describe_label_(new K::TextLabel(this)),
      system_junk_icon_label_(new K::IconLabel(this)),
      system_junk_name_label_(new K::TextLabel(this)),
      system_junk_describe_label_(new K::TextLabel(this)),
      internet_junk_icon_label_(new K::IconLabel(this)),
      internet_junk_name_label_(new K::TextLabel(this)),
      internet_junk_describe_label_(new K::TextLabel(this)),
      usage_trace_icon_label_(new K::IconLabel(this)),
      usage_trace_name_label_(new K::TextLabel(this)),
      usage_trace_describe_label_(new K::TextLabel(this)),
      start_scan_btn_(new QPushButton(this)),
      homepage_icon_label_(new K::IconLabel(this))
{
    title_label_->SetPixelSize(36, true);
    title_label_->SetText(tr("One click cleaning of computer junk"));
    describe_label_->SetPixelSize(16);
    describe_label_->SetText(tr("Regular cleaning makes your computer light, fast, and safe"));

    QColor background;
    QColor junk_describe_color;
    auto theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
    if (theme == "ukui-light" || theme == "ukui-default") {
        background = QColor(230, 230, 230);
        junk_describe_color = QColor(0, 0, 0);
        junk_describe_color.setAlphaF(0.35);
    } else {
        background = QColor(56, 56, 56);
        junk_describe_color = QColor(255, 255, 255);
        junk_describe_color.setAlphaF(0.35);
    }

    system_junk_icon_label_->setFixedSize(36, 36);
    system_junk_icon_label_->SetBackgroundColor(background);
    system_junk_icon_label_->SetIcon(QIcon::fromTheme("applications-system-symbolic"), 0.5);
    system_junk_name_label_->SetText(tr("System garbage"));
    system_junk_describe_label_->SetFontColor(junk_describe_color);
    system_junk_describe_label_->SetText(tr("Clean up the garbage generated during the daily use of the system"));
    auto *system_junk_vbox_layout = new QVBoxLayout;
    system_junk_vbox_layout->setContentsMargins(0, 0, 0, 0);
    system_junk_vbox_layout->setSpacing(0);
    system_junk_vbox_layout->addWidget(system_junk_name_label_);
    system_junk_vbox_layout->addWidget(system_junk_describe_label_);
    auto *system_junk_hbox_layout = new QHBoxLayout;
    system_junk_hbox_layout->setContentsMargins(0, 0, 0, 0);
    system_junk_hbox_layout->setSpacing(0);
    system_junk_hbox_layout->addWidget(system_junk_icon_label_);
    system_junk_hbox_layout->addSpacing(12);
    system_junk_hbox_layout->addLayout(system_junk_vbox_layout);
    system_junk_hbox_layout->addStretch();

    internet_junk_icon_label_->setFixedSize(36, 36);
    internet_junk_icon_label_->SetBackgroundColor(background);
    internet_junk_icon_label_->SetIcon(QIcon::fromTheme("ukui-network-agent-symbolic"), 0.5);
    internet_junk_name_label_->SetText(tr("Interface garbage"));
    internet_junk_describe_label_->SetFontColor(junk_describe_color);
    internet_junk_describe_label_->SetText(tr("Cleaning browser generated cache files and login information"));
    auto *internet_junk_vbox_layout = new QVBoxLayout;
    internet_junk_vbox_layout->setContentsMargins(0, 0, 0, 0);
    internet_junk_vbox_layout->setSpacing(0);
    internet_junk_vbox_layout->addWidget(internet_junk_name_label_);
    internet_junk_vbox_layout->addWidget(internet_junk_describe_label_);
    auto *internet_junk_hbox_layout = new QHBoxLayout;
    internet_junk_hbox_layout->setContentsMargins(0, 0, 0, 0);
    internet_junk_hbox_layout->setSpacing(0);
    internet_junk_hbox_layout->addWidget(internet_junk_icon_label_);
    internet_junk_hbox_layout->addSpacing(12);
    internet_junk_hbox_layout->addLayout(internet_junk_vbox_layout);
    internet_junk_hbox_layout->addStretch();

    usage_trace_icon_label_->setFixedSize(36, 36);
    usage_trace_icon_label_->SetBackgroundColor(background);
    usage_trace_icon_label_->SetIcon(QIcon::fromTheme("document-open-recent-symbolic"), 0.5);
    usage_trace_name_label_->SetText(tr("Usage traces"));
    usage_trace_describe_label_->SetFontColor(junk_describe_color);
    usage_trace_describe_label_->SetText(tr("Clean up computer usage traces and protect privacy and security"));
    auto *usage_trace_vbox_layout = new QVBoxLayout;
    usage_trace_vbox_layout->setContentsMargins(0, 0, 0, 0);
    usage_trace_vbox_layout->setSpacing(0);
    usage_trace_vbox_layout->addWidget(usage_trace_name_label_);
    usage_trace_vbox_layout->addWidget(usage_trace_describe_label_);
    auto *usage_trace_hbox_layout = new QHBoxLayout;
    usage_trace_hbox_layout->setContentsMargins(0, 0, 0, 0);
    usage_trace_hbox_layout->setSpacing(0);
    usage_trace_hbox_layout->addWidget(usage_trace_icon_label_);
    usage_trace_hbox_layout->addSpacing(12);
    usage_trace_hbox_layout->addLayout(usage_trace_vbox_layout);
    usage_trace_hbox_layout->addStretch();

    QFont font;
    font.setPointSizeF(K::Utils::AdaptPixelSizeToPointSize(24));
    start_scan_btn_->setFont(font);
    start_scan_btn_->setMinimumSize(180, 52);
    start_scan_btn_->setText(tr("Start scan"));
    start_scan_btn_->setProperty("isImportant", true);
    auto *start_sacn_hbox_layout = new QHBoxLayout;
    start_sacn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    start_sacn_hbox_layout->setSpacing(0);
    start_sacn_hbox_layout->addWidget(start_scan_btn_);
    start_sacn_hbox_layout->addStretch();
    connect(start_scan_btn_, &QPushButton::clicked, this, &Homepage::sig_StartScan);

    auto *left_vbox_layout = new QVBoxLayout;
    left_vbox_layout->setContentsMargins(0, 0, 0, 0);
    left_vbox_layout->setSpacing(0);
    left_vbox_layout->addLayout(system_junk_hbox_layout);
    left_vbox_layout->addSpacing(16);
    left_vbox_layout->addLayout(internet_junk_hbox_layout);
    left_vbox_layout->addSpacing(16);
    left_vbox_layout->addLayout(usage_trace_hbox_layout);
    left_vbox_layout->addSpacing(40);
    left_vbox_layout->addLayout(start_sacn_hbox_layout);

    homepage_icon_label_->setFixedSize(256, 256);
    if (theme == "ukui-light" || theme == "ukui-default") {
        homepage_icon_label_->SetPixmap(QPixmap(":/data/icons/home_page_icon_light.svg"));
    } else {
        homepage_icon_label_->SetPixmap(QPixmap(":/data/icons/home_page_icon_dark.svg"));
    }
    auto *right_vbox_layout = new QVBoxLayout;
    right_vbox_layout->setContentsMargins(0, 0, 0, 0);
    right_vbox_layout->setSpacing(0);
    right_vbox_layout->addWidget(homepage_icon_label_);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setContentsMargins(0, 0, 0, 0);
    hbox_layout->setSpacing(0);
    hbox_layout->addLayout(left_vbox_layout);
    hbox_layout->addSpacing(16);
    hbox_layout->addLayout(right_vbox_layout);

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(40, 100, 40, 0);
    vbox_layout->setSpacing(0);
    vbox_layout->addWidget(title_label_);
    vbox_layout->addSpacing(8);
    vbox_layout->addWidget(describe_label_);
    vbox_layout->addSpacing(24);
    vbox_layout->addLayout(hbox_layout);
    vbox_layout->addStretch();

    setLayout(vbox_layout);

    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [this]() {
        QFont font;
        font.setPointSizeF(K::Utils::AdaptPixelSizeToPointSize(24));
        start_scan_btn_->setFont(font);
    });
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [this]() {
        QColor background;
        QColor junk_describe_color;
        auto theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
        if (theme == "ukui-light" || theme == "ukui-default") {
            background = QColor(230, 230, 230);
            junk_describe_color = QColor(0, 0, 0);
            junk_describe_color.setAlphaF(0.35);
            homepage_icon_label_->SetPixmap(QPixmap(":/data/icons/home_page_icon_light.svg"));
        } else {
            background = QColor(56, 56, 56);
            junk_describe_color = QColor(255, 255, 255);
            junk_describe_color.setAlphaF(0.35);
            homepage_icon_label_->SetPixmap(QPixmap(":/data/icons/home_page_icon_dark.svg"));
        }

        system_junk_icon_label_->SetBackgroundColor(background);
        internet_junk_icon_label_->SetBackgroundColor(background);
        usage_trace_icon_label_->SetBackgroundColor(background);
        system_junk_describe_label_->SetFontColor(junk_describe_color);
        internet_junk_describe_label_->SetFontColor(junk_describe_color);
        usage_trace_describe_label_->SetFontColor(junk_describe_color);
    });
}

}
