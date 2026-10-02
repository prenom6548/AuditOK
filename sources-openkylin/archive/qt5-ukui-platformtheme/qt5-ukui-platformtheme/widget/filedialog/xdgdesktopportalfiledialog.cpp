/****************************************************************************
**
** Copyright (C) 2017-2018 Red Hat, Inc
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
****************************************************************************/

#include "xdgdesktopportalfiledialog_p.h"

#include <QtCore/qeventloop.h>

#include <QtDBus/QtDBus>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <QFile>
#include <QMetaType>
#include <QMimeType>
#include <QMimeDatabase>
#include <QRandomGenerator>
#include <QWindow>
#include <QFileDialog>
#include <QGuiApplication>
#include <QApplication>

QT_BEGIN_NAMESPACE

QDBusArgument &operator <<(QDBusArgument &arg, const XdgDesktopPortalFileDialog::FilterCondition &filterCondition)
{
    arg.beginStructure();
    arg << filterCondition.type << filterCondition.pattern;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >>(const QDBusArgument &arg, XdgDesktopPortalFileDialog::FilterCondition &filterCondition)
{
    uint type;
    QString filterPattern;
    arg.beginStructure();
    arg >> type >> filterPattern;
    filterCondition.type = (XdgDesktopPortalFileDialog::ConditionType)type;
    filterCondition.pattern = filterPattern;
    arg.endStructure();

    return arg;
}

QDBusArgument &operator <<(QDBusArgument &arg, const XdgDesktopPortalFileDialog::Filter filter)
{
    arg.beginStructure();
    arg << filter.name << filter.filterConditions;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >>(const QDBusArgument &arg, XdgDesktopPortalFileDialog::Filter &filter)
{
    QString name;
    XdgDesktopPortalFileDialog::FilterConditionList filterConditions;
    arg.beginStructure();
    arg >> name >> filterConditions;
    filter.name = name;
    filter.filterConditions = filterConditions;
    arg.endStructure();

    return arg;
}

class XdgDesktopPortalFileDialogPrivate
{
public:
    XdgDesktopPortalFileDialogPrivate(QPlatformFileDialogHelper *nativeFileDialog)
        : nativeFileDialog(nativeFileDialog)
    { }

    WId winId = 0;
    bool directoryMode = false;
    bool modal = false;
    bool multipleFiles = false;
    bool saveFile = false;
    QString acceptLabel;
    QString directory;
    QString title;
    QStringList nameFilters;
    QStringList mimeTypesFilters;
    // maps user-visible name for portal to full name filter
    QMap<QString, QString> userVisibleToNameFilter;
    QString selectedMimeTypeFilter;
    QString selectedNameFilter;
    QStringList selectedFiles;
    QPlatformFileDialogHelper *nativeFileDialog = nullptr;
};

XdgDesktopPortalFileDialog::XdgDesktopPortalFileDialog(QPlatformFileDialogHelper *nativeFileDialog)
    : QPlatformFileDialogHelper()
    , d_ptr(new XdgDesktopPortalFileDialogPrivate(nativeFileDialog))
{
    Q_D(XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        connect(d->nativeFileDialog, SIGNAL(accept()), this, SIGNAL(accept()));
        connect(d->nativeFileDialog, SIGNAL(reject()), this, SIGNAL(reject()));
    }
}

XdgDesktopPortalFileDialog::~XdgDesktopPortalFileDialog()
{
}

void XdgDesktopPortalFileDialog::initializeDialog()
{
    Q_D(XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog)
        d->nativeFileDialog->setOptions(options());

    if (options()->fileMode() == QFileDialogOptions::ExistingFiles)
        d->multipleFiles = true;

    if (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly)
        d->directoryMode = true;

    if (options()->isLabelExplicitlySet(QFileDialogOptions::Accept))
        d->acceptLabel = options()->labelText(QFileDialogOptions::Accept);

    if (!options()->windowTitle().isEmpty())
        d->title = options()->windowTitle();

    if (options()->acceptMode() == QFileDialogOptions::AcceptSave)
        d->saveFile = true;

    if (!options()->nameFilters().isEmpty())
        d->nameFilters = options()->nameFilters();

    if (!options()->mimeTypeFilters().isEmpty())
        d->mimeTypesFilters = options()->mimeTypeFilters();

    if (!options()->initiallySelectedMimeTypeFilter().isEmpty())
        d->selectedMimeTypeFilter = options()->initiallySelectedMimeTypeFilter();

    if (!options()->initiallySelectedNameFilter().isEmpty())
        d->selectedNameFilter = options()->initiallySelectedNameFilter();

    setDirectory(options()->initialDirectory());
}

void XdgDesktopPortalFileDialog::openPortal()
{
    Q_D(XdgDesktopPortalFileDialog);

//    qDebug() << "openPortal 11111111............" << d->saveFile ;

    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.portal.Desktop"),
                                                          QLatin1String("/org/freedesktop/portal/desktop"),
                                                          QLatin1String("org.freedesktop.portal.FileChooser"),
                                                          d->saveFile ? QLatin1String("SaveFile") : QLatin1String("OpenFile"));
    QString parentWindowId;

    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
    {
        parentWindowId = QLatin1String("wayland:") + QString::number(d->winId);
    } else {
        parentWindowId = QLatin1String("x11:") + QString::number(d->winId);
    }


    QVariantMap options;
    if (!d->acceptLabel.isEmpty())
        options.insert(QLatin1String("accept_label"), d->acceptLabel);

    options.insert(QLatin1String("modal"), d->modal);
    options.insert(QLatin1String("multiple"), d->multipleFiles);
    options.insert(QLatin1String("directory"), d->directoryMode);
    options.insert(QLatin1String("current_folder"), QFile::encodeName(m_initialPath).append('\0'));

    if (d->saveFile) {
        if (!m_initialSelectFiles.isEmpty()){
            options.insert(QLatin1String("current_file"), QFile::encodeName(m_initialSelectFiles.first().path()).append('\0'));
            if(!m_initialSelectFiles.first().path().endsWith("/")){
                QString name = m_initialSelectFiles.first().path().split("/").last();
                options.insert(QLatin1String("current_name"), name);
            }
        }
    }

    // Insert filters
    qDBusRegisterMetaType<FilterCondition>();
    qDBusRegisterMetaType<FilterConditionList>();
    qDBusRegisterMetaType<Filter>();
    qDBusRegisterMetaType<FilterList>();

    FilterList filterList;
    auto selectedFilterIndex = filterList.size() - 1;

    d->userVisibleToNameFilter.clear();

//    qDebug() << "mimeTypesFilters..." << d->mimeTypesFilters << d->nameFilters;
    if (!d->mimeTypesFilters.isEmpty()) {
        for (const QString &mimeTypefilter : d->mimeTypesFilters) {
            QMimeDatabase mimeDatabase;
            QMimeType mimeType = mimeDatabase.mimeTypeForName(mimeTypefilter);

            // Creates e.g. (1, "image/png")
            FilterCondition filterCondition;
            filterCondition.type = MimeType;
            filterCondition.pattern = mimeTypefilter;

            // Creates e.g. [((1, "image/png"))]
            FilterConditionList filterConditions;
            filterConditions << filterCondition;

            // Creates e.g. [("Images", [((1, "image/png"))])]
            Filter filter;
            filter.name = mimeType.comment();
            filter.filterConditions = filterConditions;

            filterList << filter;

            if (!d->selectedMimeTypeFilter.isEmpty() && d->selectedMimeTypeFilter == mimeTypefilter)
                selectedFilterIndex = filterList.size() - 1;
        }
    } else if (!d->nameFilters.isEmpty()) {
        for (const QString &nameFilter : d->nameFilters) {
            // Do parsing:
            // Supported format is ("Images (*.png *.jpg)")
            QRegularExpression regexp(QPlatformFileDialogHelper::filterRegExp);
            QRegularExpressionMatch match = regexp.match(nameFilter);
            if (match.hasMatch()) {
                QString userVisibleName = match.captured(1);
                QStringList filterStrings = match.captured(2).split(QLatin1Char(' '), QString::SkipEmptyParts);

                if (filterStrings.isEmpty()) {
                    qWarning() << "Filter " << userVisibleName << " is empty and will be ignored.";
                    continue;
                }

                FilterConditionList filterConditions;
                for (const QString &filterString : filterStrings) {
                    FilterCondition filterCondition;
                    filterCondition.type = GlobalPattern;
                    filterCondition.pattern = filterString;
                    filterConditions << filterCondition;
                }

                if(userVisibleName.isEmpty())
                    userVisibleName = " ";
                Filter filter;
                filter.name = userVisibleName;
                filter.filterConditions = filterConditions;

                filterList << filter;

                d->userVisibleToNameFilter.insert(userVisibleName, nameFilter);

                if (!d->selectedNameFilter.isEmpty() && d->selectedNameFilter == nameFilter)
                    selectedFilterIndex = filterList.size() - 1;
            }
        }
    }




    //    "[('Images', [(0, '*ico'), (1, 'image/png')]), ('Text', [(0, '*.txt')])]";


    if (!filterList.isEmpty())
        options.insert(QLatin1String("filters"), QVariant::fromValue(filterList));

    if (selectedFilterIndex != -1)
        options.insert(QLatin1String("current_filter"), QVariant::fromValue(filterList[selectedFilterIndex]));

    options.insert(QLatin1String("handle_token"), QStringLiteral("qt%1").arg(QRandomGenerator::global()->generate()));

        // TODO choices a(ssa(ss)s)
    // List of serialized combo boxes to add to the file chooser.

    message << parentWindowId << d->title << options;

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "reply error......" << reply.error();
            Q_EMIT reject();
        } else {
            QDBusConnection::sessionBus().connect(nullptr,
                                                  reply.value().path(),
                                                  QLatin1String("org.freedesktop.portal.Request"),
                                                  QLatin1String("Response"),
                                                  this,
                                                  SLOT(gotResponse(uint,QVariantMap)));
        }
    });
}

bool XdgDesktopPortalFileDialog::defaultNameFilterDisables() const
{
    return false;
}

void XdgDesktopPortalFileDialog::setDirectory(const QUrl &directory)
{
    Q_D(XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        d->nativeFileDialog->setOptions(options());
        d->nativeFileDialog->setDirectory(directory);
    }

    d->directory = directory.path();
}

QUrl XdgDesktopPortalFileDialog::directory() const
{
    Q_D(const XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog && (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly))
        return d->nativeFileDialog->directory();

    return d->directory;
}

void XdgDesktopPortalFileDialog::selectFile(const QUrl &filename)
{
    Q_D(XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        d->nativeFileDialog->setOptions(options());
        d->nativeFileDialog->selectFile(filename);
    }

    d->selectedFiles << filename.path();
}

QList<QUrl> XdgDesktopPortalFileDialog::selectedFiles() const
{
    Q_D(const XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog && (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly))
        return d->nativeFileDialog->selectedFiles();

    QList<QUrl> files;
    for (const QString &file : d->selectedFiles) {
        files << QUrl(file);
    }
    return files;
}

void XdgDesktopPortalFileDialog::setFilter()
{
    Q_D(XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        d->nativeFileDialog->setOptions(options());
        d->nativeFileDialog->setFilter();
    }
}

void XdgDesktopPortalFileDialog::selectMimeTypeFilter(const QString &filter)
{
    Q_D(XdgDesktopPortalFileDialog);
    if (d->nativeFileDialog) {
        d->nativeFileDialog->setOptions(options());
        d->nativeFileDialog->selectMimeTypeFilter(filter);
    }
}

QString XdgDesktopPortalFileDialog::selectedMimeTypeFilter() const
{
    Q_D(const XdgDesktopPortalFileDialog);
    return d->selectedMimeTypeFilter;
}

void XdgDesktopPortalFileDialog::selectNameFilter(const QString &filter)
{
    Q_D(XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        d->nativeFileDialog->setOptions(options());
        d->nativeFileDialog->selectNameFilter(filter);
    }
}

QString XdgDesktopPortalFileDialog::selectedNameFilter() const
{
    Q_D(const XdgDesktopPortalFileDialog);
    return d->selectedNameFilter;
}

void XdgDesktopPortalFileDialog::exec()
{
    Q_D(XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog && (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly)) {
        d->nativeFileDialog->exec();
        return;
    }

    // HACK we have to avoid returning until we emit that the dialog was accepted or rejected
    QEventLoop loop;
    loop.connect(this, SIGNAL(accept()), SLOT(quit()));
    loop.connect(this, SIGNAL(reject()), SLOT(quit()));
    loop.exec();
}

void XdgDesktopPortalFileDialog::hide()
{
    Q_D(XdgDesktopPortalFileDialog);

    if (d->nativeFileDialog)
        d->nativeFileDialog->hide();
}

bool XdgDesktopPortalFileDialog::show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent)
{
    Q_D(XdgDesktopPortalFileDialog);
//    qDebug() << "show 2222222222222";

    m_initialDirectory = options()->initialDirectory();
    m_initialSelectFiles = options()->initiallySelectedFiles();

    initializeDialog();

    d->modal = windowModality != Qt::NonModal;
    d->winId = 0;

    if (d->nativeFileDialog && (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly))
        return d->nativeFileDialog->show(windowFlags, windowModality, parent);

    for(QWidget *widget : qApp->allWidgets()){
        if(QFileDialog *fd = qobject_cast<QFileDialog *>(widget)){
            if(options()->windowTitle() == fd->windowTitle()){
//                qDebug() << "parent us null filedoalog set parent...." << fd->geometry() << options()->windowTitle() << fd->objectName();

                d->winId = fd->winId();

//                qDebug() << "parent us null filedoalog directory...." << fd->directory();
//                qDebug() << "parent us null filedoalog selectedFiles11111111...." << fd->selectedFiles();

                if(m_initialSelectFiles.length() > 0 && QFile::exists(m_initialSelectFiles.value(0).path())){
                    QDir dir(m_initialSelectFiles.value(0).path());
                    dir.cdUp();
//                    qDebug() <<"dirrrrr..." << dir.path();
                    m_initialPath = dir.absolutePath();
                }
                else if(fd->directory().exists() /*|| Peony::FileUtils::isFileExsit(fd->directory().path())*/){
//                    qDebug() << "updatePath........." << fd->directory();
                    m_initialPath = fd->directory().absolutePath();
                }
                break;
            }
        }
    }



    openPortal();

//    qDebug() << "show 1111111111111111111";
    return true;
}

void XdgDesktopPortalFileDialog::gotResponse(uint response, const QVariantMap &results)
{
    Q_D(XdgDesktopPortalFileDialog);

    if (!response) {
        if (results.contains(QLatin1String("uris")))
            d->selectedFiles = results.value(QLatin1String("uris")).toStringList();

        if (results.contains(QLatin1String("current_filter"))) {
            const Filter selectedFilter = qdbus_cast<Filter>(results.value(QStringLiteral("current_filter")));
            if (!selectedFilter.filterConditions.empty() && selectedFilter.filterConditions[0].type == MimeType) {
                // s.a. XdgDesktopPortalFileDialog::openPortal which basically does the inverse
                d->selectedMimeTypeFilter = selectedFilter.filterConditions[0].pattern;
                d->selectedNameFilter.clear();
            } else {
                d->selectedNameFilter = d->userVisibleToNameFilter.value(selectedFilter.name);
                d->selectedMimeTypeFilter.clear();
            }
        }
        Q_EMIT accept();
    } else {
        Q_EMIT reject();
    }
}

QT_END_NAMESPACE
