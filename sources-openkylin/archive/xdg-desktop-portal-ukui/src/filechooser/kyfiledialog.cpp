/*
 * UKUI Style
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPainter>
#include <QRect>
#include <QVector4D>
#include <QRegExp>
#include <QMessageBox>
#include <QScrollBar>
#include <QCoreApplication>
#include <QTranslator>
#include <QWindow>
#include <QScreen>
#include <QDesktopWidget>
#include <QPainterPath>
#include <QDBusReply>
#include <QMimeDatabase>
#include <QMimeType>
#include <fcntl.h>
#include <peony-qt/controls/directory-view/directory-view-container.h>
#include <peony-qt/controls/menu/directory-view-menu/directory-view-menu.h>
#include <peony-qt/controls/directory-view/directory-view-widget.h>
#include <peony-qt/controls/directory-view/directory-view-factory/directory-view-factory-manager.h>
#include <peony-qt/file-operation-utils.h>
#include <peony-qt/file-utils.h>
#include <peony-qt/create-template-operation.h>
#include <peony-qt/global-settings.h>
#include <peony-qt/search-vfs-uri-parser.h>
#include <peony-qt/file-meta-info.h>
#include <peony-qt/clipboard-utils.h>
#include <PeonyFileInfo>
#include <PeonyFileItemModel>
#include <peony-qt/thumbnail-manager.h>
//#include <KWindowEffects>
//#include <KWindowSystem>
#include "debug.h"
#include "uikyfiledialog.h"
#include "kyfiledialogprivate.h"
#include "sidebar.h"
#include "pathbar.h"
#include "kyfiledialog.h"
#include "file-operation-dialog/kyfiledialogrename.h"
#include "ukuistylehelper/ukuistylehelper.h"

#define PERSONALSIE_SCHEMA            "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY          "transparency"

const static char *filterRegExp =
"^(.*)\\(([a-zA-Z0-9_.,*? +;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$";

// Makes a list of filters from a normal filter string "Image Files (*.png *.jpg)"
QStringList cleanFilterList(const QString &filter)
{
    QRegularExpression regexp(QString::fromLatin1(filterRegExp));
    Q_ASSERT(regexp.isValid());
    QString f = filter;
    QRegularExpressionMatch match;
    f.indexOf(regexp, 0, &match);
    if(match.hasMatch())
        f = match.captured(2);

    return f.split(QLatin1Char(' '), QString::SkipEmptyParts);
}

bool isFuseFileSystem(QString path)
{
    g_autoptr (GUnixMountEntry) entry = g_unix_mount_at(path.toUtf8().constData(), NULL);
    if (!entry) {
        entry = g_unix_mount_for(path.toUtf8().constData(), NULL);
        if (!entry) {
            return false;
        }
    }
    auto fsType = g_unix_mount_get_fs_type(entry);
    if (QString(fsType).contains("fuse.kyfs")) {
        return true;
    } else {
        return false;
    }
}

KyNativeFileDialog::KyNativeFileDialog(QWidget *parent)
    : QDialog(parent),
      d_ptr(new KyNativeFileDialogPrivate)
{
    QString locale = QLocale::system().name();

    if(qApp->property("Language").isValid())
    {
        QString language = qApp->property("Language").toString();
        QFile file("/usr/share/qt5-ukui-platformtheme/qt5-ukui-filedialog_" + language + ".qm");
        if(file.exists())
            locale = language;
    }
    installTranslate(locale);

    mKyFileDialogUi = new Ui_KyFileDialog(this);

    connect(d_ptr.get()->m_timer, &QTimer::timeout, this, [&](){
//        pDebug << "timeout isActive:..........." << d_ptr.get()->m_timer->isActive();
        this->show();
        if(d_ptr.get()->m_timer->isActive()){
            pDebug << "timer stop....";
            d_ptr.get()->m_timer->stop();
        }
    });

    mKyFileDialogUi->setupUi(this);
    d_ptr.get()->m_container = mKyFileDialogUi->m_container;

//    QString path = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
//    pDebug << "initialDirPath:" << path << selectedUrls();
//    goToUri(path, false);
    getCurrentPage()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mKyFileDialogUi->m_sider, &FileDialogSideBar::goToUriRequest, this, [this](const QString &uri, bool addToHistory, bool forceUpdate){
        pDebug << "goToUri0000" << uri;
        if (uri.startsWith("favorite://") && uri != "favorite:///") {
            QString u = Peony::FileUtils::getTargetUri(uri);
            goToUri(u, addToHistory, forceUpdate);

        }
        else
            goToUri(uri, addToHistory, forceUpdate);
    });

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::signal_responseUnmounted, this, &KyNativeFileDialog::slot_responseUnmounted);
    connect(getCurrentPage(), &Peony::DirectoryViewContainer::directoryChanged, this, &KyNativeFileDialog::updateWindowState);

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::directoryChanged, this, [=](){
        pDebug << "directoryChanged........" << getCurrentUri();

        mKyFileDialogUi->m_pathbarWidget->updatePath(getCurrentUri());

        Q_EMIT directoryEntered(QUrl(getCurrentUri()));
        pDebug << "locationChangeEnd....";
//        QCursor c;
//        c.setShape(Qt::ArrowCursor);
//        this->setCursor(c);
//        mKyFileDialogUi->m_sider->setCursor(c);

        //if(m_fileDialogHelper->isViewInitialFinished())
            intiContainerSort();

        if(acceptMode() == QFileDialog::AcceptOpen){
            if(!m_isLoadSelect && !mKyFileDialogUi->m_fileNameEdit->text().isEmpty())
                mKyFileDialogUi->m_fileNameEdit->setText("");
        }
        setShortCuts();
        m_isLoading = false;

        if(getCurrentUri() == "computer:///"){
            mKyFileDialogUi->m_modeButton->setVisible(false);
            mKyFileDialogUi->m_sortButton->setVisible(false);
        }
        else{
            mKyFileDialogUi->m_modeButton->setVisible(true);
            mKyFileDialogUi->m_sortButton->setVisible(true);
        }
    });

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::directoryChanged, this, [=](){
//        if(m_longNameChangeDir){
//            onAcceptButtonClicked();
//            m_longNameChangeDir = false;
//            return;
//        }
        updateSearchProgressBar();
    });
    connect(getCurrentPage(), &Peony::DirectoryViewContainer::directoryChanged, this, &KyNativeFileDialog::locationChangeEnd);
    connect(getCurrentPage(), &Peony::DirectoryViewContainer::viewDoubleClicked, this, [=](const QString &uri){
            auto info = Peony::FileInfo::fromUri(uri);
            pDebug << "viewDoubleClicked....." << uri << info->isDir() << info->isVolume();
            if (info->isDir() || info->isVolume()) {
                pDebug << "goToUrixxxxxxx....";
                goToUri(uri);
            } else {
                onAcceptButtonClicked();
            }
        });

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::selectionChanged, this, &KyNativeFileDialog::selectionChanged);

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::menuRequest, this, &KyNativeFileDialog::containerMenuRequest);

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::signal_itemAdded, this, [=](const QString& uri){
        pDebug << "signal_itemAdded....." << m_uris_to_edit << uri;
        if(m_uris_to_edit.isEmpty())
            return;
        QString editUri = Peony::FileUtils::urlDecode(m_uris_to_edit.first());
        QString infoUri = Peony::FileUtils::urlDecode(uri);
        if (editUri == infoUri ) {
            getCurrentPage()->getView()->scrollToSelection(uri);
            getCurrentPage()->getView()->editUri(uri);
        }
        m_uris_to_edit.clear();
    });

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::updateWindowLocationRequest, this, [=](const QString &uri, bool addToHistory, bool forceUpdate){
        pDebug << "page updateWindowLocationRequest.....uri:" << uri << getCurrentUri() << forceUpdate;
        if(uri == "")
            return;
        QString s = uri;
        QString s1 = s.endsWith("/") ? s.remove(s.length() - 1, 1) : s;
        QString s2 = getCurrentUri();
        QString s3 = s2.endsWith("/") ? s2.remove(s2.length() - 1, 1) : s2;
        if(s1 != s3 /*&& m_fileDialogHelper->isViewInitialFinished()*/)
        {
            pDebug << "s1:" << s1 << "s3:" << s3;
            goToUri(uri, addToHistory);
        }
    });

    //location change
    connect(this, &KyNativeFileDialog::locationChangeStart, this, [=]() {
        pDebug << "locationChangeStart........." << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
        QCursor c;
        c.setShape(Qt::BusyCursor);
        this->setCursor(c);
        mKyFileDialogUi->setCursor(c);
        if(getCurrentPage() && getCurrentPage()->getView())
            getCurrentPage()->getView()->setCursor(c);
        mKyFileDialogUi->m_sider->setCursor(c);
    });

    connect(this, &KyNativeFileDialog::locationChangeEnd, this, [=]() {
        pDebug << "locationChangeEnd........." << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
        QCursor c;
        c.setShape(Qt::ArrowCursor);
        this->setCursor(c);
        if(getCurrentPage() && getCurrentPage()->getView())
            getCurrentPage()->getView()->setCursor(c);
        mKyFileDialogUi->m_sider->setCursor(c);
        mKyFileDialogUi->setCursor(c);
    });

    connect(mKyFileDialogUi->m_pathbar, &Peony::AdvancedLocationBar::updateWindowLocationRequest,
            this, [this](const QString &uri, bool addHistory, bool forceUpdate){
        pDebug << "goToUrijjjjj...." << uri;
        goToUri(uri, addHistory, forceUpdate);
    });

    connect(mKyFileDialogUi->m_pathbar, &Peony::AdvancedLocationBar::refreshRequest, this, [=](){
        pDebug << "AdvancedLocationBar refreshRequest......" << getCurrentUri();
    });
    connect(mKyFileDialogUi->m_pathbar, &Peony::AdvancedLocationBar::searchRequest, this, [=](const QString &path, const QString &key){
        pDebug << "path.....:" << path << "key:" << key << m_lastSearchPath;
        if(key == "")
        {
            forceStopLoading();
            pDebug << "key is null m_lastSearchPath:" << m_lastSearchPath;
            m_isClearSearchKey = true;
            m_needSearch = false;
            QCursor c;
            c.setShape(Qt::ArrowCursor);
            this->setCursor(c);
            if(getCurrentPage() && getCurrentPage()->getView())
                getCurrentPage()->getView()->setCursor(c);
            mKyFileDialogUi->m_sider->setCursor(c);
            mKyFileDialogUi->setCursor(c);

            goToUri(m_lastSearchPath, true);
        }
        else
        {
            bool isSearchEngine = true;
            const QByteArray id(UKUI_SEARCH_SCHEMAS);
            if (QGSettings::isSchemaInstalled(id)) {
                QGSettings *searchSettings = new QGSettings(id, QByteArray(), this);
                if (!searchSettings || !searchSettings->keys().contains(SEARCH_METHOD_KEY)) {
                    isSearchEngine = false;
                }
            } else {
                isSearchEngine = false;
            }

            auto targetUri = Peony::SearchVFSUriParser::parseSearchKey(path, key, true, false, "", true);
            targetUri = Peony::SearchVFSUriParser::addSearchKey(targetUri, isSearchEngine);
            pDebug << "updateSearch targetUri:" <<targetUri;
            pDebug << "updateSearch path:" <<path;
            m_needSearch = true;
            goToUri(targetUri, true);

            QCursor c;
            c.setShape(Qt::BusyCursor);
            this->setCursor(c);
            if(getCurrentPage() && getCurrentPage()->getView())
                getCurrentPage()->getView()->setCursor(c);
            mKyFileDialogUi->m_sider->setCursor(c);
            mKyFileDialogUi->setCursor(c);
        }
    });

    mKyFileDialogUi->m_backButton->setToolTip(tr("Go Back"));
    mKyFileDialogUi->m_backButton->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    mKyFileDialogUi->m_backButton->setAutoRaise(true);
    mKyFileDialogUi->m_backButton->setEnabled(false);
    connect(mKyFileDialogUi->m_backButton, &QToolButton::clicked, this, &KyNativeFileDialog::goBack);


    mKyFileDialogUi->m_forwardButton->setToolTip(tr("Go Forward"));
    mKyFileDialogUi->m_forwardButton->setIcon(QIcon::fromTheme("go-next-symbolic"));
    mKyFileDialogUi->m_forwardButton->setAutoRaise(true);
    mKyFileDialogUi->m_forwardButton->setEnabled(false);
    connect(mKyFileDialogUi->m_forwardButton, &QToolButton::clicked, this, &KyNativeFileDialog::goForward);

    mKyFileDialogUi->m_toParentButton->setToolTip(tr("Cd Up"));
    mKyFileDialogUi->m_toParentButton->setIcon(QIcon::fromTheme("go-up-symbolic"));
    mKyFileDialogUi->m_toParentButton->setAutoRaise(true);
    mKyFileDialogUi->m_toParentButton->setEnabled(false);
    connect(mKyFileDialogUi->m_toParentButton, &QToolButton::clicked, this,  &KyNativeFileDialog::goToParent);

    mKyFileDialogUi->m_searchBtn->setIcon(QIcon::fromTheme("edit-find-symbolic"));
    mKyFileDialogUi->m_searchBtn->setToolTip(tr("Search"));
    mKyFileDialogUi->m_searchBtn->setIconSize(QSize(16, 16));
    mKyFileDialogUi->m_searchBtn->setAutoRaise(true);

    mKyFileDialogUi->m_searchBtn->setProperty("useIconHighlightEffect", true);
    mKyFileDialogUi->m_searchBtn->setProperty("iconHighlightEffectMode", 1);
    connect(mKyFileDialogUi->m_searchBtn, &QToolButton::clicked, this, &KyNativeFileDialog::searchButtonClicked);

    mKyFileDialogUi->m_modeButton->setToolTip(tr("View Type"));
    mKyFileDialogUi->m_modeButton->setIcon(QIcon::fromTheme("view-grid-symbolic"));
    mKyFileDialogUi->m_modeButton->setProperty("isWindowButton", 1);
    mKyFileDialogUi->m_modeButton->setProperty("useIconHighlightEffect", 0x2);
    mKyFileDialogUi->m_modeButton->setAutoRaise(true);

    mKyFileDialogUi->m_useGlobalSortAction->setChecked(Peony::GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool());
    connect(mKyFileDialogUi->m_useGlobalSortAction, &QAction::triggered, this, [=](bool checked){
        Peony::GlobalSettings::getInstance()->setValue(USE_GLOBAL_DEFAULT_SORTING, checked);
    });

    mKyFileDialogUi->m_sortButton->setToolTip(tr("Sort Type"));
    mKyFileDialogUi->m_sortButton->setProperty("isWindowButton", 1);
    mKyFileDialogUi->m_sortButton->setProperty("useIconHighlightEffect", 0x2);
    mKyFileDialogUi->m_sortButton->setAutoRaise(true);

    mKyFileDialogUi->m_maximizeAndRestore->setToolTip(tr("Maximize"));
    mKyFileDialogUi->m_maximizeAndRestore->setIcon(QIcon::fromTheme("window-close-symbolic"));
    mKyFileDialogUi->m_maximizeAndRestore->setAutoRaise(true);
    mKyFileDialogUi->m_maximizeAndRestore->setProperty("isWindowButton", 1);
    mKyFileDialogUi->m_maximizeAndRestore->setProperty("useIconHighlightEffect", 0x2);
    connect(mKyFileDialogUi->m_maximizeAndRestore, &QToolButton::clicked, this, [=]() {
        if (!this->isMaximized()) {
            this->showMaximized();
        } else {
            this->showNormal();
        }
        updateMaximizeState();
    });

    mKyFileDialogUi->m_closeButton->setToolTip(tr("Close"));
    mKyFileDialogUi->m_closeButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    mKyFileDialogUi->m_closeButton->setAutoRaise(true);
    mKyFileDialogUi->m_closeButton->setProperty("isWindowButton", 0x2);

    connect(mKyFileDialogUi->m_modeMenu, &QMenu::triggered, this, [=](QAction *action) {
        if (action == mKyFileDialogUi->m_listModeAction) {
            this->getCurrentPage()->switchViewType("List View");
        } else {
            this->getCurrentPage()->switchViewType("Icon View");
        }
    });

    connect(mKyFileDialogUi->m_acceptButton, &QPushButton::clicked, this, &KyNativeFileDialog::onAcceptButtonClicked);

    connect(mKyFileDialogUi->m_rejectButton, &QPushButton::clicked, this, &KyNativeFileDialog::onRejectButtonClicked);

//    connect(mKyFileDialogUi->m_fileTypeCombo,
//            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
//            this, [this](const QString s){
//        pDebug << "activated..." << s;
//        selectNameFilter(s);});

    connect(mKyFileDialogUi->m_fileTypeCombo,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &KyNativeFileDialog::selectedNameFilterChanged);

    connect(mKyFileDialogUi->m_fileTypeCombo,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, [this](const QString s){
        pDebug << "currentIndexChanged..." << s << mKyFileDialogUi->m_fileTypeCombo->currentIndex();
        selectNameFilter(s);});

    connect(mKyFileDialogUi->m_closeButton, &QToolButton::clicked, this, &KyNativeFileDialog::onRejectButtonClicked);

    connect(mKyFileDialogUi->m_fileNameEdit, &QLineEdit::textChanged, this, [=](const QString &text){
        pDebug << "m_fileNameEdit textChanged...." << text.isEmpty();
        lineEditTextChange(text);
    });

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::viewTypeChanged, this, &KyNativeFileDialog::onSwitchView);

    connect(mKyFileDialogUi->m_sortOrderGroup, &QActionGroup::triggered, this, [=](QAction *action) {
        int index = mKyFileDialogUi->m_sortOrderGroup->actions().indexOf(action);
        getCurrentPage()->setSortOrder(Qt::SortOrder(index));
        setSortType();
    });

    connect(mKyFileDialogUi->m_sortTypeGroup, &QActionGroup::triggered, this, [=](QAction *action) {
        int index = mKyFileDialogUi->m_sortTypeGroup->actions().indexOf(action);
        getCurrentPage()->setSortType(Peony::FileItemModel::ColumnType(index));
    });

    connect(mKyFileDialogUi->m_sortMenu, &QMenu::aboutToShow, this, [=]() {
        bool originPathVisible = getCurrentUri()== "trash:///";
        mKyFileDialogUi->m_originalPath->setVisible(originPathVisible);
        mKyFileDialogUi->m_sortTypeGroup->actions().at(getCurrentSortColumn())->setChecked(true);
        mKyFileDialogUi->m_sortOrderGroup->actions().at(getCurrentSortOrder())->setChecked(true);
    });

    foreach (QAction *action, mKyFileDialogUi->m_sortTypeGroup->actions())
    {
        bool checked = (int(getCurrentPage()->getSortType()) ==
                        mKyFileDialogUi->m_sortTypeGroup->actions().indexOf(action) ? true : false);
        action->setChecked(checked);
    }

    refreshContainerSort();
    updateMaximizeState();
    onSwitchView();

    m_model = new QStringListModel(this);
    m_completer = new QCompleter(mKyFileDialogUi->m_fileNameEdit);
    m_completer->setModel(m_model);
    m_completer->setMaxVisibleItems(10);
    m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
//    m_completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    mKyFileDialogUi->m_fileNameEdit->setCompleter(m_completer);

    int number = QApplication::desktop()->screenNumber(QCursor::pos());
    if(number<0){
        number=0;
    }
    QSize size = QGuiApplication::screens().at(number)->geometry().size();
    pDebug << "setmaxsize:" << size;
    this->setMaximumSize(size);

    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    m_longSavePath = "file://" + downloadPath + QString("/扩展");


    //transparency
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->setBlurEffect(QRegion(), 0, this->isActiveWindow());
    m_windowHelper->removeTitleBar();
    QGSettings *settings = nullptr;
    if (QGSettings::isSchemaInstalled(PERSONALSIE_SCHEMA)) {
        settings = new QGSettings(PERSONALSIE_SCHEMA, QByteArray(), this);
        m_transparency = settings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;

        connect(settings,&QGSettings::changed,this,[=](QString changedKey) {
            if (changedKey == PERSONALSIE_TRAN_KEY) {
               m_transparency = settings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;

               this->update();
            }
        });
    } else {
        settings = nullptr;
        pDebug << PERSONALSIE_SCHEMA<<"not installed";
    }
}

KyNativeFileDialog::~KyNativeFileDialog()
{
    pDebug << "~~~~~~~~KyNativeFileDialog";
}

void KyNativeFileDialog::updateMaximizeState()
{
    bool maximized = this->isMaximized();
    if (maximized) {
        mKyFileDialogUi->m_maximizeAndRestore->setToolTip(tr("Restore"));
        mKyFileDialogUi->m_maximizeAndRestore->setIcon(QIcon::fromTheme("window-restore-symbolic"));
    } else {
        mKyFileDialogUi->m_maximizeAndRestore->setToolTip(tr("Maximize"));
        mKyFileDialogUi->m_maximizeAndRestore->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    }
}

Peony::FMWindowIface *KyNativeFileDialog::create(const QString &uri)
{
    auto dialog = new KyNativeFileDialog();

    return dialog;
}

Peony::FMWindowIface *KyNativeFileDialog::create(const QStringList &uris)
{
    auto dialog = new KyNativeFileDialog();

    return dialog;
}

const QStringList KyNativeFileDialog::getCurrentSelections()
{
    if(containerView())
        return containerView()->getSelections();
    else
        return QStringList();
}

const QStringList KyNativeFileDialog::getCurrentSelectionsList()
{
    QStringList list;
    foreach (QString str, getCurrentSelections()) {
        list.append(Peony::FileUtils::urlDecode(str));
    }
    return list;
}

const QStringList KyNativeFileDialog::getCurrentAllFileUris()
{
    if(containerView())
        return containerView()->getAllFileUris();
    return QStringList();
}
const QList<std::shared_ptr<Peony::FileInfo>> KyNativeFileDialog::getCurrentSelectionFileInfos()
{
    const QStringList uris = getCurrentSelections();
    QList<std::shared_ptr<Peony::FileInfo>> infos;
    for(auto uri : uris) {
        auto info = Peony::FileInfo::fromUri(uri);
        infos<<info;
    }
    return infos;
}

void KyNativeFileDialog::setCurrentSelections(QStringList selections)
{
    QStringList list;
    foreach (QString str, selections) {
        list.append(Peony::FileUtils::urlEncode(str));
    }
    if(containerView()){
        pDebug << "setCurrentSelections........" << list << containerView()->getSelections().length();

        //是耗时操作  且会先将之前的选中内容清空，有selectchange信号
        containerView()->setSelections(list);
        pDebug << "get setCurrentSelections....:" << containerView()->getSelections();
    }
}

Qt::SortOrder KyNativeFileDialog::getCurrentSortOrder()
{
    return getCurrentPage()->getSortOrder();
}

int KyNativeFileDialog::getCurrentSortColumn()
{
    return getCurrentPage()->getSortType();
}

bool KyNativeFileDialog::getWindowShowHidden()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        return settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
    } else {
        auto uri = getCurrentUri();
        auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            return metaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).isValid()? metaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).toBool(): (settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool());
        } else {
            pDebug<<"can not get file meta info"<<uri;
            return settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
        }
    }
}
bool KyNativeFileDialog::getWindowUseDefaultNameSortOrder()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        return settings->getValue(SORT_CHINESE_FIRST).isValid()? settings->getValue(SORT_CHINESE_FIRST).toBool(): true;
    } else {
        auto uri = getCurrentUri();
        auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            return metaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).isValid()? metaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).toBool(): (settings->getValue(SORT_CHINESE_FIRST).isValid()? settings->getValue(SORT_CHINESE_FIRST).toBool(): true);
        } else {
            pDebug<<"can not get file meta info"<<uri;
            return settings->getValue(SORT_CHINESE_FIRST).isValid()? settings->getValue(SORT_CHINESE_FIRST).toBool(): true;
        }
    }
}
bool KyNativeFileDialog::getWindowSortFolderFirst()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        return settings->getValue(SORT_FOLDER_FIRST).isValid()? settings->getValue(SORT_FOLDER_FIRST).toBool(): true;
    } else {
        auto uri = getCurrentUri();
        auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            return metaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).isValid()? metaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).toBool(): (settings->getValue(SORT_FOLDER_FIRST).isValid()? settings->getValue(SORT_FOLDER_FIRST).toBool(): true);
        } else {
            pDebug<<"can not get file meta info"<<uri;
            return settings->getValue(SORT_FOLDER_FIRST).isValid()? settings->getValue(SORT_FOLDER_FIRST).toBool(): true;
        }
    }
}

void KyNativeFileDialog::refresh()
{
    getCurrentPage()->refresh();
}

void KyNativeFileDialog::forceStopLoading()
{
    getCurrentPage()->stopLoading();

    //Key_escape also use as cancel
    if (Peony::ClipboardUtils::isClipboardHasFiles())
    {
        Peony::ClipboardUtils::clearClipboard();
        if(containerView())
            containerView()->repaintView();
    }
}

void KyNativeFileDialog::setShowHidden(bool showHidden)
{
    getCurrentPage()->setShowHidden(showHidden);
}

void KyNativeFileDialog::setUseDefaultNameSortOrder(bool use)
{
    if (!getCurrentPage()) {
        return;
    }
    getCurrentPage()->setUseDefaultNameSortOrder(use);
}

void KyNativeFileDialog::setSortFolderFirst(bool set)
{
    if (!getCurrentPage()) {
        return;
    }
    getCurrentPage()->setSortFolderFirst(set);
}

void KyNativeFileDialog::setCurrentSelectionUris(const QStringList &uris)
{
    getCurrentPage()->stopLoading();

    //Key_escape also use as cancel
    if (Peony::ClipboardUtils::isClipboardHasFiles())
    {
        Peony::ClipboardUtils::clearClipboard();
        if(containerView())
            containerView()->repaintView();
    }
}

void KyNativeFileDialog::setCurrentSortOrder (Qt::SortOrder order)
{
    getCurrentPage()->setSortOrder(order);
}

void KyNativeFileDialog::setCurrentSortColumn (int sortColumn)
{
    getCurrentPage()->setSortType(Peony::FileItemModel::ColumnType(sortColumn));
}

void KyNativeFileDialog::editUri(const QString &uri)
{
    if(containerView())
        containerView()->editUri(uri);
}

void KyNativeFileDialog::editUris(const QStringList &uris)
{
    if(containerView())
        containerView()->editUris(uris);
}

bool KyNativeFileDialog::getFilterWorking()
{
    return false;
}

void KyNativeFileDialog::beginSwitchView(const QString &viewId)
{
    if (getCurrentUri() == "computer:///")
        return;
    auto selection = getCurrentSelections();
    if(!getCurrentPage()||!(getCurrentPage()->getView()))
        return;

    if (getCurrentPage()->getView()->viewId() == viewId)
        return;

    getCurrentPage()->switchViewType(viewId);

    // change default view id
    auto factoryManager = Peony::DirectoryViewFactoryManager2::getInstance();
    auto internalViews = factoryManager->internalViews();
    if (internalViews.contains(viewId))
        Peony::GlobalSettings::getInstance()->setValue(DEFAULT_VIEW_ID, viewId);

    setCurrentSelections(selection);
    if (selection.count() >0)
        getCurrentPage()->getView()->scrollToSelection(selection.first());

}


/**
 * @brief 当前目录
 */
const QString KyNativeFileDialog::getCurrentUri()
{
    return Peony::FileUtils::urlDecode(getCurrentPage()->getCurrentUri());
}

/**
 * @param directory 路径
 * @brief 设置路径为directory
 */
void KyNativeFileDialog::setDirectory(const QString &directory)
{
    pDebug << "setDirectoryoooo...:" << directory;
    setDirectoryUrl(QUrl(directory));
}

void KyNativeFileDialog::setDirectory(const QDir &directory)
{
    setDirectoryUrl(QUrl::fromLocalFile(directory.absolutePath()));
}

QDir KyNativeFileDialog::directory() const
{
    QString uri = directoryUrl().toString();

    if (uri.startsWith("trash://") || uri.startsWith("recent://") ||
            uri.startsWith("computer://") || uri.startsWith("favorite://") ||
            uri.startsWith("filesafe://") || uri.startsWith("idm://")){
        uri = convertSpecialPath(uri);
        pDebug << "directory uri 00000000..:" << uri;
    }
    else if(uri.startsWith("mult://") || uri.startsWith("smb://") ||
            uri.startsWith("ftp://") || uri.startsWith("sftp://") ||
            uri.startsWith("mtp://") || uri.startsWith("gphoto2://")){
        uri = getCurrentPage()->getCurrentUri();//特殊路径不能先编码转换  可能会造成结果不准

        uri = convertSpecialPath(uri);
    }
    else if(!uri.isEmpty())
        uri = directoryUrl().toLocalFile();
    else{
        QDir dir;
        dir.setPath(uri);
        return dir;
    }
    pDebug << "directory 111111 uri......" << uri;
    return QDir(uri);
}

void KyNativeFileDialog::setDirectoryUrl(const QUrl &directory)
{
    pDebug << "setDirectoryUrl.....:" << directory;
    if (!getCurrentPage()) {
        return;
    }
    goToUri(directory.toString(), true);
}

QUrl KyNativeFileDialog::directoryUrl() const
{
    if (!getCurrentPage()) {
        return QUrl(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    } else {
        QString uri = getCurrentPage()->getCurrentUri();
        if(uri.startsWith("mult://") || uri.startsWith("smb://") ||
                uri.startsWith("ftp://") || uri.startsWith("sftp://") ||
                uri.startsWith("mtp://") || uri.startsWith("gphoto2://"))
            return QUrl(uri);
        return QUrl(Peony::FileUtils::urlDecode(uri));
    }
}

void KyNativeFileDialog::goToUri(const QString &uri, bool addToHistory, bool forceUpdate)
{
    pDebug << "goToUri....,,,,:" << uri << getCurrentUri();

//    if(containerView() && m_fileDialogHelper){
//        pDebug << "connect............";
//        connect(containerView(), &Peony::DirectoryViewWidget::viewDirectoryChanged,
//                m_fileDialogHelper, &KyFileDialogHelper::viewInitialFinished, Qt::UniqueConnection);
//    }

    QString gUri = uri;
    pDebug << "goto uri00000........" << gUri;
    if ((gUri == "file:///data" || gUri == "file:///data/") && Peony::FileUtils::isFileExsit("file:///data/usershare")){
        bool isDBlockHasUserFile = isDataBlockHasUserFile();
        if(!isDBlockHasUserFile) {
            pDebug << "isDataBlockHasUserFile......";
            gUri = "file:///data/usershare";
        }
    }

    if(gUri.isEmpty() || gUri == getCurrentUri())
        return;
    QString nUri = gUri, nCurrentUri = getCurrentUri();
    pDebug << "nUri000..." << nUri << "nCurrentUri...." << nCurrentUri;
    if(nUri.endsWith("/")){
        if((nUri.length() >= 2 && nUri[nUri.length() - 2] != "/") || nUri.length() <2)
            nUri = nUri.remove(nUri.length() - 1, 1);
    }
    if(nCurrentUri.endsWith("/")){
        if((nCurrentUri.length() >= 2 && nCurrentUri[nCurrentUri.length() - 2] != "/") || nCurrentUri.length() <2)
            nCurrentUri = nCurrentUri.remove(nCurrentUri.length() - 1, 1);
    }
    pDebug << "nUri..." << nUri << "nCurrentUri...." << nCurrentUri;
    if(nUri == nCurrentUri)   {
        return;
    }
    pDebug << "getCurrentUri....,,,,:" << getCurrentUri();
    if (true)//(isInitialGoToUriNum || (m_fileDialogHelper != nullptr && m_fileDialogHelper->isShow()))
    {
        isInitialGoToUriNum = false;
        getCurrentPage()->stopLoading();
//        if(!m_fileDialogHelper->isShow())
//            addToHistory = false;
//        pDebug << "getCurrentUri....,,,,:" << m_fileDialogHelper->isShow() << getCurrentUri();
        pDebug << "gotouri123123:" << uri << addToHistory << forceUpdate;
        Q_EMIT locationChangeStart();
        m_isLoading = true;
        mKyFileDialogUi->m_pathbarWidget->updatePath(gUri);

        if(gUri.startsWith("search:///search_uris="))
            getCurrentPage()->goToUri(gUri, addToHistory, forceUpdate);
        else
        getCurrentPage()->goToUri(Peony::FileUtils::urlEncode(gUri), addToHistory, forceUpdate);
    }
}

/**
 * @param filename文件名
 * @brief 选择指定文件
 */
void KyNativeFileDialog::selectFile(const QString &filename)
{
    QUrl url = getCurrentUri();
    pDebug << "selectFileoooo..........:" << filename;
//    if(!QFile::exists(filename.remove(0,7)))
//    {
//        pDebug << "selectFile not exist:" << filename.remove(0,7);
//        return;
//    }
    QString path = filename;

    if(url.toString() != path)
    {
        if(path.startsWith("/"))
            path = ("file://" + path);

        pDebug << "select filename path:" << path;

        QUrl pathUrl(path);
        pDebug << "select pathUrl:" << pathUrl.path();

        QDir dir(pathUrl.path());

        if(!path.endsWith("/"))
            dir.cdUp();
        setDirectoryUrl(QUrl("file://" + dir.path()));
        pDebug << "url2222:" << dir.path();
    }
    pDebug << "select url....";
    QList<QUrl> urls;
    if(path.startsWith("/"))
        path = "file://" + path;
    pDebug << "filename:...." << path;
    urls.append(QUrl(path));
    pDebug << "setInitiallySelectedFiles......" << path;
//    m_fileDialogHelper->options()->setInitiallySelectedFiles(urls);
//    QDir dir(url.path());
//    url.setPath(dir.absoluteFilePath(filename));
//    pDebug << "selectFile url:" << url << dir.absoluteFilePath(filename) << filename;
    selectUrl(QUrl(path));
}

QStringList KyNativeFileDialog::selectedFiles() const
{
    QStringList list;
    for (const QUrl &url : selectedUrls()) {
        QUrl fileUrl(url);
        pDebug << "selectedFiles fileUrl.....:" << fileUrl.toString() << fileUrl.path();
//        if(!fileUrl.toString().startsWith("recent://") && !fileUrl.toString().startsWith("trash://"))
//            list << fileUrl.toLocalFile();
//        else{
//            list << Peony::FileUtils::getEncodedUri(fileUrl.toString());
//        }
        list << Peony::FileUtils::getEncodedUri(fileUrl.toString());
    }
    pDebug << "selectedFiles...." << list;
    return list;
}

void KyNativeFileDialog::selectUrl(const QUrl &url)
{
    return;
    Q_D(KyNativeFileDialog);
    pDebug << "selectUrlkkkkk........url.path:" << url.path() << isDir(url.path());
    if(d->fileMode != QFileDialog::DirectoryOnly && d->fileMode != QFileDialog::Directory && isDir(url.path()))
        setCurrentInputName(QFileInfo(url.path()).fileName());
}

QList<QUrl> KyNativeFileDialog::selectedUrls() const
{
    pDebug << "selectedUrls........:";
    if (!getCurrentPage()) {
        return QList<QUrl>();
    }
    pDebug << "selectedUrls........:" << getCurrentPage()->getCurrentSelections();
    QStringList urlStrList = getCurrentPage()->getCurrentSelections();
    QList<QUrl> urls;
    for (auto uri : urlStrList) {
        pDebug << "selectedUrls urlDecode....:" << Peony::FileUtils::urlDecode(uri) << QUrl(Peony::FileUtils::urlDecode(uri)) << Peony::FileUtils::getTargetUri(uri);
        if (!uri.startsWith("trash://") && !uri.startsWith("recent://") &&
                !uri.startsWith("computer://") && !uri.startsWith("favorite://") &&
                !uri.startsWith("filesafe://") && !uri.startsWith("mult://") &&
                !uri.startsWith("smb://") && !uri.startsWith("ftp://") &&
                !uri.startsWith("sftp://") &&!uri.startsWith("mtp://") &&
                !uri.startsWith("gphoto2://") && !uri.startsWith("idm://")){

            if(uri.startsWith("file:///"))
                uri = uri.remove(0,7);
            pDebug << "open selectedFiles00000..........:" << uri;
            urls << QUrl::fromLocalFile(Peony::FileUtils::urlDecode(uri));
        }
        else
        {
            urls << convertSpecialPath(uri);
        }
        pDebug << "selectedUrls uri test......." << uri << urls;
    }
    pDebug << "selectedUrls..." << urls;
    return urls;
}

QStringList qt_strip_filters(const QStringList &filters)
{
    QStringList strippedFilters;
    QRegExp r(QString::fromLatin1("^(.*)\\(([^()]*)\\)$"));
    const int numFilters = filters.count();
    strippedFilters.reserve(numFilters);
    for (int i = 0; i < numFilters; ++i) {
        QString filterName;
        int index = r.indexIn(filters[i]);
        if (index >= 0)
            filterName = r.cap(1);
        strippedFilters.append(filterName.simplified());
    }
    return strippedFilters;
}

void KyNativeFileDialog::setComBoxItems(const QStringList &filters)
{
    mKyFileDialogUi->m_fileTypeCombo->clear();
    mKyFileDialogUi->m_fileTypeCombo->addItems(filters);
}

void KyNativeFileDialog::setNameFilters(const QStringList &filters)
{
    Q_D(KyNativeFileDialog);
    d->nameFilters = filters;
    pDebug << "setNameFilters filters..... :" << filters;
    m_nameFilterMap.clear();

    QStringList l;
    if (testOption(QFileDialog::HideNameFilterDetails) && (d->fileMode != QFileDialog::DirectoryOnly && d->fileMode != QFileDialog::Directory)) {
        l = qt_strip_filters(filters);
        pDebug << "HideNameFilterDetails..." << l;
        setComBoxItems(l);
    } else {
        l = filters;
        setComBoxItems(filters);
    }
    for(int i = 0; i < filters.length(); i++)
        m_nameFilterMap.insert(i, l[i]);

    int index = mKyFileDialogUi->m_fileTypeCombo->currentIndex();
    pDebug << "setNameFilters index.....:" << index;
    selectNameFilterCurrentIndex(index);
}

QStringList KyNativeFileDialog::nameFilters() const
{
    Q_D( const KyNativeFileDialog);
    return d->nameFilters;
}

void KyNativeFileDialog::setNameFiltersList(QList<QPair<QString, QStringList>> filtersMap)
{
    QStringList filterList;
    for (auto pair : filtersMap) {
        QStringList filters = pair.second;
        QString s;
        for(int j = 0; j < filters.length(); j++){
            s.append(filters[j] + " ");
        }

        if(!pair.first.isEmpty())
            filterList.append(pair.first + " (" + s + ")");
        else
            filterList.append(s);

    }
    pDebug << "setNameFiltersList filterList" <<filterList;
    setNameFilters(filterList);
}


void KyNativeFileDialog::selectNameFilter(const QString &filter)
{
    Q_D( const KyNativeFileDialog);

    pDebug << "selectNameFilter.,,,,,....." << filter;
    QString key = filter;
    int index = 0;
    if(!mKyFileDialogUi->m_fileTypeCombo->findText(key)){
        if (testOption(QFileDialog::HideNameFilterDetails) && (d->fileMode != QFileDialog::DirectoryOnly && d->fileMode != QFileDialog::Directory)) {
            QStringList l = qt_strip_filters(QStringList(filter));
            pDebug << "llllllllllll" << l << l.length();
            if(l.length() > 0)
                key = l.first();
        } else {
            key = filter;
        }
    }
    pDebug << "key:" << key;
    if(!key.isEmpty())
        index = mKyFileDialogUi->m_fileTypeCombo->findText(key);
    if(index < 0)
        return;
    pDebug << "index:" << index;

    selectNameFilterCurrentIndex(index);
    selectNameFilterByIndex(index);
    Q_EMIT filterSelected(filter);
}


QString KyNativeFileDialog::selectedNameFilter() const
{
    Q_D(const KyNativeFileDialog);
    const QComboBox *box = mKyFileDialogUi->m_fileTypeCombo;
    return box ? d->nameFilters.value(box->currentIndex()) : QString();
}


QString KyNativeFileDialog::selectedMimeTypeFilter() const
{
    Q_D(const KyNativeFileDialog);
    const QComboBox *box = mKyFileDialogUi->m_fileTypeCombo;
    return box ? d->mimetypeFilters.value(box->currentIndex()) : QString();
}

void KyNativeFileDialog::setMimeTypeFilters(const QStringList &filters)
{
    Q_D(KyNativeFileDialog);
    d->mimetypeFilters = filters;
    QStringList mimeList;
    for(int i = 0; i < filters.count(); i++){
        QMimeDatabase db;
        QMimeType type = db.mimeTypeForName(filters[i]);
        pDebug << "setMimeTypeFilters filters" << filters[i] << type.isValid();

        if(type.isValid()){
            pDebug << "setMimeTypeFilters filterString" << type.filterString();
            mimeList.append(type.filterString());
        }
        else{
            QString s = filters[i];
            if(s.endsWith("/*")){
                s = s.left(s.lastIndexOf("/")) + (" (*)");
             mimeList.append(s);
            }
        }
    }
    setNameFilters(mimeList);
}

void KyNativeFileDialog::setMimeTypeFiltersList(QList<QPair<QString, QStringList>> filtersHash)
{
    Q_D(KyNativeFileDialog);
    QStringList mimeList;
    for (auto pair : filtersHash) {
        QStringList filters = pair.second;
        QString mimeTypeStr;
        QString mimeFilterStr;
        QString visibleName = pair.first;
        for(int j = 0; j < filters.length(); j++){
            mimeFilterStr.append(QString(filters.value(j) + " "));
            QMimeDatabase db;
            QMimeType type = db.mimeTypeForName(filters[j]);
            pDebug << "setMimeTypeFiltersList filters" <<  filters[j] << type.isValid();
            if(type.isValid()){//解析mimetype数据
                qDebug() << "setMimeTypeFilters filterString" << type.filterString();
                QString s = type.filterString();
                if(s.contains("(*")){
                    s = s.right(s.length() - s.lastIndexOf("(*"));
                    s.remove(0,1);
                }
                if(s.endsWith(")"))
                    s.remove(s.length() - 1, 1);
                pDebug << "s:" << s;
                mimeTypeStr.append(QStringLiteral("%1%2").arg(s).arg(QLatin1Char(' ')));
            }
            else if(filters[j] == "*"){//非mimetype数据
                mimeTypeStr = filters[j];
            }
        }
        d->mimetypeFilters.append(mimeFilterStr);
        pDebug << "setMimeTypeFiltersList mimeTypeStr...." << mimeTypeStr;
        QString filter = visibleName + (" (") + mimeTypeStr + (")");
        mimeList.append(filter);
    }
    setNameFilters(mimeList);

}


void KyNativeFileDialog::selectNameFilterByIndex(int index)
{
    Q_D(KyNativeFileDialog);
    if (index < 0 || index >= d->nameFilters.length() || !getCurrentPage()) {
        return;
    }
    mKyFileDialogUi->m_fileTypeCombo->setCurrentIndex(index);
    QStringList nameFilters = d->nameFilters;
    if (index == nameFilters.size()) {
        nameFilters.append(d->nameFilters[d->nameFilters.length() - 1]);
        setNameFilters(nameFilters);
    }
    QString nameFilter = nameFilters.value(index);
    QStringList newNameFilters = nameFilters;
    pDebug << "selectNameFilterByIndex00000000000";
    if (/*d->acceptMode == QFileDialog::AcceptSave &&*/ !newNameFilters.isEmpty() && (d->fileMode != QFileDialog::DirectoryOnly && d->fileMode != QFileDialog::Directory)) {
        QMimeDatabase db;
        QString text = copyEditText();//mKyFileDialogUi->m_fileNameEdit->text();
        pDebug << "selectNameFilterByIndex text...." << text;
        pDebug << "selectNameFilterByIndex newNameFilters...." << newNameFilters;
        QStringList list = text.split(".");
        pDebug << "selectNameFilterByIndex11111111 list" << list;
        QString filter = cleanFilterList(nameFilter).value(0);
        QStringList fList = cleanFilterList(nameFilter);
        //后缀匹配  如果没有匹配成功则用当前filter的第一个
        for(int i = 0; i < fList.length(); i++){
            QString l = fList[i];
            if(l.length() >= 2)
                l = l.right(l.length() - 2);
            if(list.value(list.length() - 1) == l)
                filter = l;
        }
        if(filter != "*"){
            if(filter.startsWith("*."))
                filter = filter.remove(0,2);
            if(list.length() > 1)
            {
                if(list[list.length() - 1] != filter)
                {
                    int m = list[list.length() - 1].length();
                    int n = text.length() - m;
                    pDebug << "m:" << m << "   n: " << n;
                    QString s = text.remove(n, m);
                    pDebug << "s:" << s;
                    s = s + filter;
                    pDebug << "s s:" << s;
                    //把后缀加入输入框
                    mKyFileDialogUi->m_fileNameEdit->setText(s);
                }
            }else{

            }
        }
    }
    if ((d->fileMode == QFileDialog::DirectoryOnly || d->fileMode == QFileDialog::Directory) && QStringList("/") != newNameFilters) {
         newNameFilters = QStringList("/");
    }
}

void KyNativeFileDialog::selectNameFilterCurrentIndex(int index)
{
    Q_D(KyNativeFileDialog);

    if(index < 0 || index >= d->nameFilters.length())
        return;
    QString nameFilter = d->nameFilters[index];
    pDebug << "selectNameFilterCurrentIndex nameFilter..." << nameFilter;

    if(nameFilter.isEmpty())
        return;
    QStringList filterList = cleanFilterList(nameFilter);

    pDebug << "selectNameFilterCurrentIndex filterList:" << filterList;
    QStringList mimeTypeFilterList;

    pDebug << "selectNameFilterCurrentIndex filemode.....:" << fileMode();
    if(fileMode() == QFileDialog::Directory || fileMode() == QFileDialog::DirectoryOnly)
    {
        getCurrentPage()->addFileDialogFiltersCondition(mimeTypeFilterList, filterList, QDir::Dirs, Qt::CaseInsensitive);
    }
    else
        getCurrentPage()->addFileDialogFiltersCondition(mimeTypeFilterList, filterList, filter(), Qt::CaseInsensitive);
//    if(m_fileDialogHelper->isViewInitialFinished())
    {
        refreshContainerSort();
        refreshCompleter();
    }
    pDebug << "addFileDialogFiltersCondition 11111111111111" << getCurrentSelections().length();
    selectionChanged();
}

int KyNativeFileDialog::selectNameFilterIndex() const
{
    const QComboBox *box = mKyFileDialogUi->m_fileTypeCombo;
    return box ? box->currentIndex() : -1;
}

QDir::Filters KyNativeFileDialog::filter()
{
    Q_D(KyNativeFileDialog);
    return d->filters;
}

void KyNativeFileDialog::setFilter(QDir::Filters filters)
{
    Q_D(KyNativeFileDialog);
    d->filters = filters;
}

void KyNativeFileDialog::setAcceptMode(QFileDialog::AcceptMode mode)
{
    Q_D(KyNativeFileDialog);
    d->acceptMode = mode;
    updateAcceptButtonState();
    if (mode == QFileDialog::AcceptOpen) {
        mKyFileDialogUi->m_fileNameLabel->setText(tr("Name"));//文件名
        mKyFileDialogUi->m_fileNameLabel->setAlignment(Qt::AlignCenter);
        mKyFileDialogUi->m_newFolderButton->hide();
        mKyFileDialogUi->m_acceptButton->setText(tr("Open"));
        mKyFileDialogUi->m_rejectButton->setText(tr("Cancel"));
        connect(mKyFileDialogUi->m_fileNameEdit , &QLineEdit::textChanged, this, &KyNativeFileDialog::onCurrentInputNameChanged);

    } else {
        mKyFileDialogUi->m_fileNameLabel->setText(tr("Save as"));
        mKyFileDialogUi->m_fileNameLabel->setAlignment(Qt::AlignCenter);
        mKyFileDialogUi->m_newFolderButton->setText(tr("New Folder"));
        mKyFileDialogUi->m_newFolderButton->show();
        mKyFileDialogUi->m_acceptButton->setText(tr("Save"));
        mKyFileDialogUi->m_rejectButton->setText(tr("Cancel"));
        mKyFileDialogUi->m_acceptButton->setDefault(true);
        connect(mKyFileDialogUi->m_fileNameEdit, &QLineEdit::textChanged, this, &KyNativeFileDialog::onCurrentInputNameChanged);
        connect(mKyFileDialogUi->m_newFolderButton, &QPushButton::clicked, this, &KyNativeFileDialog::onNewFolder);
    }
}

QFileDialog::AcceptMode KyNativeFileDialog::acceptMode() const
{
    Q_D(const KyNativeFileDialog);
    return d->acceptMode;
}

void KyNativeFileDialog::setFileMode(QFileDialog::FileMode mode)
{
    Q_D(KyNativeFileDialog);
//    if (d->fileMode == QFileDialog::DirectoryOnly || d->fileMode == QFileDialog::Directory) {
//        mKyFileDialog->setNameFilters(QStringList());
//    }
    if(mode == QFileDialog::ExistingFile || mode == QFileDialog::AnyFile ||
       mode == QFileDialog::DirectoryOnly)
    {
        pDebug << "setFileMode000................" << mode;

        getCurrentPage()->setSelectionMode(QAbstractItemView::SingleSelection);
    }
    else
    {
        pDebug << "setFileMode00011111................" << mode;

        getCurrentPage()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

    if (mode == QFileDialog::DirectoryOnly || mode == QFileDialog::Directory) {
        pDebug << "m_fileDialogHelper options setNameFilters....";
        //m_fileDialogHelper->options()->setNameFilters(QStringList() << tr("Directories"));
        setNameFilters(QStringList() << tr("Directories"));
        mKyFileDialogUi->m_fileTypeCombo->clear();
        mKyFileDialogUi->m_fileTypeCombo->addItem(tr("Directories"));
        mKyFileDialogUi->m_fileTypeCombo->setEnabled(false);
    }

    d->fileMode = mode;
    updateAcceptButtonState();
}

QFileDialog::FileMode KyNativeFileDialog::fileMode()
{
    Q_D(KyNativeFileDialog);
    return d->fileMode;
}

void KyNativeFileDialog::setViewMode(ViewMode mode)
{
    if(containerView()){
        pDebug << "setViewMode...." << mode << containerView()->viewId();
        if(mode == ViewMode::List && containerView()->viewId() != "List View")
        {
            this->getCurrentPage()->switchViewType("List View");
        }
        else if(mode == ViewMode::Icon && containerView()->viewId() != "Icon View")
            this->getCurrentPage()->switchViewType("Icon View");
        onSwitchView();
    }
}

KyNativeFileDialog::ViewMode KyNativeFileDialog::viewMode() const
{
//    this->getCurrentPage()->switchViewType("List View");

    if(containerView() && containerView()->viewId() == "List View")
        return ViewMode::List;
    return  ViewMode::Icon;
}

void KyNativeFileDialog::setLabelText(QFileDialog::DialogLabel label, const QString &text)
{
    switch (static_cast<int>(label)) {
    case QFileDialog::Accept:
        mKyFileDialogUi->m_acceptButton->setText(text);
        break;
    case QFileDialog::Reject:
        mKyFileDialogUi->m_rejectButton->setText(text);
        break;
    case QFileDialog::FileName:
        mKyFileDialogUi->m_fileNameEdit->setText(text);
        break;
    default:
        break;
    }
}

QString KyNativeFileDialog::labelText(QFileDialog::DialogLabel label) const
{
    switch (static_cast<int>(label)) {
    case QFileDialog::Accept:
        return mKyFileDialogUi->m_acceptButton->text();
    case QFileDialog::Reject:
        return mKyFileDialogUi->m_rejectButton->text();
    default:
        break;
    }

    return QString();
}

void KyNativeFileDialog::setOptions(QFileDialog::Options options)
{
    Q_D(KyNativeFileDialog);
    d->options = options;
    if (options.testFlag(QFileDialog::ShowDirsOnly)) {
        d->m_container->setFilterLabelConditions(selectedNameFilter());
    }
}

void KyNativeFileDialog::setOption(QFileDialog::Option option, bool on)
{
    Q_D(KyNativeFileDialog);
    QFileDialog::Options options = d->options;
    if (on) {
        options |= option;
    } else {
        options &= ~option;
    }
    setOptions(options);
}

bool KyNativeFileDialog::testOption(QFileDialog::Option option) const
{
    Q_D(const KyNativeFileDialog);
    return d->options.testFlag(option);
}

QFileDialog::Options KyNativeFileDialog::options() const
{
    Q_D(const KyNativeFileDialog);
    return d->options;
}

void KyNativeFileDialog::setCurrentInputName(const QString &name)
{
    m_copyEditText = name;
//    mKyFileDialogUi->m_fileNameEdit->setText(name);
    QMimeDatabase db;
    const QString &suffix = db.suffixForFileName(name);
    if (suffix.isEmpty()) {
        mKyFileDialogUi->m_fileNameEdit->selectAll();
    } else {
        mKyFileDialogUi->m_fileNameEdit->setSelection(0, name.length() - suffix.length() - 1);
    }
}

bool checkNameIsValid(QString name)
{
    QRegExp qInvalidName("[\\\\/:\\*\\?\\\"<>\\|\\n\\t]");/* 文件名或文件夹名中不能出现以下字符：\、/、:、*、?、"、<、>、| 、\n 、\t */
    if (nullptr == name) {
        return false;
    }

    return !name.contains(qInvalidName);
}

void KyNativeFileDialog::onAcceptButtonClicked()
{
    Q_D(KyNativeFileDialog);
    if (!getCurrentPage()) {
        return;
    }
    pDebug << "onAcceptButtonClicked:" << d->acceptMode << d->fileMode;
    pDebug << "onAcceptButtonClicked000 sfiles..........:" << getCurrentSelectionsList();
    pDebug << "onAcceptButtonClicked111 sfiles..........:" << selectedFiles();

    if(d->fileMode == QFileDialog::ExistingFile || d->fileMode == QFileDialog::ExistingFiles || d->fileMode == QFileDialog::AnyFile)
    {
        QList<QString> sFiles = selectedFiles();
        for (int i = 0; i < sFiles.length(); ++i) {
            if(isDir(sFiles[i]))
            {
                pDebug << "gotouri666666" << "file://" + sFiles[i];
                goToUri(sFiles[i], true);
                return;
            }
        }
    }

    if(copyEditText() != "")
    {
        lineEditTextChange(copyEditText());
    }
    pDebug << "onAcceptButtonClicked directoryUrl:" << directoryUrl() << directory();
    QList<QString> sFiles = selectedFiles();
    pDebug << "onAcceptButtonClicked sfiles..........:" << sFiles;
//        pDebug << "getSelections........:" << containerView()->getSelections();
//        pDebug << "uri:" << Peony::FileUtils::getTargetUri(getCurrentUri());
//        pDebug << "uri1111111111:" << Peony::FileUtils::getParentUri(getCurrentUri());
    if(!m_searchMode)
    {
        pDebug << "onAcceptButtonClicked getCurrentUri........:" << getCurrentUri();
        if(!Peony::FileUtils::isFileExsit(getCurrentUri())){
            pDebug << directory().path() << "getCurrentUri directory not exists" << getCurrentUri();
        return;
        }
    }
    else
    {
        foreach (QString str, sFiles) {
                if(!Peony::FileUtils::isFileExsit(str))//
                {
                    pDebug << "FileUtils path,,,:" << str << "not exists!";
                    return;
                }
        }
    }


    if (d->acceptMode != QFileDialog::AcceptOpen &&
            (copyEditText() == "/" || copyEditText() == "." || !checkNameIsValid(copyEditText())))
    {
        QString msg = tr("Invalid file name %1%2%3 .").arg("\“").arg(copyEditText()).arg("\”");
        QMessageBox::warning(this, tr("Warning"), msg);
        return;
    }
    
    if (d->acceptMode == QFileDialog::AcceptSave)
    {
        if(!doSave(sFiles))
        {
            pDebug << "da save faile!";
            return;
        }
        else
            pDebug << "da save success!";

    }
    else if(d->acceptMode == QFileDialog::AcceptOpen)
    {
        if(!doOpen(sFiles))
        {
            pDebug << "da open faile!";
            return;
        }
        else
            pDebug << "da open success!";
    }

    accept();
//    Q_EMIT /*m_fileDialogHelper*/->accept();
}

bool KyNativeFileDialog::saveMessageBox(QString name)
{
    m_RecodPath = "";
    if(QMessageBox::warning(this, tr("Warning"), name + " " + tr("exist, are you sure replace?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        return true;
    }
    return false;
}

bool KyNativeFileDialog::checkSaveFileExsits(QString path, QString fileName)
{
    bool exists = false;

    QString directoryPath = directoryUrl().toString();
    if (directoryPath.startsWith("trash://") || directoryPath.startsWith("recent://")
            || directoryPath.startsWith("computer://") || directoryPath.startsWith("favorite://")
            || directoryPath.startsWith("idm://")){
        QStringList urlStrList = getCurrentPage()->getCurrentSelections();
        QStringList pathList;
        for (auto uri : urlStrList) {
            pDebug << "checkselect....:" << Peony::FileUtils::urlDecode(uri) << QUrl(Peony::FileUtils::urlDecode(uri)) << Peony::FileUtils::getTargetUri(uri);
            pathList << Peony::FileUtils::getTargetUri(uri);
            QString str = Peony::FileUtils::getFileDisplayName(uri);
            pDebug << "str....." << str << fileName;
            if(str == fileName){
                exists = true;
                pDebug << "checkSaveFileExsits path000000...." << path << exists;

                return exists;
            }
        }
    }

    if(fileMode() == QFileDialog::Directory || fileMode() == QFileDialog::DirectoryOnly)
    {
        QDir dir(path);
        exists = dir.exists();
    }
    else{
        QFile file(path);
        exists = file.exists();
        if(!exists)
        exists = Peony::FileUtils::isFileExsit(path);
    }
    pDebug << "checkSaveFileExsits path...." << path << exists;
    return exists;
}

bool KyNativeFileDialog::doSave(QStringList sFiles)
{
    Q_D(KyNativeFileDialog);

    QString file_name = copyEditText();
    QString originName = file_name;

    pDebug << "file_name000......:" << file_name;
    bool suffixCheck = false;
    QStringList nameFilters = d->nameFilters;
    QString defaultSuffix;
    if(mKyFileDialogUi->m_fileTypeCombo->currentIndex() >= 0 &&
            mKyFileDialogUi->m_fileTypeCombo->currentIndex() < nameFilters.length())
    {//匹配当前名字是否有filters里的后缀
        QString nameFilterList = nameFilters[mKyFileDialogUi->m_fileTypeCombo->currentIndex()];
        defaultSuffix = cleanFilterList(nameFilterList).value(0);
        for (QString nameFilter : cleanFilterList(nameFilterList)) {
            pDebug << "nameFilter:" << nameFilter << "nameFilterList:" << nameFilterList;
//            if(nameFilter == "*")
//                break;
            QRegExp re(nameFilter, Qt::CaseInsensitive, QRegExp::Wildcard);
            if (re.exactMatch(file_name)) {
                pDebug << "file_name:" << file_name;
                suffixCheck = true;
            };
            if (suffixCheck) {
                break;
            }
        }
    }

    if(!suffixCheck){
        if(defaultSuffix.startsWith("*."))
            defaultSuffix.remove(0,2);
        QRegExp re(defaultSuffix, Qt::CaseInsensitive, QRegExp::Wildcard);
        if (re.exactMatch(file_name)) {
            pDebug << "file_name 1111:" << file_name;
            if(!file_name.contains("."))
                file_name.append('.' + defaultSuffix);

            if(checkLongName(directory().path(), file_name)){
                if(longNameDialog(directory().path(), file_name)){
                    onAcceptButtonClicked();
                }
                return false;
            }
            setCurrentInputName(file_name);
            pDebug << "path///:" << directory().path() + "/" + file_name;
            if(checkSaveFileExsits(directory().path() + "/" + file_name, file_name))
                if(!saveMessageBox(file_name))
                    return false;
        }
    }
    pDebug << "suffixCheck:" << suffixCheck << "nameFilters:" << nameFilters << "defaultSuffix:" << defaultSuffix;
    if (!suffixCheck && !nameFilters.isEmpty()) {
        QMimeDatabase mdb;
        if(mKyFileDialogUi->m_fileTypeCombo->currentIndex() < 0 ||
                mKyFileDialogUi->m_fileTypeCombo->currentIndex() >= nameFilters.length())
            return false;
        QString nameFilter = nameFilters[mKyFileDialogUi->m_fileTypeCombo->currentIndex()];
        QStringList newNameFilters = cleanFilterList(nameFilter);
        if (!newNameFilters.isEmpty()) {
//            if(m_fileDialogHelper->options()->defaultSuffix() == ""){

            for (const QString &filter : newNameFilters) {
                QString suffix = mdb.suffixForFileName(filter);
                if(suffix.isEmpty())
                    suffix = defaultSuffix;
                    pDebug << "suffix....." << suffix;
                    if (suffix.isEmpty()) {
                        QRegExp  regExp(filter.mid(2), Qt::CaseInsensitive, QRegExp::Wildcard);
                        mdb.allMimeTypes().first().suffixes().first();
                        for (QMimeType m : mdb.allMimeTypes()) {
                            pDebug << "m.suffixes()......" << filter << m.suffixes();
                            for (QString suffixe : m.suffixes()) {
                                if (regExp.exactMatch(suffixe)) {
                                    suffix = suffixe;
                                    break;
                                }
                            }
                            if (!suffix.isEmpty()) {
                                break;
                            }
                        }

                    }

                    pDebug << "suffix:" << suffix;
                    if (!suffix.isEmpty()) {
                        QString oldName = file_name;
                        oldName.append('.' + suffix);
                        if(checkLongName(directory().path(), oldName)){
                            if(longNameDialog(directory().path(), oldName)){
                                onAcceptButtonClicked();
                            }
                            return false;
                        }
                        setCurrentInputName(oldName);
                        pDebug << "path///:" << directory().path() + "/" + oldName;
                        if(checkSaveFileExsits(directory().path() + "/" + oldName, oldName)){
                            if(!saveMessageBox(oldName))
                                return false;
                            break;
                        }
                        if(filter == newNameFilters[newNameFilters.length() - 1])
                        {
                            QString oldName = file_name;
                            oldName.append('.' +  mdb.suffixForFileName(newNameFilters[0]));
                            if(checkLongName(directory().path(), oldName)){
                                if(longNameDialog(directory().path(), oldName)){
                                    onAcceptButtonClicked();
                                }
                                return false;
                            }

                            if(checkSaveFileExsits(directory().path() + "/" + oldName, oldName)){
                                if(!saveMessageBox(oldName))
                                    return false;
                                break;
                            }
                            setCurrentInputName(oldName);
                            return true;
                        }
                    }
//                }
            }
        }
    }
    else if(suffixCheck)
    {
        pDebug << "suffixCheck.....:" << suffixCheck << "path///:" << directory().path() + "/" + file_name;
        if(checkLongName(directory().path(), file_name)){
            if(longNameDialog(directory().path(), file_name)){
                onAcceptButtonClicked();
            }
            return false;
        }
        pDebug << "suffixCheck.....:" << suffixCheck << "path///:" << directory().path() + "/" + file_name;

        if(checkSaveFileExsits(directory().path() + "/" + file_name, file_name)){
            if(!saveMessageBox(file_name))
                return false;
        }

    }
    return true;
}

bool KyNativeFileDialog::doOpen(QStringList sFiles)
{
    Q_D(KyNativeFileDialog);

//    if(d->fileMode != QFileDialog::AnyFile && sFiles.length() == 0)
//        return false;
//    if(d->fileMode == QFileDialog::DirectoryOnly && sFiles.length() > 1)
//        return false;
    pDebug << "doOpen fileMode:" << d->fileMode;
    if(d->fileMode == QFileDialog::ExistingFile || d->fileMode == QFileDialog::ExistingFiles)
    {
        int isfiles = 0;
        int isDirctory = 0;
        foreach (QString files, sFiles)
        {
            if(isDir(files))
            {
                isDirctory++;
            }
            else
                isfiles++;
        }
        pDebug << "isfiles:" << isfiles << "isDirctory:" << isDirctory;
        if((isfiles > 0 && isDirctory > 0) || (isDirctory > 1) || (isfiles <= 0 && isDirctory <= 0))
            return false;
        else{
            pDebug << "open setSelected...";
            Q_EMIT filesSelected(selectedUrls());
        }
    }
    else if(d->fileMode == QFileDialog::AnyFile)
    {
        if(sFiles.length() > 0)
        {
            Q_EMIT filesSelected(selectedUrls());
        }
        else if(copyEditText() != "")
        {
            QList<QUrl> urlList;
            pDebug << "directory path:" << directory().path() << directoryUrl().path();
            urlList.append(QUrl(directory().path() + "/" + copyEditText()));
            Q_EMIT filesSelected(urlList);
        }
        else
            return false;
    }
    else if(d->fileMode == QFileDialog::DirectoryOnly || d->fileMode == QFileDialog::Directory)
    {
        if(copyEditText() != "")
        {
            if(sFiles.isEmpty() || (!sFiles.isEmpty() && !isDir(sFiles[0])))
            {
                return false;
            }
            else
                Q_EMIT filesSelected(selectedUrls());
        }
        else
        {
            if (!getCurrentPage()) {
                return false;
            }
            QList<QUrl> urls;
            QString uri = getCurrentPage()->getCurrentUri();
            pDebug << "current uri........:" << uri;

            if (!uri.startsWith("trash://") && !uri.startsWith("recent://") &&
                    !uri.startsWith("computer://") && !uri.startsWith("favorite://") &&
                    !uri.startsWith("filesafe://") && !uri.startsWith("mult://") &&
                    !uri.startsWith("smb://") && !uri.startsWith("ftp://") &&
                    !uri.startsWith("sftp://") && !uri.startsWith("mtp://") &&
                    !uri.startsWith("gphoto2://") && !uri.startsWith("idm://")){

                if(uri.startsWith("file:///"))
                    uri = uri.remove(0,7);
                pDebug << "open selectedFiles00000..........:" << uri;
                urls << QUrl::fromLocalFile(Peony::FileUtils::urlDecode(uri));
            }
            else
            {
                urls << convertSpecialPath(uri);
            }
            if(containerView())
                pDebug << "select uri....:" << containerView()->getSelections();

            Q_EMIT filesSelected(urls);
        }
    }
    pDebug << "onAcceptButtonClicked selectedfiles:" << sFiles;

    return true;
}


void KyNativeFileDialog::onRejectButtonClicked()
{
//   Q_EMIT m_fileDialogHelper->reject();
    reject();
}

bool KyNativeFileDialog::isDir(QString path)
{
    pDebug << "isDir 0000....." << path;
    if(path.startsWith("smb://")){
        g_autofree char* gPath = g_file_get_path(const_cast<GFile*>(g_file_new_for_uri(path.toLocal8Bit())));
        pDebug << "isDir gPath:......" << (gPath == nullptr) << gPath;

        if(gPath == nullptr){
            gPath = g_file_get_path(const_cast<GFile*>(g_file_new_for_uri(Peony::FileUtils::urlDecode(path).toLocal8Bit())));
            pDebug << "isDir gPath 1111:......" << (gPath == nullptr) << gPath;
        }
        if(gPath == nullptr){
            gPath = g_file_get_path(const_cast<GFile*>(g_file_new_for_uri(Peony::FileUtils::urlEncode(path).toLocal8Bit())));
            pDebug << "isDir gPath 222222:......" << (gPath == nullptr) << gPath;
        }
        if(gPath == nullptr)
            return true;
    }
    pDebug << "isDir 1111....." << path;

    auto fileSymLinkInfo = Peony::FileInfo::fromUri(path);
    if(fileSymLinkInfo->isSymbolLink()) {
        path = fileSymLinkInfo->symlinkTarget();
    }
    else
        path = Peony::FileUtils::urlEncode(path);

    QFile file(path);
    QFileInfo fileInfo(file);
    pDebug << "isDir path1111:" << path << fileInfo.isDir() << Peony::FileUtils::isFileDirectory(path) <<  Peony::FileUtils::getFileIsFolder(path);;
    if(fileInfo.isDir())
        return true;

    pDebug << "isDir path2222222..:" << path << Peony::FileUtils::isFileDirectory(path) <<  Peony::FileUtils::getFileIsFolder(path);
    return (Peony::FileUtils::isFileDirectory(path) || Peony::FileUtils::getFileIsFolder(path));
}

void KyNativeFileDialog::goBack()
{
    pDebug << "goback...." << getCurrentPage()->canGoBack();
    getCurrentPage()->goBack();
}

void KyNativeFileDialog::goForward()
{
    pDebug << "goForward...." << getCurrentPage()->canGoForward();
    getCurrentPage()->goForward();
}

void KyNativeFileDialog::goToParent()
{
    pDebug << "goToParent...." << getCurrentPage()->canCdUp();
    getCurrentPage()->cdUp();

}

void KyNativeFileDialog::setHelper(KyFileDialogHelper* helepr)
{
    if(helepr != nullptr)
        m_fileDialogHelper = helepr;
}

void KyNativeFileDialog::onSwitchView()
{
    pDebug << "onSwitchView containerView:" << (containerView() == nullptr);
    if(containerView() != nullptr)
    {
        QString viewId = containerView()->viewId();
        pDebug << "onSwitchView:" << viewId;
        if (viewId == "List View") {
            mKyFileDialogUi->m_modeButton->setIcon(QIcon::fromTheme("view-list-symbolic"));
            this->mKyFileDialogUi->m_listModeAction->setChecked(true);
            this->mKyFileDialogUi->m_iconModeAction->setChecked(false);
        } else {
            mKyFileDialogUi->m_modeButton->setIcon(QIcon::fromTheme("view-grid-symbolic"));
            this->mKyFileDialogUi->m_listModeAction->setChecked(false);
            this->mKyFileDialogUi->m_iconModeAction->setChecked(true);
        }
    }
}

void KyNativeFileDialog::updateWindowState()
{
    Q_D(KyNativeFileDialog);
    pDebug << "updateWindowState.............." << getCurrentUri() << directory() << directoryUrl() << mKyFileDialogUi->m_fileNameEdit->text() << copyEditText();
    //mKyFileDialogUi->m_fileNameEdit->setText("");
    pDebug << "updateWindowState m_searchMode.............." <<m_searchMode << getCurrentUri() << m_isClearSearchKey;

//    if(m_searchMode && !getCurrentUri().startsWith("search:///") && !m_isClearSearchKey)
//    {
//        m_isClearSearchKey = false;
//        searchButtonClicked();
//    }

    refreshCompleter();
//    const QStringList list = getCurrentPage()->getAllFileUris();
//    pDebug << "getAllFileUris:" << list;
//    pDebug << "getselectUris:" << getCurrentSelectionsList();

//    m_CurrentPathAllFiles.clear();

//    foreach (QString str, list) {
//        str = Peony::FileUtils::urlDecode(str);
//        m_CurrentPathAllFiles.append(str.split("/").last());
//    }

//    pDebug << "updateWindowState 1111111111111";
//    QStringListModel *model = (QStringListModel *)(m_completer->model());
//    model->setStringList(m_CurrentPathAllFiles);
//    pDebug << "updateWindowState 2222222222222";

    auto uri = getCurrentUri();
    pDebug << "updateWindowState uri updatePath....." << uri << getCurrentPage()->canGoBack();
    //mKyFileDialogUi->m_pathbarWidget->updatePath(uri);
    bool canGoBack = false;
    if(getCurrentPage()->canGoBack() && !(getCurrentPage()->getBackList().length() == 1 && getCurrentPage()->getBackList().value(0).isEmpty()))
    {
        canGoBack = true;
    }

    mKyFileDialogUi->m_backButton->setEnabled(canGoBack);

    mKyFileDialogUi->m_forwardButton->setEnabled(getCurrentPage()->canGoForward());
    bool canCdUp = getCurrentPage()->canCdUp();
    if(uri.startsWith("mult://"))
        canCdUp = false;
    if ((uri == "file:///data/usershare" || uri == "file:///data/usershare/") && Peony::FileUtils::isFileExsit("file:///data/usershare")){
        bool isDBlockHasUserFile = isDataBlockHasUserFile();
        if(!isDBlockHasUserFile) {
            canCdUp = false;
        }
    }
    mKyFileDialogUi->m_toParentButton->setEnabled(canCdUp);
    updateStatusBar();
    pDebug << "updateWindowState 44444" << copyEditText().isEmpty();
    lineEditTextChange(copyEditText());
    pDebug << "updateWindowState 55555555555555555555" << uri;

}

QString KyNativeFileDialog::selectName()
{
    QStringList selectList = getCurrentPage()->getCurrentSelections();
    QString nameStr = "";
    m_displayNameMap.clear();
    bool selectFiles = false;
    for(int i = 0; i < selectList.length(); i++)
    {
        pDebug << "parenturi....." << Peony::FileUtils::getParentUri(selectList[i]);
        pDebug << "selectName ....i:" << i << selectList[i] << Peony::FileUtils::getFileDisplayName(selectList[i]) << Peony::FileUtils::urlDecode(selectList[i]) << isDir(selectList[i]);
        QString str = Peony::FileUtils::getFileDisplayName(selectList[i]);//Peony::FileUtils::urlDecode(selectList[i]);
        m_displayNameMap.insert(Peony::FileUtils::urlDecode(selectList[i]), Peony::FileUtils::getParentUri(selectList[i]) + "/" + str);

        if(acceptMode() == QFileDialog::AcceptSave && isDir(selectList[i])){
            continue;
        }
        if(acceptMode() == QFileDialog::AcceptOpen && (fileMode() == QFileDialog::ExistingFile ||
           fileMode() == QFileDialog::ExistingFiles || fileMode() == QFileDialog::AnyFile) && isDir(selectList[i])){
            continue;
        }
        selectFiles = true;
        QString name = "";
        if(selectList.length() == 1)
            name += str.split("/").last();
        else if(i < (selectList.length() -1))
            name += "\"" + str.split("/").last() + "\" ";
        else
            name += "\"" + str.split("/").last() + "\"";
        nameStr += name;
    }
    if(selectFiles || selectList.length() == 0)
        setSelectIsDir(false);
    else
        setSelectIsDir(true);

    if(acceptMode() == QFileDialog::AcceptSave){
        for(int i = 0; i < selectList.length(); i++) {
            if(isDir(selectList[i])) {
                mKyFileDialogUi->m_acceptButton->setText(tr("Open"));
                return nameStr;
            }
        }
        mKyFileDialogUi->m_acceptButton->setText(tr("Save"));
    }
    return nameStr;
}

void KyNativeFileDialog::updateStatusBar()
{
    if(getCurrentSelectionsList().length() > 0)
    {
        foreach (QString str, getCurrentSelections()) {
            pDebug << "updateStatusBar str" << str;
            Q_EMIT currentChanged(QUrl(QUrl(str).path()));
        }
    }
    updateAcceptButtonState();
    QString nameStr = selectName();

    if(!selectIsDir() && nameStr != mKyFileDialogUi->m_fileNameEdit->text()){
        pDebug << "updateStatusBar 000000" << nameStr.isEmpty() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
        if(acceptMode() == QFileDialog::AcceptOpen){
            if(!nameStr.isEmpty())//如果这里不进行判断，那选中后再修改输入框内容会被清空；这里判断的话，选中后，鼠标点击空白处，输入框不会被清空，qt原生对话框也这样
                mKyFileDialogUi->m_fileNameEdit->setText(nameStr);
        }
        else if(!nameStr.isEmpty())
            mKyFileDialogUi->m_fileNameEdit->setText(nameStr);
    }
    setSelectIsDir(false);
}

void KyNativeFileDialog::containerMenuRequest(const QPoint &pos)
{
    pDebug << "menuRequest....00000" << pos << QCursor::pos() << (this->getCurrentPage() == nullptr);
    if(this->getCurrentPage())
        pDebug << "menuRequest....1111111" << (this->getCurrentPage()->getView() == nullptr);
    Peony::DirectoryViewMenu menu(this, this);
    QStringList hideActionsName;
    hideActionsName.append(OPEN_IN_NEW_WINDOW_ACTION);
    hideActionsName.append(OPEN_IN_NEW_TAB_ACTION);
    pDebug << "hideActionsName...." << hideActionsName;
    menu.setHiddenActionsByObjectName(hideActionsName);
    pDebug << "menuRequest....11111";
    menu.exec(QCursor::pos());
    m_uris_to_edit = menu.urisToEdit();
}

void KyNativeFileDialog::delayShow()
{
    Q_D(const KyNativeFileDialog);
    //QTBUG48248 借鉴kde plasma-integration里用的QTimer
    pDebug << "delayShow start..........." << getCurrentUri();
    d->m_timer->start();
}

void KyNativeFileDialog::discardDelayedShow()
{
    Q_D(const KyNativeFileDialog);
    pDebug << "discardDelayedShow stop...........";
    d->m_timer->stop();
}


void KyNativeFileDialog::resizeEvent(QResizeEvent *e)
{
    pDebug << "resizeEvent....." << this->width() << mKyFileDialogUi->m_siderWidget->width() << mKyFileDialogUi->m_container->width();
    QDialog::resizeEvent(e);
}

void KyNativeFileDialog::paintEvent(QPaintEvent *e)
{
    QColor color = this->palette().base().color();
    QColor colorBase = QColor(this->palette().base().color());

    int R1 = color.red();
    int G1 = color.green();
    int B1 = color.blue();
    qreal a1 = 0.3;

    int R2 = colorBase.red();
    int G2 = colorBase.green();
    int B2 = colorBase.blue();
    qreal a2 = 1;

    qreal a = 1 - (1 - a1)*(1 - a2);

    qreal R = (a1*R1 + (1 - a1)*a2*R2) / a;
    qreal G = (a1*G1 + (1 - a1)*a2*G2) / a;
    qreal B = (a1*B1 + (1 - a1)*a2*B2) / a;

    colorBase.setRed(R);
    colorBase.setGreen(G);
    colorBase.setBlue(B);
    auto sidebarOpacity = Peony::GlobalSettings::getInstance()->getValue(SIDEBAR_BG_OPACITY).toInt();

    if(sidebarOpacity != 100)
        mKyFileDialogUi->m_sider->viewport()->setAttribute(Qt::WA_TranslucentBackground);

    colorBase.setAlphaF(this->isActiveWindow() ? sidebarOpacity/100.0 : 1.0);

    QPainterPath sidebarPath;
    QRect sideBarRect = QRect(mKyFileDialogUi->m_sider->mapTo(this, QPoint()), mKyFileDialogUi->m_sider->size());
    auto adjustedRect = sideBarRect.adjusted(0, 0, 0, 0);
    int radius = 6;
    if(this && this->property("normalRadius").isValid())
        radius = this->property("normalRadius").toInt();
    sidebarPath.addRoundedRect(adjustedRect, radius, radius);

//    sidebarPath.addRect(adjustedRect.adjusted(0, 0, 0, -6));
//    sidebarPath.addRect(adjustedRect.adjusted(6, 0, 0, 0));
    QPainter p(this);
    QPainterPath contentPath;


    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings =  new QGSettings("org.ukui.style");
        if(settings->keys().contains("windowRadius"))
            radius = settings->get("windowRadius").toInt();
        else if(this && this->property("maxRadius").isValid())
            radius = this->property("maxRadius").toInt();
    }
    else if(this && this->property("maxRadius").isValid())
        radius = this->property("maxRadius").toInt();

//    qDebug() << "KyNativeFileDialog radius........" << radius;
    contentPath.addRoundedRect(this->rect(), radius, radius);
    auto bgPath = contentPath - sidebarPath;
    p.fillPath(bgPath, color);
    p.fillPath(sidebarPath, colorBase);

    QDialog::paintEvent(e);
}

void KyNativeFileDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
    {
        auto uri = Peony::FileUtils::getParentUri(getCurrentUri());
        //qDebug() << "goUp Action" << getCurrentUri() << uri;
        if (uri.isNull())
            return;
        goToUri(uri, true, true);
    }

    return QDialog::keyPressEvent(e);
}

bool KyNativeFileDialog::event(QEvent *e){
    if(e->type() == QEvent::ActivationChange){
        m_windowHelper->setBlurEffect(QRegion(), 0, this->isActiveWindow());
    }
    return QDialog::event(e);
}


Peony::DirectoryViewContainer *KyNativeFileDialog::getCurrentPage()
{
    Q_D(const KyNativeFileDialog);
    return d->m_container;
}

Peony::DirectoryViewContainer *KyNativeFileDialog::getCurrentPage() const
{
    Q_D(const KyNativeFileDialog);
    return d->m_container;
}

Peony::DirectoryViewWidget *KyNativeFileDialog::containerView() const
{
    return getCurrentPage()->getView();
}

void KyNativeFileDialog::updateAcceptButtonState()
{
    Q_D(const KyNativeFileDialog);

    pDebug << "updateAcceptButtonState......" << getCurrentUri() << d->acceptMode << d->fileMode;
    if(d->acceptMode == QFileDialog::AcceptOpen)
    {
        if(d->fileMode != QFileDialog::Directory && d->fileMode != QFileDialog::DirectoryOnly)
        {
            mKyFileDialogUi->m_acceptButton->setEnabled(!(getCurrentSelectionsList().length() == 0));
            return;
            bool isSelectFile = false;
            if(copyEditText() == "")
                mKyFileDialogUi->m_acceptButton->setEnabled(!(getCurrentSelectionsList().length() == 0));

            if(copyEditText() != "")
            {
                QString text = copyEditText();
                QStringList list = text.split("\"");
                pDebug << "list234565432............" << list;
                for (int i = list.length() - 1; i >= 0; i--) {
                    if(list[i] == "" || list[i] == " ")
                        list.removeAt(i);
                }
                pDebug << "list testtttttt..............:" << list;

                foreach (QString str, list) {
                    if(str.length() > 0 && str[0] == "\"")
                        str = str.remove(0, 1);
                    if(str.length() > 0 && str[str.length() - 1] == "\"")
                        str = str.remove(str.length() - 1, 1);
                    QString parentPath = getCurrentUri();

                    if(parentPath.endsWith("/"))
                        parentPath.remove(parentPath.length() - 1, 1);
                    pDebug << "updateAcceptButtonState str0000:" << (parentPath + "/" + str);//parentPath.startsWith("recent://")

                    if (!parentPath.startsWith("trash://") && !parentPath.startsWith("recent://") &&
                            !parentPath.startsWith("computer://") && !parentPath.startsWith("favorite://") &&
                            !parentPath.startsWith("filesafe://") && !parentPath.startsWith("mult://") &&
                            !parentPath.startsWith("smb://") && !parentPath.startsWith("ftp://") &&
                            !parentPath.startsWith("sftp://") && !parentPath.startsWith("mtp://") &&
                            !parentPath.startsWith("gphoto2://") && !parentPath.startsWith("idm://")){

                        QString targetPath = Peony::FileUtils::urlDecode(parentPath + "/" + str);
                        pDebug << "targetPath11111 uri....." << targetPath << Peony::FileUtils::isFileExsit(targetPath);

                        QString targetPath1 = Peony::FileUtils::urlEncode(parentPath + "/" + str);
                        pDebug << "targetPath123432 uri....." << targetPath1 << Peony::FileUtils::isFileExsit(targetPath1);

                        if(Peony::FileUtils::isFileExsit(parentPath + "/" + str))
                            isSelectFile = true;
                        pDebug << "isSelectFileeeeeee22222...." << isSelectFile;
                        break;
                    }
                    else
                    {
                        QString targetPath = convertSpecialPath(getCurrentSelections()[0]);
                        if(Peony::FileUtils::isFileExsit(targetPath))
                        {
                            isSelectFile = true;
                            pDebug << "isSelectFileeeeeeee3333333333333...." << isSelectFile;
                            break;
                        }
                    }
                }
                mKyFileDialogUi->m_acceptButton->setEnabled(isSelectFile);
                return;
            }
        }
        else
        {
            bool isSelectFile = true;
            if(copyEditText() != "")
            {
                QString text = copyEditText();
                QStringList list = text.split("\"");
                pDebug << "list0000............" << list;
                for (int i = list.length() - 1; i >= 0; i--) {
                    if(list[i] == "" || list[i] == " ")
                        list.removeAt(i);
                }
                pDebug << "list1111..............:" << list;

                foreach (QString str, list) {
                    pDebug << "strrrrrrrrr....." << str;
                    if(str.length() > 0 && str[0] == "\"")
                        str = str.remove(0, 1);
                    if(str.length() > 0 && str[str.length() - 1] == "\"")
                        str = str.remove(str.length() - 1, 1);
                    QString parentPath = directory().path();//getCurrentUri();

                    pDebug << "parentPath......" << parentPath;
                    if(parentPath.endsWith("/"))
                        parentPath.remove(parentPath.length() - 1, 1);
                    pDebug << "updateAcceptButtonState str,,,,,,:" <<  (parentPath + "/" + str) << Peony::FileUtils::isFileExsit(parentPath + "/" + str);

                    const QStringList allUriList = getCurrentPage()->getAllFileUris();
                    pDebug << "allUriList..." << allUriList.length();
                    if(allUriList.length() <= 0)
                        isSelectFile = false;
                    else{
                        foreach (QString urlStr, allUriList) {
                            QString displayStr = Peony::FileUtils::getFileDisplayName(urlStr);
                            pDebug << "displayStr:" << displayStr << "str:" << str << "urlStr：" << Peony::FileUtils::urlDecode(urlStr);

                            if(displayStr == str && !str.isEmpty()){
                                QString path = Peony::FileUtils::urlDecode(urlStr);
                                pDebug << "00000path..........." << path;
                                auto fileSymLinkInfo = Peony::FileInfo::fromUri(path);
                                if(fileSymLinkInfo->isSymbolLink()) {
                                    path = fileSymLinkInfo->symlinkTarget();
                                }
                                else
                                    path = Peony::FileUtils::urlEncode(path);

                                QFile file(path);
                                QFileInfo fileInfo(file);
                                pDebug << "00000isDir path1111:" << path << fileInfo.isDir() << Peony::FileUtils::isFileDirectory(path) <<  Peony::FileUtils::getFileIsFolder(path);;
                                if(fileInfo.isDir() || Peony::FileUtils::isFileDirectory(path) || Peony::FileUtils::getFileIsFolder(path)){
                                    isSelectFile = true;
                                    break;
                                }
                            }
                            if(urlStr == allUriList[allUriList.length() - 1]){
                                pDebug << "urlStr...." << urlStr;
                                isSelectFile = false;
                            }
                        }
                    }

//                    if(isDir(parentPath + "/" + str))
//                    {
//                        pDebug << "isSelectFile000000...:" << isSelectFile << (parentPath + "/" + str);
//                        isSelectFile = true;
//                        break;
//                    }
//                    else
//                    {
//                        isSelectFile = false;
//                    }
                }
            }
            pDebug << "isSelectFile...." << isSelectFile;

            QString path = directoryUrl().toString();
            if(path.startsWith("mult://") || path.startsWith("smb://") ||
                    path.startsWith("ftp://") || path.startsWith("sftp://") ||
                    path.startsWith("mtp://") || path.startsWith("gphoto2://")){
                g_autofree char* gPath = nullptr;
                gPath = g_file_get_path(const_cast<GFile*>(g_file_new_for_uri(getCurrentUri().toLocal8Bit())));
                pDebug << "updateAcceptButtonState open....gpath.....:" << (gPath == nullptr) << gPath;
                if(gPath == nullptr)
                    isSelectFile = false;
            }

            mKyFileDialogUi->m_acceptButton->setEnabled(isSelectFile);
        }
    }
    else
    {
        //TODO 目录权限判断


        QString path = directoryUrl().toString();
        pDebug << "updateAcceptButtonState....path:" << path;
        if ((path == "trash:///" || path == "recent:///"
                || path == "computer:///" || path == "favorite:///"
             || path == "idm:///")){
            mKyFileDialogUi->m_newFolderButton->setEnabled(false);
            if(getCurrentSelectionsList().isEmpty()){
                mKyFileDialogUi->m_acceptButton->setEnabled(false);
                return;
            }
        }
        else if(path.startsWith("mult://") || path.startsWith("smb://") ||
                path.startsWith("ftp://") || path.startsWith("sftp://") ||
                path.startsWith("mtp://") || path.startsWith("gphoto2://")){
            g_autofree char* gPath = nullptr;
            gPath = g_file_get_path(const_cast<GFile*>(g_file_new_for_uri(getCurrentUri().toLocal8Bit())));
            pDebug << "updateAcceptButtonState save....gpath.....:" << (gPath == nullptr) << gPath;
            if(gPath == nullptr){
                mKyFileDialogUi->m_newFolderButton->setEnabled(false);
                bool enable = false;
                QStringList selectList = getCurrentPage()->getCurrentSelections();
                for(int i = 0; i < selectList.length(); i++) {
                    if(isDir(selectList[i])) {
                        pDebug << "selectList dir 00000000000" << selectList[i] << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
                        enable = true;
                    }
                }
                mKyFileDialogUi->m_acceptButton->setEnabled(enable);
                return;
            }
            else if(gPath[0] == '\0')
            {
                bool enable = false;
                QStringList selectList = getCurrentPage()->getCurrentSelections();
                for(int i = 0; i < selectList.length(); i++) {
                    if(isDir(selectList[i])) {
                        pDebug << "selectList dir 00000000000" << selectList[i] << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
                        enable = true;
                    }
                }
                pDebug << "enable......" << enable;
                mKyFileDialogUi->m_newFolderButton->setEnabled(false);
                mKyFileDialogUi->m_acceptButton->setEnabled(enable);
                pDebug << "gpath is empty.......";
                return;
            }
//            else{
//                pDebug << "updateAcceptButtonState path no exsit....." << Peony::FileUtils::isFileDirectory(path) << Peony::FileUtils::getFileIsFolder(path) <<
//                            Peony::FileUtils::isFileDirectory(gPath) << Peony::FileUtils::getFileIsFolder(gPath);

//                if(!Peony::FileUtils::isFileDirectory(path) && !Peony::FileUtils::getFileIsFolder(path) &&
//                        !Peony::FileUtils::isFileDirectory(gPath) && !Peony::FileUtils::getFileIsFolder(gPath)){
//                    mKyFileDialogUi->m_newFolderButton->setEnabled(false);
//                    mKyFileDialogUi->m_acceptButton->setEnabled(false);
//                    return;
//                }
//            }
        }

        if(getCurrentSelectionsList().length() != 0){
            mKyFileDialogUi->m_acceptButton->setEnabled(true);
            return;
        }
        if(copyEditText() == "")
            mKyFileDialogUi->m_acceptButton->setEnabled(false);
        else
            mKyFileDialogUi->m_acceptButton->setEnabled(true);
        mKyFileDialogUi->m_newFolderButton->setEnabled(true);

        pDebug << "isenabled......:" << mKyFileDialogUi->m_acceptButton->isEnabled();
    }
    pDebug << "acceptButton isEnabled:" << mKyFileDialogUi->m_acceptButton->isEnabled();
}

void KyNativeFileDialog::onCurrentInputNameChanged()
{
    Q_D(KyNativeFileDialog);
    d->currentInputName = copyEditText();
    updateAcceptButtonState();
}

void KyNativeFileDialog::onNewFolder()
{
    Peony::CreateTemplateOperation op(getCurrentUri(), Peony::CreateTemplateOperation::EmptyFolder);
    op.run();
    return;
    QDir dir = directory();
    pDebug << "onNewFolder getCurrentUri:" << getCurrentUri() << directory() << directoryUrl();
    QString path = "";
    if(directory().path() != "/")
        path = directory().path();
    path += "/" + tr("NewFolder");
    if(!dir.exists(path))
    {
        bool newFolder = dir.mkdir(path);
        pDebug << "newFolder:" << newFolder;
    }
}

void KyNativeFileDialog::setSortType()
{
    if(getCurrentPage()->getSortOrder() ==  Qt::AscendingOrder)
    {
        mKyFileDialogUi->m_sortButton->setIcon(QIcon::fromTheme("view-sort-descending-symbolic"));
        mKyFileDialogUi->m_descending->setChecked(true);
        mKyFileDialogUi->m_ascending->setChecked(false);
    }
    else
    {
        mKyFileDialogUi->m_sortButton->setIcon(QIcon::fromTheme("view-sort-ascending-symbolic"));
        mKyFileDialogUi->m_ascending->setChecked(true);
        mKyFileDialogUi->m_descending->setChecked(false);
    }
}

void KyNativeFileDialog::intiContainerSort()
{
    auto settings = Peony::GlobalSettings::getInstance();
    auto sortType = settings->isExist(SORT_COLUMN)? settings->getValue(SORT_COLUMN).toInt() : 0;
    auto sortOrder = settings->isExist(SORT_ORDER)? settings->getValue(SORT_ORDER).toInt() : 0;
    pDebug << "sortType:" << sortType << sortOrder;
    getCurrentPage()->setSortType(Peony::FileItemModel::ColumnType(sortType));
    getCurrentPage()->setSortOrder(Qt::SortOrder(sortOrder));
    setSortType();
    setShowHidden(getWindowShowHidden());
}

void KyNativeFileDialog::refreshContainerSort()
{
    intiContainerSort();
    setUseDefaultNameSortOrder(getWindowUseDefaultNameSortOrder());
    setSortFolderFirst(getWindowSortFolderFirst());
}

void KyNativeFileDialog::refreshCompleter()
{
    const QStringList list = getCurrentPage()->getAllFileUris();
    pDebug << "getAllFileUris:" << list;
    pDebug << "getselectUris:" << getCurrentSelectionsList();

    m_CurrentPathAllFiles.clear();

    foreach (QString str, list) {
        str = Peony::FileUtils::getFileDisplayName(str);
        m_CurrentPathAllFiles.append(str);
    }

    pDebug << "updateWindowState 1111111111111";
    QStringListModel *model = (QStringListModel *)(m_completer->model());
    model->setStringList(m_CurrentPathAllFiles);
    pDebug << "updateWindowState 2222222222222";
}

void KyNativeFileDialog::searchButtonClicked()
{
    m_searchMode = ! m_searchMode;
    if(m_searchMode)
    {
        mKyFileDialogUi->m_fileNameEdit->setText("");
        mKyFileDialogUi->m_fileNameEdit->setReadOnly(true);
        setCurrentSelections(QStringList());

        m_lastSearchPath = getCurrentUri();
        pDebug << "m_lastSearchPath....." << m_lastSearchPath << getCurrentUri() << getCurrentPage()->getCurrentUri();
    }
    else
    {
        mKyFileDialogUi->m_fileNameEdit->setReadOnly(false);
    }
    pDebug << "searchButtonClicked searchButtonClicked" <<m_searchMode;
    //Q_EMIT this->updateSearchRequest(m_search_mode);
    setSearchMode(m_searchMode);
}

void KyNativeFileDialog::setSearchMode(bool mode)
{
    mKyFileDialogUi->m_searchBtn->setCheckable(mode);
    mKyFileDialogUi->m_searchBtn->setChecked(mode);
    mKyFileDialogUi->m_searchBtn->setDown(mode);
    mKyFileDialogUi->m_pathbar->switchEditMode(mode);
}

void KyNativeFileDialog::lineEditTextChange(QString text)
{
    m_copyEditText = text;
    if(m_searchMode)
        return;
    QString url;
    if(directory().path() != "/")
        url = QUrl(getCurrentUri()).path();//directoryUrl().toString();//getCurrentUri()
    pDebug << "mKyFileDialogUi->m_fileNameEdit......:" << getCurrentUri() << url << text;
    pDebug << "listview55555555555555.........." << m_CurrentPathAllFiles.length();
    QString parentPath = getCurrentUri();

    if(parentPath.endsWith("/"))
        parentPath.remove(parentPath.length() - 1, 1);
    if(url.endsWith("/"))
        url.remove(url.length() - 1, 1);

    pDebug << "parentPath:" << parentPath;
    pDebug << "getCurrentSelections......:" << getCurrentSelectionsList() << text;

    if(!text.contains("\""))//输入框中只有1个文件
    {
        if ((parentPath.startsWith("trash://") || parentPath.startsWith("recent://")
             || parentPath.startsWith("computer://") || parentPath.startsWith("favorite://")
             || parentPath.startsWith("filesafe://") || parentPath.startsWith("mult://")
             || parentPath.startsWith("smb://") || parentPath.startsWith("ftp://")
             || parentPath.startsWith("sftp://") || parentPath.startsWith("mtp://")
             || parentPath.startsWith("gphoto2://") || parentPath.startsWith("idm://"))
                && getCurrentSelections().length() > 0)
        {
            for(int i = 0; i < getCurrentSelections().length(); i++)
            {
                QString targetUri = Peony::FileUtils::getTargetUri(getCurrentSelections()[i]);
                Peony::FileInfo fileInfo(targetUri);
                QString displayname = fileInfo.displayName();
                pDebug << "text......:" << text << "displayName:" << displayname;
                if(displayname.isEmpty())
                    displayname = Peony::FileUtils::getFileDisplayName(getCurrentSelections()[i]);
                pDebug << "display name......." << displayname;
                if(displayname == text){
                    updateAcceptButtonState();
                    return;
                }
            }
            QString targetUri = Peony::FileUtils::getTargetUri(getCurrentSelections()[0]);
            QString targetPath = Peony::FileUtils::urlDecode(targetUri);
            pDebug << "getTargetUri uri....." << targetUri;
            pDebug << "uriiiiii...." << targetPath;

            targetPath = convertSpecialPath(getCurrentSelections()[0]);
            if(Peony::FileUtils::isFileExsit(targetPath))
            {
                Peony::FileInfo fileInfo(targetPath);
                QString displayname = fileInfo.displayName();
                pDebug << "text......:" << text << "displayName:" << displayname;
                if(displayname.isEmpty())
                    displayname = Peony::FileUtils::getFileDisplayName(targetPath);
                pDebug << "display name......." << displayname;
                if(displayname == text){
                    QStringList selectList;
                    selectList.append(getCurrentSelections()[0]);
                    pDebug << "selectList...000000" << selectList;
                    setCurrentSelections(selectList);
                    updateAcceptButtonState();
                    return;
                }
            }
            setCurrentSelections(QStringList());
            updateAcceptButtonState();
            return;

        }
        else
        {
            pDebug << "Peony::FileUtils::isFileExsit123:" << (parentPath + "/" + text) << Peony::FileUtils::isFileExsit(parentPath + "/" + text);
            //        pDebug << "Peony::FileUtils::isFileExsit:" << (parentPath + "/" + text) << Peony::FileUtils::isFileExsit(parentPath + "/" + text);

            if(!text.isEmpty())
            {
                for(int i = 0; i < getCurrentPage()->getCurrentSelections().length(); i++)
                {
                    if(Peony::FileUtils::getFileDisplayName(getCurrentPage()->getCurrentSelections()[i]) == text){
                        updateAcceptButtonState();
                        return;
                    }
                }
            }

//            if(Peony::FileUtils::isFileExsit(parentPath + "/" + text)){
//                pDebug << "file exists....";
//                //m_container选择指定文件
//                QStringList selectList;
//                selectList.append(parentPath + "/" + text);
//                pDebug << "selectList...1111111" << selectList;
//                setCurrentSelections(selectList);
//                if(containerView())
//                    pDebug << "selectList...22222222222" << containerView()->getSelections();
//                updateAcceptButtonState();
//                return;
//            }
//            else
            {
                QString path = parentPath + "/" + text;

                if(!text.isEmpty()){
                    const QStringList list = getCurrentPage()->getAllFileUris();
                    foreach (QString str, list) {
                        QString displayStr = Peony::FileUtils::getFileDisplayName(str);
                        pDebug << "displayStr......." << displayStr << text;
                        if(text == displayStr){
                            QStringList selectStr;
                            selectStr.append(Peony::FileUtils::urlDecode(str));
                            pDebug << "selectStrqqqqqqq....." << selectStr << text;
                            setCurrentSelections(selectStr);
                            updateAcceptButtonState();
                            return;
                        }
                    }

                    QString currentfilter = cleanFilterList(mKyFileDialogUi->m_fileTypeCombo->currentText()).value(0);
                        if(currentfilter.startsWith("*."))
                            currentfilter = currentfilter.right(currentfilter.length() - 2);
                        if(text.lastIndexOf(currentfilter) < 0){
                            QString filter = text.split(".").last();
                            bool shouldBreak = false;
                            for(int i = 0; i < mKyFileDialogUi->m_fileTypeCombo->count(); i++){
                                QString nameFilterList = mKyFileDialogUi->m_fileTypeCombo->itemText(i);
                                for (QString nameFilter : cleanFilterList(nameFilterList)) {
                                    pDebug << "lineEditTextChange nameFilter" << nameFilter << filter;

                                    //                            if(nameFilter == "*"){
                                    //                                mKyFileDialogUi->m_fileTypeCombo->setCurrentIndex(i);
                                    //                                shouldBreak = true;
                                    //                                break;
                                    //                            }
                                    //pDebug << "lineEditTextChange nameFilter:" << nameFilter << "nameFilterList:" << nameFilterList << i;
                                    if(nameFilter.startsWith("*."))
                                        nameFilter = nameFilter.right(nameFilter.length() - 2);
                                    if(nameFilter.isEmpty())
                                        break;
                                    QRegExp re(nameFilter, Qt::CaseInsensitive, QRegExp::Wildcard);
                                    if (re.exactMatch(filter)) {
                                        mKyFileDialogUi->m_fileTypeCombo->setCurrentIndex(i);
                                        shouldBreak = true;
                                        break;
                                    };
                                }
                                if(shouldBreak)
                                    break;
                            }
                        }
                }
                setCurrentSelections(QStringList());


                pDebug << path << text << "not exists!";
//                if(getCurrentSelectionsList().isEmpty()){
//                    setCurrentSelections(QStringList());
//                    pDebug << "setCurrentSelections empty........";
//                }
                updateAcceptButtonState();
                return;
            }
        }
    }
    else
    {
        if (parentPath.startsWith("trash://") || parentPath.startsWith("recent://")
                || parentPath.startsWith("computer://") || parentPath.startsWith("favorite://")
                || parentPath.startsWith("filesafe://") || parentPath.startsWith("mult://")
                || parentPath.startsWith("smb://") || parentPath.startsWith("ftp://")
                || parentPath.startsWith("sftp://") || parentPath.startsWith("mtp://")
                || parentPath.startsWith("gphoto2://") || parentPath.startsWith("idm://"))
        {
            QStringList list = text.split("\"");
            pDebug << "listttt1111............" << list;
            for (int i = list.length() - 1; i >= 0; i--) {
                if(list[i] == "" || list[i] == " ")
                    list.removeAt(i);
            }
            QStringList displayNameList;
            for(int i = 0; i < getCurrentSelections().length(); i++)
            {
                QString targetUri = Peony::FileUtils::getTargetUri(getCurrentSelections()[i]);
                Peony::FileInfo fileInfo(targetUri);
                QString displayname = fileInfo.displayName();
                pDebug << "text:" << text << "displayName:" << displayname;
                if(displayname.isEmpty())
                    displayname = Peony::FileUtils::getFileDisplayName(getCurrentSelections()[i]);

                pDebug << "text:" << text << "displayName:" << fileInfo.displayName();
                displayNameList.append(displayname);
            }
            pDebug << "list:" << list;
            pDebug << "displayNameList:" << displayNameList;
            for(int i = 0; i < list.length(); i++){
                if(!displayNameList.contains(list[i]))
                    break;
                else if(i == list.length() - 1){
                    updateAcceptButtonState();
                    return;
                }
            }

            QStringList selectList;
            foreach (QString str, getCurrentSelections())
            {
                QString targetUri = Peony::FileUtils::getTargetUri(str);
                QString targetPath = Peony::FileUtils::urlDecode(targetUri);
                pDebug << "getTargetUri uri0000....." << targetUri;
                pDebug << "uriiiiii00000...." << targetPath;

                targetPath = convertSpecialPath(str);
                if(Peony::FileUtils::isFileExsit(targetPath))
                {
                    Peony::FileInfo fileInfo(targetPath);
                    QString displayname = fileInfo.displayName();
                    pDebug << "text......:" << text << "displayName:" << displayname;
                    if(displayname.isEmpty())
                        displayname = Peony::FileUtils::getFileDisplayName(targetPath);
                    pDebug << "display name......." << displayname;
                    if(list.contains(displayname)){
                        selectList.append(str);
                    }

                }
            }
            pDebug << "selectList...222222222" << selectList;
            setCurrentSelections(selectList);
            updateAcceptButtonState();
            return;
        }
        else{
            QStringList oldSelectList = getCurrentPage()->getCurrentSelections();
            QStringList selectList;
            QStringList list = text.split("\"");
            pDebug << "listttt1111............" << list;
            for (int i = list.length() - 1; i >= 0; i--) {
                if(list[i] == "" || list[i] == " " || QString(list[i]).isEmpty())
                    list.removeAt(i);
            }
            pDebug << "listtttt22222222..............:" << list;

            QStringList selectDisplayNameList;
            for(int i = 0; i < oldSelectList.length(); i++)
            {
                QString displayName = Peony::FileUtils::getFileDisplayName(oldSelectList[i]);
                selectDisplayNameList.append(displayName);
            }
            pDebug << "list:" << list;
            pDebug << "compareTwoList 1111111........." << list.length() << selectDisplayNameList.length();

            if(!mKyFileDialogUi->m_fileNameEdit->hasFocus() && compareTwoList(selectDisplayNameList, list).length() == 0){
                updateAcceptButtonState();
                pDebug << "compare 0000000..........";
                return;
            }

            pDebug << "listtttttt3333333.........................";
            QStringList strList, allDisplayNameList;
            QMap<QString, QString> displayUrlMap;
            const QStringList allUriList = getCurrentPage()->getAllFileUris();

            foreach (QString str, list) {
                if(str.length() > 0 &&  str[0] == "\"")
                    str = str.remove(0, 1);
                if(str.length() > 0 && str[str.length() - 1] == "\"")
                    str = str.remove(str.length() - 1, 1);
                if(!str.isEmpty()){
                    strList.append(str);
                }
                pDebug << "str:" << str;

            }

            foreach (QString urlStr, allUriList) {
                QString displayStr = Peony::FileUtils::getFileDisplayName(urlStr);
                if(!displayStr.isEmpty()){
                    displayUrlMap.insert(displayStr, urlStr);
                    allDisplayNameList.append(displayStr);
                }
            }
            pDebug << "selectList 11111111........." << strList.length() << allDisplayNameList.length();
            QStringList displaySelectList = sameTwoList(strList, allDisplayNameList);
            for (int i = 0; i < displaySelectList.length(); i++) {
                selectList.append(displayUrlMap.value(displaySelectList[i]));
            }
            pDebug << "selectList 222222222........." << selectList.length();

            if(compareTwoList(selectList, oldSelectList).length() != 0 || compareTwoList(oldSelectList, selectList).length() != 0){
                pDebug << "compareTwoListcompareTwoListcompareTwoList.............";
                setCurrentSelections(selectList);
                pDebug << "selectList........ 4444444444444" << selectList.length();
            }
            updateAcceptButtonState();
        }
        return;
    }
}

void KyNativeFileDialog::setShortCuts()
{
    if (!m_shortcutsSet) {
        //stop loading action
        QAction *stopLoadingAction = new QAction(this);
        stopLoadingAction->setShortcut(QKeySequence(Qt::Key_Escape));
        addAction(stopLoadingAction);
        connect(stopLoadingAction, &QAction::triggered, this, &KyNativeFileDialog::forceStopLoading);

        //show hidden action
        QAction *showHiddenAction = new QAction(this);
        showHiddenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
        showHiddenAction->setCheckable(true);
        addAction(showHiddenAction);
        connect(showHiddenAction, &QAction::triggered, this, [=]() {
            //qDebug() << "show hidden";
            this->setShowHidden(!getWindowShowHidden());
        });

        auto undoAction = new QAction(QIcon::fromTheme("edit-undo-symbolic"), tr("Undo"), this);
        undoAction->setShortcut(QKeySequence::Undo);
        addAction(undoAction);
        connect(undoAction, &QAction::triggered, [=]() {
            Peony::FileOperationManager::getInstance()->undo();
        });

        auto redoAction = new QAction(QIcon::fromTheme("edit-redo-symbolic"), tr("Redo"), this);
        redoAction->setShortcut(QKeySequence::Redo);
        addAction(redoAction);
        connect(redoAction, &QAction::triggered, [=]() {
            Peony::FileOperationManager::getInstance()->redo();
        });

        //add CTRL+D for delete operation
        auto trashAction = new QAction(this);
        trashAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Delete<<QKeySequence(Qt::CTRL + Qt::Key_D));
        connect(trashAction, &QAction::triggered, [=]() {
            auto currentUri = getCurrentUri();
            if (currentUri.startsWith("search://")
                    || currentUri.startsWith("favorite://") || currentUri == "filesafe:///"
                    || currentUri.startsWith("kmre://") || currentUri.startsWith("kydroid://"))
                return;

            auto uris = this->getCurrentSelections();

            QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
            QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            QString documentPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
            QString musicPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
            QString moviesPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
            QString picturespPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            QString downloadPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            if (!uris.isEmpty() && !uris.contains(desktopUri) && !uris.contains(homeUri) && !uris.contains(documentPath) && !uris.contains(musicPath)
                    && !uris.contains(moviesPath) && !uris.contains(picturespPath) && !uris.contains(downloadPath)) {
                bool isTrash = this->getCurrentUri() == "trash:///";
                if (!isTrash) {
                    Peony::FileOperationUtils::trash(uris, true);
                } else {
                    Peony::FileOperationUtils::executeRemoveActionWithDialog(uris);
                }
            }
        });
        addAction(trashAction);

        auto deleteAction = new QAction(this);
        deleteAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::SHIFT + Qt::Key_Delete));
        addAction(deleteAction);
        connect(deleteAction, &QAction::triggered, [=]() {
            auto currentUri = getCurrentUri();
            if (currentUri.startsWith("search://") || currentUri == "filesafe:///"
                    || currentUri.startsWith("kmre://") || currentUri.startsWith("kydroid://"))
                return;

            auto uris = this->getCurrentSelections();

            QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            QString documentPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
            QString musicPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
            QString moviesPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
            QString picturespPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            QString downloadPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
            QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            if (! uris.contains(desktopUri) && !uris.contains(homeUri) && !uris.contains(documentPath) && !uris.contains(musicPath)
                    && !uris.contains(moviesPath) && !uris.contains(picturespPath) && !uris.contains(downloadPath)) {
                Peony::FileOperationUtils::executeRemoveActionWithDialog(uris);
            }
        });

        auto searchAction = new QAction(this);
        searchAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::CTRL + Qt::Key_F));
        connect(searchAction, &QAction::triggered, this, [=]() {
            searchButtonClicked();
        });
        addAction(searchAction);

        //F4 or Alt+D, change to address
        auto locationAction = new QAction(this);
        locationAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_F4<<QKeySequence(Qt::ALT + Qt::Key_D));
        connect(locationAction, &QAction::triggered, this, [=]() {
            mKyFileDialogUi->m_pathbar->startEdit();
        });
        addAction(locationAction);

        auto closeWindowAction = new QAction(this);
        closeWindowAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::ALT + Qt::Key_F4));
        connect(closeWindowAction, &QAction::triggered, this, [=]() {
            this->close();
        });
        addAction(closeWindowAction);

        auto maxAction = new QAction(this);
        maxAction->setShortcut(QKeySequence(Qt::Key_F11));
        connect(maxAction, &QAction::triggered, this, [=]() {
            //showFullScreen has some issue, change to showMaximized, fix #20043
            mKyFileDialogUi->m_pathbar->cancelEdit();
            if (!this->isMaximized()) {
                this->showMaximized();
            } else {
                this->showNormal();
            }
            updateMaximizeState();
        });
        addAction(maxAction);

        auto refreshWindowAction = new QAction(this);
        refreshWindowAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
        connect(refreshWindowAction, &QAction::triggered, this, [=]() {
            this->refresh();
        });
        addAction(refreshWindowAction);

        auto listToIconViewAction = new QAction(this);
        listToIconViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
        connect(listToIconViewAction, &QAction::triggered, this, [=]() {
            this->beginSwitchView(QString("Icon View"));
        });
        addAction(listToIconViewAction);

        auto iconToListViewAction = new QAction(this);
        iconToListViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
        connect(iconToListViewAction, &QAction::triggered, this, [=]() {
            this->beginSwitchView(QString("List View"));
        });
        addAction(iconToListViewAction);

        auto reverseSelectAction = new QAction(this);
        reverseSelectAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
        connect(reverseSelectAction, &QAction::triggered, this, [=]() {
            this->getCurrentPage()->getView()->invertSelections();
        });
        addAction(reverseSelectAction);

        auto refreshAction = new QAction(this);
        refreshAction->setShortcut(Qt::Key_F5);
        connect(refreshAction, &QAction::triggered, this, [=]() {
            this->refresh();
        });
        addAction(refreshAction);

        //select all files in view, fix bug#115442
        auto *selectAllAction = new QAction(this);
        selectAllAction->setShortcut(QKeySequence::SelectAll);
        connect(selectAllAction, &QAction::triggered, this, [=]() {
            if (this->getCurrentPage()->getView())
            {
                /// note: 通过getAllFileUris设置的全选效率过低，如果增加接口则会导致二进制兼容性问题
                /// 所以这里使用现有的反选接口实现高效的全选，这个方法在mainwindow中也有用到
                //auto allFiles = this->getCurrentPage()->getView()->getAllFileUris();
                //this->getCurrentPage()->getView()->setSelections(allFiles);
                setCurrentSelections(QStringList());
                this->getCurrentPage()->getView()->invertSelections();
            }
        });
        addAction(selectAllAction);

        //file operations
        auto *copyAction = new QAction(this);
        copyAction->setShortcut(QKeySequence::Copy);
        connect(copyAction, &QAction::triggered, [=]() {
            bool is_recent = false;
            if (!this->getCurrentSelections().isEmpty())
            {
                //                if (this->getCurrentSelections().first().startsWith("trash://", Qt::CaseInsensitive)) {
                //                    return ;
                //                }
                if (this->getCurrentSelections().first().startsWith("recent://", Qt::CaseInsensitive)) {
                    is_recent = true;
                }
                if (this->getCurrentSelections().first().startsWith("favorite://", Qt::CaseInsensitive)) {
                    return ;
                }
            }
            else
                return;

            QStringList selections;
            if (is_recent)
            {
                for(auto uri:this->getCurrentSelections())
                {
                    uri = Peony::FileUtils::getTargetUri(uri);
                    selections << uri;
                }
            }
            else{
                selections = this->getCurrentSelections();
            }

            Peony::ClipboardUtils::setClipboardFiles(selections, false);
        });
        addAction(copyAction);

        auto *pasteAction = new QAction(this);
        pasteAction->setShortcut(QKeySequence::Paste);
        connect(pasteAction, &QAction::triggered, [=]() {
            auto currentUri = getCurrentUri();
            if (currentUri.startsWith("trash://") || currentUri.startsWith("recent://")
                    || currentUri.startsWith("computer://") || currentUri.startsWith("favorite://")
                    || currentUri.startsWith("search://") || currentUri == "filesafe:///" || currentUri == "idm:///")
            {
                /* Add hint information,link to bug#107640. */
                QMessageBox::warning(this, tr("warn"), tr("This operation is not supported."));
                return;
            }
            if (Peony::ClipboardUtils::isClipboardHasFiles()) {
                //FIXME: how about duplicated copy?
                //FIXME: how to deal with a failed move?
                auto op = Peony::ClipboardUtils::pasteClipboardFiles(this->getCurrentUri());
                if (op) {
                    connect(op, &Peony::FileOperation::operationFinished, this, [=](){
                        auto opInfo = op->getOperationInfo();
                        auto targetUirs = opInfo->dests();
                        setCurrentSelectionUris(targetUirs);
                    }, Qt::BlockingQueuedConnection);
                }
                else{
                    //fix paste file in old path not update issue, link to bug#71627
                    this->getCurrentPage()->getView()->repaintView();
                }
            }
        });
        addAction(pasteAction);

        auto *cutAction = new QAction(this);
        cutAction->setShortcut(QKeySequence::Cut);
        connect(cutAction, &QAction::triggered, [=]() {
            if (!this->getCurrentSelections().isEmpty()) {
                //                if (this->getCurrentSelections().first().startsWith("trash://", Qt::CaseInsensitive)) {
                //                    return ;
                //                }
                if (this->getCurrentSelections().first().startsWith("recent://", Qt::CaseInsensitive)) {
                    return ;
                }
                if (this->getCurrentSelections().first().startsWith("favorite://", Qt::CaseInsensitive)) {
                    return ;
                }

                auto currentUri = getCurrentUri();
                if (currentUri.startsWith("search://"))
                    return;

                QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
                QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
                if (! this->getCurrentSelections().contains(desktopUri) && ! this->getCurrentSelections().contains(homeUri))
                {
                    Peony::ClipboardUtils::setClipboardFiles(this->getCurrentSelections(), true);
                    this->getCurrentPage()->getView()->repaintView();
                }
            }
        });
        addAction(cutAction);

        m_shortcutsSet = true;
    }
}

void KyNativeFileDialog::initialViewId()
{
    if(!containerView())
        return;
    int zoomLevel = containerView()->currentZoomLevel();

    auto viewId = Peony::DirectoryViewFactoryManager2::getInstance()->getDefaultViewId(zoomLevel, getCurrentUri());
    pDebug << "initialViewId viewId:" << viewId;
    beginSwitchView(viewId);
}

QString KyNativeFileDialog::copyEditText()
{
    return m_copyEditText;
}

void KyNativeFileDialog::updateSearchProgressBar()
{
    if(m_needSearch){
        pDebug << "updateSearchProgressBar...." << m_searchMode << m_needSearch;
        Q_EMIT mKyFileDialogUi->m_pathbar->updateSearchProgress(false);

        QCursor c;
        c.setShape(Qt::ArrowCursor);
        this->setCursor(c);
        if(getCurrentPage() && getCurrentPage()->getView())
            getCurrentPage()->getView()->setCursor(c);
        mKyFileDialogUi->m_sider->setCursor(c);
        mKyFileDialogUi->setCursor(c);
    }
}

void KyNativeFileDialog::installTranslate(QString locale)
{
    m_translator = new QTranslator(this);
    pDebug << "local: " << locale;
    if (m_translator->load("/usr/share/qt5-ukui-platformtheme/qt5-ukui-filedialog_" + locale)) {
        pDebug << "Load translations file success!";
        QApplication::installTranslator(m_translator);
    } else {
        pDebug << "Load translations file failed!";
    }
    m_translator0 = new QTranslator(this);
    if (m_translator0->load("/usr/share/qt5-ukui-platformtheme/qt5-ukui-platformtheme_" + locale)) {
        pDebug << "Load translations file success!";
        QApplication::installTranslator(m_translator0);
    } else {
        pDebug << "Load translations file failed!";
    }

    m_translator1 = new QTranslator(this);
    pDebug << "QTranslator t load" << m_translator1->load("/usr/share/libpeony-qt/libpeony-qt_" + locale);
    QApplication::installTranslator(m_translator1);
    m_translator2 = new QTranslator(this);
    pDebug << "QTranslator t2 load" << m_translator2->load("/usr/share/peony-qt-desktop/peony-qt-desktop_" + locale);
    QApplication::installTranslator(m_translator2);
    m_translator3 = new QTranslator(this);
    pDebug << "QTranslator t3 load" << m_translator3->load("/usr/share/qt5/translations/qt_" + locale);
    QApplication::installTranslator(m_translator3);
}

//获得newlist里比oldlist多的东西
QStringList KyNativeFileDialog::compareTwoList(QStringList oldList, QStringList newList)
{
    QSet<QString> oldSet = oldList.toSet();
    QSet<QString> newSet = newList.toSet();
    QStringList list = newSet.subtract(oldSet).toList();
    return list;
}

//获得两个list相同部分的内容
QStringList KyNativeFileDialog::sameTwoList(QStringList oldList, QStringList newList)
{
    QSet<QString> oldSet = oldList.toSet();
    QSet<QString> newSet = newList.toSet();

    QStringList list = newSet.intersect(oldSet).toList();
    return list;
}

bool KyNativeFileDialog::selectIsDir()
{
    return m_selectIsDir;
}

void KyNativeFileDialog::setSelectIsDir(bool selectIsDir)
{
    if(m_selectIsDir != selectIsDir)
        m_selectIsDir = selectIsDir;
}

bool KyNativeFileDialog::checkLongName(QString path, QString name)
{
    pDebug << "checkLongName..,........ name:" << path << name;
    if(name.toLocal8Bit().isNull()){
        pDebug << "name local null.....";
        return false;
    }
    int nameLocalLong = name.toLocal8Bit().length();
    int nameLong = name.length();
    pDebug << "name count 000..." << nameLocalLong << nameLong;

    auto fsType = Peony::FileUtils::getFsTypeFromFile(getCurrentPage()->getCurrentUri());

    QDBusInterface iface ("com.kylin.file.system.fuse","/com/kylin/file/system/fuse","com.kylin.file.system.fuse",QDBusConnection::systemBus());
    if (iface.isValid()) {
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        if (Peony::FileUtils::isFileExsit(m_longSavePath)) {
            QDBusReply<bool> reply = iface.call("SetSetings", "true", dir.path().append("/扩展"));
        }
        else{
            pDebug << "is not FileExsit" << m_longSavePath;
        }
        auto currentPath = path;
        if(currentPath.startsWith("file://"))
            currentPath = currentPath.remove(0, 7);
        if(currentPath.endsWith("/"))
            currentPath = currentPath.left(currentPath.length() - 1);

        if (isFuseFileSystem(currentPath)) {
            fsType = "fuse.kyfs";
        }
    }


    pDebug << "checkLongName fsType,......." << fsType;


    if (fsType.contains("ext")) {
        if(nameLocalLong <= LONG_NAME_LIMIT)
            return false;
    } else if (fsType.contains("ntfs")) {
        if(nameLong <= LONG_NAME_LIMIT)
            return false;
    } else if (fsType.contains("fuse.kyfs")) {
        int32_t maxLength = LONG_NAME_LIMIT;
        //TODO超时机制
        QDBusReply<int32_t> reply = iface.call("GetFilenameLength");

        if (reply.isValid()) {
            maxLength = reply.value();
            m_maxLenth = maxLength;
        }

        if(nameLong <= maxLength)
            return false;
    }

    if(nameLocalLong <= LONG_NAME_LIMIT){
        pDebug << "checkLongName 0000000000" << name;
        return false;
    }

    return true;
}

bool KyNativeFileDialog::longNameDialog(QString path, QString name){
    pDebug << "new kyfiledialogRename.....";
    KyFileDialogRename* handle = new KyFileDialogRename();
    handle->setCurrentUri(getCurrentPage()->getCurrentUri());
    handle->setMaxLength(m_maxLenth);

    Peony::FileOperationError error;
    error.op = Peony::FileOpRename;
    error.destDirUri = path + "/" + name;
    error.errorCode = G_IO_ERROR_FILENAME_TOO_LONG;

    connect(handle, &KyFileDialogRename::changeFileName, this, [&](QString name){
        mKyFileDialogUi->m_fileNameEdit->setText(name);
    });

    connect(handle, &KyFileDialogRename::rejected, this, [&](){
    });
    connect(handle, &KyFileDialogRename::accepted, this, [&](){
        pDebug << "handle accepted.......";
        m_RecodPath = "";
    });

    connect(handle, &KyFileDialogRename::muchLongNameReject, this, [&](){
        pDebug << "muchLongNameReject.........." << m_RecodPath;
        if(!m_RecodPath.isEmpty()){
            goToUri(m_RecodPath);
            m_RecodPath = "";
        }
    });

    pDebug << "exec...." << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
    handle->handle(error);
    bool handleResult = handle->dialogResult();
    delete handle;

    if (!handleResult) {
        pDebug << "error.respCode......." << error.respCode;
        return false;
    }
    if (error.respCode == Peony::ExceptionResponse::SaveAll ||
            error.respCode == Peony::ExceptionResponse::SaveOne) {
        //todo  判断路径是否存在  不存在创建？

        QString destUri = m_longSavePath;
        QDBusInterface iface ("com.kylin.file.system.fuse","/com/kylin/file/system/fuse","com.kylin.file.system.fuse",QDBusConnection::systemBus());
        if (!iface.isValid()) {
            qWarning() << "fuse dbus has error : " << iface.lastError() << SaveErrorType::DBUS_ERROR;
            QMessageBox::warning(this, tr("Warning"), QString(tr("File save failed! ")));

            return false;
        }
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));

        if (!Peony::FileUtils::isFileExsit(destUri)) {
            if (!dir.mkdir("扩展")) {
                qWarning() << "create dir error : " << SaveErrorType::DBUS_ERROR;
                QMessageBox::warning(this, tr("Warning"), QString(tr("File save failed! ")));

                return false;
            }
        }
        auto path = dir.path().append("/扩展");
        QDBusReply<bool> reply = iface.call("SetSetings", "true", path);

        if (reply.isValid()) {
            if (reply.value()) {
                qInfo() << "fuse setings true";
                pDebug << "save on path:" << m_longSavePath;
                m_RecodPath = getCurrentPage()->getCurrentUri();

                goToUri(m_longSavePath);
                m_longNameChangeDir = true;
            } else {
                qWarning() << "fuse setings false : "<< iface.lastError() << SaveErrorType::DBUSValue_ERROR;
                QMessageBox::warning(this, tr("Warning"), QString(tr("File save failed! ")));

                return false;
            }
        } else {
            qWarning() << "fuse dbus has error : " << iface.lastError() << SaveErrorType::DBUSValid_ERROR;
            QMessageBox::warning(this, tr("Warning"), QString(tr("File save failed! ")));

            return false;
        }

    }

    pDebug << "longNameDialog true.....";
    return true;
}


void KyNativeFileDialog::selectionChanged()
{
    //        qApp->processEvents();
    pDebug << "selectionChanged 0000000............" << getCurrentSelectionsList().length() << getCurrentSelectionsList() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
    //        if(mKyFileDialogUi->m_fileNameEdit->hasFocus() && getCurrentSelectionsList().length() == 0)
    //            return;

    //        QTimer::singleShot(1000, this, [this](){
    //            pDebug << "selectionChanged 111111111............" << getCurrentSelectionsList().length() << getCurrentSelectionsList() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");

    //        });

    //文件对话框被隐藏时，不更新
    if(this->isHidden()){
        pDebug << "selectionChanged filedialog is hidden";
        return;
    }

    if(!getCurrentSelectionsList().isEmpty())
        m_isLoadSelect = m_isLoading;
    else
        m_isLoadSelect = false;
    if(getCurrentSelectionsList().length() == 1)
        pDebug << "selectionChanged getCurrentSelectionsList........" << getCurrentSelectionsList();

    updateStatusBar();
}

QString KyNativeFileDialog::convertSpecialPath(QString uri) const
{
    if (uri.startsWith("trash://") || uri.startsWith("recent://")
            || uri.startsWith("computer://") || uri.startsWith("favorite://") ||
            uri.startsWith("idm://")){
        pDebug << "convertSpecialPath directory............" << uri;

        return Peony::FileUtils::getTargetUri(uri);
    }
    else if(uri.startsWith("filesafe://")){
        QString boxpath = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.box";
        QString path = boxpath + uri.remove(0,11);
        return path;
    }
    else if(uri.startsWith("mult://") || uri.startsWith("smb://") ||
            uri.startsWith("ftp://") || uri.startsWith("sftp://") ||
            uri.startsWith("mtp://") || uri.startsWith("gphoto2://")){
//        QString copyUri = getCurrentPage()->getCurrentUri();
        pDebug << "directory uri sssssssssssss:......" << uri;

        g_autofree char* path = g_file_get_path(const_cast<GFile*>(g_file_new_for_uri(uri.toLocal8Bit())));
        pDebug << "directory mult path:......" << (path == nullptr) << path;

        if(path == nullptr){
            path = g_file_get_path(const_cast<GFile*>(g_file_new_for_uri(Peony::FileUtils::urlDecode(uri).toLocal8Bit())));
            pDebug << "directory mult path 1111:......" << (path == nullptr) << path;
        }
        if(path == nullptr){
            path = g_file_get_path(const_cast<GFile*>(g_file_new_for_uri(Peony::FileUtils::urlEncode(uri).toLocal8Bit())));
            pDebug << "directory mult path 222222:......" << (path == nullptr) << path;
        }

        if(path == nullptr)
            return uri;
        return QString("file://").append(path);
    }
    else{
        //TODO network   mtp   burn  kydroid  kmre  gphoto2  thumbnail
    }

    return uri;
}

//需求28545 数据盘路径跳转
bool KyNativeFileDialog::isDataBlockHasUserFile()
{
    QString configFilePath = "/etc/xdg/peony-data.conf";
    if (! QFile::exists(configFilePath))
        return true;

    QFile file(configFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        // 判断标识是否为true, 是的话，存在用户数据，如果是false, 则是干净的数据盘，可以使用新方案
        if (line != "true")
            return false;
        file.close();
    } else {
        qWarning() << "open /etc/xdg/peony-data.conf failed";
    }

    return true;
}

void KyNativeFileDialog::slot_responseUnmounted(const QString &destUri, const QString &sourceUri)
{
    QString decodedSrcUri = Peony::FileUtils::urlDecode(sourceUri);
    QString uri = getCurrentPage()->getCurrentUri();
    uri = Peony::FileUtils::urlDecode(uri);
    pDebug<<"decodedSrcUri:"<<decodedSrcUri<<" uri:"<<uri;
    if(!decodedSrcUri.isEmpty() && uri.contains(decodedSrcUri) && uri != "file:///" && uri!= "filesafe:///")
    {
        pDebug<<"sourceUri:"<<sourceUri;
        if (uri.startsWith("filesafe:///"))
            goToUri("filesafe:///", true, true);  /* 跳转到文件保护箱路径 */
        else
            goToUri(destUri, true, true);/* 跳转到计算机页 */
    }
}


/*
GAsyncReadyCallback KyNativeFileDialog::async_query_permisson_callback(GObject *obj, GAsyncResult *res, Peony::PermissionsPropertiesPage *p_this)
{
    GError *err = nullptr;
    auto info = g_file_query_info_finish(G_FILE(obj), res, &err);

    if (info) {
        if (p_this) {
            guint32 mode = 0;
            if (g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_UNIX_MODE))
                mode = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_UNIX_MODE);

            auto owner_readable  = mode & S_IRUSR;
            auto owner_writeable = mode & S_IWUSR;
            auto owner_executable = mode & S_IXUSR;

            qDebug() << "owner_writeable......" << owner_writeable;
        }

        g_object_unref(info);
    }
    return nullptr;
}
*/
