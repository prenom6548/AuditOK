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

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QGSettings/QGSettings>
#include <QObject>

class ConfigManager : public QObject {
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);

    int getInstallProgress() const;
    int getUninstallProgress() const;
    QString getLastDetectAndUpdateTime() const;

    void setInstallProgress(int progress);
    void setUninstallProgress(int progress);
    void setLastDetectAndUpdateTime(const QString &time);

private:
    void initConfig();

signals:

private:
    QGSettings *config_ = nullptr;
};

#endif // CONFIGMANAGER_H
