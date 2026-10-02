#include "services/configservice.h"

#include <QSettings>
#include <QUrl>

namespace {

constexpr int kDefaultAgentRtPort = 8642;
const char *kBackendUrlKey = "General/BackendUrl";
const char *kSelectedModelKey = "General/SelectedModelId";

} // namespace

ConfigService::ConfigService(QObject *parent)
    : QObject(parent)
{
    load();
}

ConfigService::~ConfigService() = default;

ConfigService &ConfigService::instance()
{
    static ConfigService service;
    return service;
}

void ConfigService::load()
{
    QSettings settings;
    m_agentRuntimeHost = normalizeAgentRuntimeHost(
        settings.value(QString::fromLatin1(kBackendUrlKey), QStringLiteral("127.0.0.1")).toString());
    m_selectedModelId = settings.value(QString::fromLatin1(kSelectedModelKey)).toString();
}

void ConfigService::save() const
{
    QSettings settings;
    settings.setValue(QString::fromLatin1(kBackendUrlKey), m_agentRuntimeHost);
    settings.setValue(QString::fromLatin1(kSelectedModelKey), m_selectedModelId);
}

QString ConfigService::normalizeAgentRuntimeHost(QString host)
{
    host = host.trimmed();
    if (host.isEmpty()) {
        return QStringLiteral("127.0.0.1");
    }

    if (host.startsWith(QStringLiteral("http://")) || host.startsWith(QStringLiteral("https://"))) {
        const QUrl parsed(host);
        if (!parsed.host().isEmpty()) {
            host = parsed.host();
            if (parsed.port() >= 0) {
                host += QStringLiteral(":%1").arg(parsed.port());
            }
        }
    }

    const int slash = host.indexOf(QLatin1Char('/'));
    if (slash >= 0) {
        host = host.left(slash);
    }
    while (host.endsWith(QLatin1Char('/'))) {
        host.chop(1);
    }
    return host;
}

QString ConfigService::agentRuntimeEndpoint() const
{
    QUrl url(QStringLiteral("http://") + normalizeAgentRuntimeHost(m_agentRuntimeHost));
    if (url.port() < 0) {
        url.setPort(kDefaultAgentRtPort);
    }
    if (url.host() == QStringLiteral("0.0.0.0")) {
        url.setHost(QStringLiteral("127.0.0.1"));
    }

    QString endpoint = url.toString();
    while (endpoint.endsWith(QLatin1Char('/'))) {
        endpoint.chop(1);
    }
    return endpoint;
}

void ConfigService::setAgentRuntimeHost(const QString &host)
{
    const QString normalized = normalizeAgentRuntimeHost(host);
    if (m_agentRuntimeHost == normalized) {
        return;
    }

    m_agentRuntimeHost = normalized;
    save();
    emit agentRuntimeChanged(m_agentRuntimeHost);
}

void ConfigService::setSelectedModelId(const QString &modelId)
{
    if (m_selectedModelId == modelId) {
        return;
    }

    m_selectedModelId = modelId;
    save();
}
