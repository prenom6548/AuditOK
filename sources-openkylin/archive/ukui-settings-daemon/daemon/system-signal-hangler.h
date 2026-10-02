#ifndef SYSTEMSIGNALHANGLER_H
#define SYSTEMSIGNALHANGLER_H
#include <QObject>

class SystemSignalHangler
{
public:
    SystemSignalHangler();
    static void sigtermHandler(int signo);
    static void handleFatalSignal(int signo);
    static QString getCallStack();
private:


    void installSystemSignalHandler();
};

#endif // SYSTEMSIGNALHANGLER_H
