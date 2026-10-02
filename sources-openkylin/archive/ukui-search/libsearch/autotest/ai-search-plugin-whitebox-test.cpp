#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTimer>

#include <cstring>

#define private public
#define protected public
#include "data-collecter.h"
#include "date-format-helper.h"
#include "global-settings.h"
#include "aisearch/ai-search-plugin.h"
#undef protected
#undef private

using namespace UkuiSearch;

namespace UkuiSearch {
QHash<QString, QVariant> g_globalSettingsValues;
bool g_aiIndexEnabled = true;
QStringList g_blockDirsOfUser;
int g_openFileResult = 0;
QString g_lastOpenPath;
bool g_lastOpenInDir = false;
QString g_lastCopiedPath;
QImage g_thumbnailImage;
QString g_thumbnailReadyUrlOverride;
}

int DataCollecter::calls = 0;
QString DataCollecter::lastPlugin;
QString DataCollecter::lastButton;

struct _DataManagementSession {
    int id;
};

namespace {

bool g_aiSubsystemInited = false;
bool g_featureSupported = true;
FeatureStatus g_featureStatus = AVAILABLE;
bool g_featureStatusSuccess = true;
bool g_createSessionFails = false;
int g_createSessionFailAtCall = 0;
int g_createCalls = 0;
int g_destroyCalls = 0;
int g_destroyResultCalls = 0;
int g_similarityCalls = 0;
int g_similaritySessionErrors = 0;
QByteArray g_similarityResult = "[]";

DataManagementStatus makeStatus(bool success = true, int errorCode = 0, const char *message = "")
{
    DataManagementStatus status{};
    status.success = success;
    status.errorCode = errorCode;
    std::strncpy(status.errorMsg, message, sizeof(status.errorMsg) - 1);
    return status;
}

char *copyBytes(const QByteArray &bytes)
{
    char *result = static_cast<char *>(std::malloc(static_cast<size_t>(bytes.size()) + 1));
    std::memcpy(result, bytes.constData(), static_cast<size_t>(bytes.size()));
    result[bytes.size()] = '\0';
    return result;
}

void resetStubState()
{
    g_aiSubsystemInited = false;
    g_featureSupported = true;
    g_featureStatus = AVAILABLE;
    g_featureStatusSuccess = true;
    g_createSessionFails = false;
    g_createSessionFailAtCall = 0;
    g_createCalls = 0;
    g_destroyCalls = 0;
    g_destroyResultCalls = 0;
    g_similarityCalls = 0;
    g_similaritySessionErrors = 0;
    g_similarityResult = "[]";

    g_globalSettingsValues.clear();
    g_globalSettingsValues.insert(STYLE_NAME_KEY, QStringLiteral("ukui-light"));
    g_globalSettingsValues.insert(ICON_THEME_KEY, QStringLiteral("default"));
    g_aiIndexEnabled = true;
    g_blockDirsOfUser.clear();
    g_openFileResult = 0;
    g_lastOpenPath.clear();
    g_lastOpenInDir = false;
    g_lastCopiedPath.clear();
    g_thumbnailImage = QImage();
    g_thumbnailReadyUrlOverride.clear();

    DataCollecter::calls = 0;
    DataCollecter::lastPlugin.clear();
    DataCollecter::lastButton.clear();

    AiSearch::destroySession();
    AiSearch::s_failed = false;
    AiSearch::s_supportState = false;
    AiSearch::s_aiNotSupported = true;
    AiSearchPlugin::uniqueSymbolAi = 0;
}

void closeModalMessageBoxes()
{
    QTimer::singleShot(0, []() {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
            if (auto *box = qobject_cast<QMessageBox *>(widget)) {
                box->accept();
            }
        }
    });
}

void clickActionLabel(ActionLabel *label)
{
    QMouseEvent event(QEvent::MouseButtonRelease,
                      QPointF(1, 1),
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier);
    QApplication::sendEvent(label, &event);
}

QString readFileText(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QString::fromLocal8Bit(file.readAll());
}

}  // namespace

extern "C" {

bool is_ai_subsystem_inited()
{
    return g_aiSubsystemInited;
}

bool data_management_is_feature_supported()
{
    return g_featureSupported;
}

DataManagementSession *data_management_create_session()
{
    ++g_createCalls;
    if (g_createSessionFails || (g_createSessionFailAtCall > 0 && g_createCalls >= g_createSessionFailAtCall)) {
        return nullptr;
    }
    return new DataManagementSession{g_createCalls};
}

void data_management_destroy_session(DataManagementSession *session)
{
    ++g_destroyCalls;
    delete session;
}

void data_management_destroy_result(char *result)
{
    ++g_destroyResultCalls;
    std::free(result);
}

DataManagementStatus data_management_get_feature_status(DataManagementSession *, FeatureStatus *status)
{
    if (status) {
        *status = g_featureStatus;
    }
    return makeStatus(g_featureStatusSuccess, g_featureStatusSuccess ? 0 : DM_ERROR_UNKNOWN, "feature");
}

DataManagementStatus data_management_similarity_search(DataManagementSession *, const char *, char **search_result)
{
    ++g_similarityCalls;
    if (g_similaritySessionErrors-- > 0) {
        *search_result = copyBytes("[]");
        return makeStatus(false, DM_SESSION_ERROR, "session");
    }

    *search_result = copyBytes(g_similarityResult);
    return makeStatus(true);
}

}

class AiSearchPluginWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void metadataAndTimerLifecycle();
    void detailPageSwitchesAcrossStatesAndThumbnailBranches();
    void openActionAndDetailButtonsCoverFileUtilsBranches();
    void resultInfoAndOpenActionEdgeBranches();
    void timerConfigActionsAndClickableLabelCoverAsyncBranches();
    void aiSearchHandlesSupportConfigAndSimilarityBranches();
    void thumbnailMismatchAndSessionRecoveryFailureBranches();
    void sessionAndModelReadyCoverSdkBranches();
    void keywordSearchRunsWorkerWithDifferentKeywords();
    void stopSearchInterruptsActiveSearch();
    void detailPageEdgeCasesAndNullResultBranches();
};

void AiSearchPluginWhiteboxTest::init()
{
    resetStubState();
}

void AiSearchPluginWhiteboxTest::metadataAndTimerLifecycle()
{
    AiSearchPlugin plugin;

    QCOMPARE(plugin.pluginType(), PluginInterface::PluginType::SearchPlugin);
    QCOMPARE(plugin.name(), QStringLiteral("Ai Search"));
    QCOMPARE(plugin.description(), QStringLiteral("Content-Aware Search"));
    QCOMPARE(plugin.getPluginName(), QStringLiteral("Content-Aware Search"));
    QCOMPARE(plugin.getActioninfo(0).size(), 3);
    QVERIFY(plugin.isEnable());
    plugin.setEnable(false);
    QVERIFY(!plugin.isEnable());
    plugin.setEnable(true);
    QVERIFY(plugin.isEnable());

    DataQueue<SearchPluginIface::ResultInfo> queue;
    const size_t id = plugin.KeywordSearch(QStringLiteral("semantic keyword"), &queue);
    QVERIFY(id > 0);
    QVERIFY(plugin.m_timer->isActive());
    plugin.stopSearch();
    QVERIFY(!plugin.m_timer->isActive());
    QVERIFY(AiSearchPlugin::uniqueSymbolAi > id);
}

void AiSearchPluginWhiteboxTest::detailPageSwitchesAcrossStatesAndThumbnailBranches()
{
    AiSearchPlugin plugin;
    SearchPluginIface::ResultInfo result;
    result.icon = QIcon::fromTheme(QStringLiteral("text-x-generic"));
    result.name = QString(260, QLatin1Char('a'));
    result.actionKey = QStringLiteral("/tmp/ai-result-a.txt");
    result.description = QVector<SearchPluginIface::DescriptionInfo>()
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Path:"), result.actionKey}
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Modified time:"), QStringLiteral("2024/01/02 03:04:05")};

    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);
    QVERIFY(plugin.m_actionLabelWithoutModel->text().contains(QStringLiteral("installed")));
    Q_EMIT GlobalSettings::getInstance().valueChanged(STYLE_NAME_KEY, QStringLiteral("ukui-dark"));
    QCoreApplication::processEvents();

    g_aiSubsystemInited = true;
    g_aiIndexEnabled = false;
    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);
    QVERIFY(plugin.m_actionLabelWithoutModel->text().contains(QStringLiteral("disabled")));
    QVERIFY(plugin.m_clickableActionLabelWithoutModel->text().contains(QStringLiteral("enable")));

    g_aiIndexEnabled = true;
    g_thumbnailImage = QImage(24, 12, QImage::Format_ARGB32_Premultiplied);
    g_thumbnailImage.fill(Qt::red);
    DateFormat::getInstance().updateDateFormat(QStringLiteral("MM-dd-yyyy"));
    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);
    plugin.m_thumbnailPool.waitForDone();
    QCoreApplication::processEvents();
    QVERIFY(plugin.m_useImage);
    QVERIFY(!plugin.m_nameLabel->toolTip().isEmpty());
    QCOMPARE(plugin.m_pluginLabel->text(), QStringLiteral("File"));
    QVERIFY(!plugin.m_timeLabel2->text().isEmpty());

    SearchPluginIface::ResultInfo shortResult = result;
    shortResult.name = QStringLiteral("short.txt");
    shortResult.actionKey = QStringLiteral("/tmp/ai-result-b.txt");
    g_thumbnailImage = QImage();
    QCOMPARE(plugin.detailPage(shortResult), plugin.m_detailPage);
    plugin.m_thumbnailPool.waitForDone();
    QCoreApplication::processEvents();
    QVERIFY(!plugin.m_useImage);
    QCOMPARE(plugin.m_nameLabel->toolTip(), QString());
    Q_EMIT GlobalSettings::getInstance().valueChanged(ICON_THEME_KEY, QStringLiteral("whitebox"));
    QCoreApplication::processEvents();
}

void AiSearchPluginWhiteboxTest::openActionAndDetailButtonsCoverFileUtilsBranches()
{
    AiSearchPlugin plugin;
    SearchPluginIface::ResultInfo result;
    result.icon = QIcon::fromTheme(QStringLiteral("text-x-generic"));
    result.name = QStringLiteral("open-me.txt");
    result.actionKey = QStringLiteral("/tmp/open-me.txt");
    result.description = QVector<SearchPluginIface::DescriptionInfo>()
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Path:"), result.actionKey}
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Modified time:"), QStringLiteral("2024/01/02 03:04:05")};

    g_aiSubsystemInited = true;
    g_aiIndexEnabled = true;
    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);

    g_openFileResult = -1;
    closeModalMessageBoxes();
    plugin.openAction(0, result.actionKey, 0);
    QCOMPARE(g_lastOpenPath, result.actionKey);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("openFile"));

    plugin.openAction(1, result.actionKey, 0);
    QCOMPARE(g_lastOpenPath, result.actionKey);
    QVERIFY(g_lastOpenInDir);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("openFileInDir"));

    plugin.openAction(2, result.actionKey, 0);
    QCOMPARE(g_lastCopiedPath, result.actionKey);
    QCOMPARE(QApplication::clipboard()->text(), result.actionKey);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("copyPath"));

    closeModalMessageBoxes();
    clickActionLabel(plugin.m_actionLabel1);
    clickActionLabel(plugin.m_actionLabel2);
    clickActionLabel(plugin.m_actionLabel3);
    QCOMPARE(g_lastCopiedPath, result.actionKey);

    g_openFileResult = 0;
    g_lastOpenPath.clear();
    clickActionLabel(plugin.m_actionLabel1);
    QCOMPARE(g_lastOpenPath, result.actionKey);
}

void AiSearchPluginWhiteboxTest::resultInfoAndOpenActionEdgeBranches()
{
    DataQueue<SearchPluginIface::ResultInfo> queue;
    AiSearch search(&queue, 1, QStringLiteral("semantic"));

    SearchPluginIface::ResultInfo missing;
    QVERIFY(!search.createResultInfo(missing, QStringLiteral("/tmp/ukui-search-ai-missing-file")));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString filePath = dir.filePath(QStringLiteral("line\nbreak\rname.txt"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("content");
    file.close();

    SearchPluginIface::ResultInfo result;
    QVERIFY(search.createResultInfo(result, filePath));
    QCOMPARE(result.actionKey, filePath);
    QCOMPARE(result.resourceType, QStringLiteral("file"));
    QVERIFY(!result.name.contains(QLatin1Char('\n')));
    QVERIFY(!result.name.contains(QLatin1Char('\r')));
    QCOMPARE(result.description.size(), 2);

    AiSearchPlugin plugin;
    DataCollecter::calls = 0;
    plugin.openAction(99, result.actionKey, 0);
    QCOMPARE(DataCollecter::calls, 0);

    g_aiSubsystemInited = true;
    g_aiIndexEnabled = true;
    g_featureStatus = AVAILABLE;
    g_featureStatusSuccess = true;
    DataCollecter::calls = 0;
    plugin.openAction(0, result.actionKey, 1);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("openFile"));
    QCOMPARE(DataCollecter::calls, 1);

    result.icon = QIcon::fromTheme(QStringLiteral("text-x-generic"));
    result.description = QVector<SearchPluginIface::DescriptionInfo>()
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Path:"), result.actionKey}
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Modified time:"), QStringLiteral("2024/01/02 03:04:05")};
    g_thumbnailImage = QImage(16, 16, QImage::Format_ARGB32_Premultiplied);
    g_thumbnailImage.fill(Qt::green);
    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);
    plugin.m_thumbnailPool.waitForDone();
    QCoreApplication::processEvents();
    QVERIFY(plugin.m_useImage);

    const QMargins firstMargins = plugin.m_contentDetailLyt->contentsMargins();
    result.name = QStringLiteral("same-key.txt");
    g_thumbnailImage = QImage();
    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);
    plugin.m_thumbnailPool.waitForDone();
    QCoreApplication::processEvents();
    QCOMPARE(plugin.m_contentDetailLyt->contentsMargins(), firstMargins);
}

void AiSearchPluginWhiteboxTest::aiSearchHandlesSupportConfigAndSimilarityBranches()
{
    DataQueue<SearchPluginIface::ResultInfo> queue;

    g_featureSupported = false;
    AiSearch unsupported(&queue, 1, QStringLiteral("semantic"));
    unsupported.run();
    QVERIFY(queue.isEmpty());

    g_featureSupported = true;
    AiSearch::s_supportState = false;
    AiSearch::s_aiNotSupported = true;
    g_aiSubsystemInited = false;
    AiSearchPlugin::uniqueSymbolAi = 10;
    AiSearch configHint(&queue, 10, QStringLiteral("semantic"));
    configHint.run();
    QCOMPARE(queue.length(), 1);
    SearchPluginIface::ResultInfo configResult = queue.tryDequeue();
    QCOMPARE(configResult.searchID, static_cast<size_t>(10));
    QCOMPARE(configResult.type, 1);
    QCOMPARE(configResult.iconName, QStringLiteral("no-model-symbolic"));

    AiSearchPlugin::uniqueSymbolAi = 12;
    AiSearch staleConfigHint(&queue, 11, QStringLiteral("semantic"));
    staleConfigHint.run();
    QVERIFY(queue.isEmpty());

    g_aiSubsystemInited = true;
    g_aiIndexEnabled = true;
    AiSearch::s_supportState = false;
    AiSearch::s_aiNotSupported = true;
    g_featureStatus = AVAILABLE;
    g_featureStatusSuccess = true;
    g_similaritySessionErrors = 1;

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("blocked")));
    const QString allowedPath = dir.filePath(QStringLiteral("allowed.txt"));
    const QString blockedPath = dir.filePath(QStringLiteral("blocked/blocked.txt"));
    QFile allowedFile(allowedPath);
    QVERIFY(allowedFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    allowedFile.write("allowed");
    allowedFile.close();
    QFile blockedFile(blockedPath);
    QVERIFY(blockedFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    blockedFile.write("blocked");
    blockedFile.close();
    g_blockDirsOfUser = QStringList{dir.filePath(QStringLiteral("blocked"))};
    QJsonArray similarityResults;
    similarityResults.append(QJsonObject{{QStringLiteral("filepath"), blockedPath}});
    similarityResults.append(QJsonObject{{QStringLiteral("filepath"), allowedPath}});
    similarityResults.append(QJsonObject{{QStringLiteral("filepath"), dir.filePath(QStringLiteral("missing.txt"))}});
    g_similarityResult = QJsonDocument(similarityResults).toJson(QJsonDocument::Compact);

    AiSearchPlugin::uniqueSymbolAi = 20;
    AiSearch search(&queue, 20, QStringLiteral("semantic"));
    search.run();
    QCOMPARE(g_similarityCalls, 2);
    QCOMPARE(queue.length(), 1);
    const SearchPluginIface::ResultInfo allowedResult = queue.tryDequeue();
    QCOMPARE(allowedResult.searchID, static_cast<size_t>(20));
    QCOMPARE(allowedResult.actionKey, allowedPath);
    QCOMPARE(allowedResult.type, 0);

    AiSearchPlugin::uniqueSymbolAi = 22;
    AiSearch staleSearch(&queue, 21, QStringLiteral("semantic"));
    staleSearch.run();
    QVERIFY(queue.isEmpty());
}

void AiSearchPluginWhiteboxTest::timerConfigActionsAndClickableLabelCoverAsyncBranches()
{
    AiSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> queue;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString launcherPath = dir.filePath(QStringLiteral("ukui-control-center"));
    const QString outputPath = dir.filePath(QStringLiteral("ai-control-center.txt"));
    QFile launcher(launcherPath);
    QVERIFY(launcher.open(QIODevice::WriteOnly | QIODevice::Truncate));
    launcher.write("#!/bin/sh\n");
    launcher.write("printf '%s ' \"$@\" > \"$AI_SEARCH_CONTROL_CENTER_OUT\"\n");
    launcher.close();
    QVERIFY(QFile::setPermissions(launcherPath,
                                  QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
    const QByteArray oldPath = qgetenv("PATH");
    const QByteArray oldOut = qgetenv("AI_SEARCH_CONTROL_CENTER_OUT");
    qputenv("PATH", QFile::encodeName(dir.path() + QStringLiteral(":") + QString::fromLocal8Bit(oldPath)));
    qputenv("AI_SEARCH_CONTROL_CENTER_OUT", QFile::encodeName(outputPath));

    g_featureSupported = true;
    AiSearch::s_supportState = false;
    AiSearch::s_aiNotSupported = true;
    g_aiSubsystemInited = false;
    const size_t searchId = plugin.KeywordSearch(QStringLiteral("semantic timer"), &queue);
    QVERIFY(plugin.m_timer->isActive());
    QTRY_VERIFY_WITH_TIMEOUT(!plugin.m_timer->isActive(), 1500);
    plugin.m_pool.waitForDone();
    QTRY_COMPARE_WITH_TIMEOUT(queue.length(), 1, 500);
    const SearchPluginIface::ResultInfo configResult = queue.tryDequeue();
    QCOMPARE(configResult.searchID, searchId);
    QCOMPARE(configResult.type, 1);

    SearchPluginIface::ResultInfo result;
    result.icon = QIcon::fromTheme(QStringLiteral("text-x-generic"));
    result.name = QStringLiteral("config-target.txt");
    result.actionKey = QStringLiteral("/tmp/config-target.txt");
    result.description = QVector<SearchPluginIface::DescriptionInfo>()
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Path:"), result.actionKey}
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Modified time:"), QStringLiteral("2024/01/02 03:04:05")};

    g_aiSubsystemInited = true;
    g_aiIndexEnabled = true;
    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);

    g_aiSubsystemInited = false;
    plugin.openAction(0, result.actionKey, 1);
    QCoreApplication::processEvents();
    QTest::qWait(100);
    QCoreApplication::processEvents();
    if (QFile::exists(outputPath)) {
        QVERIFY(readFileText(outputPath).contains(QStringLiteral("aisubsystem")));
    }

    g_aiSubsystemInited = true;
    g_aiIndexEnabled = false;
    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);
    QFile::remove(outputPath);
    clickActionLabel(plugin.m_clickableActionLabelWithoutModel);
    QCoreApplication::processEvents();
    QTest::qWait(100);
    QCoreApplication::processEvents();
    if (QFile::exists(outputPath)) {
        QVERIFY(readFileText(outputPath).contains(QStringLiteral("search")));
    }

    g_aiIndexEnabled = true;
    g_featureStatus = AVAILABLE;
    g_featureStatusSuccess = true;
    plugin.openAction(0, result.actionKey, 1);
    QCoreApplication::processEvents();
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("openFile"));

    qputenv("PATH", oldPath);
    if (oldOut.isEmpty()) {
        qunsetenv("AI_SEARCH_CONTROL_CENTER_OUT");
    } else {
        qputenv("AI_SEARCH_CONTROL_CENTER_OUT", oldOut);
    }
}

void AiSearchPluginWhiteboxTest::thumbnailMismatchAndSessionRecoveryFailureBranches()
{
    AiSearchPlugin plugin;
    SearchPluginIface::ResultInfo result;
    result.icon = QIcon::fromTheme(QStringLiteral("text-x-generic"));
    result.name = QStringLiteral("thumbnail-mismatch.txt");
    result.actionKey = QStringLiteral("/tmp/thumbnail-mismatch.txt");
    result.description = QVector<SearchPluginIface::DescriptionInfo>()
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Path:"), result.actionKey}
            << SearchPluginIface::DescriptionInfo{QStringLiteral("Modified time:"), QStringLiteral("2024/01/02 03:04:05")};

    g_aiSubsystemInited = true;
    g_aiIndexEnabled = true;
    g_thumbnailImage = QImage(32, 16, QImage::Format_ARGB32_Premultiplied);
    g_thumbnailImage.fill(Qt::blue);
    g_thumbnailReadyUrlOverride = QStringLiteral("/tmp/other-target.txt");
    QCOMPARE(plugin.detailPage(result), plugin.m_detailPage);
    plugin.m_thumbnailPool.waitForDone();
    QCoreApplication::processEvents();
    QVERIFY(!plugin.m_useImage);
    QCOMPARE(plugin.m_contentDetailLyt->contentsMargins().top(), 50);

    DataQueue<SearchPluginIface::ResultInfo> queue;
    g_featureSupported = true;
    AiSearch::s_supportState = false;
    AiSearch::s_aiNotSupported = true;
    AiSearch::destroySession();
    AiSearch::s_failed = false;
    g_featureStatus = AVAILABLE;
    g_featureStatusSuccess = true;
    g_similaritySessionErrors = 1;
    g_createSessionFailAtCall = g_createCalls + 2;
    AiSearchPlugin::uniqueSymbolAi = 30;
    AiSearch search(&queue, 30, QStringLiteral("semantic"));
    search.run();
    QCOMPARE(g_similarityCalls, 1);
    QCOMPARE(g_destroyResultCalls, 1);
    QVERIFY(queue.isEmpty());
    QVERIFY(AiSearch::s_failed);
}

void AiSearchPluginWhiteboxTest::sessionAndModelReadyCoverSdkBranches()
{
    g_createSessionFails = true;
    QVERIFY(!AiSearch::createSession());
    QVERIFY(AiSearch::s_failed);

    g_createSessionFails = false;
    AiSearch::s_failed = false;
    QVERIFY(AiSearch::createSession());
    QVERIFY(AiSearch::s_session != nullptr);

    g_featureStatusSuccess = true;
    g_featureStatus = AVAILABLE;
    QVERIFY(AiSearch::modelReady());

    g_featureStatus = NOT_INSTALLED;
    QVERIFY(!AiSearch::modelReady());

    g_featureStatusSuccess = false;
    QVERIFY(!AiSearch::modelReady());

    AiSearch::destroySession();
    AiSearch::s_failed = false;
    g_createSessionFails = true;
    QVERIFY(!AiSearch::modelReady());

    g_featureSupported = false;
    AiSearch::s_supportState = false;
    AiSearch::s_aiNotSupported = true;
    QVERIFY(AiSearch::notSupported());
    g_featureSupported = true;
    QVERIFY(AiSearch::notSupported());
}

void AiSearchPluginWhiteboxTest::keywordSearchRunsWorkerWithDifferentKeywords()
{
    resetStubState();
    g_aiSubsystemInited = true;
    g_featureSupported = true;

    AiSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> output;

    plugin.KeywordSearch(QStringLiteral("hello"), &output);
    QTest::qWait(100);
    plugin.stopSearch();

    plugin.KeywordSearch(QStringLiteral("test query"), &output);
    QTest::qWait(100);
    plugin.stopSearch();

    plugin.KeywordSearch(QString(), &output);
    QTest::qWait(100);
    plugin.stopSearch();
}

void AiSearchPluginWhiteboxTest::stopSearchInterruptsActiveSearch()
{
    resetStubState();
    g_aiSubsystemInited = true;
    g_featureSupported = true;

    AiSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> output;

    plugin.KeywordSearch(QStringLiteral("interrupt test"), &output);
    plugin.stopSearch();
    plugin.stopSearch(); // double stop
}

void AiSearchPluginWhiteboxTest::detailPageEdgeCasesAndNullResultBranches()
{
    resetStubState();
    AiSearchPlugin plugin;

    SearchPluginIface::ResultInfo ri;
    ri.type = 0;
    ri.actionKey = QString();
    QWidget *page = plugin.detailPage(ri);
    QVERIFY(page != nullptr);

    SearchPluginIface::ResultInfo ri2;
    ri2.type = 1;
    ri2.actionKey = QStringLiteral("some-key");
    QWidget *page2 = plugin.detailPage(ri2);
    QVERIFY(page2 != nullptr);
}

QTEST_MAIN(AiSearchPluginWhiteboxTest)

#include "ai-search-plugin-whitebox-test.moc"
