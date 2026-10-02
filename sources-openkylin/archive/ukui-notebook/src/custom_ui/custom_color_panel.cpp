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

#include "custom_color_panel.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>
#include <QStyle>

QString CustomColorPanel::BLACK = "#000000";
QString CustomColorPanel::WHITE = "#FFFFFF";


CustomColorPanel::CustomColorPanel(QWidget *parent)
    : kdk::KTranslucentFloor(parent)
{
    setEnableBlur(false);
    setShadow(true);
    setBorderRadius(8);
    init();
    connect(&UkuiSettingsMonitor::instance(), &UkuiSettingsMonitor::styleStatusUpdate,
            this, &CustomColorPanel::handleStyleStatus);
    connect(&UkuiSettingsMonitor::instance(), &UkuiSettingsMonitor::tabletModeUpdate,
            this, &CustomColorPanel::handleTabletMode);
    handleStyleStatus(UkuiSettingsMonitor::instance().styleStatus());
    handleTabletMode(UkuiSettingsMonitor::instance().tabletMode());
}

void CustomColorPanel::addColor(const QColor& color, bool isClicked)
{
    kdk::KColorButton *btn = new kdk::KColorButton(this);
    btn->setFixedSize(16,16);
    btn->setBackgroundColor(color);
    btn->setButtonType(kdk::KColorButton::CheckedRect);
    if (isClicked == true) {
        setSelectColorButton(btn);
    }
    m_layout->addWidget(btn);
    m_group->addButton(btn);
    m_buttons.append(btn);
    if (isBlackOrWhite(color)) {
        if (m_blackOrWhiteButton != nullptr) {
            qDebug() << "Black-or-white button is set repeatedly!";
        }
        m_blackOrWhiteButton = btn;
    }
    handleTabletMode(UkuiSettingsMonitor::instance().tabletMode());
}

void CustomColorPanel::init()
{
    m_layout = new QVBoxLayout(this);
    m_group = new QButtonGroup(this);
    connect(m_group, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),[=](QAbstractButton *button){
        kdk::KColorButton* btn =static_cast<kdk::KColorButton*>(button);
        emit currentColorChanged(btn->backgroundColor());
    });
}

void CustomColorPanel::setSelectColorButton(kdk::KColorButton *button)
{
    button->click();
    m_selectButton = button;
}

void CustomColorPanel::setSelectColor(const QColor& color)
{
    if (isBlackOrWhite(color) && m_blackOrWhiteButton) {
        setSelectColorButton(m_blackOrWhiteButton);
        return ;
    }
    for (auto button : m_buttons) {
        if (button->backgroundColor() == color) {
            setSelectColorButton(button);
            return ;
        }
    }
    qDebug() << "Cannot find the button!";
}

void CustomColorPanel::showEvent(QShowEvent *event)
{
    if (parent() != nullptr) {
        QWidget *parentWidget = static_cast<QWidget*>(parent());
        qDebug() << parentWidget->size();
        QPoint size = parentWidget->mapToGlobal(QPoint(0,0));
        QPoint temp = QPoint(
            size.x() + parentWidget->width() - this->width(),
            size.y() + parentWidget->height());

        QDesktopWidget* desktopWidget = QApplication::desktop();
        QRect clientRect = desktopWidget->availableGeometry();

        if(height() + temp.y() < clientRect.height()) {
            this->move(temp);
        }
        else {
            this->move(QPoint(size.x(), size.y() - this->height()));
        }

    }

    kdk::KTranslucentFloor::showEvent(event);
}

void CustomColorPanel::handleStyleStatus(UkuiSettingsMonitor::StyleStatus status)
{
    switch (status) {
    case UkuiSettingsMonitor::StyleStatus::LIGHT:
        handleLightMode();
        break;
    case UkuiSettingsMonitor::StyleStatus::DARK:
        handleDarkMode();
        break;
    default:
        break;
    }
}

void CustomColorPanel::handleDarkMode()
{
    if (m_blackOrWhiteButton) {
        if (m_blackOrWhiteButton == m_selectButton && m_blackOrWhiteButton->backgroundColor().name().toUpper() == BLACK) {
            emit currentColorChanged(m_blackOrWhiteButton->backgroundColor());
        }
        m_blackOrWhiteButton->setBackgroundColor(QColor(WHITE));
    }
}

void CustomColorPanel::handleLightMode()
{
    if (m_blackOrWhiteButton) {
        if (m_blackOrWhiteButton == m_selectButton && m_blackOrWhiteButton->backgroundColor().name().toUpper() == WHITE) {
            emit currentColorChanged(m_blackOrWhiteButton->backgroundColor());
        }
        m_blackOrWhiteButton->setBackgroundColor(QColor(BLACK));
    }
}

bool CustomColorPanel::isBlackOrWhite(const QColor& color)
{
    if (color.name().toUpper() == WHITE || color.name().toUpper() == BLACK) {
        return true;
    }
    return false;
}

void CustomColorPanel::handleTabletMode(UkuiSettingsMonitor::TabletStatus status)
{
    switch (status) {
    case UkuiSettingsMonitor::TabletStatus::PCMode:
    case UkuiSettingsMonitor::TabletStatus::PADHorizontalMode:
    case UkuiSettingsMonitor::TabletStatus::PADVerticalMode:
        handlePADMode();
        break;
    default:
        break;
    }
}

void CustomColorPanel::handlePADMode()
{
    m_layout->setContentsMargins(16,16,16,16);
    m_layout->setSpacing(16);
    for (auto button : m_buttons) {
        button->setButtonType(kdk::KColorButton::ButtonType::Circle);
    }
}

void CustomColorPanel::handlePCMode()
{
    m_layout->setContentsMargins(8,8,8,8);
    m_layout->setSpacing(8);
    for (auto button : m_buttons) {
        button->setButtonType(kdk::KColorButton::ButtonType::CheckedRect);
    }
}
