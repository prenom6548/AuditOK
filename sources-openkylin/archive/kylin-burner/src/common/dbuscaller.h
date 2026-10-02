#ifndef DBUSCALLER_H
#define DBUSCALLER_H

#include <QObject>
#include <QCoreApplication>

class DBusCaller : public QObject {
    Q_OBJECT
public:
    enum CallResult {
        Failure = 1,
        Success = 0,
        Timeout = -1,
        MethodNotExist = -2,
        DBusError = -3,
    };

    explicit DBusCaller(QObject *parent = nullptr);

    int callSensitiveInfoCheck(const QString &orderNo, const QStringList &filePathGroup, int timeoutMs = 5000);
    bool handleSensitiveInfoCheckResult(int result);
};

#endif // DBUSCALLER_H

