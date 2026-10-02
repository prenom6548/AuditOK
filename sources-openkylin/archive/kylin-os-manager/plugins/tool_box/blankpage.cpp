#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <gsettingmonitor.h>
#include "kom-ukui-gsettings.h"
#include "utils.h"
#include "frame.h"
#include "blankpage.h"

namespace tool_box {
BlankPage::BlankPage(QWidget *parent)
    : QWidget(parent)
    , icon_label_(new K::IconLabel(this))
    , text_label_(new K::TextLabel(this))
{
    // 图标
    icon_label_->setFixedSize(128, 128);
    changeThemeIcon();
    
    // 文字
    text_label_->SetPixelSize(14);
    text_label_->SetText(tr("No tools found"));
    text_label_->SetFontColor(QColor("#595959"));

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addSpacing(100);
    layout->addWidget(icon_label_, 0, Qt::AlignHCenter);
    layout->addWidget(text_label_, 0, Qt::AlignHCenter);
    layout->addSpacing(240);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &BlankPage::changeThemeIcon);

}

void BlankPage::changeThemeIcon()
{
    const auto theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
    if (theme == "ukui-default" || theme == "ukui-light") {
        icon_label_->SetPixmap(QPixmap(":/fault_diagnosis/data/icons/no_tool_light.svg"));
    } else {
        icon_label_->SetPixmap(QPixmap(":/fault_diagnosis/data/icons/no_tool_dark.svg"));
    }
}
}