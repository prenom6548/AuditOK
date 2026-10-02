#ifndef PLASMASHELLMANAGER_H
#define PLASMASHELLMANAGER_H

#include <QObject>
#include <QWindow>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/shell.h>

class PlasmaShellManager : public QObject
{
    Q_OBJECT
public:
    static PlasmaShellManager *getInstance();

    bool setAppWindowActive();
    bool setAppWindowKeepAbove(bool keep);
    bool setMaximized(QWindow *window);
    bool setRole(QWindow *window, KWayland::Client::PlasmaShellSurface::Role role);
    bool setPos(QWindow *window, const QPoint &pos);
    bool supportPlasmaShell();
    bool supportShell();
    bool supportPlasmaWindowManagement();

private:
    explicit PlasmaShellManager(QObject *parent = nullptr);

    KWayland::Client::PlasmaShell *m_plasmaShell = nullptr;
    KWayland::Client::Shell *m_shell = nullptr;
    KWayland::Client::PlasmaWindowManagement *m_windowManager = nullptr;
    KWayland::Client::PlasmaWindow *m_appWindow = nullptr;

    bool isFirstCreate = true;
};


#endif // PLASMASHELLMANAGER_H
