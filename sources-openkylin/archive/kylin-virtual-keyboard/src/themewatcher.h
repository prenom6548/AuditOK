/*
 * Copyright (c) KylinSoft Co., Ltd. 2025.All rights reserved.
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

#ifndef THEMEWATCHER_H
#define THEMEWATCHER_H

#include <memory>
#include <QGSettings>
#include <QObject>

class ThemeWatcher : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentThemeColor READ currentThemeColor NOTIFY
                   currentThemeColorChanged)
    Q_PROPERTY(
        QString currentTheme READ currentTheme NOTIFY currentThemeChanged)

public:
    explicit ThemeWatcher(QObject *parent = nullptr);
    ~ThemeWatcher() override = default;

    QString currentTheme() const;
    QString currentThemeColor() const;

signals:
    void currentThemeChanged();
    void currentThemeColorChanged();

private:
    void updateTheme();
    void updateThemeColor();
    void connectSignals();

private:
    QString currentTheme_ = "default";
    QString currentThemeColor_ = "light";
    std::unique_ptr<QGSettings> styleSettings_;
};

#endif // THEMEWATCHER_H
