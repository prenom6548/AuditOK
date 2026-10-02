#include <QMenu>

#include "kom_tray_interface.h"
#include "action.h"
#include "tray.h"

namespace tray {

Tray::Tray(KomTrayInterface *interface) {
    m_interface.reset(interface);
}

Tray::~Tray() {

}

std::string Tray::name() {
    return m_interface->name();
}

time_t Tray::uptime() {
    time_t now;
    time(&now);
    return now - m_startTime;
}

void Tray::enroll() {
    // ToDo: 根据配置文件注册

    simpleEnroll();

    time(&m_startTime);
}

void Tray::simpleEnroll() {
    m_qtTray = std::make_shared<QSystemTrayIcon>();
    connect(m_qtTray.get(), &QSystemTrayIcon::activated, this, &Tray::onQtTrayActivated);

    // 设置托盘图标
    std::string iconName = m_interface->icon();
    m_qtTray->setIcon(getIcon(iconName));

    // 设置 tooltip
    std::string tooltip = m_interface->tooltip();
    m_qtTray->setToolTip(QString::fromStdString(tooltip));

    // 设置右键菜单
    m_menu = std::make_shared<QMenu>();
    std::vector<KomActionEntry> actionEntry = m_interface->actions();
    for (const auto &item: actionEntry) {
        if (item.type == KomActionType::SEPARATORS) {
            m_menu->addSeparator();
            continue;
        }
        std::shared_ptr<Action> action = std::make_shared<Action>(item.name);
        action->enroll(m_menu.get());
        action->setType(item.type);
        action->setIcon(getIcon(item.icon));
        action->setLabel(item.label);
        action->setTooltip(item.tooltip);
        action->setActive(item.isActive);
        action->setCallback(item.callback);
        m_actions.emplace(std::make_pair(item.name, action));
    }
    m_qtTray->setContextMenu(m_menu.get());

    m_qtTray->show();
}

QIcon Tray::getIcon(const std::string &name) {
    if (name.empty())
        return QIcon();

    if (name.find("/") == std::string::npos)
        return QIcon::fromTheme(QString::fromStdString(name));
    else
        return QIcon(QString::fromStdString(name));
}

void Tray::onQtTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            m_interface->trigger();
            break;
        case QSystemTrayIcon::MiddleClick:
            break;
        default:
            ;
    }
}

}
