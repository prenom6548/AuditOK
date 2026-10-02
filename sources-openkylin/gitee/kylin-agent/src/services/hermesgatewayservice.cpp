#include "services/hermesgatewayservice.h"

#include "services/gatewayservice.h"

namespace {
HermesGatewayService *s_activeHermesGatewayService = nullptr;
}

HermesGatewayService::HermesGatewayService(QObject *parent)
    : QObject(parent)
    , m_gatewayProcess(nullptr)
    , m_progressDialog(nullptr)
    , m_progressStatus(nullptr)
    , m_progressLog(nullptr)
    , m_progressCancelButton(nullptr)
    , m_progressSessionActive(false)
    , m_progressCancelled(false)
    , m_startupRuntimeUserCancelled(false)
    , m_gatewayService(new GatewayService(this))
{
    s_activeHermesGatewayService = this;
}

HermesGatewayService::~HermesGatewayService()
{
    if (s_activeHermesGatewayService == this) {
        s_activeHermesGatewayService = nullptr;
    }
}

HermesGatewayService *HermesGatewayService::activeInstance()
{
    return s_activeHermesGatewayService;
}

bool HermesGatewayService::ensureReady()
{
    return m_gatewayService && m_gatewayService->ensureReady();
}

bool HermesGatewayService::restartGateway()
{
    return m_gatewayService && m_gatewayService->restartGateway();
}

void HermesGatewayService::stopManagedGateway()
{
    if (m_gatewayService) {
        m_gatewayService->stopManagedGateway();
    }
}
