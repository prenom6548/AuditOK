/*
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
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#include "ai-search-plugin.h"
#include "file-utils.h"
#include "thumbnail-creator.h"
#include "file-indexer-config.h"
#include "data-collecter.h"
#include "file-utils.h"
#include "dir-watcher.h"
#include "global-settings.h"
#include <kylin-ai/common/common.h>
#include "date-format-helper.h"
#include <unistd.h>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <qt5xdg/XdgDesktopFile>
#endif
#include <QProcess>
#include <QApplication>

using namespace UkuiSearch;
size_t AiSearchPlugin::uniqueSymbolAi = 0;
QMutex AiSearchPlugin::mutex;
static const int THUMBNAIL_HEIGHT = 247;
static const int THUMBNAIL_WIDTH = 352;

void AiSearchPlugin::setAutoTestAttribute(QWidget *widget, const QString &widgetType, const QString &widgetName, const QString &desc) {
    QString name = "ukui-search_AiSearchPlugin_" + widgetType + "_" + widgetName;
    widget->setAccessibleName(name);
    widget->setAccessibleDescription(desc);
}

AiSearchPlugin::AiSearchPlugin(QObject *parent) : QObject(parent)
{
    SearchPluginIface::Actioninfo open { 0, tr("Open")};
    SearchPluginIface::Actioninfo OpenPath {1, tr("Open path")};
    SearchPluginIface::Actioninfo CopyPath { 2, tr("Copy Path")};
    m_actionInfo << open << OpenPath << CopyPath;
    m_pool.setMaxThreadCount(1);
    m_thumbnailPool.setMaxThreadCount(1);
    m_pool.setExpiryTimeout(1000);
    m_timer = new QTimer(this);
    m_timer->setInterval(500);
    connect(m_timer, &QTimer::timeout, this, [ = ] {
        auto aiSearch = new AiSearch(m_searchResult, uniqueSymbolAi, m_keyword);
        m_pool.start(aiSearch);
        m_timer->stop();
    });
    initDetailPage();
    connect(&GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [ & ](const QString &key) {
        if (key == ICON_THEME_KEY && !m_useImage) {
            m_iconLabel->setPixmap(m_detailPageIcon.pixmap(120, 120));
        }
    });
}

const QString AiSearchPlugin::name()
{
    return "Ai Search";
}

const QString AiSearchPlugin::description()
{
    return tr("Content-Aware Search");
}

QString AiSearchPlugin::getPluginName()
{
    return tr("Content-Aware Search");
}

size_t AiSearchPlugin::KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult)
{
    mutex.lock();
    uniqueSymbolAi++;
    mutex.unlock();
    m_keyword = keyword;
    m_searchResult = searchResult;
    m_timer->start();
    return uniqueSymbolAi;
}

void AiSearchPlugin::stopSearch()
{
    mutex.lock();
    ++uniqueSymbolAi;
    mutex.unlock();
    m_thumbnailPool.clear();
    m_pool.clear();
    m_timer->stop();
}

QList<SearchPluginIface::Actioninfo> AiSearchPlugin::getActioninfo(int type)
{
    Q_UNUSED(type)
    return m_actionInfo;
}

void AiSearchPlugin::openAction(int actionkey, QString key, int type)
{
    //TODO add some return message here.
    switch (actionkey) {
        case 0:
            DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("openFile"));
            if (type == 1) {
                QStringList args{"-m"};
                if (!is_ai_subsystem_inited() || !AiSearch::modelReady()) {
                    args.append(QStringLiteral("aisubsystem"));
                } else if (!FileIndexerConfig::getInstance()->isAiIndexEnable()) {
                    args.append(QStringLiteral("search"));
                } else {
                    break;
                }

                QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("com.kylin.ProcessManager"),
                                                                      QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                                      QStringLiteral("com.kylin.ProcessManager.AppLauncher"),
                                                                      QStringLiteral("LaunchAppWithArguments"));
                QString desktopFile("ukui-control-center.desktop");
                message << desktopFile << args;
                auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
                QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [desktopFile, args] (QDBusPendingCallWatcher *self) {
                    if (self->isError()) {
                        qWarning() << "Fail to call com.kylin.ProcessManager.AppLauncher" << self->error();
//                        XdgDesktopFile xdf;
//                        xdf.load(desktopFile);
//                        if(!xdf.startDetached(args)) {
//                            qWarning() << "Fail to Launch" << desktopFile << "with args " << args;
//                        }
                        QProcess::startDetached("ukui-control-center", args);
                    }
                    self->deleteLater();
                });
            } else if (FileUtils::openFile(key) == -1) {
                QMessageBox msgBox(m_contentDetailPage);
                msgBox.setModal(true);
                const auto confirmButton = msgBox.addButton(tr("OK"), QMessageBox::YesRole);
                setAutoTestAttribute(confirmButton, "QPushButton", "confirmButton", "the confirm button of the message box when there's no default app for opening file");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Can not get a default application for opening %1.").arg(key));
                msgBox.exec();
            }
            break;
        case 1:
            DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("openFileInDir"));
            FileUtils::openFile(key, true);
            break;
        case 2:
            DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("copyPath"));
            FileUtils::copyPath(key);
        default:
            break;
    }
}

QWidget *AiSearchPlugin::detailPage(const SearchPluginIface::ResultInfo &ri)
{
    if (!is_ai_subsystem_inited() || !AiSearch::modelReady()) {
        m_actionLabelWithoutModel->setText(tr("AI subsystem is not installed"));
        m_clickableActionLabelWithoutModel->setText(tr("click to install subsystem"));
        m_contentDetailPage->hide();
        m_detailLyt->removeWidget(m_contentDetailPage);
        m_detailLyt->addWidget(m_detailPageWithoutModel);
        m_detailPageWithoutModel->show();
        return m_detailPage;
    }

    if (!FileIndexerConfig::getInstance()->isAiIndexEnable()) {
        m_actionLabelWithoutModel->setText(tr("AI index is disabled"));
        m_clickableActionLabelWithoutModel->setText(tr("click to enable AI index"));
        m_contentDetailPage->hide();
        m_detailLyt->removeWidget(m_contentDetailPage);
        m_detailLyt->addWidget(m_detailPageWithoutModel);
        m_detailPageWithoutModel->show();
        return m_detailPage;
    }

    if(ri.actionKey != m_currentActionKey) {
        m_useImage = false;
        auto creator = new ThumbnailCreator(ri.actionKey, QSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT), m_contentDetailPage->window()->devicePixelRatio());
        connect(creator, &ThumbnailCreator::ready, this, [&, ri](const QString& uri, const QImage &image){
            if(uri != m_currentActionKey) {
                return;
            }
            QPixmap pixmap = QPixmap::fromImage(image);
            if(!pixmap.isNull()) {
                m_useImage = true;
                m_iconLabel->setPixmap(pixmap);
                m_contentDetailLyt->setContentsMargins(8, (THUMBNAIL_HEIGHT - pixmap.height()) / 2 + 8, 16, 0);
            } else {
                m_detailPageIcon = ri.icon;
                m_iconLabel->setPixmap(m_detailPageIcon.pixmap(120, 120));
            }
        }, Qt::QueuedConnection);
        m_thumbnailPool.start(creator);
        m_detailPageIcon = ri.icon;
        m_iconLabel->setPixmap(m_detailPageIcon.pixmap(120, 120));
        m_contentDetailLyt->setContentsMargins(8, 50, 16, 0);
        m_currentActionKey = ri.actionKey;
    }

    //当字体长度超过215时显示为省略号
    QString showName = m_nameLabel->fontMetrics().elidedText(ri.name, Qt::ElideRight, 215);
    m_nameLabel->setText(FileUtils::setAllTextBold(showName));
    if(QString::compare(showName, ri.name)) {
        m_nameLabel->setToolTip(ri.name);
    } else {
        m_nameLabel->setToolTip("");
    }
    m_pluginLabel->setText(tr("File"));

    m_pathLabel2->setText(m_pathLabel2->fontMetrics().elidedText(m_currentActionKey, Qt::ElideRight, m_pathLabel2->width()));
    m_pathLabel2->setToolTip(m_currentActionKey);
    QString modTimeText = ri.description.at(1).value;
    if (!DateFormat::getInstance().format().isEmpty()) {
        QDateTime modTime = QDateTime::fromString(modTimeText, DEFAULT_DATETIME_FORMAT);
        modTimeText = modTime.toString(DateFormat::getInstance().format());
    }
    m_timeLabel2->setText(modTimeText);
    m_detailPageWithoutModel->hide();
    m_detailLyt->removeWidget(m_detailPageWithoutModel);
    m_detailLyt->addWidget(m_contentDetailPage);
    m_contentDetailPage->show();
    return m_detailPage;
}

void AiSearchPlugin::initDetailPage()
{
    m_detailPage = new QWidget();
    setAutoTestAttribute(m_detailPage, "QWidget", "detailPage", "the detail page of ai search");
    m_detailPage->setFixedWidth(360);
    m_detailPage->setAttribute(Qt::WA_TranslucentBackground);
    m_detailLyt = new QVBoxLayout(m_detailPage);

    m_detailPageWithoutModel = new QWidget(m_detailPage);
    setAutoTestAttribute(m_detailPageWithoutModel, "QWidget", "detailPageWithoutModel", "the detail page of ai search in the state of no model");
    m_detailPageWithoutModel->setFixedWidth(360);
    m_detailPageWithoutModel->setAttribute(Qt::WA_TranslucentBackground);
    m_detailLytWithoutModel = new QVBoxLayout(m_detailPageWithoutModel);
    m_detailPageWithoutModel->setLayout(m_detailLytWithoutModel);
    m_detailLytWithoutModel->setContentsMargins(8, 0, 16, 0);
    m_iconLabelWithoutModel = new QLabel(m_detailPageWithoutModel);
    setAutoTestAttribute(m_iconLabelWithoutModel, "QLabel", "iconLabelWithoutModel", "the icon label of the ai search detail page in the state of no model");
    m_iconLabelWithoutModel->setAlignment(Qt::AlignCenter);
    QString prefix = GlobalSettings::getInstance().getValue(STYLE_NAME_KEY).toString() == "ukui-dark" ? ":/res/icons/dark/" : ":/res/icons/default/";
    m_iconLabelWithoutModel->setPixmap(QIcon::fromTheme(prefix + "no-model-symbolic").pixmap(128, 128));

    connect(&GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [this] (const QString& key) {
        if(key == STYLE_NAME_KEY) {
            QString prefix = GlobalSettings::getInstance().getValue(STYLE_NAME_KEY).toString() == "ukui-dark" ? ":/res/icons/dark/" : ":/res/icons/default/";
            m_iconLabelWithoutModel->setPixmap(QIcon::fromTheme(prefix + "no-model-symbolic").pixmap(128, 128));
        }
    });

    m_actionLabelWithoutModel = new QLabel(m_actionFrameWithoutModel);
    setAutoTestAttribute(m_actionLabelWithoutModel, "QLabel", "actionLabelWithoutModel", "the action label in the state of no model");
    m_actionLabelWithoutModel->setText(tr("AI subsystem is not installed"));
    m_actionLabelWithoutModel->setEnabled(false);
    m_actionLabelWithoutModel->setAlignment(Qt::AlignCenter);
    m_actionLabelWithoutModel->adjustSize();

    m_actionFrameWithoutModel = new QFrame(m_detailPageWithoutModel);
    m_actionFrameLytWithoutModel = new QVBoxLayout(m_actionFrameWithoutModel);
    m_actionFrameLytWithoutModel->setContentsMargins(0, 0, 0, 0);
    m_actionFrameLytWithoutModel->setAlignment(Qt::AlignHCenter);
    m_clickableActionLabelWithoutModel = new ActionLabel(tr("click to install subsystem"), "search", m_actionFrameWithoutModel);
    setAutoTestAttribute(m_clickableActionLabelWithoutModel, "ActionLabel", "clickableActionLabelWithoutModel", "action label of the detail page in the state of no model");
    m_clickableActionLabelWithoutModel->adjustSize();
    m_actionFrameLytWithoutModel->addWidget(m_clickableActionLabelWithoutModel);
    m_actionFrameWithoutModel->setLayout(m_actionFrameLytWithoutModel);

    m_detailLytWithoutModel->addSpacing(146);
    m_detailLytWithoutModel->addWidget(m_iconLabelWithoutModel);
    m_detailLytWithoutModel->addWidget(m_actionLabelWithoutModel);
    m_detailLytWithoutModel->addWidget(m_actionFrameWithoutModel);
    m_detailLytWithoutModel->addStretch();

    m_contentDetailPage = new QWidget(m_detailPage);
    m_contentDetailPage->setFixedWidth(360);
    m_contentDetailPage->setAttribute(Qt::WA_TranslucentBackground);
    m_contentDetailLyt = new QVBoxLayout(m_contentDetailPage);
    m_contentDetailLyt->setContentsMargins(8, 0, 16, 0);
    m_iconLabel = new QLabel(m_contentDetailPage);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setMinimumHeight(120);

    m_nameFrame = new QFrame(m_contentDetailPage);
    m_nameFrameLyt = new QHBoxLayout(m_nameFrame);
    m_nameFrame->setLayout(m_nameFrameLyt);
    m_nameFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_nameLabel = new QLabel(m_nameFrame);
    setAutoTestAttribute(m_nameLabel, "QLabel", "nameLabel", "the file name label of the detail page");
    m_nameLabel->setMaximumWidth(280);
    m_pluginLabel = new QLabel(m_nameFrame);
    m_pluginLabel->setEnabled(false);
    m_nameFrameLyt->addWidget(m_nameLabel);
    m_nameFrameLyt->addStretch();
    m_nameFrameLyt->addWidget(m_pluginLabel);

    m_line_1 = new SeparationLine(m_contentDetailPage);

    m_pathFrame = new QFrame(m_contentDetailPage);
    m_pathFrameLyt = new QHBoxLayout(m_pathFrame);
    m_pathLabel1 = new QLabel(m_pathFrame);
    m_pathLabel2 = new QLabel(m_pathFrame);
    setAutoTestAttribute(m_pathLabel2, "QLabel", "pathLabel2", "the label shows file full path on the detail page");
    m_pathLabel1->setText(tr("Path"));
    m_pathLabel2->setFixedWidth(240);
    m_pathLabel2->setAlignment(Qt::AlignRight);
    m_pathFrameLyt->addWidget(m_pathLabel1);
    m_pathFrameLyt->addStretch();
    m_pathFrameLyt->addWidget(m_pathLabel2);

    m_timeFrame = new QFrame(m_contentDetailPage);
    m_timeFrameLyt = new QHBoxLayout(m_timeFrame);
    m_timeLabel1 = new QLabel(m_timeFrame);
    m_timeLabel2 = new QLabel(m_timeFrame);
    setAutoTestAttribute(m_timeLabel2, "QLabel", "timeLabel2", "the label show last modified time on the detail page");
    m_timeLabel2->setAlignment(Qt::AlignRight);
    m_timeLabel1->setText(tr("Last time modified"));
    m_timeFrameLyt->addWidget(m_timeLabel1);
    m_timeFrameLyt->addStretch();
    m_timeFrameLyt->addWidget(m_timeLabel2);

    m_line_2 = new SeparationLine(m_contentDetailPage);

    m_actionFrame = new QFrame(m_contentDetailPage);
    m_actionFrameLyt = new QVBoxLayout(m_actionFrame);
    m_actionFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_actionLabel1 = new ActionLabel(tr("Open"), m_currentActionKey, m_actionFrame);
    setAutoTestAttribute(m_actionLabel1, "ActionLabel", "actionLabel1", "the label of open action");
    m_actionLabel2 = new ActionLabel(tr("Open path"), m_currentActionKey, m_actionFrame);
    setAutoTestAttribute(m_actionLabel2, "ActionLabel", "actionLabel2", "the label of open path action");
    m_actionLabel3 = new ActionLabel(tr("Copy path"), m_currentActionKey, m_actionFrame);
    setAutoTestAttribute(m_actionLabel3, "ActionLabel", "actionLabel3", "the label of copy path action");

    m_actionFrameLyt->addWidget(m_actionLabel1);
    m_actionFrameLyt->addWidget(m_actionLabel2);
    m_actionFrameLyt->addWidget(m_actionLabel3);
    m_actionFrame->setLayout(m_actionFrameLyt);

    m_contentDetailLyt->addWidget(m_iconLabel);
    m_contentDetailLyt->addWidget(m_nameFrame);
    m_contentDetailLyt->addWidget(m_line_1);
    m_contentDetailLyt->addWidget(m_pathFrame);
    m_contentDetailLyt->addWidget(m_timeFrame);
    m_contentDetailLyt->addWidget(m_line_2);
    m_contentDetailLyt->addWidget(m_actionFrame);
    m_contentDetailPage->setLayout(m_contentDetailLyt);
    m_contentDetailLyt->addStretch();

    m_detailLyt->addWidget(m_contentDetailPage);

    connect(m_clickableActionLabelWithoutModel, &ActionLabel::actionTriggered, [ & ]() {
        openAction(0, m_currentActionKey, 1);
    });

    connect(m_actionLabel1, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("openFile"));
        if(FileUtils::openFile(m_currentActionKey) == -1) {
            QMessageBox msgBox(m_contentDetailPage);
            msgBox.setModal(true);
            const auto confirmButton = msgBox.addButton(tr("OK"), QMessageBox::YesRole);
            setAutoTestAttribute(confirmButton, "QPushButton", "confirmButton", "the confirm button of the message box when there's no default app for opening file");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(tr("Can not get a default application for opening %1.").arg(m_currentActionKey));
            msgBox.exec();
        }
    });
    connect(m_actionLabel2, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("openFileInDir"));
        FileUtils::openFile(m_currentActionKey, true);
    });
    connect(m_actionLabel3, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("copyPath"));
        FileUtils::copyPath(m_currentActionKey);
    });
}

AiSearchPlugin::~AiSearchPlugin()
{
    m_pool.clear();
    m_thumbnailPool.clear();
    m_thumbnailPool.waitForDone();
    m_pool.waitForDone();
    AiSearch::destroySession();
    if(m_detailPage) {
        delete m_detailPage;
        m_detailPage = nullptr;
    }
}

bool AiSearch::s_failed = false;
DataManagementSession* AiSearch::s_session = nullptr;
QMutex AiSearch::s_sessionMutex;
bool AiSearch::s_supportState = false;
bool AiSearch::s_aiNotSupported = true;

AiSearch::AiSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, size_t uniqueSymbol, const QString &keyword)
{
    setAutoDelete(true);
    m_searchResult = searchResult;
    m_uniqueSymbol = uniqueSymbol;
    m_keyword = keyword;
}

void AiSearch::run()
{
    this->keywordSearch();
}

void AiSearch::keywordSearch()
{
    if (notSupported()) {
        return;
    }

    if (!is_ai_subsystem_inited() || !modelReady() || !FileIndexerConfig::getInstance()->isAiIndexEnable()) {
        SearchPluginIface::ResultInfo ri;
        ri.iconName = "no-model-symbolic";
        ri.name = QObject::tr("Click to complete the config for using content-aware search");
        ri.toolTip = ri.name;
        ri.type = 1;
        ri.showInBestMatch = false;
        ri.resourceType = QStringLiteral("file");
        ri.displayNameAsUrl = true;
        AiSearchPlugin::mutex.lock();
        if (m_uniqueSymbol == AiSearchPlugin::uniqueSymbolAi) {
            ri.searchID = m_uniqueSymbol;
            m_searchResult->enqueue(ri);
        }
        AiSearchPlugin::mutex.unlock();
        return;
    }

    // if (!FileIndexerConfig::getInstance()->isAiIndexEnable()) {
    //     return;
    // }
    QJsonObject keyword;
    keyword.insert(QStringLiteral("text"), m_keyword);
    char* results = nullptr;
    DataManagementStatus status = data_management_similarity_search(s_session, QJsonDocument(keyword).toJson().data(), &results);
    if (status.errorCode == 101) {
        qWarning() << "AiSearch:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        destroySession();
        if(!createSession()) {
            qWarning() << "AiSearch:Failed to create session";
            data_management_destroy_result(results);
            return;
        }
        qDebug() << "Creating session success.";
        data_management_similarity_search(s_session, QJsonDocument(keyword).toJson().data(), &results);
    }

    for (const auto &aResult : QJsonDocument::fromJson(results).array()) {
        if (aResult.isObject()) {
            bool isBlocked(false);
            QStringList blockList = DirWatcher::getDirWatcher()->getBlockDirsOfUser();
            QString filePath = aResult.toObject().value("filepath").toString();
            for (const auto& blockDir : blockList) {
                if (FileUtils::isOrUnder(filePath, blockDir)) {
                    isBlocked = true;
                    break;
                }
            }

            if (isBlocked) {
                continue;
            }

            SearchPluginIface::ResultInfo ri;
            ri.type = 0;
            if (createResultInfo(ri, aResult.toObject().value("filepath").toString())) {
                AiSearchPlugin::mutex.lock();
                if (m_uniqueSymbol == AiSearchPlugin::uniqueSymbolAi) {
                    ri.searchID = m_uniqueSymbol;
                    m_searchResult->enqueue(ri);
                    AiSearchPlugin::mutex.unlock();
                } else {
                    AiSearchPlugin::mutex.unlock();
                    break;
                }
            }
        }
    }
    data_management_destroy_result(results);
}

bool AiSearch::createResultInfo(SearchPluginIface::ResultInfo &ri, const QString &path)
{
    QFileInfo info(path);
    if(!info.exists()) {
        return false;
    }
    ri.icon = FileUtils::getFileIcon(QUrl::fromLocalFile(path).toString(), false);
    ri.name = info.fileName().replace("\r", " ").replace("\n", " ");
    ri.toolTip = info.fileName();
    ri.resourceType = QStringLiteral("file");
    ri.description = QVector<SearchPluginIface::DescriptionInfo>() \
                    << SearchPluginIface::DescriptionInfo{"Path:", path} \
                    << SearchPluginIface::DescriptionInfo{"Modified time:", info.lastModified().toString("yyyy/MM/dd hh:mm:ss")};
    ri.actionKey = path;
    return true;
}

AiSearch::~AiSearch() {}

bool AiSearch::createSession() {
    QMutexLocker locker(&s_sessionMutex);
    if (!s_session || s_failed) {
        s_session = data_management_create_session();
        if (!s_session) {
            s_failed = true;
            qWarning() << "===Create data management session failed!===" << s_session;
            return false;
        }
    }
    return true;
}

void AiSearch::destroySession() {
    QMutexLocker locker(&s_sessionMutex);
    data_management_destroy_session(s_session);
    s_session = nullptr;
}

bool AiSearch::modelReady()
{
    if (!AiSearch::createSession()) {
        return false;
    }
    FeatureStatus status = FeatureStatus::UNKNOWN;
    DataManagementStatus result = data_management_get_feature_status(s_session, &status);
    if (result.success) {
        if (status == FeatureStatus::AVAILABLE) {
            return true;
        } else {
            qWarning() << "AI search Model is not available";
        }
    } else {
        qWarning() << "AI search: Get feature status failed, result is" << result.errorMsg;
    }
    return false;
}

bool AiSearch::notSupported()
{
    if (!s_supportState) {
        s_supportState = true;
        s_aiNotSupported = !data_management_is_feature_supported();
        qDebug() << "AI search not supported status:" << s_aiNotSupported;
    }
    return s_aiNotSupported;
}
