/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MICROPHONEMUTEDIALOG_H
#define MICROPHONEMUTEDIALOG_H

#include <QQuickView>
#include <ukuiwindowhelper.h>

class MicrophoneMuteDialog : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(QString infoIconSource READ infoIconSource NOTIFY infoIconSourceChanged)

public:
    explicit MicrophoneMuteDialog(QWindow *parent = nullptr);

    QString infoIconSource() const;

    void showCentered();

public slots:
    void onCancelClicked();
    void onSettingsClicked();
    void onCloseClicked();

signals:
    void infoIconSourceChanged();

private:
    bool event(QEvent *event) override;
    void initWindowHelper();
    void updateCenterPosition();
    void openSoundSettings();

    UkuiWindowHelper *m_windowHelper = nullptr;
    QString m_infoIconSource;
    static constexpr int kDialogWidth = 420;
    static constexpr int kDialogHeight = 160;
};

#endif // MICROPHONEMUTEDIALOG_H
