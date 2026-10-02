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

#include "appchooser.h"
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDialog>
#include <QDebug>
#include <KWindowSystem>
#include "windowutils.h"
#include "request.h"

AppChooserPortal::AppChooserPortal(QObject *parent)
    : QDBusAbstractAdaptor{parent}
{
}

//uint AppChooserPortal::ChooseApplication(
//    const QDBusObjectPath &handle, const QString &appId,
//    const QString &parentWindow, const QStringList &choices,
//    const QVariantMap &options, QVariantMap &results)
//{
//    QString uri = options.value(QStringLiteral("uri")).toString();
//    if (uri.isEmpty()) {
//        qWarning() << "Choose application failed, the uri is empty.";
//        return 1;
//    }

//    // todo: last_choice, content_type
//    Peony::FileLauchDialog *appChooserDialog =
//        createAppChooserDialog(uri, handle, parentWindow);

//    m_appChooserDialogs.insert(handle.path(), appChooserDialog);
    
//    int result = appChooserDialog->exec();
//    if (result) {
//        // todo: selectedApplication()
//        // results.insert(QStringLiteral("choice"), appChooserDialog->selectedApplication());
//    }

//    m_appChooserDialogs.remove(handle.path());
//    appChooserDialog->deleteLater();

//    return !result;
//}

//void AppChooserPortal::UpdateChoices(const QDBusObjectPath &handle, const QStringList &choices)
//{
//    if (m_appChooserDialogs.contains(handle.path())) {
//        // todo: updateChoice
//        // m_appChooserDialogs.value()->updateChoice(choices);
//    }
//}

//Peony::FileLauchDialog *AppChooserPortal::createAppChooserDialog(
//    const QString &uri, const QDBusObjectPath &handle, const QString &parentWindow)
//{
//    Peony::FileLauchDialog *appChooserDialog = new Peony::FileLauchDialog(uri);
//    WindowUtils::setParentWindow(appChooserDialog, parentWindow);
//    Request::makeClosableDialogRequest(handle, appChooserDialog);
//    return appChooserDialog;
//}
