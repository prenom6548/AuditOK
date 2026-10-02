#ifndef TRAYICON_H
#define TRAYICON_H

#include <QWidget>
#include <QSystemTrayIcon>
#include "sysresource/cpuhistorywidget.h"
#include "sysresource/memhistorywidget.h"
#include "sysresource/nethistorywidget.h"
#include "filesystem/filesystemwidget.h"

class TrayIcon : public QWidget
{
    Q_OBJECT
public:
    explicit TrayIcon(CpuHistoryWidget*cpu, MemHistoryWidget*mem, NetHistoryWidget*net, FileSystemWidget*disk, QWidget *parent = 0);
    virtual ~TrayIcon();

private:
    void initTrayIcon();
    void initMenu();
    void initConnect();

    QSystemTrayIcon *mSystemTray;
    QMenu           *mMenu;
    QAction         *mQuitAction;
    QAction         *mOpenAction;

    CpuHistoryWidget *m_cpuHisWidget = nullptr;
    MemHistoryWidget *m_memHisWidget = nullptr;
    NetHistoryWidget *m_netHisWidget = nullptr;
    FileSystemWidget *m_fileSystemWidget = nullptr;

    QTimer *m_pTimer;

public slots:
    void ActiveTray(QSystemTrayIcon::ActivationReason reason);
    void updateToolTips();

signals:
    void openMonitor(const QString &);
    void needSaveSetting();

};

#endif // TRAYICON_H
