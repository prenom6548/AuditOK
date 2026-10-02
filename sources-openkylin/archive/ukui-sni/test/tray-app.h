#ifndef TRAYAPP_H
#define TRAYAPP_H

#include <QObject>
#include "../common/dbus-types.h"
#include "statusnotifierwatcher_interface.h"

class TrayApp : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString attentionIconName READ attentionIconName)
    Q_PROPERTY(DbusImageVector AttentionIconPixmap READ AttentionIconPixmap)
    Q_PROPERTY(QString AttentionMovieName READ AttentionMovieName)
    Q_PROPERTY(QString Category READ Category)
    Q_PROPERTY(QString IconName READ IconName)
    Q_PROPERTY(DbusImageVector IconPixmap READ IconPixmap)
    Q_PROPERTY(QString IconThemePath READ IconThemePath)
    Q_PROPERTY(QString Id READ Id)
    Q_PROPERTY(bool ItemIsMenu READ ItemIsMenu)
    Q_PROPERTY(QDBusObjectPath Menu READ Menu)
    Q_PROPERTY(QString OverlayIconName READ OverlayIconName)
    Q_PROPERTY(DbusImageVector OverlayIconPixmap READ OverlayIconPixmap)
    Q_PROPERTY(QString Status READ Status)
    Q_PROPERTY(QString Title READ Title)
    Q_PROPERTY(DbusToolTipStruct ToolTip READ ToolTip)
    Q_PROPERTY(int WindowId READ WindowId)
public:
    TrayApp(QObject *parent = nullptr);
    void init(qint64 pid);

    QString attentionIconName() const { return m_attentionIconName; }
    DbusImageVector AttentionIconPixmap() const { return m_attentionIconPixmap; }
    QString AttentionMovieName() const { return m_attentionMovieName; }
    QString Category() const { return m_category; }
    QString IconName() const { return m_iconName; }
    DbusImageVector IconPixmap() const { return m_iconPixmap; }
    QString IconThemePath() const { return m_iconThemePath; }
    QString Id() const { return m_id; }
    bool ItemIsMenu() const { return m_itemIsMenu; }
    QDBusObjectPath Menu() const { return m_menu; }
    QString OverlayIconName() const { return m_overlayIconName; }
    DbusImageVector OverlayIconPixmap() const { return m_overlayIconPixmap; }
    QString Status() const { return m_status; }
    QString Title() const { return m_title; }
    DbusToolTipStruct ToolTip() const { return m_toolTip; }
    int WindowId() const { return m_windowId; }

public Q_SLOTS: // METHODS
    void Activate(int x, int y);
    void ContextMenu(int x, int y);
    void ProvideXdgActivationToken(const QString &token);
    void Scroll(int delta, const QString &orientation);
    void SecondaryActivate(int x, int y);

Q_SIGNALS: // SIGNALS
    void NewAttentionIcon();
    void NewIcon();
    void NewMenu();
    void NewOverlayIcon();
    void NewStatus(const QString &status);
    void NewTitle();
    void NewToolTip();
private:
    org::kde::StatusNotifierWatcher *m_statusNotifierWatcher = nullptr;

    QString m_attentionIconName = "";
    DbusImageVector m_attentionIconPixmap;
    QString m_attentionMovieName = "";
//    QString m_attentionMovieName = "file:///home/gjq/gitcode/ukui-sni/test/res/icon/actionmovie.gif";
    QString m_category = "";
    QString m_iconName = "ukui-virtual-keyboard-symbolic";
    DbusImageVector m_iconPixmap;
    QString m_iconThemePath;
    QString m_id = "Tray App";
    bool m_itemIsMenu = false;
    QDBusObjectPath m_menu = QDBusObjectPath("/MenuBar");
    QString m_overlayIconName = "";
    DbusImageVector m_overlayIconPixmap;
    QString m_status = "NeedsAttention";
    QString m_title = "Tray App";
    DbusToolTipStruct m_toolTip;
    int m_windowId;
};

#endif // TRAYAPP_H
