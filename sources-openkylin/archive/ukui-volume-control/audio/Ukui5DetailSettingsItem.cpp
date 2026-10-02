/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "Ukui5DetailSettingsItem.h"

#include <QHBoxLayout>

#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

Ukui5DetailSettings::Ukui5DetailSettings(QPushButton* btn, const QString& objName)
    : m_pSettingBtn(btn)
{
    KDK_EXTEND_ALL_INFO_FORMAT(m_pSettingBtn, "Audio", objName, "");
}

QWidget* Ukui5DetailSettings::getWidget()
{
    return m_pSettingBtn;
}

Ukui5DetailSettingsItem::Ukui5DetailSettingsItem(std::shared_ptr<IDetailSettings> setting, QLabel* label, QWidget* parent)
    : IDetailSettingsItem(setting, label), QFrame(parent)
{
    initUi();
}

void Ukui5DetailSettingsItem::initUi()
{
    setFixedHeight(60);

    m_pDisplayLabel->setFixedHeight(28);
    auto hLayout = new QHBoxLayout(this);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pDisplayLabel);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    hLayout->addWidget(m_pDetailSettings->getWidget());
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);
    setFrameShape(QFrame::Shape::Box);
}

void Ukui5DetailSettingsItem::initSlots()
{

}

QWidget* Ukui5DetailSettingsItem::getWidget()
{
    return this;
}
