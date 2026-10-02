#ifndef UKUIVERSIONTWOTRAYICON_H
#define UKUIVERSIONTWOTRAYICON_H

#include <memory>
#include "ISoundSystemTrayIcon.h"

class IMainWindow;
class UkuiVersionTwoTrayIcon : public QObject, public ISoundSystemTrayIcon
{
    Q_OBJECT
public:
    UkuiVersionTwoTrayIcon(UkmediaTrayIcon*, IMainWindow*);
    ~UkuiVersionTwoTrayIcon() = default;

public:
    virtual void initTray() override;

    virtual void initSlot() override;

    virtual void setVisible(bool) override;

    virtual void setIcon(const QIcon&);

private Q_SLOTS:
    void soundPreferenceActionTriggered();

    void activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason);

private:
    IMainWindow* m_pMainWid;
};

#endif // UKUIVERSIONTWOTRAYICON_H
