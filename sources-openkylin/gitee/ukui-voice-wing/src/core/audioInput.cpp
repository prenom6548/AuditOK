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

#include "audioInput.h"

#include "logger.h"
#include <QtMath>
#include <QTimer>

#include <pulse/pulseaudio.h>

namespace {

constexpr pa_sample_spec kSampleSpec = {PA_SAMPLE_S16LE, 16000, 1};

qreal peakLevel(const void *data, size_t bytes)
{
    if (!data || bytes < sizeof(qint16)) {
        return 0.0;
    }

    const auto *samples = static_cast<const qint16 *>(data);
    const size_t count = bytes / sizeof(qint16);
    qreal peak = 0.0;
    for (size_t i = 0; i < count; ++i) {
        peak = qMax(peak, qAbs(samples[i]) / 32768.0);
    }
    return qBound(0.0, peak, 1.0);
}

} // namespace

AudioInput *AudioInput::instance()
{
    static AudioInput inst;
    return &inst;
}

AudioInput::AudioInput(QObject *parent)
    : QObject(parent)
{
    initPulse();
}

AudioInput::~AudioInput()
{
    stopMonitoring();
    teardownPulse();
}

void AudioInput::initPulse()
{
    m_mainloop = pa_mainloop_new();
    if (!m_mainloop) {
        VW_LOG(Logger::Level::Error) << "Failed to create PulseAudio mainloop";
        return;
    }

    m_pulseTimer = new QTimer(this);
    m_pulseTimer->setInterval(20);
    connect(m_pulseTimer, &QTimer::timeout, this, &AudioInput::dispatchPulse);
    m_pulseTimer->start();

    m_context = pa_context_new(pa_mainloop_get_api(m_mainloop), "ukui-voice-wing");
    if (!m_context) {
        VW_LOG(Logger::Level::Error) << "Failed to create PulseAudio context";
        teardownPulse();
        return;
    }

    pa_context_set_state_callback(m_context, onContextState, this);

    if (pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
        VW_LOG(Logger::Level::Error) << "Failed to connect PulseAudio context:"
                                     << pa_strerror(pa_context_errno(m_context));
        teardownPulse();
    }
}

void AudioInput::teardownPulse()
{
    closeRecordStream();

    if (m_context) {
        pa_context_disconnect(m_context);
        pa_context_unref(m_context);
        m_context = nullptr;
    }

    if (m_mainloop) {
        pa_mainloop_free(m_mainloop);
        m_mainloop = nullptr;
    }

    m_contextReady = false;
    if (m_pulseTimer) {
        m_pulseTimer->stop();
        m_pulseTimer = nullptr;
    }
}

void AudioInput::dispatchPulse()
{
    if (!m_mainloop) {
        return;
    }

    int ret = 0;
    do {
        ret = pa_mainloop_iterate(m_mainloop, 0, nullptr);
    } while (ret > 0);
}

void AudioInput::onContextState(pa_context *ctx, void *userdata)
{
    auto *self = static_cast<AudioInput *>(userdata);
    if (!self) {
        return;
    }

    switch (pa_context_get_state(ctx)) {
    case PA_CONTEXT_READY:
        if (!self->m_contextReady) {
            self->m_contextReady = true;
        }
        if (self->m_wantMonitoring && !self->m_monitoring) {
            self->requestOpenStream();
        }
        break;
    case PA_CONTEXT_FAILED:
        VW_LOG(Logger::Level::Error) << "PulseAudio context failed:"
                                     << pa_strerror(pa_context_errno(ctx));
        break;
    default:
        break;
    }
}

void AudioInput::requestOpenStream()
{
    if (!m_context || m_stream || !m_contextReady) {
        return;
    }

    pa_operation *op = pa_context_get_server_info(m_context, onOpenStreamServerInfo, this);
    if (op) {
        pa_operation_unref(op);
    }
}

void AudioInput::onOpenStreamServerInfo(pa_context *, const pa_server_info *info, void *userdata)
{
    auto *self = static_cast<AudioInput *>(userdata);
    if (!self || !info || !info->default_source_name || self->m_stream) {
        return;
    }
    self->openRecordStream(info->default_source_name);
}

void AudioInput::openRecordStream(const char *sourceName)
{
    if (!m_context || m_stream || !sourceName) {
        return;
    }

    m_stream = pa_stream_new(m_context, "ukui-voice-wing-input", &kSampleSpec, nullptr);
    if (!m_stream) {
        VW_LOG(Logger::Level::Error) << "Failed to create PulseAudio stream";
        return;
    }

    pa_stream_set_read_callback(m_stream, onStreamPeak, this);

    if (pa_stream_connect_record(m_stream, sourceName, nullptr, PA_STREAM_ADJUST_LATENCY) < 0) {
        VW_LOG(Logger::Level::Error) << "Failed to connect record stream:"
                                     << pa_strerror(pa_context_errno(m_context));
        pa_stream_unref(m_stream);
        m_stream = nullptr;
        return;
    }

    m_monitoring = true;
}

void AudioInput::onStreamPeak(pa_stream *stream, size_t length, void *userdata)
{
    auto *self = static_cast<AudioInput *>(userdata);
    if (!self) {
        return;
    }

    const void *data = nullptr;
    if (pa_stream_peek(stream, &data, &length) < 0 || !data || length == 0) {
        if (length > 0) {
            pa_stream_drop(stream);
        }
        return;
    }

    const qreal level = peakLevel(data, length);
    pa_stream_drop(stream);
    Q_EMIT self->volumeChanged(level);
}

void AudioInput::closeRecordStream()
{
    if (!m_stream) {
        return;
    }

    pa_stream_disconnect(m_stream);
    pa_stream_unref(m_stream);
    m_stream = nullptr;
}

void AudioInput::startMonitoring()
{
    if (m_monitoring) {
        return;
    }

    m_wantMonitoring = true;

    if (m_contextReady) {
        requestOpenStream();
    }
}

void AudioInput::stopMonitoring()
{
    m_wantMonitoring = false;

    if (!m_monitoring && !m_stream) {
        return;
    }

    closeRecordStream();
    m_monitoring = false;
    Q_EMIT volumeChanged(0.0);
}
