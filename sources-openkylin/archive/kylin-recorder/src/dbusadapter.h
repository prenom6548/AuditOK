#ifndef DBUSADAPTER_H
#define DBUSADAPTER_H
#include <QObject>
#include <QDebug>
#include <QDBusConnection>
class DbusAdapter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

signals:
    void sigShowPause();

public:
    explicit DbusAdapter(QObject *parent = nullptr);

public Q_SLOTS:
    // mpris 使用
    void Pause();
};
#endif // DBUSADAPTER_H
