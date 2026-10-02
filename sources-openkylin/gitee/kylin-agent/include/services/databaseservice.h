#ifndef DATABASESERVICE_H
#define DATABASESERVICE_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include <QMap>

class DatabaseService : public QObject
{
    Q_OBJECT

public:
    static DatabaseService& instance();
    
    bool initialize();
    QString databasePath() const { return m_dbPath; }
    
    // Session operations
    bool createSession(const QString &id, const QString &source, const QString &model);
    bool updateSessionTitle(const QString &sessionId, const QString &title);
    bool endSession(const QString &sessionId);
    bool incrementMessageCount(const QString &sessionId);
    bool deleteSession(const QString &sessionId);
    QVector<QMap<QString, QVariant>> getSessions(int limit = 30, int offset = 0);
    QVector<QMap<QString, QVariant>> searchSessions(const QString &query, int limit = 20);
    QVector<QMap<QString, QVariant>> getSessionMessages(const QString &sessionId);
    
    // Message operations
    bool addMessage(const QString &sessionId, const QString &role, const QString &content);
    
    // Model operations
    QVector<QMap<QString, QVariant>> getModels();
    
    // FTS operations
    bool createFtsIndex();
    bool rebuildFtsIndex();

signals:
    void databaseError(const QString &error);

private:
    explicit DatabaseService(QObject *parent = nullptr);
    ~DatabaseService();
    DatabaseService(const DatabaseService&) = delete;
    DatabaseService& operator=(const DatabaseService&) = delete;
    
    bool createTables();
    QSqlDatabase getDatabase();
    void seedDefaultModelsJson();
    
    QString m_dbPath;
    bool m_initialized = false;
};

#endif // DATABASESERVICE_H
