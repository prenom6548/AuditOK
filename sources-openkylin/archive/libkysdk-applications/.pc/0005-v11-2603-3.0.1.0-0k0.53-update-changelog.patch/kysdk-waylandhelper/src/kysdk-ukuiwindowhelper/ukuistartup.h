#pragma once

#include <QObject>

class UkuiStartupManager;
class UkuiStartupManagerPrivate;

UkuiStartupManager *ukuiStartupManager();

class UkuiStartupManager
{
public:
    ~UkuiStartupManager();
    void setStartupGeometryByPid(uint32_t pid, int x, int y, uint32_t width, uint32_t height);

private:
    UkuiStartupManagerPrivate *p = nullptr;

    explicit UkuiStartupManager();
    friend UkuiStartupManager *ukuiStartupManager();
};
