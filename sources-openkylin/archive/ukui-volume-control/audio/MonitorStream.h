#ifndef MONITORSTREAM_H
#define MONITORSTREAM_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/ext-device-restore.h>
#include <pulse/ext-device-manager.h>

#include "IVolumeSliderItem.h"

#define DECAY_STEP .04


class MonitorStream
{
public:
    MonitorStream(std::shared_ptr<IVolumeSlider>);
    ~MonitorStream();

public:
    void startMonitor(uint32_t);

    void stopMonitor();

    void wait(int);

    bool isReady() const;


private:
    void init();

    static void contextStateCallback(pa_context*, void*);

    static void readCallback(pa_stream*, size_t, void*);

    static void suspended_callback(pa_stream*, void*);

    void updateVolumeMeter(uint32_t, uint32_t, double);

    pa_stream* createMonitorStreamForSource(uint32_t, uint32_t, bool);

private:
    std::shared_ptr<IVolumeSlider> m_pInputLevelBar = nullptr;
    pa_context* m_pContext = nullptr;
    pa_stream* m_pStream = nullptr;
    pa_mainloop_api* m_pMainLoopApi = nullptr;

    std::atomic<bool> m_bReady {false};
    double m_lastPeak = 0.0;

    std::mutex m_mutex;
    std::condition_variable m_condition;
};

#endif // MONITORSTREAM_H
