#include "hostresolver.h"

int HostResolver::resolve(const QString &hostname, int timeout)
{
    QFuture<int> future = QtConcurrent::run([hostname]() {
        QHostInfo host = QHostInfo::fromName(hostname);
        return static_cast<int>(host.error());
    });

    QFutureWatcher<int> watcher;
    QEventLoop loop;

    QObject::connect(&watcher, &QFutureWatcher<int>::finished, &loop, &QEventLoop::quit);
    watcher.setFuture(future);

    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeout);

    loop.exec();

    if (timer.isActive()) {
        timer.stop();
        return future.result();
    } else {
        qDebug() << "解析超时";
        return 1;  // 返回一个表示超时的错误代码
    }
}
