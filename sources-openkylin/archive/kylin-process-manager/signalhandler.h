#include <QObject>
#include <QSocketNotifier>
#include <csignal>

class SignalHandler : public QObject
{
    Q_OBJECT
public:
    explicit SignalHandler(QObject *parent = nullptr);
    ~SignalHandler();
    static void setupHandlers();

Q_SIGNALS:
    void sigTermReceived();

private Q_SLOTS:
    void handleSignalActivated();

private:
    static int sigtermFd[2];
    QSocketNotifier *notifier;

    static void termSignalHandler(int);
};
