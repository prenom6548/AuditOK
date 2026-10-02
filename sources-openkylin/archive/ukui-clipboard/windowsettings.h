#ifndef WINDOWSETTINGS_H
#define WINDOWSETTINGS_H

#include <QMap>
#include <QObject>
#include <QWindow>

// kysdk
#include <kysdk/applications/windowmanager/windowmanager.h>

class WindowSettings : public QObject
{
    Q_OBJECT
public:
    explicit WindowSettings(QObject *parent = nullptr);

private:
    void setKeepAbove();
    void setSkipTaskbarAndSwitcher();
    void removeTitleBar();

signals:

private:
    bool m_isWayland = false;
    QMap<QWindow*, kdk::WindowId> m_windowMap;
    QWindow *m_functionMenuWindow = nullptr;
    QWindow *m_previewWindow = nullptr;
};

#endif // WINDOWSETTINGS_H
