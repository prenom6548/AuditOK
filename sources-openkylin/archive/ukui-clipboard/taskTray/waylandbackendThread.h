#ifndef WAYLANDBACKENDTHREAD_H
#define WAYLANDBACKENDTHREAD_H

#include <QObject>
#include <QThread>
#include <atomic>
#include <memory>
#include "wlr-data-control-unstable-v1.h"

class WaylandBackendThread : public QThread
{
    Q_OBJECT

protected:
    virtual void run();

public:
    explicit WaylandBackendThread(QObject *parent = nullptr);
    ~WaylandBackendThread();

    void stop();

signals:
    void selectionNil();
    void selectionNew();
    void errorOccurred(const QString &error);

private slots:
    void processEvents();
    bool initialize();

private:
    void cleanup();

    wayland::display_t display;
    wayland::registry_t registry;
    wayland::seat_t seat;
    wayland::zwlr_data_control_manager_v1_t dataControlManager;
    wayland::zwlr_data_control_device_v1_t dataDevice;
    wayland::zwlr_data_control_offer_v1_t dataOffer;

    std::atomic<bool> is_running{false};

    bool initialized = false;   // 标记是否已初始化
};

#endif // WAYLANDBACKENDTHREAD_H
