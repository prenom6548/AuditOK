
#include "signalhandler.h"
#include "loghelper.h"
#include <sys/socket.h>
#include <unistd.h>

int SignalHandler::sigtermFd[2];

SignalHandler::SignalHandler(QObject *parent)
    : QObject(parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd)) {
        PRINT_NOTICE("Couldn't create TERM socketpair\n");
    }

    notifier = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &SignalHandler::handleSignalActivated);
}

SignalHandler::~SignalHandler()
{
    ::close(sigtermFd[0]);
    ::close(sigtermFd[1]);
}

void SignalHandler::setupHandlers()
{
    struct sigaction term;
    term.sa_handler = SignalHandler::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = SA_RESTART;
    if (sigaction(SIGTERM, &term, 0) > 0) {
        PRINT_NOTICE("Failed to install SIGTERM handler\n");
    }
}

void SignalHandler::termSignalHandler(int)
{
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}

void SignalHandler::handleSignalActivated()
{
    notifier->setEnabled(false);
    char tmp;
    ::read(sigtermFd[1], &tmp, sizeof(tmp));

    Q_EMIT sigTermReceived();

    notifier->setEnabled(true);
}