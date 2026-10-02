/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "configmanager.h"

#include <QDebug>

namespace {
const char *subsystemSchema = "org.kylin-ai-subsystem-plugin";
const char *installProgress = "install-progress";
const char *uninstallProgress = "uninstall-progress";
const char *lastDetectAndUpdateTime = "last-detect-install-time";
} // namespace

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent) {
    initConfig();
}

int ConfigManager::getInstallProgress() const {
    if (!config_) {
        qWarning() << "Failed to get install progress!";
        return -1;
    }
    return config_->get(installProgress).toInt();
}

int ConfigManager::getUninstallProgress() const {
    if (!config_) {
        qWarning() << "Failed to get uninstall progress!";
        return -1;
    }
    return config_->get(uninstallProgress).toInt();
}

QString ConfigManager::getLastDetectAndUpdateTime() const {
    if (!config_) {
        qWarning() << "Failed to get detect and update time!";
        return "";
    }
    return config_->get(lastDetectAndUpdateTime).toString();
}

void ConfigManager::setInstallProgress(int progress) {
    if (config_->get(installProgress).toInt() == progress) {
        return;
    }
    config_->set(installProgress, progress);
}

void ConfigManager::setUninstallProgress(int progress) {
    if (config_->get(uninstallProgress).toInt() == progress) {
        return;
    }
    config_->set(uninstallProgress, progress);
}

void ConfigManager::setLastDetectAndUpdateTime(const QString &time) {
    if (config_->get(lastDetectAndUpdateTime).toString() == time) {
        return;
    }
    config_->set(lastDetectAndUpdateTime, time);
}

void ConfigManager::initConfig() {
    if (!QGSettings::isSchemaInstalled(subsystemSchema)) {
        qWarning() << "Kylin ai subsystem plugin Schema not installed";
        return;
    }

    config_ = new QGSettings(subsystemSchema);
    if (config_ == nullptr) {
        qWarning() << "Failed to create kylin ai subsystem plugin GSettings";
        return;
    }
}
