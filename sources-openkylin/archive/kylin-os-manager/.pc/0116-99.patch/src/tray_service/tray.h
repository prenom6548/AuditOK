#ifndef KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_TRAY_H
#define KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_TRAY_H

#include <time.h>
#include <memory>
#include <string>
#include <QSystemTrayIcon>
#include <QObject>
#include <QIcon>

#include "action.h"

class KomTrayInterface;
class QMenu;

namespace tray {

class Tray : public QObject {
    Q_OBJECT

public:
    Tray(KomTrayInterface *interface);
    ~Tray();

    void enroll();
    std::string name();
    time_t uptime();

private:
    QIcon getIcon(const std::string &name);
    void simpleEnroll();

    time_t m_startTime;
    std::shared_ptr<KomTrayInterface> m_interface;
    std::shared_ptr<QSystemTrayIcon> m_qtTray;
    std::shared_ptr<QMenu> m_menu;
    std::map<std::string, std::shared_ptr<Action>> m_actions;

private Q_SLOTS:
    void onQtTrayActivated(QSystemTrayIcon::ActivationReason reason);
};

}

#endif
