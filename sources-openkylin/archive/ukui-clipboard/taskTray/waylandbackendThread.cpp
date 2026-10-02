#include "waylandbackendThread.h"
#include <poll.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <QCoreApplication>
#include <QDebug>
#include <cstring>

WaylandBackendThread::WaylandBackendThread(QObject *parent)
    : QThread(parent)
{
    if (is_running) {
        qDebug() << "WaylandBackend 已经在运行";
        return;
    }

    is_running = true;
    setObjectName("WaylandEventThread");
}

WaylandBackendThread::~WaylandBackendThread()
{
    stop();
}

bool WaylandBackendThread::initialize()
{
    try {
        qDebug() << "正在初始化 Wayland...";

        // 获取注册表
        registry = display.get_registry();

        // 设置全局对象回调
        registry.on_global() = [this](uint32_t name, const std::string &interface, uint32_t version) {

            if (interface == wayland::detail::zwlr_data_control_manager_v1_interface.name) {
                dataControlManager = static_cast<wayland::zwlr_data_control_manager_v1_t>(
                    registry.bind(name, dataControlManager,
                                 std::min<uint32_t>(version, wayland::detail::zwlr_data_control_manager_v1_interface.version)));
            } else if (interface == wayland::detail::seat_interface.name) {
                seat = static_cast<wayland::seat_t>(
                    registry.bind(name, seat,
                                 std::min<uint32_t>(version, wayland::detail::seat_interface.version)));
            }
        };

        // 等待注册完成
        display.roundtrip();

        if (!seat || !dataControlManager) {
            qCritical() << "无法获取必要的 Wayland 对象";
            emit errorOccurred("Failed to get required Wayland objects");
            return false;
        }

        // 创建数据设备
        dataDevice = dataControlManager.get_data_device(seat);
        qDebug() << "创建数据设备成功";

        // 设置 selection 信号处理器
        dataDevice.on_selection() = [this](wayland::zwlr_data_control_offer_v1_t offer) {
            if (offer) {
                emit selectionNew();
            } else {
                emit selectionNil();
            }
        };

        dataDevice.on_data_offer() = [this](wayland::zwlr_data_control_offer_v1_t offer) {
            dataOffer = offer;
        };

        qDebug() << "Wayland 初始化完成";
        initialized = true;
        return true;
    } catch (const std::exception& e) {
        qCritical() << "初始化异常:" << e.what();
        emit errorOccurred(QString("Failed to initialize WaylandBackend: %1").arg(e.what()));
        return false;
    }
}

void WaylandBackendThread::stop()
{
    if (!is_running) {
        return;
    }

    qDebug() << "正在停止 Wayland 后端...";
    is_running = false;

    // 清理
    cleanup();

    qDebug() << "Wayland 后端已停止";
}

void WaylandBackendThread::run()
{
    qDebug() << "Wayland 线程启动，线程ID:" << QThread::currentThreadId();

    if (initialize()) {
        qDebug() << "初始化成功";
    } else {
        qWarning() << "初始化失败";
        stop();
    }

    while (true) {
        processEvents();
    }
}

void WaylandBackendThread::processEvents()
{
    if (!is_running || !initialized) {
        return;
    }

    try {
        auto intent = display.obtain_read_intent();
        display.flush();
        struct pollfd pfd;
        pfd.fd = display.get_fd();
        pfd.events = POLLIN;
        poll(&pfd, 1, -1);
        if (pfd.revents & POLLIN) {
            intent.read();
        } else {
            intent.cancel();
        }
        display.dispatch_pending();

        // 处理Qt事件
        QCoreApplication::processEvents();
    } catch (const std::exception& e) {
        qDebug() << "处理事件异常:" << e.what();
    }
}

void WaylandBackendThread::cleanup()
{
    // 清理 Wayland 资源
    dataDevice = wayland::zwlr_data_control_device_v1_t();
    dataOffer = wayland::zwlr_data_control_offer_v1_t();
    seat = wayland::seat_t();
    dataControlManager = wayland::zwlr_data_control_manager_v1_t();
    registry = wayland::registry_t();
    initialized = false;
}
