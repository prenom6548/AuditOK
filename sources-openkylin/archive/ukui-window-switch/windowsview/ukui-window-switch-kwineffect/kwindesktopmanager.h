/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 *
 * Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 *
 */

#ifndef KWINDESKTOPMANAGER_H
#define KWINDESKTOPMANAGER_H

#include "../abstractdesktopmanager.h"
#include <kwineffects.h>

class KWinDesktopManager : public AbstractDesktopManager
{
    Q_OBJECT
public:
    explicit KWinDesktopManager(QObject *parent = nullptr);
    ~KWinDesktopManager() = default;

    // AbstractDesktopManager interface
    void removeDesktop(const QString &desktop) override;
    void moveDesktop(const QString &srcDesktop, int destDesktopIndex) override;
    void appendDesktop() override;
    void setCurrentDesktop(const QString &desktop) override;
    QString getCurrentDesktop() override;
    int getNumberOfDesktops() override;
    bool isCurrentDesktopShowingDesktop() override;
    QRect getFullScreenGeometry(const QString &desktop, int screenIndex) override;
    void setDesktopName(const QString &desktop, const QString name) override;
    int getDesktopIndex(const QString &id) override;
    QString getDesktopUuid(int desktopIndex) override;
    QString getOutputUuid(int outputIndex) override;
    bool isInitialized() override;

private:
    bool validateDesktopIndex(const QString &desktop);
};

#endif // KWINDESKTOPMANAGER_H
