#ifndef WAYLANDSCREENMAP_H
#define WAYLANDSCREENMAP_H

#include <QObject>

namespace KWayland::Client {
class Registry;
class ConnectionThread;
class ScreenMap;
}
class QThread;
class WaylandScreenMap : public QObject
{
    Q_OBJECT
public:
    WaylandScreenMap(QObject* parent = nullptr);
    ~WaylandScreenMap();

    void mapToOutput(int inputDeviceId, const QString& outputName);

signals:
    void initFinish();
    void initError();

private slots:
    void setupRegistry();

private:
    void createScreenMap();

    KWayland::Client::ConnectionThread* m_connection = nullptr;
    KWayland::Client::Registry* m_registry = nullptr;
    QThread* m_thread = nullptr;
    KWayland::Client::ScreenMap* m_screenMap = nullptr;
};

#endif // WAYLANDSCREENMAP_H
