#include "waylandscreenmap.h"

#include <KF5/KWayland/Client/registry.h>
#include <KF5/KWayland/Client/screen_map.h>
#include <KF5/KWayland/Client/connection_thread.h>
#include <QThread>
#include <QDebug>

WaylandScreenMap::WaylandScreenMap(QObject* parent)
    : QObject(parent)
    , m_connection(new KWayland::Client::ConnectionThread())
    , m_registry(new KWayland::Client::Registry())
    , m_thread(new QThread())
{
    // can't use ConnectionThread::fromApplication() because hw environment use xcb QPA.
    m_connection->moveToThread(m_thread);
    m_thread->start();

    connect(m_connection, &KWayland::Client::ConnectionThread::connected, this, [this]() {
        setupRegistry();
    });
    connect(m_connection, &KWayland::Client::ConnectionThread::failed, this, [this]() {
        qWarning() << "Connect to wayland server error:" << m_connection->errorCode();
        emit initError();
    });
    connect(m_connection, &KWayland::Client::ConnectionThread::connectionDied, this, [this]() {
        qWarning() << "Connection to wayland server died.";
        emit initError();
    });

    m_connection->initConnection();

}

WaylandScreenMap::~WaylandScreenMap()
{
    /*
    * Registry must delete before ConnectionThread, because
    * when Registry delete, will call Registry::release(),
    * Registry::release() need connect to wayland server,
    * if ConnectionThread delete, the connection will break.
    */
    if (m_screenMap)
        m_screenMap->deleteLater();

    m_registry->deleteLater();

    m_thread->quit();
    m_thread->wait();
    m_connection->deleteLater();
    m_thread->deleteLater();
}

void WaylandScreenMap::setupRegistry()
{
    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, this, [this](){
        createScreenMap();
    });
    m_registry->create(m_connection);
    m_registry->setup();
}

void WaylandScreenMap::mapToOutput(int inputDeviceId, const QString &outputName)
{
    if (!m_screenMap) {
        qWarning() << "ScreenMap don't be inited,  can't do mapToOutput";
        return;
    }

    qDebug() << "map input device" << inputDeviceId << "to" << outputName;
    m_screenMap->set_screen_map_status(inputDeviceId, outputName);
    m_connection->flush();
}

void WaylandScreenMap::createScreenMap()
{
    if (!m_registry || !m_registry->isValid()) {
        qWarning() << "Registry isn't valid, can't create ScreenMap";
        emit initError();
        return;
    }
    bool hasScreenMap = m_registry->hasInterface(KWayland::Client::Registry::Interface::ScreenMap);
    if (!hasScreenMap) {
        qWarning() << "ScreenMap interface is not available, can't create ScreenMap";
        emit initError();
        return;
    }

    auto interface = m_registry->interface(KWayland::Client::Registry::Interface::ScreenMap);
    if (0 == interface.name) {
        qWarning() << "ScreenMap interface is not available, can't create ScreenMap";
        emit initError();
        return;
    }

    m_screenMap = m_registry->createScreenMap(interface.name, interface.version);
    connect(m_screenMap, &KWayland::Client::ScreenMap::removed, this, [this]() {
        qInfo() << "ScreenMap interface removed, delete the ScreenMap interface object";
        m_screenMap->deleteLater();
        m_screenMap = nullptr;
    });

    emit initFinish();
}
