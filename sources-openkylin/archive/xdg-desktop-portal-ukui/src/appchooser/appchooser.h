/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#ifndef APP_CHOOSER_PORTAL
#define APP_CHOOSER_PORTAL

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class AppChooserPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.AppChooser")
public:
    explicit AppChooserPortal(QObject *parent = nullptr);

//public Q_SLOTS:
//    uint ChooseApplication(
//        const QDBusObjectPath &handle, const QString &appId,
//        const QString &parentWindow, const QStringList &choices,
//        const QVariantMap &options, QVariantMap &results);

//    void UpdateChoices(const QDBusObjectPath &handle, const QStringList &choices);

//private:
//    Peony::FileLauchDialog *createAppChooserDialog(
//        const QString &uri, const QDBusObjectPath &handle, const QString &parentWindow);

//private:
//    QMap<QString, Peony::FileLauchDialog *> m_appChooserDialogs;
};

#endif // APP_CHOOSER_PORTAL
