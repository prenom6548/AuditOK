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

#ifndef VOICEINPUT_H
#define VOICEINPUT_H

#include <QObject>
#include <QVariant>

class VoiceInputWindow;
class MicrophoneMuteDialog;

class VoiceInput : public QObject
{
    Q_OBJECT
public:
    static VoiceInput *instance();

    explicit VoiceInput(QObject *parent = nullptr);

private Q_SLOTS:
    void onShortcutClicked();
    void onShortcutLongPressed();
    void onShortcutReleased();
    void onFocusChanged();

private:
    bool hasFocusWindow() const;
    QString appName() const;
    void commitString(const QString &text, int flags);
    void stopVoiceInput();
    void callKylinAiAssistantAsync(const QString &methodName,
                                 const QList<QVariant> &arguments = {});

    VoiceInputWindow *m_window = nullptr;
    MicrophoneMuteDialog *m_muteDialog = nullptr;
    QString m_focusAppName;
    int m_lastInterimLength = 0;
    bool m_voiceInputActive = false;
};

#endif // VOICEINPUT_H
