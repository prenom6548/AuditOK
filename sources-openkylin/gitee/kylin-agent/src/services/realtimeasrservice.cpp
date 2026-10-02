#include "services/realtimeasrservice.h"

RealtimeAsrService::RealtimeAsrService(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_audioInput(nullptr)
    , m_audioDevice(nullptr)
    , m_protocol(Protocol::DashScopeTask)
    , m_running(false)
    , m_taskStarted(false)
    , m_finishing(false)
{
}

RealtimeAsrService::~RealtimeAsrService() = default;

void RealtimeAsrService::start() {}
void RealtimeAsrService::stop() {}
void RealtimeAsrService::onConnected() {}
void RealtimeAsrService::onTextMessageReceived(const QString &) {}
void RealtimeAsrService::onAudioReadyRead() {}
void RealtimeAsrService::cleanup() {}
