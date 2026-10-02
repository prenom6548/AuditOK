/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "font_property_widget.h"
#include "ui_font_property_widget.h"
#include "ukui_settings_monitor.h"

#include <QFrame>



FontPropertyWidget::FontPropertyWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FontPropertyWidget)
{
    ui->setupUi(this);

    ui->boldBtn->setToolTip(tr("Bold"));
    ui->boldBtn->setProperty("useIconHighlightEffect", 0x10);
    ui->italicBtn->setToolTip(tr("Italic"));
    ui->underlineBtn->setToolTip(tr("Underline"));
    ui->strikeOutBtn->setToolTip(tr("Strikeout"));
    ui->unorderedBtn->setToolTip(tr("Unordered"));
    ui->orderedBtn->setToolTip(tr("Ordered"));
    ui->insertBtn->setToolTip(tr("InsertPicture"));

    ui->boldBtn->setIcon(QIcon::fromTheme("format-text-bold-symbolic"));
    ui->italicBtn->setIcon(QIcon::fromTheme("format-text-italic-symbolic"));
    ui->underlineBtn->setIcon(QIcon::fromTheme("format-text-underline-symbolic"));
    ui->strikeOutBtn->setIcon(QIcon::fromTheme("format-text-strikethrough-symbolic"));
    ui->unorderedBtn->setIcon(QIcon::fromTheme("view-list-symbolic"));
    ui->orderedBtn->setIcon(QIcon::fromTheme("ukui-view-list-numbe-symbolic"));
    ui->insertBtn->setIcon(QIcon::fromTheme("folder-pictures-symbolic"));

    ui->boldBtn->setProperty("useIconHighlightEffect", 0x10);
    ui->italicBtn->setProperty("useIconHighlightEffect", 0x10);
    ui->underlineBtn->setProperty("useIconHighlightEffect", 0x10);
    ui->strikeOutBtn->setProperty("useIconHighlightEffect", 0x10);
    ui->unorderedBtn->setProperty("useIconHighlightEffect", 0x10);
    ui->orderedBtn->setProperty("useIconHighlightEffect", 0x10);
    ui->insertBtn->setProperty("useIconHighlightEffect", 0x10);

    ui->insertBtn->setCheckable(false);

    ui->frame->setStyleSheet("QFrame{background-color: palette(Button);}");
    ui->frame->setFixedSize(QSize(1, 21));
    ui->frame->setFrameShape(QFrame::VLine);
    ui->frame->setFrameShadow(QFrame::Raised);

    connect(&UkuiSettingsMonitor::instance(), &UkuiSettingsMonitor::tabletModeUpdate,
                     this, &FontPropertyWidget::handleTabletMode);

}

FontPropertyWidget::~FontPropertyWidget()
{
    delete ui;
}

FontPropertyPushButton *FontPropertyWidget::boldBtn()
{
    return ui->boldBtn;
}

FontPropertyPushButton *FontPropertyWidget::italicBtn()
{
    return ui->italicBtn;
}

FontPropertyPushButton *FontPropertyWidget::underlineBtn()
{
    return ui->underlineBtn;
}

FontPropertyPushButton *FontPropertyWidget::strikeOutBtn()
{
    return ui->strikeOutBtn;
}

FontPropertyPushButton *FontPropertyWidget::unorderedBtn()
{
    return ui->unorderedBtn;
}

FontPropertyPushButton *FontPropertyWidget::orderedBtn()
{
    return ui->orderedBtn;
}

FontPropertyPushButton *FontPropertyWidget::insertBtn()
{
    return ui->insertBtn;
}

void FontPropertyWidget::showEvent(QShowEvent *event)
{
    handleTabletMode(UkuiSettingsMonitor::instance().tabletMode());
    QWidget::showEvent(event);
}

void FontPropertyWidget::handleTabletMode(UkuiSettingsMonitor::TabletStatus status)
{
    switch (status) {
    case UkuiSettingsMonitor::TabletStatus::PCMode:
        handlePCMode();
        break;
    case UkuiSettingsMonitor::TabletStatus::PADHorizontalMode:
    case UkuiSettingsMonitor::TabletStatus::PADVerticalMode:
        handlePADMode();
        break;
    default:
        break;
    }
}
void FontPropertyWidget::handlePADMode()
{
    ui->frame->hide();
    ui->buttonLayout->setSpacing(16);
}

void FontPropertyWidget::handlePCMode()
{
    ui->frame->show();
    ui->buttonLayout->setSpacing(0);
}
