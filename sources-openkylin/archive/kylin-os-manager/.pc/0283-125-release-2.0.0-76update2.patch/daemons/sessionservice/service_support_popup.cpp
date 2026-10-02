#include "service_support_popup.h"
#include <QIcon>
#include <QLabel>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include <kborderlessbutton.h>
#include <kbackgroundgroup.h>
#include "text_label.h"

ServiceSupportPopup::ServiceSupportPopup(QWidget *parent)
    : kdk::KWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setFixedSize(455, 328);
    setIcon(QIcon::fromTheme("kylin-os-manager"));
    setWidgetName(tr("Kylin Os Manager"));
    windowButtonBar()->minimumButton()->hide();
    windowButtonBar()->maximumButton()->hide();
    windowButtonBar()->menuButton()->hide();

    // 隐藏任务栏图标
    const KWindowInfo window_info(winId(), NET::WMState);
    if (!window_info.hasState(NET::SkipTaskbar) || !window_info.hasState(NET::SkipPager)) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    }

    // 图标
    auto *icon_label = new QLabel(this);
    icon_label->setFixedSize(16, 16);
    icon_label->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16, 16));

    // 描述
    auto *desc_label = new K::TextLabel(this);
    desc_label->SetText(tr("Detected system abnormality, please contact the technical service immediately or call 400!"));

    auto *desc_hbox_layout = new QHBoxLayout;
    desc_hbox_layout->setContentsMargins(0, 0, 0, 0);
    desc_hbox_layout->setSpacing(0);
    desc_hbox_layout->addWidget(icon_label);
    desc_hbox_layout->addSpacing(8);
    desc_hbox_layout->addWidget(desc_label);
    desc_hbox_layout->addStretch();

    // 服务支持
    auto *service_support_widget = new QWidget(this);
    service_support_widget->setFixedHeight(144);

    // 联系我们
    auto *contact_us_label = new K::TextLabel(service_support_widget);
    contact_us_label->SetPixelSize(14, true);
    contact_us_label->SetText(tr("Or contact us"));

    // 电话和邮箱
    auto *telephone_label = new K::TextLabel(service_support_widget);
    telephone_label->SetText(tr("Telephone"));

    auto *telephone_body_label = new K::TextLabel(service_support_widget);
    telephone_body_label->SetText("400-089-1870");

    auto *mail_label = new K::TextLabel(service_support_widget);
    mail_label->SetText(tr("Mail"));

    auto *mail_body_label = new K::TextLabel(service_support_widget);
    mail_body_label->SetText("support@kylinos.cn");

    auto *telephone_mail_grid_layout = new QGridLayout;
    telephone_mail_grid_layout->setContentsMargins(0, 0, 0, 0);
    telephone_mail_grid_layout->setHorizontalSpacing(16);
    telephone_mail_grid_layout->setVerticalSpacing(8);
    telephone_mail_grid_layout->addWidget(telephone_label, 0, 0);
    telephone_mail_grid_layout->addWidget(telephone_body_label, 0, 1);
    telephone_mail_grid_layout->addWidget(mail_label, 1, 0);
    telephone_mail_grid_layout->addWidget(mail_body_label, 1, 1);

    auto *telephone_mail_hbox_layout = new QHBoxLayout;
    telephone_mail_hbox_layout->setContentsMargins(0, 0, 0, 0);
    telephone_mail_hbox_layout->setSpacing(0);
    telephone_mail_hbox_layout->addLayout(telephone_mail_grid_layout);
    telephone_mail_hbox_layout->addStretch();

    // 官网
    auto *comm_problem_label = new K::TextLabel(service_support_widget);
    comm_problem_label->SetText(tr("For common questions, please refer to"));

    auto *offcial_website_btn = new kdk::KBorderlessButton(service_support_widget);
    offcial_website_btn->setText(tr("KylinOs offical website"));
    connect(offcial_website_btn, &kdk::KBorderlessButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://www.kylinos.cn/"));
    });

    auto *comm_problem_hbox_layout = new QHBoxLayout;
    comm_problem_hbox_layout->setContentsMargins(0, 0, 0, 0);
    comm_problem_hbox_layout->setSpacing(0);
    comm_problem_hbox_layout->addWidget(comm_problem_label);
    comm_problem_hbox_layout->addWidget(offcial_website_btn);

    auto *information_vbox_layout = new QVBoxLayout;
    information_vbox_layout->setContentsMargins(0, 0, 0, 0);
    information_vbox_layout->setSpacing(0);
    information_vbox_layout->addWidget(contact_us_label);
    information_vbox_layout->addSpacing(8);
    information_vbox_layout->addLayout(telephone_mail_hbox_layout);
    information_vbox_layout->addSpacing(8);
    information_vbox_layout->addLayout(comm_problem_hbox_layout);

    // 二维码
    auto *QR_label = new QLabel(service_support_widget);
    QR_label->setFixedSize(110, 110);
    QR_label->setPixmap(QPixmap(":/data/icons/service_support_QR.png").scaled(110, 110, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));

    auto *service_support_hbox_layout = new QHBoxLayout;
    service_support_hbox_layout->setContentsMargins(16, 16, 16, 16);
    service_support_hbox_layout->setSpacing(0);
    service_support_hbox_layout->addLayout(information_vbox_layout);
    service_support_hbox_layout->addStretch();
    service_support_hbox_layout->addSpacing(8);
    service_support_hbox_layout->addWidget(QR_label);
    service_support_widget->setLayout(service_support_hbox_layout);

    // 将服务支持放到背景卡片中
    auto *service_support_card = new kdk::KBackgroundGroup(this);
    service_support_card->setBackgroundRole(QPalette::ColorRole::Window);
    service_support_card->addWidget(service_support_widget);

    // 按钮
    auto *cache_btn = new QPushButton(this);
    cache_btn->setText(tr("Cancel"));
    connect(cache_btn, &QPushButton::clicked, this, [this]() {
        close();
    });

    auto *clean_btn = new QPushButton(this);
    clean_btn->setProperty("isImportant", true);
    clean_btn->setText(tr("Clean"));
    connect(clean_btn, &QPushButton::clicked, this, [this]() {
        QProcess process;
        process.setProgram("kylin-os-manager");
        process.setArguments({"--jumpTab=JunkClean", "--pluginMessage=MaxDiskCapacity"});
        process.startDetached();
        close();
    });

    auto *btn_hbox_layout = new QHBoxLayout;
    btn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    btn_hbox_layout->setSpacing(0);
    btn_hbox_layout->addStretch();
    btn_hbox_layout->addWidget(cache_btn);
    btn_hbox_layout->addSpacing(8);
    btn_hbox_layout->addWidget(clean_btn);

    // 总布局
    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(24, 8, 24, 24);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(desc_hbox_layout);
    vbox_layout->addSpacing(8);
    vbox_layout->addWidget(service_support_card);
    vbox_layout->addStretch();
    vbox_layout->addLayout(btn_hbox_layout);
    baseBar()->setLayout(vbox_layout);
}

ServiceSupportPopup::~ServiceSupportPopup() {

}
