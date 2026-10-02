#include "services/apiservice.h"
#include "services/configservice.h"
#include "services/databaseservice.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QUuid>
#include <QTimer>
#include <QSettings>
#include <QUrl>
#include <QEventLoop>
#include <QDateTime>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QSet>
#include <QProcess>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>

#include <algorithm>
#include <functional>

namespace {

constexpr int kRuntimeDefaultPort = 8642;
const QString kStartupGreetingMarker = QStringLiteral("[[kylin-agent-startup-greeting]]");

QString stripCronjobResponsePrefix(QString content)
{
    static const QRegularExpression prefixPattern(
        QStringLiteral("^Cronjob Response:\\s*"));
    content.remove(prefixPattern);
    return content;
}

QString normalizeApiUrl(QString url)
{
    url = url.trimmed();
    if (url.isEmpty()) {
        return QStringLiteral("127.0.0.1");
    }

    if (url.startsWith(QStringLiteral("http://")) ||
        url.startsWith(QStringLiteral("https://"))) {
        const QUrl parsed(url);
        if (!parsed.host().isEmpty()) {
            url = parsed.host();
            if (parsed.port() >= 0) {
                url += QStringLiteral(":%1").arg(parsed.port());
            }
        }
    } else {
        const int slash = url.indexOf(QLatin1Char('/'));
        if (slash >= 0) {
            url = url.left(slash);
        }
    }

    while (url.endsWith(QLatin1Char('/'))) {
        url.chop(1);
    }
    return url;
}

QString networkApiUrlFor(QString url)
{
    url = normalizeApiUrl(url);
    QUrl parsed(QStringLiteral("http://") + url);
    if (parsed.port() < 0) {
        parsed.setPort(kRuntimeDefaultPort);
    }
    if (parsed.host() == QStringLiteral("0.0.0.0")) {
        parsed.setHost(QStringLiteral("127.0.0.1"));
    }
    QString networkUrl = parsed.toString();
    while (networkUrl.endsWith(QLatin1Char('/'))) {
        networkUrl.chop(1);
    }
    return networkUrl;
}

QString yamlQuoted(QString text)
{
    text.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    text.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    return QStringLiteral("\"%1\"").arg(text);
}

void applyApiAuth(QNetworkRequest *request)
{
    const QString apiKey = QSettings().value(QStringLiteral("General/BackendApiKey")).toString().trimmed();
    if (!apiKey.isEmpty()) {
        request->setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());
    }
}

QString runtimeHome()
{
    const QByteArray agentRuntimeHome = qgetenv("AGENT_RUNTIME_HOME");
    if (!agentRuntimeHome.isEmpty()) {
        return QString::fromLocal8Bit(agentRuntimeHome);
    }

    const QByteArray envHome = qgetenv("HERMES_HOME");
    if (!envHome.isEmpty()) {
        return QString::fromLocal8Bit(envHome);
    }
    return QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        .filePath(QStringLiteral(".kylin-agent-runtime"));
}

bool sameConversationRow(const QMap<QString, QVariant> &left,
                         const QMap<QString, QVariant> &right)
{
    return left.value(QStringLiteral("role")).toString() == right.value(QStringLiteral("role")).toString() &&
           left.value(QStringLiteral("content")).toString().trimmed() ==
               right.value(QStringLiteral("content")).toString().trimmed();
}

QVector<QMap<QString, QVariant>> dedupeAssistantRun(QVector<QMap<QString, QVariant>> run)
{
    bool changed = true;
    while (changed && run.size() >= 2 && run.size() % 2 == 0) {
        changed = false;
        const int half = run.size() / 2;
        bool repeated = true;
        for (int i = 0; i < half; ++i) {
            if (!sameConversationRow(run.at(i), run.at(i + half))) {
                repeated = false;
                break;
            }
        }
        if (repeated) {
            run = run.mid(0, half);
            changed = true;
        }
    }
    return run;
}

QVector<QMap<QString, QVariant>> filterConversationMessages(const QVector<QMap<QString, QVariant>> &rows)
{
    QVector<QMap<QString, QVariant>> result;
    QVector<QMap<QString, QVariant>> assistantRun;

    auto flushAssistantRun = [&]() {
        const QVector<QMap<QString, QVariant>> deduped = dedupeAssistantRun(assistantRun);
        for (const auto &row : deduped) {
            result.append(row);
        }
        assistantRun.clear();
    };

    for (const auto &row : rows) {
        const QString role = row.value(QStringLiteral("role")).toString();
        const QVariant contentValue = row.value(QStringLiteral("content"));
        if (contentValue.toString().trimmed().startsWith(kStartupGreetingMarker)) {
            continue;
        }
        if ((role != QStringLiteral("user") && role != QStringLiteral("assistant")) || contentValue.toString().isNull()) {
            continue;
        }

        QMap<QString, QVariant> normalizedRow = row;
        if (role == QStringLiteral("assistant")) {
            const QString content = stripCronjobResponsePrefix(contentValue.toString());
            if (content.isEmpty()) {
                continue;
            }
            normalizedRow.insert(QStringLiteral("content"), content);
            assistantRun.append(normalizedRow);
        } else {
            flushAssistantRun();
            if (!result.isEmpty() && sameConversationRow(result.last(), normalizedRow)) {
                continue;
            }
            result.append(normalizedRow);
        }
    }
    flushAssistantRun();
    return result;
}

} // namespace

ApiService::ApiService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_apiUrl(normalizeApiUrl(QSettings().value(QStringLiteral("General/BackendUrl"), QStringLiteral("0.0.0.0")).toString()))
    , m_connected(false)
    , m_currentModel()
{
    // Try to connect and verify
    checkConnection();
}

ApiService::~ApiService()
{
    const QList<QString> sessionIds = m_streams.keys();
    for (const QString &sessionId : sessionIds) {
        cancelStreamForSession(sessionId, false);
    }
}

ApiService& ApiService::instance()
{
    static ApiService instance;
    return instance;
}

void ApiService::setApiUrl(const QString& url)
{
    const QString normalizedUrl = normalizeApiUrl(url);
    const bool changed = normalizedUrl != m_apiUrl;
    if (changed) {
        const QStringList trackedSessionIds = m_sessionMemoryStates.keys();
        for (const QString &sessionId : trackedSessionIds) {
            clearSessionMemoryTracking(sessionId);
        }
    }
    m_apiUrl = normalizedUrl;
    QSettings settings;
    settings.setValue(QStringLiteral("General/BackendUrl"), m_apiUrl);
    settings.sync();
    if (changed) {
        m_sessionUsageCache.clear();
    }
    checkConnection();
    if (changed) {
        emit backendChanged();
    }
}

void ApiService::setCurrentModel(const QString &model)
{
    m_currentModel = model;
}

bool ApiService::usesLocalStorageMode() const
{
    return normalizeApiUrl(m_apiUrl).section(QLatin1Char(':'), 0, 0) == QStringLiteral("0.0.0.0");
}

void ApiService::checkConnection()
{
    if (usesLocalStorageMode()) {
        const QDir legacyRoot(QDir(runtimeHome()).filePath(QStringLiteral("import-skill")));
        const QString skillsRoot = QDir(runtimeHome()).filePath(QStringLiteral("skills/import-skills"));
        if (legacyRoot.exists()) {
            QDir().mkpath(skillsRoot);
            const QFileInfoList legacySkills = legacyRoot.entryInfoList(
                QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
            for (const QFileInfo &legacySkill : legacySkills) {
                if (legacySkill.fileName() == QStringLiteral(".draft")) {
                    continue;
                }
                const QString target = QDir(skillsRoot).filePath(legacySkill.fileName());
                if (QFileInfo::exists(target)) {
                    qWarning() << "Keeping legacy imported skill because target already exists:"
                               << legacySkill.absoluteFilePath();
                    continue;
                }
                if (!QDir().rename(legacySkill.absoluteFilePath(), target)) {
                    qWarning() << "Failed to migrate imported skill to default skills directory:"
                               << legacySkill.absoluteFilePath();
                }
            }
        }
        if (!m_connected) {
            m_connected = true;
            emit connectedChanged(true);
        }
        qDebug() << "ApiService: using local runtime storage mode for 0.0.0.0";
        loadSessions();
        loadModels();
        return;
    }

    QNetworkRequest request(QUrl(networkApiUrlFor(m_apiUrl) + "/api/status"));
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    applyApiAuth(&request);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            if (!m_connected) {
                m_connected = true;
                emit connectedChanged(true);
                qDebug() << "Connected to kylin-agent-runtime service API";
                loadSessions();
                loadModels();
            }
        } else {
            if (m_connected) {
                m_connected = false;
                emit connectedChanged(false);
                qDebug() << "Disconnected from kylin-agent-runtime service API";
            }
        }
        reply->deleteLater();
    });
}

void ApiService::loadSessions()
{
    if (usesLocalStorageMode()) {
        QVector<QJsonObject> sessionList;
        DatabaseService::instance().initialize();
        const auto rows = DatabaseService::instance().getSessions(50, 0);
        for (const auto &row : rows) {
            cacheSessionUsage(row);
            QJsonObject obj;
            for (auto it = row.begin(); it != row.end(); ++it) {
                obj[it.key()] = QJsonValue::fromVariant(it.value());
            }
            sessionList.append(obj);
        }
        emit sessionsLoaded(sessionList);
        return;
    }

    QNetworkRequest request(QUrl(networkApiUrlFor(m_apiUrl) + "/api/sessions"));
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    applyApiAuth(&request);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonArray sessions = QJsonDocument::fromJson(reply->readAll()).array();
            QVector<QJsonObject> sessionList;
            for (const QJsonValue &val : sessions) {
                const QJsonObject object = val.toObject();
                QMap<QString, QVariant> row;
                for (auto it = object.begin(); it != object.end(); ++it) {
                    row[it.key()] = it.value().toVariant();
                }
                cacheSessionUsage(row);
                sessionList.append(object);
            }
            emit sessionsLoaded(sessionList);
        } else {
            emit error("Failed to load sessions: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

void ApiService::loadSessionMessages(const QString &sessionId)
{
    if (sessionId.trimmed().isEmpty()) {
        emit sessionMessagesLoaded(sessionId, {});
        return;
    }

    if (usesLocalStorageMode()) {
        // The database is the canonical conversation source. In particular,
        // do not scan and parse the potentially very large details log on the
        // GUI thread merely to display a conversation.
        DatabaseService::instance().initialize();
        const auto rows = filterConversationMessages(
            DatabaseService::instance().getSessionMessages(sessionId));
        QTimer::singleShot(0, this, [this, sessionId, rows]() {
            emit sessionMessagesLoaded(sessionId, rows);
        });
        return;
    }

    const QString encoded = QString::fromUtf8(QUrl::toPercentEncoding(sessionId));
    QNetworkRequest request(QUrl(networkApiUrlFor(m_apiUrl) +
                                 QStringLiteral("/api/sessions/%1/messages").arg(encoded)));
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                      QStringLiteral("application/json"));
    applyApiAuth(&request);

    QNetworkReply *reply = m_networkManager->get(request);
    QTimer::singleShot(10000, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, sessionId]() {
        QVector<QMap<QString, QVariant>> rows;
        if (reply->error() == QNetworkReply::NoError) {
            const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isArray()) {
                QVector<QMap<QString, QVariant>> parsedRows;
                for (const QJsonValue &value : doc.array()) {
                    QMap<QString, QVariant> row;
                    const QJsonObject object = value.toObject();
                    for (auto it = object.begin(); it != object.end(); ++it) {
                        row[it.key()] = it.value().toVariant();
                    }
                    parsedRows.append(row);
                }
                rows = filterConversationMessages(parsedRows);
            }
        } else {
            const QString message = tr("加载会话消息失败：%1").arg(reply->errorString());
            emit error(message);
            emit sessionMessagesLoadFailed(sessionId, message);
            reply->deleteLater();
            return;
        }
        reply->deleteLater();
        emit sessionMessagesLoaded(sessionId, rows);
    });
}

void ApiService::loadTaskRecovery(const QString &sessionId)
{
    if (sessionId.trimmed().isEmpty()) {
        QTimer::singleShot(0, this, [this, sessionId]() {
            emit taskRecoveryLoaded(sessionId, QJsonObject());
        });
        return;
    }

    const QString encoded = QString::fromUtf8(QUrl::toPercentEncoding(sessionId));
    QNetworkRequest request(QUrl(
        networkApiUrlFor(m_apiUrl) +
        QStringLiteral("/api/sessions/%1/recovery").arg(encoded)));
    request.setHeader(
        QNetworkRequest::KnownHeaders::ContentTypeHeader,
        QStringLiteral("application/json"));
    applyApiAuth(&request);

    QNetworkReply *reply = m_networkManager->get(request);
    QTimer::singleShot(5000, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, sessionId]() {
        QJsonObject recovery;
        if (reply->error() == QNetworkReply::NoError) {
            const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject()) {
                recovery = doc.object();
            }
        }
        reply->deleteLater();
        emit taskRecoveryLoaded(sessionId, recovery);
    });
}

void ApiService::interruptSessionTask(const QString &sessionId, bool recoverable)
{
    if (sessionId.trimmed().isEmpty()) {
        QTimer::singleShot(0, this, [this, sessionId]() {
            emit sessionTaskInterrupted(sessionId, false);
        });
        return;
    }

    const QString encoded = QString::fromUtf8(QUrl::toPercentEncoding(sessionId));
    QNetworkRequest request(QUrl(
        networkApiUrlFor(m_apiUrl) +
        QStringLiteral("/api/sessions/%1/interrupt").arg(encoded)));
    request.setHeader(
        QNetworkRequest::KnownHeaders::ContentTypeHeader,
        QStringLiteral("application/json"));
    applyApiAuth(&request);

    QNetworkReply *reply = m_networkManager->post(
        request,
        QJsonDocument(QJsonObject{
            {QStringLiteral("recoverable"), recoverable},
            {QStringLiteral("reason"), recoverable
                ? QStringLiteral("user_stopped")
                : QStringLiteral("queued_message_superseded")},
        }).toJson(QJsonDocument::Compact));
    QTimer::singleShot(5000, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, sessionId]() {
        bool recoverable = false;
        if (reply->error() == QNetworkReply::NoError) {
            const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject()) {
                recoverable =
                    doc.object().value(QStringLiteral("recoverable")).toBool();
            }
        }
        reply->deleteLater();
        emit sessionTaskInterrupted(sessionId, recoverable);
    });
}

void ApiService::createSession(const QString &title)
{
    if (usesLocalStorageMode()) {
        const QString sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        if (createSessionSync(sessionId, title)) {
            QJsonObject session;
            session["id"] = sessionId;
            session["title"] = title;
            emit sessionCreated(session);
        }
        return;
    }

    QJsonObject body;
    if (!title.isEmpty()) {
        body["title"] = title;
    }

    QNetworkRequest request(QUrl(networkApiUrlFor(m_apiUrl) + "/api/sessions"));
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    applyApiAuth(&request);

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonObject session = QJsonDocument::fromJson(reply->readAll()).object();
            emit sessionCreated(session);
        } else {
            emit error("Failed to create session: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

bool ApiService::createSessionSync(const QString &sessionId, const QString &title, const QString &model)
{
    if (usesLocalStorageMode()) {
        DatabaseService::instance().initialize();
        if (!DatabaseService::instance().createSession(sessionId, QStringLiteral("desktop"), model)) {
            return false;
        }
        if (title.trimmed().isEmpty() || title.trimmed() == QStringLiteral("新会话")) {
            return true;
        }
        return DatabaseService::instance().updateSessionTitle(sessionId, title);
    }

    bool ok = false;
    QJsonObject body;
    body["id"] = sessionId;
    body["title"] = title;
    body["source"] = "desktop";
    body["model"] = model;
    requestSync(QStringLiteral("POST"), QStringLiteral("/api/sessions"), body, &ok);
    return ok;
}

QJsonObject ApiService::getUserProfileSync()
{
    bool ok = false;
    const QJsonDocument doc = requestSync(
        QStringLiteral("GET"), QStringLiteral("/api/user-profile"), {}, &ok);
    return ok && doc.isObject() ? doc.object() : QJsonObject();
}

bool ApiService::putUserProfileSync(const QJsonObject &entries,
                                    const QString &expectedUpdatedAt)
{
    QJsonObject body;
    body.insert(QStringLiteral("entries"), entries);
    if (!expectedUpdatedAt.isEmpty()) {
        body.insert(QStringLiteral("expected_updated_at"), expectedUpdatedAt);
    }
    bool ok = false;
    const QJsonDocument doc = requestSync(
        QStringLiteral("PUT"), QStringLiteral("/api/user-profile"), body, &ok);
    return ok && doc.isObject() && doc.object().value(QStringLiteral("ok")).toBool();
}

static QVector<QMap<QString, QVariant>> jsonArrayToVariantMaps(const QJsonArray &array)
{
    QVector<QMap<QString, QVariant>> rows;
    for (const QJsonValue &value : array) {
        const QJsonObject object = value.toObject();
        QMap<QString, QVariant> row;
        for (auto it = object.begin(); it != object.end(); ++it) {
            row[it.key()] = it.value().toVariant();
        }
        rows.append(row);
    }
    return rows;
}

static QString readUtf8File(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

static bool writeUtf8File(const QString &path, const QString &content)
{
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(content.toUtf8());
    return file.commit();
}

void ApiService::cacheSessionUsage(const QMap<QString, QVariant> &session)
{
    const QString sessionId = session.value(QStringLiteral("id")).toString().trimmed();
    if (sessionId.isEmpty()) {
        return;
    }

    QMap<QString, QVariant> usage;
    const QStringList columns = {
        QStringLiteral("input_tokens"),
        QStringLiteral("output_tokens"),
        QStringLiteral("cache_read_tokens"),
        QStringLiteral("cache_write_tokens"),
        QStringLiteral("reasoning_tokens"),
        QStringLiteral("api_call_count"),
        QStringLiteral("last_turn_elapsed_ms")
    };
    for (const QString &column : columns) {
        if (session.contains(column)) {
            usage.insert(column, session.value(column));
        }
    }
    if (!usage.isEmpty()) {
        m_sessionUsageCache.insert(sessionId, usage);
    }
}

void ApiService::clearSessionMemoryTracking(const QString &sessionId)
{
    auto it = m_sessionMemoryStates.find(sessionId);
    if (it == m_sessionMemoryStates.end()) {
        return;
    }
    if (it->inactivityTimer) {
        it->inactivityTimer->stop();
        it->inactivityTimer->deleteLater();
    }
    m_sessionMemoryStates.erase(it);
}

void ApiService::loadModels()
{
    QVector<QJsonObject> modelList;
    const QVector<QMap<QString, QVariant>> rows = getConfigModelsSync();
    for (const QMap<QString, QVariant> &row : rows) {
        QJsonObject object;
        for (auto it = row.begin(); it != row.end(); ++it) {
            object.insert(it.key(), QJsonValue::fromVariant(it.value()));
        }
        modelList.append(object);
    }
    emit modelsLoaded(modelList);
}

QJsonDocument ApiService::requestSync(const QString &method,
                                      const QString &endpoint,
                                      const QJsonObject &body,
                                      bool *ok)
{
    if (ok) {
        *ok = false;
    }

    QNetworkRequest request(QUrl(networkApiUrlFor(m_apiUrl) + endpoint));
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                      QStringLiteral("application/json"));
    applyApiAuth(&request);

    QNetworkReply *reply = nullptr;
    const QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    if (method == QStringLiteral("GET")) {
        reply = m_networkManager->get(request);
    } else if (method == QStringLiteral("POST")) {
        reply = m_networkManager->post(request, payload);
    } else if (method == QStringLiteral("PUT")) {
        reply = m_networkManager->put(request, payload);
    } else if (method == QStringLiteral("PATCH")) {
        reply = m_networkManager->sendCustomRequest(request, QByteArrayLiteral("PATCH"), payload);
    } else if (method == QStringLiteral("DELETE")) {
        reply = m_networkManager->deleteResource(request);
    } else {
        return QJsonDocument();
    }

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start(10000);
    loop.exec();

    const bool timedOut = !timeout.isActive();
    if (timedOut && reply->isRunning()) {
        reply->abort();
    }

    QJsonDocument document;
    if (!timedOut && reply->error() == QNetworkReply::NoError) {
        document = QJsonDocument::fromJson(reply->readAll());
        if (ok) {
            *ok = true;
        }
    } else {
        qWarning() << "ApiService request failed:" << method << endpoint << reply->errorString();
    }
    reply->deleteLater();
    return document;
}

QVector<QMap<QString, QVariant>> ApiService::getConfigModelsSync()
{
    if (usesLocalStorageMode()) {
        DatabaseService::instance().initialize();
        return DatabaseService::instance().getModels();
    }

    bool ok = false;
    const QJsonDocument document = requestSync(
        QStringLiteral("GET"), QStringLiteral("/api/config/models"), {}, &ok);
    if (!ok || !document.isArray()) {
        return {};
    }
    return jsonArrayToVariantMaps(document.array());
}

bool ApiService::replaceConfigModelsSync(const QJsonArray &models,
                                         const QString &preferredDefaultModel,
                                         const QString &preferredDefaultBaseUrl)
{
    if (usesLocalStorageMode()) {
        Q_UNUSED(preferredDefaultModel)
        Q_UNUSED(preferredDefaultBaseUrl)
        const QString path = QDir(runtimeHome()).filePath(QStringLiteral("models.json"));
        if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
            return false;
        }
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }
        file.write(QJsonDocument(models).toJson(QJsonDocument::Indented));
        return file.commit();
    }

    QJsonObject body;
    body.insert(QStringLiteral("models"), models);
    const QString defaultModel = preferredDefaultModel.trimmed();
    if (!defaultModel.isEmpty()) {
        body.insert(QStringLiteral("preferredDefaultModel"), defaultModel);
    }
    if (!preferredDefaultBaseUrl.trimmed().isEmpty()) {
        body.insert(QStringLiteral("preferredDefaultBaseUrl"), preferredDefaultBaseUrl.trimmed());
    }

    bool ok = false;
    requestSync(QStringLiteral("PUT"), QStringLiteral("/api/config/models"), body, &ok);
    return ok;
}

void ApiService::sendChatCompletion(const QString &sessionId,
                                    const QJsonArray &messages,
                                    const QString &model)
{
    const QString normalizedSessionId = sessionId.trimmed();
    if (normalizedSessionId.isEmpty()) {
        emit chatCompletionFinished(sessionId, QString(), false);
        return;
    }

    cancelStreamForSession(normalizedSessionId, true);

    const QString selectedModel = model.trimmed().isEmpty() ? m_currentModel : model.trimmed();
    if (selectedModel.isEmpty()) {
        emit chatCompletionFinished(
            normalizedSessionId,
            tr("没有可用模型。请先在设置中配置模型。"),
            false);
        return;
    }

    QJsonObject body;
    body.insert(QStringLiteral("model"), selectedModel);
    body.insert(QStringLiteral("messages"), messages);
    body.insert(QStringLiteral("stream"), true);

    QNetworkRequest request(QUrl(networkApiUrlFor(m_apiUrl) + QStringLiteral("/v1/chat/completions")));
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader,
                      QStringLiteral("application/json"));
    request.setRawHeader("X-Hermes-Session-Id", normalizedSessionId.toUtf8());
    applyApiAuth(&request);

    QNetworkReply *reply = m_networkManager->post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    StreamState state;
    state.reply = reply;
    m_streams.insert(normalizedSessionId, state);

    connect(reply, &QNetworkReply::readyRead, this, [this, reply, normalizedSessionId]() {
        auto it = m_streams.find(normalizedSessionId);
        if (it == m_streams.end() || it->reply != reply) {
            return;
        }

        StreamState &stream = *it;
        stream.partialData += QString::fromUtf8(reply->readAll());
        stream.partialData.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));

        while (stream.partialData.contains(QStringLiteral("\n\n"))) {
            const int blockEnd = stream.partialData.indexOf(QStringLiteral("\n\n"));
            const QString block = stream.partialData.left(blockEnd).trimmed();
            stream.partialData = stream.partialData.mid(blockEnd + 2);
            if (block.isEmpty()) {
                continue;
            }

            QString dataLine;
            const QStringList lines = block.split(QLatin1Char('\n'));
            for (const QString &line : lines) {
                if (line.startsWith(QStringLiteral("data:"))) {
                    dataLine += line.mid(5).trimmed();
                }
            }
            if (dataLine.isEmpty()) {
                continue;
            }
            if (dataLine == QStringLiteral("[DONE]")) {
                stream.completionEmitted = true;
                continue;
            }

            const QJsonObject chunk = QJsonDocument::fromJson(dataLine.toUtf8()).object();
            const QJsonObject errorObject = chunk.value(QStringLiteral("error")).toObject();
            if (!errorObject.isEmpty()) {
                const QString message = errorObject.value(QStringLiteral("message")).toString();
                emit chatCompletionFinished(normalizedSessionId, message, false);
                stream.errorEmitted = true;
                stream.completionEmitted = true;
                continue;
            }

            QString content = chunk.value(QStringLiteral("content")).toString();
            if (content.isEmpty()) {
                const QJsonArray choices = chunk.value(QStringLiteral("choices")).toArray();
                if (!choices.isEmpty()) {
                    content = choices.first().toObject()
                        .value(QStringLiteral("delta")).toObject()
                        .value(QStringLiteral("content")).toString();
                }
            }
            if (!content.isEmpty()) {
                stream.receivedContent = true;
                emit chatCompletionDelta(normalizedSessionId, content);
            }
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, normalizedSessionId]() {
        auto it = m_streams.find(normalizedSessionId);
        if (it == m_streams.end() || it->reply != reply) {
            reply->deleteLater();
            return;
        }

        const StreamState stream = *it;
        m_streams.erase(it);

        bool ok = reply->error() == QNetworkReply::NoError ||
                  reply->error() == QNetworkReply::UnknownContentError;
        QString fallback;
        if (!ok) {
            fallback = reply->errorString();
        } else if (!stream.receivedContent && !stream.errorEmitted) {
            ok = false;
            fallback = tr("运行时未返回任何回复，请检查模型配置或运行时日志。");
        }
        emit chatCompletionFinished(normalizedSessionId, fallback, ok);
        reply->deleteLater();
    });
}

void ApiService::cancelChatCompletion(const QString &sessionId)
{
    cancelStreamForSession(sessionId, true);
}

void ApiService::cancelCurrentStream()
{
    const QList<QString> sessionIds = m_streams.keys();
    for (const QString &sessionId : sessionIds) {
        cancelStreamForSession(sessionId, true);
    }
}

void ApiService::cancelCurrentStream(const QString &sessionId)
{
    cancelStreamForSession(sessionId, true);
}

void ApiService::cancelStreamForSession(const QString &sessionId, bool emitCompletion)
{
    auto it = m_streams.find(sessionId);
    if (it == m_streams.end()) {
        return;
    }

    QNetworkReply *reply = it->reply;
    const bool completed = it->completionEmitted;
    m_streams.erase(it);
    if (reply) {
        reply->abort();
        reply->deleteLater();
    }
    if (emitCompletion && !completed) {
        emit chatCompletionFinished(sessionId, tr("生成已停止。"), false);
    }
}

QMap<QString, QVariant> ApiService::getUsageSync()
{
    bool ok = false;
    const QJsonDocument document = requestSync(
        QStringLiteral("GET"), QStringLiteral("/api/usage"), {}, &ok);
    QMap<QString, QVariant> row;
    if (ok && document.isObject()) {
        const QJsonObject object = document.object();
        for (auto it = object.begin(); it != object.end(); ++it) {
            row.insert(it.key(), it.value().toVariant());
        }
        return row;
    }

    const QVector<QMap<QString, QVariant>> sessions = getSessionsSync(500, 0);
    int messages = 0;
    for (const QMap<QString, QVariant> &session : sessions) {
        messages += session.value(QStringLiteral("message_count")).toInt();
    }
    row.insert(QStringLiteral("sessions"), sessions.size());
    row.insert(QStringLiteral("messages"), messages);
    row.insert(QStringLiteral("input_tokens"), 0);
    row.insert(QStringLiteral("output_tokens"), 0);
    return row;
}

QVector<QMap<QString, QVariant>> ApiService::getSessionsSync(int limit, int offset)
{
    if (usesLocalStorageMode()) {
        DatabaseService::instance().initialize();
        const QVector<QMap<QString, QVariant>> rows =
            DatabaseService::instance().getSessions(limit, offset);
        for (const QMap<QString, QVariant> &row : rows) {
            cacheSessionUsage(row);
        }
        return rows;
    }

    bool ok = false;
    const QJsonDocument document = requestSync(
        QStringLiteral("GET"),
        QStringLiteral("/api/sessions?limit=%1&offset=%2").arg(limit).arg(offset),
        {},
        &ok);
    if (!ok || !document.isArray()) {
        return {};
    }
    const QVector<QMap<QString, QVariant>> rows = jsonArrayToVariantMaps(document.array());
    for (const QMap<QString, QVariant> &row : rows) {
        cacheSessionUsage(row);
    }
    return rows;
}

QString ApiService::getPersonaSync()
{
    if (usesLocalStorageMode()) {
        return readUtf8File(QDir(runtimeHome()).filePath(QStringLiteral("SOUL.md")));
    }

    bool ok = false;
    const QJsonDocument document = requestSync(
        QStringLiteral("GET"), QStringLiteral("/api/persona"), {}, &ok);
    return ok && document.isObject()
        ? document.object().value(QStringLiteral("content")).toString()
        : QString();
}

bool ApiService::putPersonaSync(const QString &content)
{
    if (usesLocalStorageMode()) {
        return writeUtf8File(QDir(runtimeHome()).filePath(QStringLiteral("SOUL.md")), content);
    }

    bool ok = false;
    requestSync(QStringLiteral("PUT"),
                QStringLiteral("/api/persona"),
                QJsonObject{{QStringLiteral("content"), content}},
                &ok);
    return ok;
}

QVector<QMap<QString, QVariant>> ApiService::getLogsSync()
{
    if (usesLocalStorageMode()) {
        QVector<QMap<QString, QVariant>> rows;
        const QDir logDir(QDir(runtimeHome()).filePath(QStringLiteral("logs")));
        const QFileInfoList files = logDir.entryInfoList(
            {QStringLiteral("*.log")}, QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
        for (const QFileInfo &fileInfo : files) {
            QMap<QString, QVariant> row;
            row.insert(QStringLiteral("name"), fileInfo.fileName());
            row.insert(QStringLiteral("size"), fileInfo.size());
            row.insert(QStringLiteral("modified_at"), fileInfo.lastModified().toSecsSinceEpoch());
            rows.append(row);
        }
        return rows;
    }

    bool ok = false;
    const QJsonDocument document = requestSync(
        QStringLiteral("GET"), QStringLiteral("/api/logs"), {}, &ok);
    return ok && document.isArray() ? jsonArrayToVariantMaps(document.array())
                                    : QVector<QMap<QString, QVariant>>();
}

QString ApiService::getLogContentSync(const QString &name)
{
    if (usesLocalStorageMode()) {
        const QString fileName = QFileInfo(name).fileName();
        if (!fileName.endsWith(QStringLiteral(".log"))) {
            return QString();
        }
        return readUtf8File(QDir(QDir(runtimeHome()).filePath(QStringLiteral("logs"))).filePath(fileName));
    }

    bool ok = false;
    const QString encoded = QString::fromUtf8(QUrl::toPercentEncoding(name));
    const QJsonDocument document = requestSync(
        QStringLiteral("GET"), QStringLiteral("/api/logs/%1").arg(encoded), {}, &ok);
    return ok && document.isObject()
        ? document.object().value(QStringLiteral("content")).toString()
        : QString();
}
