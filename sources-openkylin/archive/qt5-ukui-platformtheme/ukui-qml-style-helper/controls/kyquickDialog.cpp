/*
 * libkysdk-qtwidgets's Library
 *
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kyquickDialog.h"
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QDir>
#include <QApplication>
#include <QSettings>
#include <QDesktopServices>
#include <QMessageBox>
#include <QIcon>
#include <QUrl>

using namespace UKUISDKStyle;

KyQuickDialog::KyQuickDialog(QQuickItem *parent) : QQuickItem(parent)
{
    connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow *window) {
        qDebug() << "menubutton window changeddddd" << window;
            m_quickWindow = window;
            if(m_quickWindow){
                if(m_windowHelper == nullptr)
                    m_windowHelper = new UkuiWindowHelper(m_quickWindow);
//                m_windowHelper->removeTitleBar();
//                m_windowHelper->setSkipSwitcher(true);
//                m_windowHelper->setSkipTaskBar(true);


            }
    });
}

KyQuickDialog::~KyQuickDialog()
{
}

KyQuickDialog* KyQuickDialog::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new KyQuickDialog(p);
}

void KyQuickDialog::feedbackShow()
{
    QString directoryPath = "/usr/lib/kylin-os-manager/bin/service-support-mini";
    QFile file(directoryPath);
    if (file.exists()) {
        QStringList arguments;
        QString processName;
        QString fileName = QString("/proc/%1/cmdline").arg(QApplication::applicationPid());
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            int pos = data.indexOf('\0');
            if (pos != -1) {
                QStringList list = QString::fromUtf8(data.left(pos)).trimmed().split("/");
                processName = list.last();
            }
            file.close();
        }

        arguments << "-mode=1"
                  << "-exe=" + processName;

        QProcess feedbackProcess;
        feedbackProcess.startDetached(directoryPath, arguments);
        feedbackProcess.waitForFinished();
    }
}

bool KyQuickDialog::feedbackItemVisible(){
    QString directoryPath = "/usr/lib/kylin-os-manager/bin/service-support-mini";
    QFile file(directoryPath);
    return file.exists();
}

void KyQuickDialog::showTileFlyout(int x, int y, int w, int h){
    if(m_windowHelper)
    m_windowHelper->showTileFlyout(QRect(x, y, w, h));
}

void KyQuickDialog::openSupportLink(const QString &href)
{
    if (href.isEmpty())
        return;

    QString appid = getDefaultAppId(MAILTYPE);
    if (appid.isEmpty()) {
//        QMessageBox msg;
//        msg.setIcon(QMessageBox::Information);
//        msg.setIconPixmap(QIcon::fromTheme(QStringLiteral("dialog-info")).pixmap(24, 24));
//        msg.setText(tr("Unable to open mail application"));
//        msg.setInformativeText(tr("Your system does not have any email application installed or the default mail application is not set up"));
//        msg.exec();
//        return;
    }

    QString normalized = href;
    if (normalized.startsWith(QStringLiteral("mailto://")))
        normalized = QStringLiteral("mailto:") + normalized.mid(QStringLiteral("mailto://").length());

    const QUrl u = QUrl::fromUserInput(normalized);
    if (u.isValid())
        QDesktopServices::openUrl(u);
}

void KyQuickDialog::appSupport()
{
    openSupportLink(QStringLiteral("mailto:support@kylinos.cn"));
}


QString KyQuickDialog::getDefaultAppId(const char *contentType)
{
    QString localfile = QDir::homePath() + LOCAL_CONFIG_DIR + "mimeapps.list";
    QString systemfile = SYSTEM_CONFIG_DIR + QString("ukui-mimeapps.list");
    QString str = "";
    if (QFile(localfile).exists()) {
        QSettings *mimeappFile = new QSettings(localfile, QSettings::IniFormat);
        mimeappFile->setIniCodec("utf-8");
        str = mimeappFile->value(QString("Default Applications/%1").arg(contentType)).toString();
        for(int i = 0; i <getSystemConfigDir().count(); i++)
        {
            QString filePath = getSystemConfigDir().at(i) + "/applications/" + str;
            if (!str.isEmpty()) {
                if (QFile(filePath).exists()) {
                    delete mimeappFile;
                    mimeappFile = nullptr;
                    return str;
                }
            }
        }
        delete mimeappFile;
        mimeappFile = nullptr;
        str = "";
    }
    if (QFile(systemfile).exists()) {
        QSettings *mimeappFile = new QSettings(systemfile, QSettings::IniFormat);
        mimeappFile->setIniCodec("utf-8");
        str = mimeappFile->value(QString("Default Applications/%1").arg(contentType)).toString();

        for(int i = 0; i <getSystemConfigDir().count(); i++)
        {
            QString filePath = getSystemConfigDir().at(i) + "/applications/" + str;
            if (!str.isEmpty()) {
                if (QFile(filePath).exists()) {
                    delete mimeappFile;
                    mimeappFile = nullptr;
                    return str;
                }
            }
        }
        delete mimeappFile;
        mimeappFile = nullptr;
        str = "";
    }
    return str;
}

QStringList KyQuickDialog::getSystemConfigDir()
{
    QString xdgDataDirs = QProcessEnvironment::systemEnvironment().value("XDG_DATA_DIRS");

    if (xdgDataDirs.isEmpty()) {
        qWarning() << "XDG_DATA_DIRS environment variable is not set!";
    }

    QStringList paths = xdgDataDirs.split(':', Qt::SkipEmptyParts);

    for (QString &path : paths)
    {
        path = path.trimmed();
    }
    return paths;
}

void KyQuickDialog::privacyClicked()
{
    QProcess *process = new QProcess(this);
    QStringList l;
    l << "-A"
      << "kylin-os";
    process->start("kylin-user-guide", l);
}
