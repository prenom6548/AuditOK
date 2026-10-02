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
#include <QPointer>
#include <QVariant>

class VoiceInputWindow;
class TipDialog;

class VoiceInput : public QObject
{
    Q_OBJECT
public:
    static VoiceInput *instance();

    explicit VoiceInput(QObject *parent = nullptr);

    bool isRecognizing() const;

signals:
    void isRecognizingChanged();

private Q_SLOTS:
    void onShortcutClicked();
    void onShortcutLongPressed();
    void onShortcutReleased();
    void onFocusChanged();

private:
    bool hasFocusWindow() const;
    void commitString(const QString &text, int flags);
    void stopVoiceInput();
    void setIsRecognizing(bool isRecognizing);
    void callDBusServiceAsync(const char *service,
                              const char *path,
                              const char *interface,
                              const QString &methodName,
                              const QList<QVariant> &arguments = {});
    void callKylinAiAssistantAsync(const QString &methodName,
                                   const QList<QVariant> &arguments = {});
    void callKylinPearlAsync(const QString &methodName,
                             const QList<QVariant> &arguments = {});
    /** 按需创建提示框；关闭后会销毁，QPointer 自动置空，已存在则复用 */
    TipDialog *createTipDialog();

    VoiceInputWindow *m_window = nullptr;
    QPointer<TipDialog> m_tipDialog;
    int m_lastInterimLength = 0;
    bool m_voiceInputActive = false;
    bool m_isRecognizing = false;
};

#endif // VOICEINPUT_H
