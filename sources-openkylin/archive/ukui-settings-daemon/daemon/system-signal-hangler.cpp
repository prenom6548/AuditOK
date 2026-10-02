#include <csignal>
#include <execinfo.h>

#include <QDebug>
#include "system-signal-hangler.h"
#include "clib-syslog.h"
SystemSignalHangler::SystemSignalHangler()
{
    installSystemSignalHandler();
}


void SystemSignalHangler::sigtermHandler(int signo)
{
    qDebug() << "Caught SIGTERM signal, exit with SIGTERM";
    exit(signo);
}

QString SystemSignalHangler::getCallStack() {
    constexpr int maxStackDepth = 128;
    void* stackBuffer[maxStackDepth];
    const int stackDepth = backtrace(stackBuffer, maxStackDepth);
    char** symbols = backtrace_symbols(stackBuffer, stackDepth);
    if (symbols == nullptr) {
        return {};
    }

    qWarning() << "The stack depth is " << stackDepth;

    QString symbolList;
    for (int i = 0; i < stackDepth; i++) {
        SYS_LOG(LOG_DEBUG,"%s",symbols[i]);
    }

    free(symbols);

    return symbolList;
}

void SystemSignalHangler::handleFatalSignal(int signo)
{
    qWarning() << "Signal is caught. The number is: " << signo;
    getCallStack();

    // 安装默认信号处理器并将当前信号转发出去，
    // 从而可以使系统执行默认处理逻辑
    std::signal(signo, SIG_DFL);
    std::raise(signo);
}

void SystemSignalHangler::installSystemSignalHandler()
{
    std::signal(SIGBUS, handleFatalSignal);
    std::signal(SIGSEGV, handleFatalSignal);
    std::signal(SIGTERM, sigtermHandler);
}

