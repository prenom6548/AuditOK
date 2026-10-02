#ifndef CLIPBOARD_DBUS_INTERFACE_H
#define CLIPBOARD_DBUS_INTERFACE_H

#include <QObject>
#include <QtDBus>

namespace KWin
{
namespace Xwl
{

class DataBridge;

class ClipboardDBusInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KWin.Clipboard")

    Q_PROPERTY(int GetClipboardSelectionPid READ getClipboardSelectionPid)
    Q_PROPERTY(int GetPrimarySelectionPid READ getPrimarySelectionPid)

public:
    explicit ClipboardDBusInterface(DataBridge *parent);
    ~ClipboardDBusInterface() override = default;

Q_SIGNALS:
    void clipboardSelectionPidChanged(int pid);
    void primarySelectionPidChanged(int pid);

public Q_SLOTS:
    int getClipboardSelectionPid();
    int getPrimarySelectionPid();

private:
    DataBridge *m_bridge;
};

} // namespace Xwl
} // namesapce KWin

#endif // CLIPBOARD_DBUS_INTERFACE_H
