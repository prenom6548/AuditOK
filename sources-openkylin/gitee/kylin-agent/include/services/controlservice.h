#ifndef CONTROLSERVICE_H
#define CONTROLSERVICE_H

#include <QObject>
#include <QJsonObject>
#include <functional>

class QLocalServer;

// One newline-delimited JSON request/response per local connection.
// Lives on the GUI thread, so handlers may safely operate widgets.
class ControlService : public QObject
{
public:
    using Handler = std::function<QJsonObject(const QJsonObject &)>;
    explicit ControlService(Handler handler, QObject *parent = nullptr);
    bool start(const QString &path = QString());
    QString errorString() const;
    static QString defaultSocketPath();
    static QJsonObject error(const QString &code, const QString &message);

private:
    void acceptConnections();
    QLocalServer *m_server;
    Handler m_handler;
    bool m_handling = false;
};

#endif
