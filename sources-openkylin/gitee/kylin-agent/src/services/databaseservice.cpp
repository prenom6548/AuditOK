#include "services/databaseservice.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QSqlError>
#include <QSqlQuery>

namespace {

QString titleFromFirstMessage(const QString &content)
{
    QString title = content.trimmed().simplified();
    if (title.length() > 40) {
        title = title.left(40) + QStringLiteral("..");
    }
    return title;
}

QString databaseRuntimeHome()
{
    const QByteArray agentRuntimeHome = qgetenv("AGENT_RUNTIME_HOME");
    if (!agentRuntimeHome.isEmpty()) {
        return QString::fromLocal8Bit(agentRuntimeHome);
    }
    const QByteArray hermesHome = qgetenv("HERMES_HOME");
    if (!hermesHome.isEmpty()) {
        return QString::fromLocal8Bit(hermesHome);
    }
    return QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        .filePath(QStringLiteral(".kylin-agent-runtime"));
}

} // namespace

DatabaseService::DatabaseService(QObject *parent)
    : QObject(parent)
{
    const QString home = QDir::homePath() + QStringLiteral("/.hermes");
    QDir().mkpath(home);
    m_dbPath = QDir(home).filePath(QStringLiteral("state.db"));
}

DatabaseService::~DatabaseService() = default;

DatabaseService &DatabaseService::instance()
{
    static DatabaseService service;
    return service;
}

bool DatabaseService::initialize()
{
    if (m_initialized) {
        return true;
    }

    QSqlDatabase db = getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        emit databaseError(db.lastError().text());
        return false;
    }

    if (!createTables()) {
        emit databaseError(db.lastError().text());
        return false;
    }

    m_initialized = true;
    return true;
}

QSqlDatabase DatabaseService::getDatabase()
{
    const QString connectionName = QStringLiteral("kylin_agent_db");
    if (QSqlDatabase::contains(connectionName)) {
        return QSqlDatabase::database(connectionName);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return QSqlDatabase();
    }

    QSqlQuery query(db);
    query.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
    return db;
}

bool DatabaseService::createTables()
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        return false;
    }

    QSqlQuery query(db);
    if (!query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS sessions ("
        "id TEXT PRIMARY KEY,"
        "source TEXT NOT NULL DEFAULT 'desktop',"
        "started_at INTEGER NOT NULL,"
        "ended_at INTEGER,"
        "message_count INTEGER NOT NULL DEFAULT 0,"
        "model TEXT,"
        "title TEXT)"
    ))) {
        qWarning() << "Failed to create sessions table:" << query.lastError().text();
        return false;
    }

    if (!query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "session_id TEXT NOT NULL,"
        "role TEXT NOT NULL,"
        "content TEXT,"
        "timestamp INTEGER NOT NULL,"
        "FOREIGN KEY (session_id) REFERENCES sessions(id) ON DELETE CASCADE)"
    ))) {
        qWarning() << "Failed to create messages table:" << query.lastError().text();
        return false;
    }

    query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_messages_session ON messages(session_id)"));
    query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_messages_timestamp ON messages(timestamp)"));
    return true;
}

bool DatabaseService::createSession(const QString &id, const QString &source, const QString &model)
{
    QSqlQuery query(getDatabase());
    query.prepare(QStringLiteral(
        "INSERT INTO sessions (id, source, started_at, model, message_count) "
        "VALUES (?, ?, ?, ?, 0)"
    ));
    query.addBindValue(id);
    query.addBindValue(source);
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(model);
    return query.exec();
}

bool DatabaseService::updateSessionTitle(const QString &sessionId, const QString &title)
{
    QSqlQuery query(getDatabase());
    query.prepare(QStringLiteral("UPDATE sessions SET title = ? WHERE id = ?"));
    query.addBindValue(title);
    query.addBindValue(sessionId);
    return query.exec() && query.numRowsAffected() > 0;
}

bool DatabaseService::endSession(const QString &sessionId)
{
    QSqlQuery query(getDatabase());
    query.prepare(QStringLiteral("UPDATE sessions SET ended_at = ? WHERE id = ?"));
    query.addBindValue(QDateTime::currentSecsSinceEpoch());
    query.addBindValue(sessionId);
    return query.exec();
}

bool DatabaseService::incrementMessageCount(const QString &sessionId)
{
    QSqlQuery query(getDatabase());
    query.prepare(QStringLiteral("UPDATE sessions SET message_count = message_count + 1 WHERE id = ?"));
    query.addBindValue(sessionId);
    return query.exec();
}

QVector<QMap<QString, QVariant>> DatabaseService::getSessions(int limit, int offset)
{
    QVector<QMap<QString, QVariant>> rows;
    QSqlQuery query(getDatabase());
    query.prepare(QStringLiteral(
        "SELECT id, source, started_at, ended_at, message_count, model, title, "
        "(SELECT content FROM messages WHERE session_id = sessions.id AND role = 'user' "
        "ORDER BY timestamp, id LIMIT 1) AS first_user_message "
        "FROM sessions WHERE source = 'desktop' "
        "ORDER BY started_at DESC LIMIT ? OFFSET ?"
    ));
    query.addBindValue(limit);
    query.addBindValue(offset);

    if (!query.exec()) {
        return rows;
    }

    while (query.next()) {
        QMap<QString, QVariant> row;
        row[QStringLiteral("id")] = query.value(QStringLiteral("id"));
        row[QStringLiteral("source")] = query.value(QStringLiteral("source"));
        row[QStringLiteral("started_at")] = query.value(QStringLiteral("started_at"));
        row[QStringLiteral("ended_at")] = query.value(QStringLiteral("ended_at"));
        row[QStringLiteral("message_count")] = query.value(QStringLiteral("message_count"));
        row[QStringLiteral("model")] = query.value(QStringLiteral("model"));
        QString title = query.value(QStringLiteral("title")).toString();
        if (title.trimmed().isEmpty()) {
            title = titleFromFirstMessage(query.value(QStringLiteral("first_user_message")).toString());
        }
        row[QStringLiteral("title")] = title;
        rows.append(row);
    }
    return rows;
}

QVector<QMap<QString, QVariant>> DatabaseService::searchSessions(const QString &text, int limit)
{
    QVector<QMap<QString, QVariant>> rows;
    const QString term = text.trimmed();
    if (term.isEmpty()) {
        return rows;
    }

    QSqlQuery query(getDatabase());
    query.prepare(QStringLiteral(
        "SELECT DISTINCT s.id, s.source, s.started_at, s.ended_at, s.message_count, "
        "s.model, s.title, m.content AS snippet "
        "FROM sessions s "
        "LEFT JOIN messages m ON m.session_id = s.id "
        "WHERE s.source = 'desktop' "
        "AND (s.title LIKE ? OR m.content LIKE ?) "
        "ORDER BY s.started_at DESC LIMIT ?"
    ));
    const QString pattern = QStringLiteral("%%1%").arg(term);
    query.addBindValue(pattern);
    query.addBindValue(pattern);
    query.addBindValue(limit);

    if (!query.exec()) {
        return rows;
    }

    while (query.next()) {
        QMap<QString, QVariant> row;
        row[QStringLiteral("session_id")] = query.value(QStringLiteral("id"));
        row[QStringLiteral("title")] = query.value(QStringLiteral("title"));
        row[QStringLiteral("started_at")] = query.value(QStringLiteral("started_at"));
        row[QStringLiteral("source")] = query.value(QStringLiteral("source"));
        row[QStringLiteral("message_count")] = query.value(QStringLiteral("message_count"));
        row[QStringLiteral("model")] = query.value(QStringLiteral("model"));
        row[QStringLiteral("snippet")] = query.value(QStringLiteral("snippet"));
        rows.append(row);
    }
    return rows;
}

QVector<QMap<QString, QVariant>> DatabaseService::getSessionMessages(const QString &sessionId)
{
    QVector<QMap<QString, QVariant>> rows;
    QSqlQuery query(getDatabase());
    query.prepare(QStringLiteral(
        "SELECT id, role, content, timestamp FROM messages "
        "WHERE session_id = ? ORDER BY timestamp, id"
    ));
    query.addBindValue(sessionId);
    if (!query.exec()) {
        return rows;
    }

    while (query.next()) {
        QMap<QString, QVariant> row;
        row[QStringLiteral("id")] = query.value(QStringLiteral("id"));
        row[QStringLiteral("role")] = query.value(QStringLiteral("role"));
        row[QStringLiteral("content")] = query.value(QStringLiteral("content"));
        row[QStringLiteral("timestamp")] = query.value(QStringLiteral("timestamp"));
        rows.append(row);
    }
    return rows;
}

bool DatabaseService::addMessage(const QString &sessionId, const QString &role, const QString &content)
{
    QSqlQuery query(getDatabase());
    query.prepare(QStringLiteral(
        "INSERT INTO messages (session_id, role, content, timestamp) VALUES (?, ?, ?, ?)"
    ));
    query.addBindValue(sessionId);
    query.addBindValue(role);
    query.addBindValue(content);
    query.addBindValue(QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
        return false;
    }

    return incrementMessageCount(sessionId);
}

bool DatabaseService::deleteSession(const QString &sessionId)
{
    QSqlQuery query(getDatabase());
    query.prepare(QStringLiteral("DELETE FROM sessions WHERE id = ?"));
    query.addBindValue(sessionId);
    return query.exec();
}

QVector<QMap<QString, QVariant>> DatabaseService::getModels()
{
    QVector<QMap<QString, QVariant>> result;
    const QString modelsPath = QDir(databaseRuntimeHome()).filePath(QStringLiteral("models.json"));
    QFile file(modelsPath);
    if (!file.exists()) {
        return result;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open models file:" << modelsPath;
        return result;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isArray()) {
        return result;
    }

    for (const QJsonValue &value : document.array()) {
        const QJsonObject object = value.toObject();
        QMap<QString, QVariant> row;
        for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
            row.insert(it.key(), it.value().toVariant());
        }
        result.append(row);
    }
    return result;
}
