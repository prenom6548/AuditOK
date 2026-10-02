#include "ukuistartup.h"
#include "ukuidisplay_p.h"

#include "wayland-ukui-startup-v1-client-protocol.h"

UkuiStartupManager *ukuiStartupManager()
{
    static UkuiStartupManager *instance = nullptr;
    if (!instance) {
        instance = new UkuiStartupManager();
    }
    return instance;
}

UkuiStartupManager::UkuiStartupManager()
{
    m_ukuiStartupManager = static_cast<ukui_startup_management_v1 *>(ukuiDisplay()->bind(ukui_startup_management_v1_interface.name, &ukui_startup_management_v1_interface, ukui_startup_management_v1_interface.version));
}

UkuiStartupManager::~UkuiStartupManager()
{
    if (m_ukuiStartupManager)
        ukui_startup_management_v1_destroy(m_ukuiStartupManager);

    m_ukuiStartupManager = nullptr;
}

void UkuiStartupManager::setStartupGeometryByPid(uint32_t pid, int x, int y, uint32_t width, uint32_t height)
{
    if (!m_ukuiStartupManager) {
        return;
    }

    if (width <= 0 || height <= 0) {
        return;
    }

    ukui_startup_management_v1_set_startup_geometry(m_ukuiStartupManager, pid, x, y, width, height);
}
