#ifndef APP_H
#define APP_H

#include <QApplication>
#include <QString>

class QLocalServer;
class QEvent;

class App : public QApplication
{
    Q_OBJECT

public:
    explicit App(int &argc, char **argv);
    ~App();

    bool isPrimaryInstance() const;
    bool sendActivationMessage(const QString &message = QStringLiteral("activate"));

signals:
    void aboutToQuit();
    void messageReceived(const QString &message);

public slots:
    void handleQuit();

private:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool createLocalServer();
    bool tryConnectToPrimary();
    void cleanupLocalServer();

private slots:
    void handleNewConnection();

private:
    QLocalServer *m_localServer;
    QString m_socketName;
    bool m_primaryInstance;
};

#endif // APP_H
