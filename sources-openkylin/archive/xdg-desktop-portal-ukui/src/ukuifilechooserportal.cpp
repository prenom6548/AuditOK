/*
 * Copyright © 2016-2018 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 * Modifier:
 *       Yue Lan <lanyue@kylinos.cn>
 */

#include "ukuifilechooserportal.h"

#include <QDialogButtonBox>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QLoggingCategory>
#include <QFile>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUrl>
#include <QStandardPaths>

#include "kyfiledialog.h"
#include "windowutils.h"
#include "ukuiwindowhelper.h"

Q_LOGGING_CATEGORY(XdgDesktopPortalUKUIFileChooser, "xdp-ukui-file-chooser")

// Keep in sync with qflatpakfiledialog from flatpak-platform-plugin
Q_DECLARE_METATYPE(FileChooserPortal::Filter)
Q_DECLARE_METATYPE(FileChooserPortal::Filters)
Q_DECLARE_METATYPE(FileChooserPortal::FilterList)
Q_DECLARE_METATYPE(FileChooserPortal::FilterListList)

QDBusArgument &operator << (QDBusArgument &arg, const FileChooserPortal::Filter &filter)
{
    arg.beginStructure();
    arg << filter.type << filter.filterString;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >> (const QDBusArgument &arg, FileChooserPortal::Filter &filter)
{
    uint type;
    QString filterString;
    arg.beginStructure();
    arg >> type >> filterString;
    filter.type = type;
    filter.filterString = filterString;
    arg.endStructure();

    return arg;
}

QDBusArgument &operator << (QDBusArgument &arg, const FileChooserPortal::FilterList &filterList)
{
    arg.beginStructure();
    arg << filterList.userVisibleName << filterList.filters;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >> (const QDBusArgument &arg, FileChooserPortal::FilterList &filterList)
{
    QString userVisibleName;
    FileChooserPortal::Filters filters;
    arg.beginStructure();
    arg >> userVisibleName >> filters;
    filterList.userVisibleName = userVisibleName;
    filterList.filters = filters;
    arg.endStructure();

    return arg;
}

FileChooserPortal::FileChooserPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDBusRegisterMetaType<Filter>();
    qDBusRegisterMetaType<Filters>();
    qDBusRegisterMetaType<FilterList>();
    qDBusRegisterMetaType<FilterListList>();

}

FileChooserPortal::~FileChooserPortal()
{
}

uint FileChooserPortal::OpenFile(const QDBusObjectPath &handle,
                           const QString &app_id,
                           const QString &parent_window,
                           const QString &title,
                           const QVariantMap &options,
                           QVariantMap &results)
{
    Q_UNUSED(app_id);

    qCDebug(XdgDesktopPortalUKUIFileChooser) << "OpenFile called with parameters:";
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    title: " << title;
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    options: " << options;

    bool modalDialog = true;
    bool multipleFiles = false;
    QString acceptLabel;
    QList<QPair<QString, QStringList>> nameFilters;//QString代表visiblename，如果没有则为""   QStringList表示filter过滤数据项
    bool isHideNameFilterDetails = false;//是否开启QFileDialog::HideNameFilterDetails   这里是有visiblename就隐藏  后面可以优化
    bool isUsrMimeTypeFilter = false;//是否用mimetype过滤
    QList<QPair<QString, QStringList>> mimeMap;//QString代表visiblename，如果没有则为""   QStringList表示mimetype过滤数据项
    bool isDirectory = false;

    bool isOpenDirectory = false;
    QString currentFolder;

    /* TODO
     * choices a(ssa(ss)s)
     * List of serialized combo boxes to add to the file chooser.
     *
     * For each element, the first string is an ID that will be returned with the response, te second string is a user-visible label.
     * The a(ss) is the list of choices, each being a is an ID and a user-visible label. The final string is the initial selection,
     * or "", to let the portal decide which choice will be initially selected. None of the strings, except for the initial selection, should be empty.
     *
     * As a special case, passing an empty array for the list of choices indicates a boolean choice that is typically displayed as a check button, using "true" and "false" as the choices.
     * Example: [('encoding', 'Encoding', [('utf8', 'Unicode (UTF-8)'), ('latin15', 'Western')], 'latin15'), ('reencode', 'Reencode', [], 'false')]
     */

    if (options.contains(QStringLiteral("accept_label"))) {
        acceptLabel = options.value(QStringLiteral("accept_label")).toString();
    }

    if (options.contains(QStringLiteral("modal"))) {
        modalDialog = options.value(QStringLiteral("modal")).toBool();
    }

    if (options.contains(QStringLiteral("multiple"))) {
        multipleFiles = options.value(QStringLiteral("multiple")).toBool();
    }

    if (options.contains(QStringLiteral("filters"))) {
        FilterListList filterListList = qdbus_cast<FilterListList>(options.value(QStringLiteral("filters")));
        qCDebug(XdgDesktopPortalUKUIFileChooser) << "filterListList length" <<filterListList.length();

        for (const FilterList &filterList : filterListList) {
            qCDebug(XdgDesktopPortalUKUIFileChooser) << "filterlist visiblename" << filterList.userVisibleName;
            QStringList filterStrings;
            QStringList mimeTypeString;
            for (const Filter &filterStruct : filterList.filters) {
                QString s = filterStruct.filterString;
                QString filterS;
                //特殊字符串处理[Hh][tT][Mm][Ll] 如firefox
                if(s.contains("[") && s.contains("]") && s.length() > 3){
                    for(int i = 0; i < s.length(); i++){
                        if(i + 3 < s.length()){
                            QChar c = s[i + 1];
                            if((c > 'a' && c < 'z') || (c > 'A' && c < 'Z')){
                                if((s[i] == '[') && QString(s[i + 1]).toLower() == QString(s[i + 2]).toLower() && (s[i + 3] == ']')){
                                    filterS += QString(s[i + 1]).toLower();
                                    i = i + 3;
                                    continue;
                                }
                            }
                        }
                        filterS += s[i];
                    }
                }
                else
                    filterS = s;
                //用mimetype过滤  存在有的type=0 有的 type=1的情况（文档查看器）
                if (filterStruct.type == 0) {
                    filterStrings << filterS;
                    mimeTypeString << filterS;
                } else {
                    mimeTypeString << filterS;
                    isUsrMimeTypeFilter = true;
                }
                if(filterStruct.type == 0)
                    qCDebug(XdgDesktopPortalUKUIFileChooser) << "open file filterStruct type 0;";
                else
                    qCDebug(XdgDesktopPortalUKUIFileChooser) << "open file filterStruct type isn't 0;";

                qCDebug(XdgDesktopPortalUKUIFileChooser) << "open file filterStrings:" << filterStrings;

            }
            qCDebug(XdgDesktopPortalUKUIFileChooser) << "open file mimeTypeFilters:" << mimeTypeString << filterStrings;

            mimeMap.append(qMakePair(QString(filterList.userVisibleName), QStringList(mimeTypeString)));

            if (!filterStrings.isEmpty()) {
                if(filterList.userVisibleName.isEmpty() || filterList.userVisibleName == " "){
                    nameFilters.append(qMakePair(QString(""), QStringList(filterStrings)));
                    //nameFilters << QStringLiteral("%1").arg(filterStrings.join(QLatin1Char(' ')));
                }
                else{
                    nameFilters.append(qMakePair(QString(filterList.userVisibleName), QStringList(filterStrings)));

                //nameFilters << QStringLiteral("%1|%2").arg(filterStrings.join(QLatin1Char(' '))).arg(filterList.userVisibleName);
                }
            }
            //如果userVisibleName不为空  则简化显示
            if(!filterList.userVisibleName.isEmpty())
                isHideNameFilterDetails = true;
        }
    }
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "open file nameFilters:" << nameFilters;


    if (options.contains("directory")) {
        isOpenDirectory = options.value("directory").toBool();
        if(isOpenDirectory){
            nameFilters.clear();
            nameFilters.append(qMakePair(QString(""), QStringList(tr("Directories"))));
            isUsrMimeTypeFilter = false;
            isDirectory = true;
        }
    }
    //如果mimeMap nameFilters均为空  则设置一个默认的
    if(mimeMap.count() == 0 & nameFilters.length() == 0){
        nameFilters.append(qMakePair(QString(tr("All Files ")), QStringList("*")));
    }
    if (options.contains("current_folder")) {
        currentFolder = options.value("current_folder").toString();
    }

    KyNativeFileDialog d;

    // force set window modal
    WindowUtils::setParentWindow(&d, parent_window);
    d.setWindowModality(Qt::ApplicationModal);

    d.setWindowTitle(title);
    d.setViewMode(KyNativeFileDialog::List);
    d.setAcceptMode(QFileDialog::AcceptOpen);
    if (isOpenDirectory) {
        d.setFileMode(QFileDialog::Directory);
    }

    /*
     * Set filedialog folder to local home path when the current_folder
     * get relative path.Because xdg-desktop-portal filechooser incognizance
     * the relative path. BUG304281.
    */
    QFileInfo currentFolderInfo(currentFolder);
    if (currentFolder.isEmpty() || !currentFolderInfo.isAbsolute()) {
        d.setDirectory(QString("file://%1").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
    } else {
//        d.setDirectoryUrl(QUrl::fromLocalFile(currentFolder));
        d.setDirectory(QString("file://%1").arg(currentFolder));
    }

    qCDebug(XdgDesktopPortalUKUIFileChooser) << "open file isHideNameFilterDetails:" << isHideNameFilterDetails;

    if(isHideNameFilterDetails)
        d.setOption(QFileDialog::HideNameFilterDetails, true);

    if (isUsrMimeTypeFilter && !mimeMap.isEmpty()) {
        d.setMimeTypeFiltersList(mimeMap);
    }

    if (!isUsrMimeTypeFilter && !nameFilters.isEmpty()) {
        d.setNameFiltersList(nameFilters);
    }
    if(isDirectory){
        d.setFileMode(QFileDialog::Directory);
    }

//    if (!nameFilters.isEmpty()) {
//        d.selectNameFilter(nameFilters);
//    }

    //remove ukui-panel application icon
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(&d);
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);

    if (d.exec() == QDialog::Accepted) {
        auto uris = d.selectedFiles();
        if (isOpenDirectory && uris.isEmpty()) {
            uris << d.getCurrentUri();
        }

        if (uris.isEmpty()) {
            windowhelper->deleteLater();
            return 2;
        }

        results.insert(QStringLiteral("uris"), uris);
        results.insert(QStringLiteral("writable"), true);

        windowhelper->deleteLater();
        return 0;
    }

    windowhelper->deleteLater();
    return 1;

//    QScopedPointer<FileDialog, QScopedPointerDeleteLater> fileDialog(new FileDialog());
//    Utils::setParentWindow(fileDialog.data(), parent_window);
//    fileDialog->setWindowTitle(title);
//    fileDialog->setModal(modalDialog);
//    fileDialog->m_fileWidget->setMode(multipleFiles ? KFile::Mode::Files | KFile::Mode::ExistingOnly : KFile::Mode::File | KFile::Mode::ExistingOnly);
//    fileDialog->m_fileWidget->okButton()->setText(!acceptLabel.isEmpty() ? acceptLabel : i18n("Open"));

//    if (!nameFilters.isEmpty()) {
//        fileDialog->m_fileWidget->setFilter(nameFilters.join(QLatin1Char('\n')));
//    }

//    if (!mimeTypeFilters.isEmpty()) {
//        fileDialog->m_fileWidget->setMimeFilter(mimeTypeFilters);
//    }

//    if (fileDialog->exec() == QDialog::Accepted) {
//        QStringList files;
//        for (const QString &filename : fileDialog->m_fileWidget->selectedFiles()) {
//           QUrl url = QUrl::fromLocalFile(filename);
//           files << url.toDisplayString();
//        }

//        if (files.isEmpty()) {
//            qCDebug(XdgDesktopPortalUKUIFileChooser) << "Failed to open file: no local file selected";
//            return 2;
//        }

//        results.insert(QStringLiteral("uris"), files);
//        results.insert(QStringLiteral("writable"), true);
//        return 0;
//    }

//    return 1;
}

uint FileChooserPortal::SaveFile(const QDBusObjectPath &handle,
                           const QString &app_id,
                           const QString &parent_window,
                           const QString &title,
                           const QVariantMap &options,
                           QVariantMap &results)
{
    Q_UNUSED(app_id);

    qCDebug(XdgDesktopPortalUKUIFileChooser) << "SaveFile called with parameters:";
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    title: " << title;
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    options: " << options;

    bool modalDialog = true;
    QString acceptLabel;
    QString currentName;
    QString currentFolder;
    QString currentFile;
    QList<QPair<QString, QStringList>> nameFilters;
    bool isHideNameFilterDetails = false;
    bool isUsrMimeTypeFilter = false;
    QList<QPair<QString, QStringList>> mimeMap;

    // TODO parse options - choices

    if (options.contains(QStringLiteral("modal"))) {
        modalDialog = options.value(QStringLiteral("modal")).toBool();
    }

    if (options.contains(QStringLiteral("accept_label"))) {
        acceptLabel = options.value(QStringLiteral("accept_label")).toString();
    }

    if (options.contains(QStringLiteral("current_name"))) {
        currentName = options.value(QStringLiteral("current_name")).toString();
    }

    if (options.contains(QStringLiteral("current_folder"))) {
        currentFolder = QFile::decodeName(options.value(QStringLiteral("current_folder")).toByteArray());
    }

    if (options.contains(QStringLiteral("current_file"))) {
        currentFile = QFile::decodeName(options.value(QStringLiteral("current_file")).toByteArray());
    }

    if (options.contains(QStringLiteral("filters"))) {
        FilterListList filterListList = qdbus_cast<FilterListList>(options.value(QStringLiteral("filters")));
        for (const FilterList &filterList : filterListList) {
            QStringList filterStrings;
            QStringList mimeTypeString;
            for (const Filter &filterStruct : filterList.filters) {
                //特殊字符串处理[Hh][tT][Mm][Ll]
                QString s = filterStruct.filterString;
                QString filterS;

                if(s.contains("[") && s.contains("]") && s.length() > 3){
                    for(int i = 0; i < s.length(); i++){
                        if(i + 3 < s.length()){
                            QChar c = s[i + 1];
                            if((c > 'a' && c < 'z') || (c > 'A' && c < 'Z')){
                                if((s[i] == '[') && QString(s[i + 1]).toLower() == QString(s[i + 2]).toLower() && (s[i + 3] == ']')){
                                    filterS += QString(s[i + 1]).toLower();
                                    i = i + 3;
                                    continue;
                                }
                            }
                        }
                        filterS += s[i];
                    }
                }
                else
                    filterS = s;

                if (filterStruct.type == 0) {
                    filterStrings << filterS;
                    mimeTypeString << filterS;
                } else {
                    mimeTypeString << filterS;
                    isUsrMimeTypeFilter = true;
                }
                qCDebug(XdgDesktopPortalUKUIFileChooser) << "save file filterStrings:" << filterStrings;

            }
            qCDebug(XdgDesktopPortalUKUIFileChooser) << "save file mimeTypeString:" << mimeTypeString << filterList.userVisibleName;

            mimeMap.append(qMakePair(QString(filterList.userVisibleName), QStringList(mimeTypeString)));

            if (!filterStrings.isEmpty()) {
                if(filterList.userVisibleName.isEmpty() || filterList.userVisibleName == " "){
                    nameFilters.append(qMakePair(QString(""), QStringList(filterStrings)));

                    //nameFilters << QStringLiteral("%1").arg(filterStrings.join(QLatin1Char(' ')));
                }
                else{
                    //nameFilters << QStringLiteral("%1|%2").arg(filterStrings.join(QLatin1Char(' '))).arg(filterList.userVisibleName);
                    nameFilters.append(qMakePair(QString(filterList.userVisibleName), QStringList(filterStrings)));

                }
            }
            if(!filterList.userVisibleName.isEmpty())
                isHideNameFilterDetails = true;
        }
    }

    if(mimeMap.count() == 0 & nameFilters.length() == 0){
        nameFilters.append(qMakePair(QString(tr("All Files ")), QStringList("*")));
    }

    KyNativeFileDialog d;

    // force set window modal
    WindowUtils::setParentWindow(&d, parent_window);
    d.setWindowModality(Qt::ApplicationModal);

    d.setWindowTitle(title);
    d.setAcceptMode(QFileDialog::AcceptSave);

    QFileInfo currentFolderInfo(currentFolder);
    if (currentFolder.isEmpty() || !currentFolderInfo.isAbsolute()) {
        d.setDirectory(QString("file://%1").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
    } else {
        d.setDirectoryUrl(QUrl::fromLocalFile(currentFolder));
    }

    if (!currentFile.isEmpty()) {
        QUrl url = QUrl::fromLocalFile(currentFile);
        d.setCurrentSelections(QStringList()<<url.toString());
    }

    if(isHideNameFilterDetails)
        d.setOption(QFileDialog::HideNameFilterDetails, true);

    if (isUsrMimeTypeFilter && !mimeMap.isEmpty()) {
        d.setMimeTypeFiltersList(mimeMap);
    }
    if (!isUsrMimeTypeFilter && !nameFilters.isEmpty()) {
        d.setNameFiltersList(nameFilters);
    }
    d.setCurrentInputName(currentName);
    d.setLabelText(QFileDialog::FileName, currentName);

    //remove ukui-panel application icon
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(&d);
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);

    if (d.exec() == QDialog::Accepted) {
        QString newName = d.copyEditText();
        if (newName.isEmpty()) {
            newName = currentName;
        }
        auto uri = QStringLiteral("%1/%2").arg(d.getCurrentUri()).arg(newName);
        QStringList uris;
        uris<<uri;
        if (!uris.isEmpty()) {
            results.insert(QStringLiteral("uris"), uris);
            windowhelper->deleteLater();
            return 0;
        } else {
            windowhelper->deleteLater();
            return 2;
        }
    }

    windowhelper->deleteLater();
    return 1;

//    QScopedPointer<FileDialog, QScopedPointerDeleteLater> fileDialog(new FileDialog());
//    Utils::setParentWindow(fileDialog.data(), parent_window);
//    fileDialog->setWindowTitle(title);
//    fileDialog->setModal(modalDialog);
//    fileDialog->m_fileWidget->setOperationMode(KFileWidget::Saving);
//    fileDialog->m_fileWidget->setConfirmOverwrite(true);

//    if (!currentFolder.isEmpty()) {
//        fileDialog->m_fileWidget->setUrl(QUrl::fromLocalFile(currentFolder));
//    }

//    if (!currentFile.isEmpty()) {
//        fileDialog->m_fileWidget->setSelectedUrl(QUrl::fromLocalFile(currentFile));
//    }

//    if (!currentName.isEmpty()) {
//        const QUrl url = fileDialog->m_fileWidget->baseUrl();
//        fileDialog->m_fileWidget->setSelectedUrl(QUrl::fromLocalFile(QStringLiteral("%1/%2").arg(url.toDisplayString(QUrl::StripTrailingSlash), currentName)));
//    }

//    if (!acceptLabel.isEmpty()) {
//        fileDialog->m_fileWidget->okButton()->setText(acceptLabel);
//    }

//    if (!nameFilters.isEmpty()) {
//        fileDialog->m_fileWidget->setFilter(nameFilters.join(QLatin1Char('\n')));
//    }

//    if (!mimeTypeFilters.isEmpty()) {
//        fileDialog->m_fileWidget->setMimeFilter(mimeTypeFilters);
//    }

//    if (fileDialog->exec() == QDialog::Accepted) {
//        QStringList files;
//        QUrl url = QUrl::fromLocalFile(fileDialog->m_fileWidget->selectedFile());
//        files << url.toDisplayString();
//        results.insert(QStringLiteral("uris"), files);
//        return 0;
//    }

//    return 1;
}

uint FileChooserPortal::SaveFiles(const QDBusObjectPath &handle, const QString &app_id, const QString &parent_window, const QString &title, const QVariantMap &options, QVariantMap &results)
{
    Q_UNUSED(app_id);

    qCDebug(XdgDesktopPortalUKUIFileChooser) << "SaveFile called with parameters:";
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    title: " << title;
    qCDebug(XdgDesktopPortalUKUIFileChooser) << "    options: " << options;

    bool modalDialog = true;
    QString acceptLabel;
    QString currentName;
    QString currentFolder;
    QString currentFile;
    QList<QPair<QString, QStringList>> nameFilters;
    bool isHideNameFilterDetails = false;
    bool isUsrMimeTypeFilter = false;
    QList<QPair<QString, QStringList>> mimeMap;

    // TODO parse options - choices

    if (options.contains(QStringLiteral("modal"))) {
        modalDialog = options.value(QStringLiteral("modal")).toBool();
    }

    if (options.contains(QStringLiteral("accept_label"))) {
        acceptLabel = options.value(QStringLiteral("accept_label")).toString();
    }

    if (options.contains(QStringLiteral("current_name"))) {
        currentName = options.value(QStringLiteral("current_name")).toString();
    }

    if (options.contains(QStringLiteral("current_folder"))) {
        currentFolder = QFile::decodeName(options.value(QStringLiteral("current_folder")).toByteArray());
    }

    if (options.contains(QStringLiteral("current_file"))) {
        currentFile = QFile::decodeName(options.value(QStringLiteral("current_file")).toByteArray());
    }

    if (options.contains(QStringLiteral("filters"))) {
        FilterListList filterListList = qdbus_cast<FilterListList>(options.value(QStringLiteral("filters")));
        for (const FilterList &filterList : filterListList) {
            QStringList filterStrings;
            QStringList mimeTypeString;

            for (const Filter &filterStruct : filterList.filters) {
                //特殊字符串处理[Hh][tT][Mm][Ll]
                QString s = filterStruct.filterString;
                QString filterS;

                if(s.contains("[") && s.contains("]") && s.length() > 3){
                    for(int i = 0; i < s.length(); i++){
                        if(i + 3 < s.length()){
                            QChar c = s[i + 1];
                            if((c > 'a' && c < 'z') || (c > 'A' && c < 'Z')){
                                if((s[i] == '[') && QString(s[i + 1]).toLower() == QString(s[i + 2]).toLower() && (s[i + 3] == ']')){
                                    filterS += QString(s[i + 1]).toLower();
                                    i = i + 3;
                                    continue;
                                }
                            }
                        }
                        filterS += s[i];
                    }
                }
                else
                    filterS = s;

                if (filterStruct.type == 0) {
                    filterStrings << filterS;
                    mimeTypeString << filterS;
                } else {
                    mimeTypeString << filterS;
                    isUsrMimeTypeFilter = true;
                }
            }
            qCDebug(XdgDesktopPortalUKUIFileChooser) << "save file mimeTypeString:" << mimeTypeString << filterList.userVisibleName;

            mimeMap.append(qMakePair(QString(filterList.userVisibleName), QStringList(mimeTypeString)));

            if (!filterStrings.isEmpty()) {
                if(filterList.userVisibleName.isEmpty() || filterList.userVisibleName == " "){
                    //nameFilters << QStringLiteral("%1").arg(filterStrings.join(QLatin1Char(' ')));
                    nameFilters.append(qMakePair(QString(""), QStringList(filterStrings)));

                }
                else
                    nameFilters.append(qMakePair(QString(filterList.userVisibleName), QStringList(filterStrings)));

                //nameFilters << QStringLiteral("%1|%2").arg(filterStrings.join(QLatin1Char(' '))).arg(filterList.userVisibleName);
            }
            if(!filterList.userVisibleName.isEmpty())
                isHideNameFilterDetails = true;
        }
    }

    if(mimeMap.count() == 0 & nameFilters.length() == 0){
        nameFilters.append(qMakePair(QString(tr("All Files ")), QStringList("*")));
    }

    KyNativeFileDialog d;
    WindowUtils::setParentWindow(&d, parent_window);
    d.setWindowModality(Qt::ApplicationModal);

    d.setFileMode(QFileDialog::ExistingFiles);
    d.setWindowTitle(title);
    d.setAcceptMode(QFileDialog::AcceptSave);

    QFileInfo currentFolderInfo(currentFolder);
    if (currentFolder.isEmpty() || !currentFolderInfo.isAbsolute()) {
        d.setDirectory(QString("file://%1").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
    } else {
        d.setDirectoryUrl(QUrl::fromLocalFile(currentFolder));
    }

    if (!currentFile.isEmpty()) {
        QUrl url = QUrl::fromLocalFile(currentFile);
        d.setCurrentSelections(QStringList()<<url.toString());
    }

    if(isHideNameFilterDetails)
        d.setOption(QFileDialog::HideNameFilterDetails, true);

    if (isUsrMimeTypeFilter && !mimeMap.isEmpty()) {
        d.setMimeTypeFiltersList(mimeMap);
    }
    if (!isUsrMimeTypeFilter && !nameFilters.isEmpty()) {
        d.setNameFiltersList(nameFilters);
    }
    //remove ukui-panel application icon
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(&d);
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);

    if (d.exec() == QDialog::Accepted) {
        auto uris = d.selectedFiles();
        if (!uris.isEmpty()) {
            results.insert(QStringLiteral("uris"), uris);
            windowhelper->deleteLater();
            return 0;
        } else {
            windowhelper->deleteLater();
            return 2;
        }
    }

    windowhelper->deleteLater();
    return 1;

//    QScopedPointer<FileDialog, QScopedPointerDeleteLater> fileDialog(new FileDialog());
//    Utils::setParentWindow(fileDialog.data(), parent_window);
//    fileDialog->setWindowTitle(title);
//    fileDialog->setModal(modalDialog);
//    fileDialog->m_fileWidget->setOperationMode(KFileWidget::Saving);
//    fileDialog->m_fileWidget->setConfirmOverwrite(true);

//    if (!currentFolder.isEmpty()) {
//        fileDialog->m_fileWidget->setUrl(QUrl::fromLocalFile(currentFolder));
//    }

//    if (!currentFile.isEmpty()) {
//        fileDialog->m_fileWidget->setSelectedUrl(QUrl::fromLocalFile(currentFile));
//    }

//    if (!currentName.isEmpty()) {
//        const QUrl url = fileDialog->m_fileWidget->baseUrl();
//        fileDialog->m_fileWidget->setSelectedUrl(QUrl::fromLocalFile(QStringLiteral("%1/%2").arg(url.toDisplayString(QUrl::StripTrailingSlash), currentName)));
//    }

//    if (!acceptLabel.isEmpty()) {
//        fileDialog->m_fileWidget->okButton()->setText(acceptLabel);
//    }

//    if (!nameFilters.isEmpty()) {
//        fileDialog->m_fileWidget->setFilter(nameFilters.join(QLatin1Char('\n')));
//    }

//    if (!mimeTypeFilters.isEmpty()) {
//        fileDialog->m_fileWidget->setMimeFilter(mimeTypeFilters);
//    }

//    if (fileDialog->exec() == QDialog::Accepted) {
//        QStringList files;
//        QUrl url = QUrl::fromLocalFile(fileDialog->m_fileWidget->selectedFile());
//        files << url.toDisplayString();
//        results.insert(QStringLiteral("uris"), files);
//        return 0;
//    }

//    return 1;
}

