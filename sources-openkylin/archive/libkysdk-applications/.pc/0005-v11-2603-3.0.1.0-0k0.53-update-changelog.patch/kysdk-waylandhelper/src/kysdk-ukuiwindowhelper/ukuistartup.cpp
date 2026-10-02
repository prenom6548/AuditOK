#include "ukuistartup.h"
#include "ukuidisplay_p.h"

#include "wayland-ukui-startup-v1-client-protocol.h"

#include <QDebug>

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

private:
    ukui_startup_management_v1 *m_ukuiStartupManager = nullptr;
};

UkuiStartupManagerPrivate::UkuiStartupManagerPrivate()
{
    m_ukuiStartupManager = static_cast<ukui_startup_management_v1 *>(ukuiDisplay()->bind(ukui_startup_management_v1_interface.name, &ukui_startup_management_v1_interface, ukui_startup_management_v1_interface.version));
}

UkuiStartupManagerPrivate::~UkuiStartupManagerPrivate()
{
    if (m_ukuiStartupManager) {
        ukui_startup_management_v1_destroy(m_ukuiStartupManager);
        m_ukuiStartupManager = nullptr;
    }
}

void UkuiStartupManagerPrivate::setStartupGeometryByPid(uint32_t pid, int x, int y, uint32_t width, uint32_t height)
{
    if (!m_ukuiStartupManager) {
        qWarning() << "UkuiStartupManager is not initialized";
        return;
    }

    if (width <= 0 || height <= 0) {
        qWarning() << "Invalid width or height for startup geometry";
        return;
    }

    ukui_startup_management_v1_set_startup_geometry(m_ukuiStartupManager, pid, x, y, width, height);
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