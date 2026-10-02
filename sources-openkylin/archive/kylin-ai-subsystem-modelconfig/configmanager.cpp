#include "configmanager.h"

#include <QDebug>

namespace {
const char *modelConfigSchema = "org.kylin-ai-subsystem-modelconfig";
const char *enableComputePowerSharing = "enable-compute-power-sharing";
const char *computePowerSharingRole = "compute-power-sharing-role";
const char *localIp = "local-ip";
const char *enableRequestComputePower = "enable-request-compute-power";
const char *enableSharedComputePower = "enable-shared-compute-power";
const char *verificationCode = "verification-code";
const char *enableDomainControl = "enable-domain-control";
// 本地模型排序
const char* localNlpModelAddOrder = "local-nlp-model-add-order";
// 自选本地模型排序
const char* privateNlpModelAddOrder = "private-nlp-model-add-order";
}  // namespace

ConfigManager::ConfigManager(QObject *parent) : QObject(parent) { initConfig(); }

void ConfigManager::initConfig() {
    if (!QGSettings::isSchemaInstalled(modelConfigSchema)) {
        qWarning() << "ConfigManager: Kylin ai subsystem model config Schema not installed";
        return;
    }

    m_config = new QGSettings(modelConfigSchema);
    if (m_config == nullptr) {
        qWarning() << "ConfigManager: failed to create kylin ai subsystem model config GSettings";
        return;
    }
}

void ConfigManager::setEnableComputePowerSharing(bool enable) {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to set enableComputePowerSharing!";
        return;
    }
    if (m_config->get(enableComputePowerSharing).toBool() == enable) {
        return;
    }
    m_config->set(enableComputePowerSharing, enable);
}

void ConfigManager::setComputePowerSharingRole(const QString &role) {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to set computePowerSharingRole!";
        return;
    }
    if (m_config->get(computePowerSharingRole).toString() == role) {
        return;
    }
    m_config->set(computePowerSharingRole, role);
}

void ConfigManager::setLocalIp(const QString &ip) {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to set localIp!";
        return;
    }
    if (m_config->get(localIp).toString() == ip) {
        return;
    }
    m_config->set(localIp, ip);
}

void ConfigManager::setEnableRequestComputePower(bool enable) {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to set enableRequestComputePower!";
        return;
    }
    if (m_config->get(enableRequestComputePower).toBool() == enable) {
        return;
    }
    m_config->set(enableRequestComputePower, enable);
}

void ConfigManager::setEnableSharedComputePower(bool enable) {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to set enableSharedComputePower!";
        return;
    }
    if (m_config->get(enableSharedComputePower).toBool() == enable) {
        return;
    }
    m_config->set(enableSharedComputePower, enable);
}

void ConfigManager::setVerificationCode(const QString &code) {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to set verification code!";
        return;
    }
    if (m_config->get(verificationCode).toString() == code) {
        return;
    }
    m_config->set(verificationCode, code);
}

bool ConfigManager::getEnableComputePowerSharing() {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to determine whether computing power sharing is enabled!";
        return false;
    }
    return m_config->get(enableComputePowerSharing).toBool();
}

QString ConfigManager::getComputePowerSharingRole() {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to get compute power sharing role!";
        return "";
    }
    return m_config->get(computePowerSharingRole).toString();
}

QString ConfigManager::getLocalIp() {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to get local ip!";
        return "";
    }
    return m_config->get(localIp).toString();
}

bool ConfigManager::getEnableRequestComputePower() {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to determine whether request computing power is enabled!";
        return false;
    }
    return m_config->get(enableRequestComputePower).toBool();
}

bool ConfigManager::getEnableSharedComputePower() {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to determine whether share computing power is enabled!";
        return false;
    }
    return m_config->get(enableSharedComputePower).toBool();
}

QString ConfigManager::getVerificationCode() {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to get verification code!";
        return "";
    }
    return m_config->get(verificationCode).toString();
}

bool ConfigManager::getEnableDomainControl() {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to get verification code!";
        return false;
    }
    return m_config->get(enableDomainControl).toBool();
}

QStringList ConfigManager::getLocalNlpModelAddOrder() {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to get local NLP model add order!";
        return QStringList();
    }
    return m_config->get(localNlpModelAddOrder).toStringList();
}

void ConfigManager::setLocalNlpModelAddOrder(const QStringList& order) {
    if (!m_config) {
        qWarning() << "ConfigManager: failed to set local NLP model add order!";
        return;
    }
    if (m_config->get(localNlpModelAddOrder).toStringList() == order) {
        return;
    }
    m_config->set(localNlpModelAddOrder, order);
}

QStringList ConfigManager::getPrivateNlpModelAddOrder() {
    if (!m_config) {
        qWarning()
            << "ConfigManager: failed to get private NLP model add order!";
        return QStringList();
    }
    return m_config->get(privateNlpModelAddOrder).toStringList();
}

void ConfigManager::setPrivateNlpModelAddOrder(const QStringList& order) {
    if (!m_config) {
        qWarning()
            << "ConfigManager: failed to set private NLP model add order!";
        return;
    }
    if (m_config->get(privateNlpModelAddOrder).toStringList() == order) {
        return;
    }
    m_config->set(privateNlpModelAddOrder, order);
}