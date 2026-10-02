#ifndef DESKTOP_WIDGETS_MANAGER_H
#define DESKTOP_WIDGETS_MANAGER_H

#include <QObject>

class PlatformWindow;

class DesktopWidgetsManager : public QObject
{
    Q_OBJECT
public:
    static DesktopWidgetsManager *getInstance();

    int getNextInstanceId();
    int addWidget(PlatformWindow *window, const QString &pluginId);
    void removeWidget(int instanceId, PlatformWindow *window = nullptr);
    void mergeWidgetsToPrimaryScreen();
    void restoreWidgetsFromPrimaryScreen();
    void swapWidgets(PlatformWindow *window1, PlatformWindow *window2);

Q_SIGNALS:
private:
    explicit DesktopWidgetsManager(QObject *parent = nullptr);
    ~DesktopWidgetsManager();
};

#endif // DESKTOP_WIDGETS_MANAGER_H
