#ifndef KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_H
#define KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_H

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <utility>
#include <QObject>

#include "tray.h"

namespace kyplugin {
class KyPlugin;
}

namespace tray {

class Daemon : public QObject {
    Q_OBJECT

public:
    Daemon();
    ~Daemon();

    void enrollTrays();
    std::pair<bool, std::string> startTray(std::string name);
    std::pair<bool, std::string> stopTray(std::string name);
    std::map<std::string, std::shared_ptr<Tray>> trays();
    std::map<std::string, KomTrayProvider *> trayProviders();

private:
    void loadPlugin();

    std::vector<std::string> m_loadPath;
    kyplugin::KyPlugin *m_pluginManager;
    std::map<std::string, KomTrayProvider *> m_trayProviders;
    std::map<std::string, std::shared_ptr<Tray>> m_trays;    // 在容器中，则证明托盘已启动
};

}

#endif
