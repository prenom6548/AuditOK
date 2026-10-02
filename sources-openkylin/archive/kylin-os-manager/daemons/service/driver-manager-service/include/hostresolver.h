#ifndef HOSTRESOLVER_H
#define HOSTRESOLVER_H

#include <QObject>

#include <QThread>
#include <QTimer>
#include <QFuture>
#include <QtConcurrent>
#include <QDebug>
#include <QHostInfo>

class HostResolver : public QObject {
    Q_OBJECT
public:
    static int resolve(const QString &hostname, int timeout);

};

#endif // HOSTRESOLVER_H
