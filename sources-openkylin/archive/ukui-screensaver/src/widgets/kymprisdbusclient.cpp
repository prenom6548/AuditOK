/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "kymprisdbusclient.h"

#define MPRIS_DBUS_INTERFACE "org.mpris.MediaPlayer2.Player"

KyMPRISDBusClient::KyMPRISDBusClient(
    const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, MPRIS_DBUS_INTERFACE, connection, parent)
{
    QDBusConnection::sessionBus().connect(
        this->service(),
        this->path(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        "sa{sv}as",
        this,
        SLOT(onPropertiesChanged(QDBusMessage)));
}

KyMPRISDBusClient::~KyMPRISDBusClient()
{
    QDBusConnection::sessionBus().disconnect(
        service(),
        path(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        "sa{sv}as",
        this,
        SLOT(onPropertiesChanged(QDBusMessage)));
}

QString KyMPRISDBusClient::getPlaybackStatus() const
{
    return qvariant_cast<QString>(property("PlaybackStatus"));
}

QString KyMPRISDBusClient::getLoopStatus() const
{
    return qvariant_cast<QString>(property("LoopStatus"));
}

void KyMPRISDBusClient::setLoopStatus(const QString &value)
{
    KyMPRISDBusClient::setProperty("LoopStatus", QVariant::fromValue(value));
}

double KyMPRISDBusClient::getRate() const
{
    return qvariant_cast<double>(property("Rate"));
}
void KyMPRISDBusClient::setRate(double value)
{
    KyMPRISDBusClient::setProperty("Rate", QVariant::fromValue(value));
}

bool KyMPRISDBusClient::getShuffle() const
{
    return qvariant_cast<bool>(property("Shuffle"));
}
void KyMPRISDBusClient::setShuffle(bool value)
{
    KyMPRISDBusClient::setProperty("Shuffle", QVariant::fromValue(value));
}

QVariantMap KyMPRISDBusClient::getMetadata() const
{
    return qvariant_cast<QVariantMap>(property("Metadata"));
}

double KyMPRISDBusClient::getVolume() const
{
    return qvariant_cast<double>(property("Volume"));
}
void KyMPRISDBusClient::setVolume(double value)
{
    KyMPRISDBusClient::setProperty("Volume", QVariant::fromValue(value));
}

qlonglong KyMPRISDBusClient::getPosition() const
{
    return qvariant_cast<qlonglong>(property("Position"));
}

double KyMPRISDBusClient::getMinimumRate() const
{
    return qvariant_cast<double>(property("MinimumRate"));
}

double KyMPRISDBusClient::getMaximumRate() const
{
    return qvariant_cast<double>(property("MaximumRate"));
}

bool KyMPRISDBusClient::getCanGoNext() const
{
    return qvariant_cast<bool>(property("CanGoNext"));
}

bool KyMPRISDBusClient::getCanGoPrevious() const
{
    return qvariant_cast<bool>(property("CanGoPrevious"));
}

bool KyMPRISDBusClient::getCanPlay() const
{
    return qvariant_cast<bool>(property("CanPlay"));
}

bool KyMPRISDBusClient::getCanPause() const
{
    return qvariant_cast<bool>(property("CanPause"));
}

bool KyMPRISDBusClient::getCanSeek() const
{
    return qvariant_cast<bool>(property("CanSeek"));
}

bool KyMPRISDBusClient::getCanControl() const
{
    return qvariant_cast<bool>(property("CanControl"));
}

void KyMPRISDBusClient::onPropertiesChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (3 != arguments.count())
        return;
    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != MPRIS_DBUS_INTERFACE)
        return;
    QVariantMap changedProperties = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    for (const QString &prop : changedProperties.keys()) {
        const QMetaObject *self = metaObject();
        for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
            QMetaProperty p = self->property(i);
            QGenericArgument value(QMetaType::typeName(p.type()), const_cast<void *>(changedProperties[prop].constData()));
            if (p.name() == prop) {
                Q_EMIT p.notifySignal().invoke(this, value);
            }
        }
    }
}

QDBusPendingReply<> KyMPRISDBusClient::doNext()
{
    QList<QVariant> argumentList;
    qDebug()<<"media Next!!";
    return asyncCallWithArgumentList(QStringLiteral("Next"), argumentList);
}

QDBusPendingReply<> KyMPRISDBusClient::doPrevious()
{
    QList<QVariant> argumentList;
    qDebug()<<"media Previous!!";
    return asyncCallWithArgumentList(QStringLiteral("Previous"), argumentList);
}

QDBusPendingReply<> KyMPRISDBusClient::doPause()
{
    QList<QVariant> argumentList;
    qDebug()<<"media Pause!!";
    return asyncCallWithArgumentList(QStringLiteral("Pause"), argumentList);
}

QDBusPendingReply<> KyMPRISDBusClient::doPlayPause()
{
    QList<QVariant> argumentList;
    qDebug()<<"media PlayPause!!";
    return asyncCallWithArgumentList(QStringLiteral("PlayPause"), argumentList);
}

QDBusPendingReply<> KyMPRISDBusClient::doStop()
{
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("Stop"), argumentList);
}

QDBusPendingReply<> KyMPRISDBusClient::doPlay()
{
    QList<QVariant> argumentList;
    qDebug()<<"media Play!!";
    return asyncCallWithArgumentList(QStringLiteral("Play"), argumentList);
}

QDBusPendingReply<> KyMPRISDBusClient::doSeek(qlonglong Offset)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(Offset);
    return asyncCallWithArgumentList(QStringLiteral("Seek"), argumentList);
}

QDBusPendingReply<> KyMPRISDBusClient::doSetPosition(const QDBusObjectPath &TrackId, qlonglong Position)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(TrackId) << QVariant::fromValue(Position);
    return asyncCallWithArgumentList(QStringLiteral("SetPosition"), argumentList);
}

QDBusPendingReply<> KyMPRISDBusClient::doOpenUri(const QString &Uri)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(Uri);
    return asyncCallWithArgumentList(QStringLiteral("OpenUri"), argumentList);
}
