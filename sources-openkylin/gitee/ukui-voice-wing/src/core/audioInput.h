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

#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QObject>

class QTimer;

struct pa_context;
struct pa_server_info;
struct pa_stream;

class AudioInput : public QObject
{
    Q_OBJECT
public:
    static AudioInput *instance();

public slots:
    void startMonitoring();
    void stopMonitoring();

signals:
    void volumeChanged(qreal level);

private:
    explicit AudioInput(QObject *parent = nullptr);
    ~AudioInput() override;

    void initPulse();
    void teardownPulse();
    void dispatchPulse();
    void requestOpenStream();
    void openRecordStream(const char *sourceName);
    void closeRecordStream();

    static void onContextState(pa_context *ctx, void *userdata);
    static void onOpenStreamServerInfo(pa_context *ctx, const pa_server_info *info, void *userdata);
    static void onStreamPeak(pa_stream *stream, size_t length, void *userdata);

    QTimer *m_pulseTimer = nullptr;
    struct pa_mainloop *m_mainloop = nullptr;
    struct pa_context *m_context = nullptr;
    struct pa_stream *m_stream = nullptr;
    bool m_contextReady = false;
    bool m_wantMonitoring = false;
    bool m_monitoring = false;
};

#endif // AUDIOINPUT_H
