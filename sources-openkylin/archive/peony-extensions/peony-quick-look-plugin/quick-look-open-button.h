/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef QUICK_LOOK_OPEN_BUTTON_H
#define QUICK_LOOK_OPEN_BUTTON_H

#include <QWidget>

class QToolButton;

class QuickLookOpenButton : public QWidget
{
    Q_OBJECT

public:
    explicit QuickLookOpenButton(QWidget *parent = nullptr);
    ~QuickLookOpenButton() override;

    void setUri(const QString &uri);
    void clear();

private:
    void refreshPrimaryOpenButton();
    void openUri();
    QString resolveLaunchUri(const QString &uri) const;

    QToolButton *m_openWithButton = nullptr;
    QString m_uri;
};

#endif
