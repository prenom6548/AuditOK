#pragma once

#include <QObject>

class UkuiStartupManager;
struct ukui_startup_management_v1;

UkuiStartupManager *ukuiStartupManager();

class UkuiStartupManager
{
public:
    ~UkuiStartupManager();
    void setStartupGeometryByPid(uint32_t pid, int x, int y, uint32_t width, uint32_t height);

private:
    ukui_startup_management_v1 *m_ukuiStartupManager = nullptr;

    explicit UkuiStartupManager();
    friend UkuiStartupManager *ukuiStartupManager();
};
