#include "dbusadapter.h"
DbusAdapter::DbusAdapter(QObject *parent) : QObject(parent)
{
    QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.KylinRecorder");
    QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.KylinRecorder");
//    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", this, QDBusConnection::ExportNonScriptableSlots | QDBusConnection::ExportScriptableProperties);
    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", this, QDBusConnection::ExportAllContents);
}


/**
 * @brief Pause - 暂停播放当前曲目。
 *
 * @note
 * - 若处于播放（Playing）状态，调用此方法会暂停播放。
 * - 若处于暂停（Paused）状态，调用此方法无效果。
 * - 调用 Play 方法后，播放将从暂停的位置继续进行。
 */
void DbusAdapter::Pause()
{
    qInfo()<<"调用暂停信号";
    emit sigShowPause();
}
