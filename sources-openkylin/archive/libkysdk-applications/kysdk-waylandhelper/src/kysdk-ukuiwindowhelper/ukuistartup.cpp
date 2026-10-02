#include "ukuistartup.h"
#include "ukuidisplay_p.h"

#include "wayland-ukui-startup-v1-client-protocol.h"
#include "wayland-ukui-startup-v2-client-protocol.h"

#include <QDebug>
#include <QFile>
#include <QGuiApplication>

UkuiStartupManager *ukuiStartupManager()
{
    static UkuiStartupManager *instance = nullptr;
    if (!instance) {
        instance = new UkuiStartupManager();
    }
    return instance;
}

class UkuiStartupManagerPrivate
{
public:
    UkuiStartupManagerPrivate();
    ~UkuiStartupManagerPrivate();

    void setStartupGeometryByPid(uint32_t pid, int x, int y, uint32_t width, uint32_t height);
    void setStartupGeometryEx(std::string appId, uint32_t pid, int x, int y, uint32_t width, uint32_t height);

private:
    ukui_startup_management_v1 *m_ukuiStartupManager = nullptr;
    ukui_startup_management_v2 *m_ukuiStartupManagerV2 = nullptr;
};

UkuiStartupManagerPrivate::UkuiStartupManagerPrivate()
{
    if (qApp->platformName() != QLatin1String("wayland")) {
        qWarning() << "UkuiStartupManager only works on wayland platform";
        return;
    }

    m_ukuiStartupManager = static_cast<ukui_startup_management_v1 *>(ukuiDisplay()->bind(ukui_startup_management_v1_interface.name, &ukui_startup_management_v1_interface, ukui_startup_management_v1_interface.version));
    m_ukuiStartupManagerV2 = static_cast<ukui_startup_management_v2 *>(ukuiDisplay()->bind(ukui_startup_management_v2_interface.name, &ukui_startup_management_v2_interface, ukui_startup_management_v2_interface.version));
}

UkuiStartupManagerPrivate::~UkuiStartupManagerPrivate()
{
    if (m_ukuiStartupManagerV2) {
        ukui_startup_management_v2_destroy(m_ukuiStartupManagerV2);
        m_ukuiStartupManagerV2 = nullptr;
    }

    if (m_ukuiStartupManager) {
        ukui_startup_management_v1_destroy(m_ukuiStartupManager);
        m_ukuiStartupManager = nullptr;
    }
}

void UkuiStartupManagerPrivate::setStartupGeometryByPid(uint32_t pid, int x, int y, uint32_t width, uint32_t height)
{
    if (width <= 0 || height <= 0) {
        qWarning() << "Invalid width or height for startup geometry";
        return;
    }

    if (m_ukuiStartupManagerV2) {
        ukui_startup_info_v2 *start_info = ukui_startup_management_v2_create_startup_info(m_ukuiStartupManagerV2);
        ukui_startup_info_v2_set_startup_geometry(start_info, x, y, width, height);
        ukui_startup_info_v2_set_pid(start_info, pid);

        QFile file("/proc/" + QString::number(pid) + "/cmdline");
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray raw = file.readAll();
            QString cmdline = QString::fromUtf8(raw).replace('\0', ' ');
            QString appId;
            if (!cmdline.trimmed().isEmpty()) {
                QStringList parts = cmdline.split(" ", Qt::SkipEmptyParts);
                if (!parts.isEmpty()) {
                    QString first = parts.first();
                    QStringList pathParts = first.split("/", Qt::SkipEmptyParts);
                    if (!pathParts.isEmpty()) {
                        appId = pathParts.last();
                    }
                }
            }
            if (!appId.isEmpty()) {
                ukui_startup_info_v2_set_appid(start_info, appId.toUtf8().constData());
            } else {
                qWarning() << "Failed to extract appId from cmdline:" << cmdline;
            }
        }

        wl_display_roundtrip(ukuiDisplay()->display());
        ukui_startup_info_v2_destroy(start_info);

    } else if (m_ukuiStartupManager) {
        ukui_startup_management_v1_set_startup_geometry(m_ukuiStartupManager, pid, x, y, width, height);
        wl_display_roundtrip(ukuiDisplay()->display());
    }
}

void UkuiStartupManagerPrivate::setStartupGeometryEx(std::string appId, uint32_t pid, int x, int y, uint32_t width, uint32_t height)
{
    if (width <= 0 || height <= 0) {
        qWarning() << "Invalid width or height for startup geometry";
        return;
    }

    if (m_ukuiStartupManagerV2) {
        ukui_startup_info_v2 *start_info = ukui_startup_management_v2_create_startup_info(m_ukuiStartupManagerV2);
        ukui_startup_info_v2_set_startup_geometry(start_info, x, y, width, height);
        ukui_startup_info_v2_set_pid(start_info, pid);
        ukui_startup_info_v2_set_appid(start_info, appId.c_str());
        wl_display_roundtrip(ukuiDisplay()->display());
        ukui_startup_info_v2_destroy(start_info);
    } else if (m_ukuiStartupManager) {
        ukui_startup_management_v1_set_startup_geometry(m_ukuiStartupManager, pid, x, y, width, height);
        wl_display_roundtrip(ukuiDisplay()->display());
    }
}

UkuiStartupManager::UkuiStartupManager()
    : p(new UkuiStartupManagerPrivate())
{
}

UkuiStartupManager::~UkuiStartupManager()
{
    delete p;
    p = nullptr;
}

void UkuiStartupManager::setStartupGeometryByPid(uint32_t pid, int x, int y, uint32_t width, uint32_t height)
{
    p->setStartupGeometryByPid(pid, x, y, width, height);
}

void UkuiStartupManager::setStartupGeometryEx(std::string appId, uint32_t pid, int x, int y, uint32_t width, uint32_t height)
{
    p->setStartupGeometryEx(appId, pid, x, y, width, height);
}
