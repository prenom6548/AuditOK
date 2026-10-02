#ifndef VADBUSOPERATOR_H
#define VADBUSOPERATOR_H

#include <QObject>
#include <QString>

class VADBusOperator : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.voice_assistant")

public:
    explicit VADBusOperator(QObject *parent = nullptr);
    ~VADBusOperator();

    static QString pidFilePath();

    enum ExitReason {
        CloseButton = 0,
        Abnormal = 1
    };

signals:
    void ProcessExiting(int reason);
    void sigShowWindow();
    void sigQuit();

public slots:
    void ShowWindow();
    void Quit();
    bool Ping();
    qlonglong ProcessId();
};

#endif // VADBUSOPERATOR_H
