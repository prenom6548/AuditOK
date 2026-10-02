/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */

#include "notification-settings-item.h"
#include <QFileInfo>
#include "security-control-config.h"

NotificationSettingsItem::NotificationSettingsItem(const QString &localName, const QString &nameEn, SingleApplicationSettings* settings, QWidget *parent)
    : QFrame(parent)
    , m_settings(settings)
{
    this->setMinimumWidth(550);
    this->setMaximumWidth(16777215);
    this->setFixedHeight(60);
    this->setFrameShape(QFrame::Shape::NoFrame);
    this->setAttribute(Qt::WA_DeleteOnClose);

    //icon
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(32, 32);

    //name
    m_nameLabel = new QLabel(this);
    m_nameLabel->setText(localName);

    //menu button
    m_menuButton = new QToolButton(this);
    m_menuButton->setProperty("useButtonPalette", true);
    m_menuButton->setPopupMode(QToolButton::InstantPopup);
    m_menuButton->setAutoRaise(true);
    m_menuButton->setFixedSize(QSize(36, 36));
    m_menuButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    m_menuButton->setAccessibleName(localName + tr("notification menu"));
    m_menuButton->setProperty("localName", localName);
    m_menu = new NoticeMenu(settings, m_menuButton);
    m_menuButton->setMenu(m_menu);

    m_notificationSwitch = new KSwitchButton(this);
    m_notificationSwitch->setAccessibleName(localName + tr("allow notification"));



    m_horLayout = new QHBoxLayout(this);
    m_horLayout->setSpacing(8);
    m_horLayout->setContentsMargins(16, 0, 16, 0);
    m_horLayout->addWidget(m_iconLabel);
    m_horLayout->addWidget(m_nameLabel);
    m_horLayout->addStretch();
    m_horLayout->addWidget(m_menuButton);
    m_horLayout->addWidget(m_notificationSwitch);

    m_notificationSwitch->blockSignals(true);
    m_notificationSwitch->setChecked(m_settings->allowNotify());
    m_notificationSwitch->setEnabled(m_settings->isNotificationSwitchEditable());
    m_notificationSwitch->blockSignals(false);

    m_menu->blockSignals(true);
    m_menu->initStatus();
    m_menu->blockSignals(false);

    connect(m_notificationSwitch, &KSwitchButton::stateChanged, this, [this](bool state) {
        m_settings->setAllowNotify(state);
    });

    connect(m_settings, &SingleApplicationSettings::allowNotifyChanged, this, [this](bool state) {
        m_notificationSwitch->blockSignals(true);
        m_notificationSwitch->setChecked(state);
        m_notificationSwitch->blockSignals(false);
    });
    connect(m_settings, &SingleApplicationSettings::allowSoundChanged, this, [this](bool state) {
        m_menu->blockSignals(true);
        m_menu->setVoiceEnable(state);
        m_menu->blockSignals(false);
    });
    connect(m_settings, &SingleApplicationSettings::showContentOnLockScreenChanged, this, [this](bool state) {
        m_menu->blockSignals(true);
        m_menu->setDetailShowOnLockScreenEnable(state);
        m_menu->blockSignals(false);
    });
    connect(m_settings, &SingleApplicationSettings::showNotificationOnLockScreenChanged, this, [this](bool state) {
        m_menu->blockSignals(true);
        m_menu->setShowOnLockScreenEnable(state);
        m_menu->blockSignals(false);
    });

    connect(m_menu, &NoticeMenu::voiceSignals, this, [this](bool checked) {
        m_settings->setAllowSound(checked);
    });

    connect(m_menu, &NoticeMenu::detailSignals, this, [this](bool checked) {
        m_settings->setShowContentOnLockScreen(checked);
    });

    connect(m_menu, &NoticeMenu::showSignals, this, [this](bool checked) {
        m_settings->setShowNotificationOnLockScreen(checked);
    });

    connect(&SecurityControlConfig::instance(), &SecurityControlConfig::configChanged, this, [this](SettingsProperty::Property property) {
        if (property == SettingsProperty::AllowNotify || property == SettingsProperty::ShowNotificationOnLockScreen) {
            updateState();
        }
    });
    connect(&SecurityControlConfig::instance(), &SecurityControlConfig::exitControl, this, [this](){
        updateState();
    });
}

void NotificationSettingsItem::setIcon(const QString &icon)
{
    const QFileInfo iconFile(QString("/usr/share/pixmaps/" + icon + ".png"));
    QPixmap pixmap;
    if (const QIcon currentIcon = QIcon::fromTheme(icon);!currentIcon.isNull()) {
        pixmap = currentIcon.pixmap(QSize(32, 32));
    } else if (iconFile.exists()) {
        pixmap = QPixmap(iconFile.filePath()).scaled(32, 32);
    } else {
        pixmap = QPixmap(QString(":/img/plugins/autoboot/desktop.png"));
    }
    m_iconLabel->setPixmap(pixmap);
}

void NotificationSettingsItem::updateState() const
{
    m_notificationSwitch->blockSignals(true);
    m_notificationSwitch->setChecked(m_settings->allowNotify());
    m_notificationSwitch->setEnabled(m_settings->isNotificationSwitchEditable());
    m_notificationSwitch->blockSignals(false);
    m_menu->updateState();
}
