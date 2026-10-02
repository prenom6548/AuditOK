#include <iostream>
#include <QAction>
#include <QMenu>
#include "action.h"

namespace tray {

Action::Action(const std::string &name)
    : m_type(KomActionType::NONE)
    , m_name(name)
    , m_qtAction(std::make_shared<QAction>())
{
    /* nothing to do */
}

Action::~Action() {
    std::cout << "Action destruction exec." << std::endl;
}

void Action::enroll(QMenu *menu) {
    menu->addAction(m_qtAction.get());
}

void Action::setType(KomActionType type) {
    m_type = type;
}

void Action::setIcon(QIcon icon) {
    if (m_type == KomActionType::NORMAL) {
        m_qtAction->setIcon(icon);
    }
}

void Action::setLabel(const std::string &label) {
    if (m_type == KomActionType::CHECK || m_type == KomActionType::NORMAL)
        m_qtAction->setText(QString::fromStdString(label));
}

void Action::setTooltip(const std::string &tooltip) {
    if (m_type != KomActionType::CHECK || m_type == KomActionType::NORMAL)
        m_qtAction->setToolTip(QString::fromStdString(tooltip));
}

void Action::setActive(bool isActive) {
    if (m_type == KomActionType::CHECK) {
        if (isActive) {
            m_qtAction->setIcon(QIcon::fromTheme("object-select-symbolic"));
        } else {
            std::cout << "++++++++++++" << QIcon().isNull() << std::endl;
            m_qtAction->setIcon(QIcon());
        }
    }
}

void Action::setCallback(std::function<bool()> callback) {
    if (!callback)
        return;

    if (m_type == KomActionType::CHECK) {
        connect(m_qtAction.get(), &QAction::triggered, this, [=]() {
            bool ret = callback();
            if (ret) {
                setActive(true);
            } else {
                setActive(false);
            }
        });
    }
    if (m_type == KomActionType::NORMAL) {
        connect(m_qtAction.get(), &QAction::triggered, this, [=]() {
            callback();
        });
    }
}

}
