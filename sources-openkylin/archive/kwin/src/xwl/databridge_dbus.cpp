#include "databridge_dbus.h"
#include "databridge.h"

namespace KWin
{
namespace Xwl
{

ClipboardDBusInterface::ClipboardDBusInterface(Xwl::DataBridge *parent)
    : QObject(parent)
    , m_bridge(parent)
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Clipboard"),
                                                 QStringLiteral("org.kde.KWin.Clipboard"),
                                                 this,
                                                 QDBusConnection::ExportAllProperties | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllSlots);

    connect(m_bridge, &DataBridge::clipboardSelectionPidChanged, this, &ClipboardDBusInterface::clipboardSelectionPidChanged);
    connect(m_bridge, &DataBridge::primarySelectionPidChanged, this, &ClipboardDBusInterface::primarySelectionPidChanged);
}

int ClipboardDBusInterface::getClipboardSelectionPid()
{
    return m_bridge->getClipboardSelectionPid();
}

int ClipboardDBusInterface::getPrimarySelectionPid()
{
    return m_bridge->getPrimarySelectionPid();
}

} // namespace Xwl
} // namespace KWin