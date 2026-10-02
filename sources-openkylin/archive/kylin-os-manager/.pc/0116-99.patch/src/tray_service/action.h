#ifndef KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_ACTION_H
#define KYLIN_OS_MANAGER_SRC_TRAY_SERVICE_ACTION_H

#include <memory>
#include <string>
#include <functional>
#include <QObject>
#include <QIcon>

#include "kom_tray_interface.h"

class QAction;
class QMenu;

namespace tray {

class Action : public QObject {
    Q_OBJECT

public:
    Action(const std::string &name);
    ~Action();

    void enroll(QMenu *menu);
    void setType(KomActionType type);
    void setIcon(QIcon icon);
    void setLabel(const std::string &label);
    void setTooltip(const std::string &tooltip);
    void setActive(bool isActive);
    void setCallback(std::function<bool()> callback);

private:
    KomActionType m_type;   // 普通类型和 check 类型
    std::string m_name;
    std::shared_ptr<QAction> m_qtAction;
};

}

#endif
