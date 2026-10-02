/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "MonitorStream.h"

#include <iostream>
#include <thread>
#include <future>
#include <QDebug>


#define MS_CTX_STATE(C) pa_context_get_state((C))
#define MS_CTX_ERRNO(C) pa_context_errno((C))
#define MS_STRERROR(E) pa_strerror((E))
#define MS_STREAM_DEV_IDX(S) pa_stream_get_device_index((S))
#define MS_STREAM_PEEK(S, D, L) pa_stream_peek((S), (D), (L))
#define MS_STREAM_DROP(S) pa_stream_drop((S))
#define MS_STREAM_MON_STREAM(S) pa_stream_get_monitor_stream((S))
#define MS_STREAM_SUSPENDED(S) pa_stream_is_suspended((S))
#define MS_STREAM_NEW(C, N, SS, M) pa_stream_new((C), (N), (SS), (M))
#define MS_STREAM_SET_MON(S, I) pa_stream_set_monitor_stream((S), (I))
#define MS_STREAM_SET_READ_CB(S, CB, U) pa_stream_set_read_callback((S), (CB), (U))
#define MS_STREAM_SET_SUSP_CB(S, CB, U) pa_stream_set_suspended_callback((S), (CB), (U))
#define MS_STREAM_CONNECT_REC(S, D, A, F) pa_stream_connect_record((S), (D), (A), (F))
#define MS_STREAM_GET_INDEX(S) pa_stream_get_index((S))
#define MS_STREAM_UNREF(S) pa_stream_unref((S))
#define MS_CTX_KILL_SRC_OUT(C, I, CB, U) pa_context_kill_source_output((C), (I), (CB), (U))
#define MS_OP_UNREF(O) pa_operation_unref((O))


MonitorStream::MonitorStream(std::shared_ptr<IVolumeSlider> s)
    : m_pInputLevelBar(s)
{
    init();
}

void MonitorStream::init()
{
    pa_glib_mainloop* m = pa_glib_mainloop_new(g_main_context_default());
    m_pMainLoopApi = pa_glib_mainloop_get_api(m);
    pa_proplist* proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "Ukui Media Volume Control");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.ukui.media.vucontrol");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "ukui-control-center");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "PACKAGE_VERSION");
    m_pContext = pa_context_new_with_proplist(m_pMainLoopApi, nullptr, proplist);
    g_assert(m_pContext);

    pa_proplist_free(proplist);
    pa_context_set_state_callback(m_pContext, contextStateCallback, this);

    if (pa_context_connect(m_pContext, NULL, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
        std::cout << "pa_context_connect failed." << std::endl;
    }
}

void MonitorStream::contextStateCallback(pa_context* ctx, void* data)
{
    MonitorStream* m = static_cast<MonitorStream*>(data);
    switch (MS_CTX_STATE(ctx)) {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;

    case PA_CONTEXT_READY: {
        std::unique_lock<std::mutex> lock(m->m_mutex);
        m->m_bReady = true;
        m->m_condition.notify_one();

        break;
    }

    case PA_CONTEXT_TERMINATED:
        break;

    case PA_CONTEXT_FAILED:
    default:
        m->m_pInputLevelBar->setValue(0);  // Anyway, make sure UI is correct first, right
        qDebug() << "Context error: " << MS_STRERROR(MS_CTX_ERRNO(ctx));
    }
}

void MonitorStream::readCallback(pa_stream *s, size_t length, void *userdata)
{
    MonitorStream* m = static_cast<MonitorStream*>(userdata);
    const void *data;
    double v;
    int index;

    index = MS_STREAM_DEV_IDX(s);

    if (MS_STREAM_PEEK(s, &data, &length) < 0) {
        qDebug() << "Failed to read data from stream";
        return;
    }

    if (!data) {
        /* nullptr data means either a hole or empty buffer.
         * Only drop the stream when there is a hole (length > 0) */
        if (length)
            MS_STREAM_DROP(s);
        return;
    }

    assert(length > 0);
    assert(length % sizeof(float) == 0);

    v = ((const float*) data)[length / sizeof(float) -1];

    MS_STREAM_DROP(s);

    if (v < 0)
        v = 0;
    if (v > 1)
        v = 1;

    m->updateVolumeMeter(index, MS_STREAM_MON_STREAM(s), v);
}

void MonitorStream::suspended_callback(pa_stream* s, void* userdata)
{
    MonitorStream* m = static_cast<MonitorStream*>(userdata);

    if (MS_STREAM_SUSPENDED(s))
        m->updateVolumeMeter(MS_STREAM_DEV_IDX(s), PA_INVALID_INDEX, -1);
}

pa_stream* MonitorStream::createMonitorStreamForSource(uint32_t sourceIdx, uint32_t streamIdx = -1, bool suspend = false)
{
    char t[16];
    pa_buffer_attr attr;
    pa_sample_spec ss;
    pa_stream_flags_t flags;

    ss.channels = 1;
    ss.format = PA_SAMPLE_FLOAT32;
    ss.rate = 25;

    memset(&attr, 0, sizeof(attr));
    attr.fragsize = sizeof(float);
    attr.maxlength = (uint32_t) -1;

    snprintf(t, sizeof(t), "%u", sourceIdx);
    if (!(m_pStream = MS_STREAM_NEW(m_pContext, "Peak detect", &ss, nullptr))) {
        qDebug() << "Create Peak detect failed...";
        return nullptr;
    }

    if (streamIdx != (uint32_t) -1)
        MS_STREAM_SET_MON(m_pStream, streamIdx);

    MS_STREAM_SET_READ_CB(m_pStream, readCallback, this);
    MS_STREAM_SET_SUSP_CB(m_pStream, suspended_callback, this);

    flags = (pa_stream_flags_t) (PA_STREAM_PEAK_DETECT | PA_STREAM_ADJUST_LATENCY |
                                 (suspend ? PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND : PA_STREAM_NOFLAGS));

    int ret;
    if ((ret = MS_STREAM_CONNECT_REC(m_pStream, t, &attr, flags)) < 0) {
        qDebug() << "Failed to connect monitoring stream." << MS_STRERROR(MS_CTX_ERRNO(m_pContext));
        MS_STREAM_UNREF(m_pStream);
        return nullptr;
    }

    qDebug() << "createMonitorStreamForSource, index:" << sourceIdx << " sreamidx:" << MS_STREAM_GET_INDEX(m_pStream);
    return m_pStream;
}

void MonitorStream::updateVolumeMeter(uint32_t index, uint32_t sinkInputIdx, double v)
{
    Q_UNUSED(index);
    Q_UNUSED(sinkInputIdx);
    if (m_lastPeak >= DECAY_STEP)
        if (v < m_lastPeak - DECAY_STEP)
            v = m_lastPeak - DECAY_STEP;

    m_lastPeak = v;

    m_pInputLevelBar->setValue(qRound(v * 100));
}

void MonitorStream::startMonitor(uint32_t idx)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_bReady)
        m_condition.wait_for(lock, std::chrono::milliseconds(3000));

    if (!m_pStream && m_bReady)
        std::async(std::launch::async, std::bind(&MonitorStream::createMonitorStreamForSource, this, idx, -1, !!(pa_source_flags{} & PA_SOURCE_NETWORK)));
    else
        qDebug() << QString("The Peak detect flow already exists and does not need to be created again.");
}

void MonitorStream::stopMonitor()
{
    m_pInputLevelBar->setValue(0);  // Anyway, make sure UI is correct first, right

    if (m_pStream) {
        auto idx = MS_STREAM_GET_INDEX(m_pStream);

        if (idx != PA_INVALID_INDEX)
            MS_OP_UNREF(MS_CTX_KILL_SRC_OUT(m_pContext, idx, nullptr, nullptr));

        MS_STREAM_UNREF(m_pStream);
        m_pStream = nullptr;
    }
}

void MonitorStream::wait(int timeout)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_bReady) {
        m_condition.wait_for(lock, std::chrono::milliseconds(timeout));
    }
}

bool MonitorStream::isReady() const
{
    return m_bReady;
}

MonitorStream::~MonitorStream()
{
}

