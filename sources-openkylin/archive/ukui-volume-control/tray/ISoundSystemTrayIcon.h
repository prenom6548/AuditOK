#ifndef UKUIVOLUMECONTROLAPPLETTRAY_H
#define UKUIVOLUMECONTROLAPPLETTRAY_H

#include <QMenu>
#include <QObject>

#include "UkmediaTrayIcon.h"

class ISoundSystemTrayIcon
{
public:
    ISoundSystemTrayIcon(UkmediaTrayIcon* icon) : m_pTrayIcon(icon) {}
    ~ISoundSystemTrayIcon() = default;

public:
    virtual void initTray() = 0;

    virtual void initSlot() = 0;

    virtual void setVisible(bool) = 0;

    virtual void setIcon(const QIcon&) = 0;

protected:
    QMenu* m_pMenu = nullptr;
    UkmediaTrayIcon* m_pTrayIcon = nullptr;
};

#endif // UKUIVOLUMECONTROLAPPLETTRAY_H
