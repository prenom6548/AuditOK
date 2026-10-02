#include "search-config.h"

#include <QDBusError>
#include <QDebug>

namespace {
const QByteArray SEARCH_SCHEMA = QByteArrayLiteral("org.ukui.search.settings");
const QString FILE_INDEX_ENABLE_KEY = QStringLiteral("file-index-enable");
const QString WEB_ENGINE_KEY = QStringLiteral("web-engine");
const QStringList SUPPORTED_WEB_ENGINES = {
    QStringLiteral("baidu"),
    QStringLiteral("sougou"),
    QStringLiteral("360")
};
const QString EMPTY_STRING;
}

SearchConfig::SearchConfig(QObject *parent) : QObject(parent)
{
    if (!UKUI::QGSettings::isSchemaInstalled(SEARCH_SCHEMA)) {
        qWarning() << SEARCH_SCHEMA << "is not installed";
        return;
    }

    m_settings = new UKUI::QGSettings(SEARCH_SCHEMA, QByteArray(), this);
    connect(m_settings, &UKUI::QGSettings::changed, this, &SearchConfig::onSettingsChanged);
}

SearchConfig::~SearchConfig()
{
}

QString SearchConfig::GetDefaultSearchEngine() const
{
    if (!isAvailable()) {
        if (calledFromDBus()) {
            sendUnavailableError();
        }
        return EMPTY_STRING;
    }

    return m_settings->get(WEB_ENGINE_KEY).toString();
}

bool SearchConfig::SetDefaultSearchEngine(const QString &engine)
{
    if (!isAvailable()) {
        if (calledFromDBus()) {
            sendUnavailableError();
        }
        return false;
    }

    if (!SUPPORTED_WEB_ENGINES.contains(engine)) {
        if (calledFromDBus()) {
            sendInvalidEngineError(engine);
        }
        return false;
    }

    if (GetDefaultSearchEngine() == engine) {
        return true;
    }

    const QString error = m_settings->set(WEB_ENGINE_KEY, engine);
    if (!error.isEmpty()) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::Failed, error);
        }
        return false;
    }
    return true;
}

QStringList SearchConfig::GetSupportedDefaultSearchEngines() const
{
    return SUPPORTED_WEB_ENGINES;
}

bool SearchConfig::IsFileNameSearchEnabled() const
{
    if (!isAvailable()) {
        if (calledFromDBus()) {
            sendUnavailableError();
        }
        return false;
    }

    return m_settings->get(FILE_INDEX_ENABLE_KEY).toBool();
}

bool SearchConfig::SetFileNameSearchEnabled(bool enabled)
{
    if (!isAvailable()) {
        if (calledFromDBus()) {
            sendUnavailableError();
        }
        return false;
    }

    if (IsFileNameSearchEnabled() == enabled) {
        return true;
    }

    const QString error = m_settings->set(FILE_INDEX_ENABLE_KEY, enabled);
    if (!error.isEmpty()) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::Failed, error);
        }
        return false;
    }
    return true;
}

void SearchConfig::onSettingsChanged(const QString &key)
{
    if (key == WEB_ENGINE_KEY) {
        emit DefaultSearchEngineChanged(GetDefaultSearchEngine());
        return;
    }

    if (key == FILE_INDEX_ENABLE_KEY) {
        emit FileNameSearchEnabledChanged(IsFileNameSearchEnabled());
    }
}

bool SearchConfig::isAvailable() const
{
    return m_settings != nullptr;
}

void SearchConfig::sendUnavailableError() const
{
    const_cast<SearchConfig *>(this)->sendErrorReply(QDBusError::InvalidService,
                                                     tr("Search settings schema is not available"));
}

void SearchConfig::sendInvalidEngineError(const QString &engine) const
{
    const_cast<SearchConfig *>(this)->sendErrorReply(
        QDBusError::InvalidArgs,
        tr("Invalid search engine: %1, supported engines: [%2]")
            .arg(engine, SUPPORTED_WEB_ENGINES.join(QStringLiteral(", "))));
}
