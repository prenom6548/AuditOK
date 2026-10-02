#include "setup_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include "text_label.h"
#include "kom-configure.h"

SetupWidget::SetupWidget(QWidget *parent)
    : kdk::KDialog(parent),
      allow_popup_check_box_(new QCheckBox(this))
{
    setFixedSize(452, 229);
    setWindowIcon(QIcon::fromTheme("kylin-os-manager"));
    setWindowTitle(tr("Kylin Os Manager"));

    auto *title_lable = new K::TextLabel(this);
    title_lable->SetPixelSize(16, true);
    title_lable->SetText(tr("Pop up settings"));
    auto *title_hbox_layout = new QHBoxLayout;
    title_hbox_layout->setContentsMargins(0, 0, 0, 0);
    title_hbox_layout->setSpacing(0);
    title_hbox_layout->addWidget(title_lable);
    title_hbox_layout->addStretch();

    allow_popup_check_box_->setFixedSize(20, 20);
    allow_popup_check_box_->setObjectName("Kylin-os-manager_QCheckBox_AllowPopup");
    allow_popup_check_box_->setAccessibleName(allow_popup_check_box_->objectName());
    auto *allow_popup_text_label = new K::TextLabel(this);
    allow_popup_text_label->SetText(tr("Allow pop up prompts"));
    auto *allow_popup_hbox_layout = new QHBoxLayout;
    allow_popup_hbox_layout->setContentsMargins(0, 0, 0, 0);
    allow_popup_hbox_layout->setSpacing(0);
    allow_popup_hbox_layout->addWidget(allow_popup_check_box_);
    allow_popup_hbox_layout->addSpacing(8);
    allow_popup_hbox_layout->addWidget(allow_popup_text_label);
    allow_popup_hbox_layout->addStretch();

    auto *allow_popup_desc_label = new K::TextLabel(this);
    allow_popup_desc_label->SetFontColor(K::TextLabel::FontColor::kPlaceholderText);
    allow_popup_desc_label->SetText(tr("Program crashes, insufficient disk capacity, abnormal log files, and other prompts"));
    auto *allow_popup_desc_hbox_layout = new QHBoxLayout;
    allow_popup_desc_hbox_layout->setContentsMargins(0, 0, 0, 0);
    allow_popup_desc_hbox_layout->setSpacing(0);
    allow_popup_desc_hbox_layout->addSpacing(28);
    allow_popup_desc_hbox_layout->addWidget(allow_popup_desc_label);
    allow_popup_desc_hbox_layout->addStretch();

    auto *cancel_btn = new QPushButton(this);
    cancel_btn->setText(tr("Cancel"));
    connect(cancel_btn, &QPushButton::clicked, this, [this]() {
        close();
    });
    auto *ok_btn = new QPushButton(this);
    ok_btn->setProperty("isImportant", true);
    ok_btn->setText(tr("Ok"));
    connect(ok_btn, &QPushButton::clicked, this, &SetupWidget::on_Ok);
    auto *btn_hbox_layout = new QHBoxLayout;
    btn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    btn_hbox_layout->setSpacing(0);
    btn_hbox_layout->addStretch();
    btn_hbox_layout->addWidget(cancel_btn);
    btn_hbox_layout->addSpacing(8);
    btn_hbox_layout->addWidget(ok_btn);

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(24, 8, 24, 24);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(title_hbox_layout);
    vbox_layout->addSpacing(8);
    vbox_layout->addLayout(allow_popup_hbox_layout);
    vbox_layout->addLayout(allow_popup_desc_hbox_layout);
    vbox_layout->addStretch();
    vbox_layout->addLayout(btn_hbox_layout);
    mainWidget()->setLayout(vbox_layout);

    // 初始化
    kom::Configure conf;
    auto allow_popup = conf.value(CONFIG_SETUP, CONFIG_SETUP_ALLOW_POPUP, true).toBool();
    if (allow_popup) {
        allow_popup_check_box_->setCheckState(Qt::CheckState::Checked);
    } else {
        allow_popup_check_box_->setCheckState(Qt::CheckState::Unchecked);
    }
}

SetupWidget::~SetupWidget() {

}

void SetupWidget::on_Ok() {
    kom::Configure conf;
    auto allow_popup_check_state = allow_popup_check_box_->checkState();
    if (allow_popup_check_state == Qt::CheckState::Checked) {
        conf.setValue(CONFIG_SETUP, CONFIG_SETUP_ALLOW_POPUP, true);
    } else {
        conf.setValue(CONFIG_SETUP, CONFIG_SETUP_ALLOW_POPUP, false);
    }

    close();
}

