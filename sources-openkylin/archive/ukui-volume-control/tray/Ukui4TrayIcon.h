#ifndef UKUIVERSION4TRAYICON_H
#define UKUIVERSION4TRAYICON_H

#include <memory>
#include <QAction>
#include <QObject>

#include "ISoundSystemTrayIcon.h"

class IMainWindow;
class UkuiVersion4TrayIcon : public QObject, public ISoundSystemTrayIcon
{
    Q_OBJECT
public:
    UkuiVersion4TrayIcon(UkmediaTrayIcon*, IMainWindow*);
    ~UkuiVersion4TrayIcon() = default;

public:
    virtual void initTray() override;

    virtual void initSlot() override;

    virtual void setVisible(bool) override;

    virtual void setIcon(const QIcon&) override;

private slots:
    void soundPreferenceActionTriggered();

    void activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason);

private:
    IMainWindow* m_pMainWid;
};

#endif // UKUIVERSION4TRAYICON_H
