#include <QDebug>
#include <QHBoxLayout>
#include <QApplication>
#include <QPalette>
#include <gsettingmonitor.h>
#include "kom-ukui-gsettings.h"
#include "utils.h"
#include "frame.h"
#include "config_win.h"
#include "home_page_widget.h"

namespace fault_diagnosis {

const static int HOME_PAGE_ICON_WIDTH = 256;
const static int HOME_PAGE_ICON_HEIGHT = 256;

CategoryDiagnosisBtn::CategoryDiagnosisBtn(const QString &icon, const QString &name, const QString desc, QWidget *parent)
    : kdk::KPushButton(parent),
      icon_btn_(new QPushButton(this)),
      name_label_(new K::TextLabel(this)),
      desc_label_(new K::TextLabel(this))
{
    setMinimumSize(232, 156);

    QIcon target_icon;
    if (icon.contains("/")) {
        // 路径
        QIcon temp_icon(icon);
        target_icon.swap(temp_icon);
    } else {
        // 从主题中获取
        target_icon = QIcon::fromTheme(icon);
    }

    icon_btn_->setAttribute(Qt::WA_TransparentForMouseEvents);
    icon_btn_->setFlat(true);
    icon_btn_->setFixedSize(48, 48);
    icon_btn_->setIconSize(QSize(48, 48));
    icon_btn_->setIcon(target_icon);
    auto *icon_hbox_layout = new QHBoxLayout;
    icon_hbox_layout->setSpacing(0);
    icon_hbox_layout->setContentsMargins(0, 0, 0, 0);
    icon_hbox_layout->addWidget(icon_btn_);
    icon_hbox_layout->addStretch();

    name_label_->SetPixelSize(16, true);
    name_label_->SetText(name);
    desc_label_->SetFontColor(qApp->palette().color(QPalette::PlaceholderText));
    desc_label_->SetText(desc);

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(24, 24, 24, 21);
    vbox_layout->addLayout(icon_hbox_layout);
    vbox_layout->addSpacing(16);
    vbox_layout->addWidget(name_label_);
    vbox_layout->addSpacing(2);
    vbox_layout->addWidget(desc_label_);
    setLayout(vbox_layout);
}

CategoryDiagnosisBtn::~CategoryDiagnosisBtn() {

}

HomePageWidget::HomePageWidget()
    : describe_label_(new K::TextLabel(this)),
      details_label_(new K::TextLabel(this)),
      icon_label_(new K::IconLabel(this)),
      one_click_detection_btn_(new QPushButton(this)),
      internal_network_detection_btn_(new kdk::KBorderlessButton(this))
{
    describe_label_->SetText(tr("One click troubleshooting of computer malfunctions"));
    describe_label_->SetPixelSize(36, true);
    details_label_->SetText(tr("Quick detection and repair of fault issues, safeguarding the health of your computer"));
    details_label_->SetPixelSize(16);

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

    auto *information_vbox_layout = new QVBoxLayout;
    information_vbox_layout->setSpacing(0);
    information_vbox_layout->setContentsMargins(0, 0, 0, 0);
    information_vbox_layout->addStretch();
    information_vbox_layout->addWidget(describe_label_);
    information_vbox_layout->addSpacing(8);
    information_vbox_layout->addWidget(details_label_);
    information_vbox_layout->addSpacing(40);
    information_vbox_layout->addLayout(one_click_detection_hbox_layout);
    information_vbox_layout->addStretch();

    // 图标
    icon_label_->setFixedSize(HOME_PAGE_ICON_WIDTH, HOME_PAGE_ICON_HEIGHT);
    icon_label_->SetPixmap(QPixmap(":/fault_diagnosis/data/icons/home_page_dark.png"));

    auto *information_hbox_layout = new QHBoxLayout;
    information_hbox_layout->setSpacing(0);
    information_hbox_layout->setContentsMargins(0, 0, 0, 0);
    information_hbox_layout->addLayout(information_vbox_layout);
    information_hbox_layout->addSpacing(16);
    information_hbox_layout->addWidget(icon_label_);

    // 分类检测
    auto *network_diagnosis_btn = new CategoryDiagnosisBtn("ukui-network-agent-symbolic",
                                                           tr("Network detection"),
                                                           tr("Troubleshooting issues with inablilty to access the internet"),
                                                           this);
    auto *performance_component_diagnosis_btn = new CategoryDiagnosisBtn("ukui-bf-fast-symbolic",
                                                                         tr("System performance and component detection"),
                                                                         tr("Hardware health and system performance issues"),
                                                                         this);
    auto *application_diagnosis_btn = new CategoryDiagnosisBtn("ukui-defaultapp-symbolic",
                                                               tr("Application detection"),
                                                               tr("Application caching and permission settings"),
                                                               this);
    connect(network_diagnosis_btn, &kdk::KPushButton::clicked, this, &HomePageWidget::sig_NetworkDetection);
    connect(performance_component_diagnosis_btn, &kdk::KPushButton::clicked, this, &HomePageWidget::sig_PerformanceAndComponentsDetection);
    connect(application_diagnosis_btn, &kdk::KPushButton::clicked, this, &HomePageWidget::sig_ApplicationDetection);

    auto *category_diagnosis_hbox_layout = new QHBoxLayout;
    category_diagnosis_hbox_layout->setSpacing(0);
    category_diagnosis_hbox_layout->setContentsMargins(0, 0, 0, 0);
    category_diagnosis_hbox_layout->addWidget(network_diagnosis_btn);
    category_diagnosis_hbox_layout->addSpacing(24);
    category_diagnosis_hbox_layout->addWidget(performance_component_diagnosis_btn);
    category_diagnosis_hbox_layout->addSpacing(24);
    category_diagnosis_hbox_layout->addWidget(application_diagnosis_btn);

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
    vbox_layout->setContentsMargins(40, 40, 40, 40);
    vbox_layout->addLayout(information_hbox_layout);
    vbox_layout->addSpacing(40);
    vbox_layout->addLayout(category_diagnosis_hbox_layout);
    vbox_layout->addStretch();
    vbox_layout->addLayout(internal_network_detection_hbox_layout);

    setLayout(vbox_layout);

    connect(kom::UkuiGsettings::getInstance(), &kom::UkuiGsettings::fontSizeChange, this, [this]() {
        QFont font;
        font.setPointSizeF(K::Utils::AdaptPixelSizeToPointSize(24));
        one_click_detection_btn_->setFont(font);
    });
}

HomePageWidget::~HomePageWidget() {

}

void HomePageWidget::SetState(enum State state) {
    switch (state) {
        case HomePageWidget::State::kInit: {
            one_click_detection_btn_->setText(tr("One click detection"));
            break;
        }
        case HomePageWidget::State::kPreparing: {
            one_click_detection_btn_->setText(tr("Preparing..."));
            break;
        }
        default:
            break;
    }
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
