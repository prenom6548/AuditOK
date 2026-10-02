#include "computer_treasure_chest_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QPalette>
#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVariant>
#include <frame.h>
#include "text_label.h"
#include <gsettingmonitor.h>

namespace home_page {

ComputerTreasureChestWidget::ComputerTreasureChestWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(237, 342);

    auto *title_label = new K::TextLabel(this);
    title_label->SetPixelSize(16, true);
    title_label->SetText(tr("Computer treasure chest"));
    auto *title_hbox_layout = new QHBoxLayout;
    title_hbox_layout->setContentsMargins(0, 0, 0, 0);
    title_hbox_layout->setSpacing(0);
    title_hbox_layout->addStretch();
    title_hbox_layout->addWidget(title_label);
    title_hbox_layout->addStretch();

    auto *icon_label = new QLabel(this);
    icon_label->setFixedSize(122, 145);
    // 根据当前主题设置图片的函数
    auto updateIconForTheme = [icon_label]() {
        const auto current_theme = kdk::GsettingMonitor::getSystemTheme();
        QString icon_path;
        if (current_theme == "ukui-default" || current_theme == "ukui-light") {
            // 浅色主题
            icon_path = ":/home_page/data/icons/default_computer_treasure_chest.svg";
        } else {
            // 深色主题
            icon_path = ":/home_page/data/icons/default_computer_treasure_chest_dark.svg";
        }
        auto pixmap = QPixmap(icon_path).scaled(122, 145, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
        icon_label->setPixmap(pixmap);
        icon_label->setAlignment(Qt::AlignCenter);
    };
    // 初始设置图片
    updateIconForTheme();

    // 监听主题变化
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, updateIconForTheme);

    auto *icon_hbox_layout = new QHBoxLayout;
    icon_hbox_layout->setContentsMargins(0, 0, 0, 0);
    icon_hbox_layout->setSpacing(0);
    icon_hbox_layout->addStretch();
    icon_hbox_layout->addWidget(icon_label);
    icon_hbox_layout->addStretch();

    auto *desc_label = new K::TextLabel(this);
    desc_label->SetFontColor(K::TextLabel::FontColor::kPlaceholderText);
    desc_label->SetText(tr("Multiple small tools waiting for you to discover"));
    auto *desc_hbox_layout = new QHBoxLayout;
    desc_hbox_layout->setContentsMargins(0, 0, 0, 0);
    desc_hbox_layout->setSpacing(0);
    desc_hbox_layout->addStretch();
    desc_hbox_layout->addWidget(desc_label);
    desc_hbox_layout->addStretch();

    auto *go_to_btn = new QPushButton(this);
    go_to_btn->setProperty("isImportant", true);
    go_to_btn->setText(tr("Go to the treasure chest"));
    auto *go_to_hbox_layout = new QHBoxLayout;
    go_to_hbox_layout->setContentsMargins(0, 0, 0, 0);
    go_to_hbox_layout->setSpacing(0);
    go_to_hbox_layout->addStretch();
    go_to_hbox_layout->addWidget(go_to_btn);
    go_to_hbox_layout->addStretch();
    connect(go_to_btn, &QPushButton::clicked, this, []() {
        Frame::SwtichToToolBox();
    });

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(0, 24, 0, 32);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(title_hbox_layout);
    vbox_layout->addSpacing(18);
    vbox_layout->addLayout(icon_hbox_layout);
    vbox_layout->addSpacing(15);
    vbox_layout->addLayout(desc_hbox_layout);
    vbox_layout->addStretch();
    vbox_layout->addLayout(go_to_hbox_layout);
    setLayout(vbox_layout);
}

ComputerTreasureChestWidget::~ComputerTreasureChestWidget() {

}

void ComputerTreasureChestWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = qApp->palette().color(QPalette::Window);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setOpacity(0.8);
    painter.setPen(Qt::transparent);
    QStyleOption opt;
    opt.init(this);
    painter.drawRoundedRect(opt.rect, 8, 8);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}

}
