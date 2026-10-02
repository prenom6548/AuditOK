#ifndef GLOBALSIGNAL_H
#define GLOBALSIGNAL_H

#include <QObject>

class GlobalSignal : public QObject {
Q_OBJECT
public:
    static GlobalSignal *getInstance();

signals:
    void installSuccessSignal();

private:
    explicit GlobalSignal(QObject *parent = nullptr);

    static GlobalSignal *instance;

    ~GlobalSignal();
};

#define g_signal GlobalSignal::getInstance()

#endif // GLOBALSIGNAL_H
