#include <QtTest>
#include <QAbstractListModel>
#include <QAction>
#include <QDBusPendingCallWatcher>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>
#include <algorithm>

#ifdef signals
#undef signals
#define UKUI_MENU_RESTORE_QT_SIGNALS
#endif
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#ifdef UKUI_MENU_RESTORE_QT_SIGNALS
#define signals public
#undef UKUI_MENU_RESTORE_QT_SIGNALS
#endif

#define private public
#include "../recent-file-extension.h"
#undef private

using namespace UkuiMenu;

Q_DECLARE_METATYPE(UkuiMenu::RecentFile)
Q_DECLARE_METATYPE(QVector<UkuiMenu::RecentFile>)

namespace UkuiMenu {

class RecentFileProvider : public QObject
{
public:
    explicit RecentFileProvider(QObject *parent = nullptr);
    void dataProcess(QVector<RecentFile> &recentFiles);
    void getRecentData();
    void openFileByGFile(const QString &fileUrl);
};

class RecentFilesModel : public QAbstractListModel
{
public:
    explicit RecentFilesModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    QString getInfoId(const int &index);
    QStringList getAllInfoId();
    void updateData();
    void updateRecentFiles(QVector<RecentFile> recentFiles);
};

class GVFSRecentFileData
{
public:
    static int s_queryFileNum;
    static GFile *s_recentFileRootDir;
    static GFileMonitor *s_recentFileMonitor;

    static void loadRecentFileASync(RecentFileProvider *p_recentFileProvider);
    static void fileMonitor(RecentFileProvider *p_recentFileProvider);
    static void removeRecentFileByInfoId(const QString &infoId);
};

}

namespace {

constexpr int UriRole = Qt::UserRole;
constexpr int NameRole = Qt::UserRole + 1;
constexpr int IconRole = Qt::UserRole + 2;
constexpr int PathRole = Qt::UserRole + 3;
constexpr int DateRole = Qt::UserRole + 4;

enum class GioEnumerateMode
{
    Real,
    Error,
    Empty,
    Files,
    ParseError,
};

enum class AppInfoLookupMode
{
    Real,
    UriScheme,
    FileHandler,
    DefaultForType,
};

enum class QueryInfoMode
{
    Real,
    EmptyContentType,
    FastContentType,
};

GioEnumerateMode g_gioEnumerateMode = GioEnumerateMode::Real;
AppInfoLookupMode g_appInfoLookupMode = AppInfoLookupMode::Real;
QueryInfoMode g_queryInfoMode = QueryInfoMode::Real;
GFileMonitor *g_fakeMonitor = nullptr;
GCallback g_monitorChangedCallback = nullptr;
gpointer g_monitorChangedData = nullptr;
bool g_monitorShouldError = false;
bool g_fileDeleteShouldSucceed = false;
bool g_fileNewForUriShouldFail = false;
bool g_appLaunchShouldSucceed = false;
QString g_fakeDesktopFilePath;

QAbstractListModel *recentFilesModel(RecentFileExtension *extension)
{
    return extension->findChild<QAbstractListModel *>();
}

QVector<RecentFile> buildRecentFiles()
{
    RecentFile latest;
    latest.accessTime = 1700000100;
    latest.uri = "file:///tmp/newer.txt";
    latest.name = "newer.txt";
    latest.icon = "text-plain";
    latest.infoId = "recent:///newer";

    RecentFile older;
    older.accessTime = 1700000000;
    older.uri = "file:///tmp/older.txt";
    older.name = "older.txt";
    older.icon = "text-x-generic";
    older.infoId = "recent:///older";

    return {latest, older};
}

void flushEvents()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

int pendingWatcherCount(RecentFileExtension *extension)
{
    return extension->findChildren<QDBusPendingCallWatcher *>().count();
}

QString writeTestIcon(const QString &baseDir, const QString &fileName)
{
    const QString filePath = baseDir + "/" + fileName;
    QImage iconImage(16, 16, QImage::Format_ARGB32);
    iconImage.fill(Qt::red);
    return iconImage.save(filePath) ? filePath : QString();
}

QString writeDesktopFile(const QString &baseDir, const QString &fileName, const QString &iconPath)
{
    const QString applicationsPath = baseDir + "/applications";
    if (!QDir().mkpath(applicationsPath)) {
        return {};
    }

    const QString filePath = applicationsPath + "/" + fileName;
    QFile desktopFile(filePath);
    if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return {};
    }

    const QByteArray desktopContent = "[Desktop Entry]\n"
                                      "Type=Application\n"
                                      "Name=Wrapped Recent File Test\n"
                                      "Exec=/bin/true %U\n"
                                      "Icon=" + iconPath.toUtf8() + "\n";
    if (desktopFile.write(desktopContent) != desktopContent.size()) {
        return {};
    }

    desktopFile.close();
    return filePath;
}

QString prepareFakeApplication(QTemporaryDir &temporaryDir)
{
    if (!temporaryDir.isValid()) {
        return {};
    }

    qputenv("HOME", temporaryDir.path().toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    const QString applicationsPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    if (!QDir().mkpath(applicationsPath)) {
        return {};
    }

    const QString iconPath = writeTestIcon(temporaryDir.path(), "wrapped-handler.png");
    if (iconPath.isEmpty()) {
        return {};
    }

    g_fakeDesktopFilePath = writeDesktopFile(QFileInfo(applicationsPath).absolutePath(),
                                             "wrapped-handler.desktop",
                                             iconPath);
    return g_fakeDesktopFilePath;
}

void writeMimeAppsList(const QString &content)
{
    const QString mimeAppsListPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/mimeapps.list";
    QVERIFY(QDir().mkpath(QFileInfo(mimeAppsListPath).absolutePath()));

    QFile mimeAppsList(mimeAppsListPath);
    QVERIFY(mimeAppsList.open(QIODevice::WriteOnly | QIODevice::Text));
    QCOMPARE(mimeAppsList.write(content.toUtf8()), qint64(content.toUtf8().size()));
    mimeAppsList.close();
}

GFileEnumerator *fakeEnumerator()
{
    return reinterpret_cast<GFileEnumerator *>(g_object_new(G_TYPE_OBJECT, nullptr));
}

GFileInfo *fakeInfo(const char *targetUri, const char *displayName, guint64 accessTime, const char *iconName)
{
    GFileInfo *info = g_file_info_new();
    if (targetUri) {
        g_file_info_set_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI, targetUri);
    }
    if (displayName) {
        g_file_info_set_display_name(info, displayName);
    }
    g_file_info_set_attribute_uint64(info, G_FILE_ATTRIBUTE_TIME_MODIFIED, accessTime);

    if (iconName) {
        GIcon *icon = g_themed_icon_new(iconName);
        g_file_info_set_icon(info, icon);
        g_object_unref(icon);
    }

    return info;
}

GList *fakeRecentFileList()
{
    GList *list = nullptr;
    list = g_list_append(list, fakeInfo("file:///tmp/wrapped-newer.txt", "wrapped-newer.txt", 1700000200, "definitely-missing-recent-file-newer-icon"));
    list = g_list_append(list, fakeInfo("file:///tmp/wrapped-older.txt", "wrapped-older.txt", 1700000000, "text-plain"));
    list = g_list_append(list, fakeInfo(nullptr, nullptr, 1700000300, nullptr));
    list = g_list_append(list, fakeInfo("file:///tmp/wrapped-unknown-icon.txt", "wrapped-unknown-icon.txt", 1700000150, "definitely-missing-recent-file-test-icon"));
    return list;
}

GAppInfo *fakeAppInfo()
{
    if (g_fakeDesktopFilePath.isEmpty()) {
        return nullptr;
    }

    GDesktopAppInfo *desktopInfo = g_desktop_app_info_new_from_filename(g_fakeDesktopFilePath.toUtf8().constData());
    return desktopInfo ? G_APP_INFO(desktopInfo) : nullptr;
}

void resetGioWrapper()
{
    g_gioEnumerateMode = GioEnumerateMode::Real;
    g_appInfoLookupMode = AppInfoLookupMode::Real;
    g_queryInfoMode = QueryInfoMode::Real;
    g_monitorShouldError = false;
    g_fileDeleteShouldSucceed = false;
    g_fileNewForUriShouldFail = false;
    g_appLaunchShouldSucceed = false;
}

void triggerMonitorChanged(GFileMonitorEvent eventType)
{
    QVERIFY(g_monitorChangedCallback != nullptr);
    auto callback = reinterpret_cast<void (*)(GFileMonitor *, GFile *, GFile *, GFileMonitorEvent, RecentFileProvider *)>(
        g_monitorChangedCallback);
    callback(g_fakeMonitor, nullptr, nullptr, eventType, static_cast<RecentFileProvider *>(g_monitorChangedData));
}

}

extern "C" {

GFileMonitor *__real_g_file_monitor_directory(GFile *file,
                                              GFileMonitorFlags flags,
                                              GCancellable *cancellable,
                                              GError **error);
GFile *__real_g_file_new_for_uri(const char *uri);
gulong __real_g_signal_connect_data(gpointer instance,
                                    const gchar *detailedSignal,
                                    GCallback handler,
                                    gpointer data,
                                    GClosureNotify destroyData,
                                    GConnectFlags connectFlags);
void __real_g_file_enumerate_children_async(GFile *file,
                                            const char *attributes,
                                            GFileQueryInfoFlags flags,
                                            int ioPriority,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer userData);
GFileEnumerator *__real_g_file_enumerate_children_finish(GFile *file,
                                                         GAsyncResult *result,
                                                         GError **error);
void __real_g_file_enumerator_next_files_async(GFileEnumerator *enumerator,
                                               int numFiles,
                                               int ioPriority,
                                               GCancellable *cancellable,
                                               GAsyncReadyCallback callback,
                                               gpointer userData);
GList *__real_g_file_enumerator_next_files_finish(GFileEnumerator *enumerator,
                                                  GAsyncResult *result,
                                                  GError **error);
GFile *__real_g_file_enumerator_get_child(GFileEnumerator *enumerator, GFileInfo *info);
GAppInfo *__real_g_app_info_get_default_for_uri_scheme(const char *uriScheme);
GAppInfo *__real_g_file_query_default_handler(GFile *file, GCancellable *cancellable, GError **error);
GFileInfo *__real_g_file_query_info(GFile *file,
                                    const char *attributes,
                                    GFileQueryInfoFlags flags,
                                    GCancellable *cancellable,
                                    GError **error);
GAppInfo *__real_g_app_info_get_default_for_type(const char *contentType, gboolean mustSupportUris);
GDesktopAppInfo *__real_g_desktop_app_info_new(const char *desktopId);
gboolean __real_g_app_info_launch(GAppInfo *appinfo,
                                  GList *files,
                                  GAppLaunchContext *launchContext,
                                  GError **error);
gboolean __real_g_file_delete(GFile *file, GCancellable *cancellable, GError **error);

GFileMonitor *__wrap_g_file_monitor_directory(GFile *file,
                                              GFileMonitorFlags flags,
                                              GCancellable *cancellable,
                                              GError **error)
{
    Q_UNUSED(file)
    Q_UNUSED(flags)
    Q_UNUSED(cancellable)

    if (error) {
        *error = nullptr;
    }

    if (g_monitorShouldError) {
        if (error) {
            *error = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_FAILED, "wrapped monitor error");
        }
        return nullptr;
    }

    if (!g_fakeMonitor) {
        g_fakeMonitor = reinterpret_cast<GFileMonitor *>(g_object_new(G_TYPE_OBJECT, nullptr));
    }

    return g_fakeMonitor;
}

GFile *__wrap_g_file_new_for_uri(const char *uri)
{
    if (g_fileNewForUriShouldFail) {
        Q_UNUSED(uri)
        return nullptr;
    }

    return __real_g_file_new_for_uri(uri);
}

gulong __wrap_g_signal_connect_data(gpointer instance,
                                    const gchar *detailedSignal,
                                    GCallback handler,
                                    gpointer data,
                                    GClosureNotify destroyData,
                                    GConnectFlags connectFlags)
{
    if (instance == g_fakeMonitor && qstrcmp(detailedSignal, "changed") == 0) {
        g_monitorChangedCallback = handler;
        g_monitorChangedData = data;
        return 1;
    }

    return __real_g_signal_connect_data(instance, detailedSignal, handler, data, destroyData, connectFlags);
}

void __wrap_g_file_enumerate_children_async(GFile *file,
                                            const char *attributes,
                                            GFileQueryInfoFlags flags,
                                            int ioPriority,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer userData)
{
    if (g_gioEnumerateMode == GioEnumerateMode::Real) {
        __real_g_file_enumerate_children_async(file, attributes, flags, ioPriority, cancellable, callback, userData);
        return;
    }

    callback(G_OBJECT(file), nullptr, userData);
}

GFileEnumerator *__wrap_g_file_enumerate_children_finish(GFile *file, GAsyncResult *result, GError **error)
{
    if (g_gioEnumerateMode == GioEnumerateMode::Real) {
        return __real_g_file_enumerate_children_finish(file, result, error);
    }

    Q_UNUSED(file)
    Q_UNUSED(result)

    if (g_gioEnumerateMode == GioEnumerateMode::Error) {
        if (error) {
            *error = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_FAILED, "wrapped enumerate error");
        }
        return nullptr;
    }

    if (error) {
        *error = nullptr;
    }

    return fakeEnumerator();
}

void __wrap_g_file_enumerator_next_files_async(GFileEnumerator *enumerator,
                                               int numFiles,
                                               int ioPriority,
                                               GCancellable *cancellable,
                                               GAsyncReadyCallback callback,
                                               gpointer userData)
{
    if (g_gioEnumerateMode == GioEnumerateMode::Real) {
        __real_g_file_enumerator_next_files_async(enumerator, numFiles, ioPriority, cancellable, callback, userData);
        return;
    }

    Q_UNUSED(numFiles)
    Q_UNUSED(ioPriority)
    Q_UNUSED(cancellable)
    callback(G_OBJECT(enumerator), nullptr, userData);
}

GList *__wrap_g_file_enumerator_next_files_finish(GFileEnumerator *enumerator, GAsyncResult *result, GError **error)
{
    if (g_gioEnumerateMode == GioEnumerateMode::Real) {
        return __real_g_file_enumerator_next_files_finish(enumerator, result, error);
    }

    Q_UNUSED(enumerator)
    Q_UNUSED(result)

    if (g_gioEnumerateMode == GioEnumerateMode::ParseError) {
        if (error) {
            *error = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_FAILED, "wrapped parse error");
        }
        return nullptr;
    }

    if (error) {
        *error = nullptr;
    }

    return g_gioEnumerateMode == GioEnumerateMode::Files ? fakeRecentFileList() : nullptr;
}

GFile *__wrap_g_file_enumerator_get_child(GFileEnumerator *enumerator, GFileInfo *info)
{
    if (g_gioEnumerateMode == GioEnumerateMode::Real) {
        return __real_g_file_enumerator_get_child(enumerator, info);
    }

    Q_UNUSED(enumerator)

    const char *targetUri = g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
    return g_file_new_for_uri(targetUri ? targetUri : "file:///tmp/wrapped-recent-file.txt");
}

GAppInfo *__wrap_g_app_info_get_default_for_uri_scheme(const char *uriScheme)
{
    if (g_appInfoLookupMode == AppInfoLookupMode::UriScheme) {
        Q_UNUSED(uriScheme)
        return fakeAppInfo();
    }

    if (g_appInfoLookupMode != AppInfoLookupMode::Real) {
        return nullptr;
    }

    return __real_g_app_info_get_default_for_uri_scheme(uriScheme);
}

GAppInfo *__wrap_g_file_query_default_handler(GFile *file, GCancellable *cancellable, GError **error)
{
    if (g_appInfoLookupMode == AppInfoLookupMode::FileHandler) {
        Q_UNUSED(file)
        Q_UNUSED(cancellable)
        if (error) {
            *error = nullptr;
        }
        return fakeAppInfo();
    }

    return __real_g_file_query_default_handler(file, cancellable, error);
}

GFileInfo *__wrap_g_file_query_info(GFile *file,
                                    const char *attributes,
                                    GFileQueryInfoFlags flags,
                                    GCancellable *cancellable,
                                    GError **error)
{
    if (g_queryInfoMode == QueryInfoMode::FastContentType || g_queryInfoMode == QueryInfoMode::EmptyContentType) {
        Q_UNUSED(file)
        Q_UNUSED(attributes)
        Q_UNUSED(flags)
        Q_UNUSED(cancellable)
        if (error) {
            *error = nullptr;
        }

        GFileInfo *info = g_file_info_new();
        g_file_info_set_content_type(info, "");
        if (g_queryInfoMode == QueryInfoMode::FastContentType) {
            g_file_info_set_attribute_string(info, "standard::fast-content-type", "text/plain");
        }
        return info;
    }

    return __real_g_file_query_info(file, attributes, flags, cancellable, error);
}

GAppInfo *__wrap_g_app_info_get_default_for_type(const char *contentType, gboolean mustSupportUris)
{
    if (g_appInfoLookupMode == AppInfoLookupMode::DefaultForType) {
        Q_UNUSED(contentType)
        Q_UNUSED(mustSupportUris)
        return fakeAppInfo();
    }

    return __real_g_app_info_get_default_for_type(contentType, mustSupportUris);
}

GDesktopAppInfo *__wrap_g_desktop_app_info_new(const char *desktopId)
{
    if (g_appInfoLookupMode == AppInfoLookupMode::DefaultForType) {
        Q_UNUSED(desktopId)
        return g_fakeDesktopFilePath.isEmpty()
                   ? nullptr
                   : g_desktop_app_info_new_from_filename(g_fakeDesktopFilePath.toUtf8().constData());
    }

    return __real_g_desktop_app_info_new(desktopId);
}

gboolean __wrap_g_app_info_launch(GAppInfo *appinfo,
                                  GList *files,
                                  GAppLaunchContext *launchContext,
                                  GError **error)
{
    if (g_appInfoLookupMode != AppInfoLookupMode::Real) {
        Q_UNUSED(appinfo)
        Q_UNUSED(files)
        Q_UNUSED(launchContext)
        if (error) {
            *error = nullptr;
        }
        return g_appLaunchShouldSucceed;
    }

    return __real_g_app_info_launch(appinfo, files, launchContext, error);
}

gboolean __wrap_g_file_delete(GFile *file, GCancellable *cancellable, GError **error)
{
    if (g_fileDeleteShouldSucceed) {
        Q_UNUSED(file)
        Q_UNUSED(cancellable)
        if (error) {
            *error = nullptr;
        }
        return TRUE;
    }

    return __real_g_file_delete(file, cancellable, error);
}

}

class recentFile_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void cleanupTestCase();

    void test_RecentFileExtension();
    void test_ModelDataAndSignals();
    void test_ReceiveAndContextMenu();
    void test_MenuActionsAndOpenFile();
    void test_HelperMethods();
    void test_GVFSAsyncLoadPaths();
    void test_ProviderDataProcessAndOpenFile();
    void test_OpenFileByProvider();
    void test_Plugin();

private:
    RecentFileExtension *m_extension = nullptr;
};

void recentFile_test::initTestCase()
{
    qRegisterMetaType<RecentFile>("RecentFile");
    qRegisterMetaType<QVector<RecentFile>>("QVector<RecentFile>");

    m_extension = new RecentFileExtension();
    QVERIFY(m_extension != nullptr);
    QVERIFY(m_extension->m_recentFilesProviderThread != nullptr);
    QVERIFY(m_extension->m_recentFilesProviderThread->isRunning());
}

void recentFile_test::cleanup()
{
    QVERIFY(m_extension != nullptr);
    resetGioWrapper();

    if (m_extension->m_contextMenu) {
        m_extension->m_contextMenu->close();
        flushEvents();
    }

    QAbstractListModel *model = recentFilesModel(m_extension);
    QVERIFY(model != nullptr);

    QVector<RecentFile> recentFiles;
    QVERIFY(QMetaObject::invokeMethod(model,
                                      "updateRecentFiles",
                                      Qt::DirectConnection,
                                      Q_ARG(QVector<RecentFile>, recentFiles)));
}

void recentFile_test::cleanupTestCase()
{
    delete m_extension;
    m_extension = nullptr;
}

void recentFile_test::test_RecentFileExtension()
{
    QVERIFY(m_extension != nullptr);

    QCOMPARE(m_extension->metadata().value(WidgetMetadata::Id), "recent-file");
    QCOMPARE(m_extension->metadata().value(WidgetMetadata::Version), "1.0.0");
    QCOMPARE(m_extension->metadata().value(WidgetMetadata::Description), "recent-file");
    QCOMPARE(m_extension->metadata().value(WidgetMetadata::Main), "qrc:///extensions/RecentFileExtension.qml");
    QCOMPARE(m_extension->index(), 1);

    QVariantMap exportedData = m_extension->data();
    QVERIFY(exportedData.contains("recentFilesModel"));

    QVERIFY(m_extension->m_recentFilesModel != nullptr);
    QVERIFY(m_extension->m_recentFileProvider != nullptr);
    QVERIFY(recentFilesModel(m_extension) != nullptr);
}

void recentFile_test::test_ModelDataAndSignals()
{
    QVERIFY(m_extension != nullptr);

    QAbstractListModel *model = recentFilesModel(m_extension);
    QVERIFY(model != nullptr);

    QSignalSpy resetSpy(model, &QAbstractItemModel::modelReset);
    QVector<RecentFile> recentFiles = buildRecentFiles();
    QVERIFY(QMetaObject::invokeMethod(model,
                                      "updateRecentFiles",
                                      Qt::DirectConnection,
                                      Q_ARG(QVector<RecentFile>, recentFiles)));
    QCOMPARE(resetSpy.count(), 1);

    QCOMPARE(model->rowCount(), 2);

    QModelIndex firstIndex = model->index(0, 0);
    QVERIFY(firstIndex.isValid());
    QCOMPARE(model->data(firstIndex, UriRole).toString(), QString("file:///tmp/newer.txt"));
    QCOMPARE(model->data(firstIndex, NameRole).toString(), QString("newer.txt"));
    QCOMPARE(model->data(firstIndex, IconRole).toString(), QString("text-plain"));
    QCOMPARE(model->data(firstIndex, PathRole).toString(), QString("/tmp"));

    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
    QVERIFY(QMetaObject::invokeMethod(model,
                                      "setDateFormat",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QString("yyyy-MM-dd"))));
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(model->data(firstIndex, DateRole).toString(),
             QDateTime::fromSecsSinceEpoch(recentFiles.first().accessTime).toString(QStringLiteral("yyyy-MM-dd")));

    QList<QVariant> signalArguments = dataChangedSpy.takeFirst();
    QCOMPARE(signalArguments.at(2).value<QVector<int>>(), QVector<int>({DateRole}));

    QSignalSpy updateSpy(model, SIGNAL(updateRecentData()));
    QVERIFY(QMetaObject::invokeMethod(model, "updateData", Qt::DirectConnection));
    QCOMPARE(updateSpy.count(), 1);

    QVERIFY(!model->data(QModelIndex(), UriRole).isValid());
    QVERIFY(!model->data(model->index(10, 0), UriRole).isValid());
    QVERIFY(!model->data(firstIndex, Qt::DisplayRole).isValid());

    const QHash<int, QByteArray> roles = model->roleNames();
    QCOMPARE(roles.value(UriRole), QByteArray("uri"));
    QCOMPARE(roles.value(NameRole), QByteArray("name"));
    QCOMPARE(roles.value(IconRole), QByteArray("icon"));
    QCOMPARE(roles.value(PathRole), QByteArray("path"));
    QCOMPARE(roles.value(DateRole), QByteArray("date"));

    auto *typedModel = static_cast<RecentFilesModel *>(model);
    QCOMPARE(typedModel->getInfoId(0), QString("recent:///newer"));
    QCOMPARE(typedModel->getAllInfoId(), QStringList({"recent:///newer", "recent:///older"}));

    QVector<RecentFile> emptyRecentFiles;
    QVERIFY(QMetaObject::invokeMethod(model,
                                      "updateRecentFiles",
                                      Qt::DirectConnection,
                                      Q_ARG(QVector<RecentFile>, emptyRecentFiles)));
    QVERIFY(!model->data(model->index(0, 0), UriRole).isValid());
    QCOMPARE(typedModel->getAllInfoId(), QStringList());
}

void recentFile_test::test_ReceiveAndContextMenu()
{
    QVERIFY(m_extension != nullptr);

    QAbstractListModel *model = recentFilesModel(m_extension);
    QVERIFY(model != nullptr);
    QVector<RecentFile> recentFiles = buildRecentFiles();
    QVERIFY(QMetaObject::invokeMethod(model,
                                      "updateRecentFiles",
                                      Qt::DirectConnection,
                                      Q_ARG(QVector<RecentFile>, recentFiles)));

    m_extension->receive({{"action", "contextMenu"}, {"url", "invalid://not-exist"}, {"index", 0}});
    QVERIFY(m_extension->m_contextMenu != nullptr);
    QCOMPARE(m_extension->m_contextMenu->actions().count(), 5);

    QList<QAction *> actions = m_extension->m_contextMenu->actions();
    QVERIFY(actions.at(0) != nullptr);
    QVERIFY(actions.at(1) != nullptr);
    QVERIFY(actions.at(2)->isSeparator());
    QVERIFY(actions.at(3) != nullptr);
    QVERIFY(actions.at(4) != nullptr);
    QVERIFY(!actions.at(0)->text().isEmpty());
    QVERIFY(!actions.at(1)->text().isEmpty());
    QVERIFY(!actions.at(3)->text().isEmpty());
    QVERIFY(!actions.at(4)->text().isEmpty());

    m_extension->receive({{"action", "contextMenu"}, {"url", "invalid://not-exist"}, {"index", 0}});
    flushEvents();
    QTRY_VERIFY(m_extension->m_contextMenu.isNull());

    m_extension->receive({{"action", "emptyAreaContextMenu"}});
    QVERIFY(m_extension->m_contextMenu != nullptr);
    QCOMPARE(m_extension->m_contextMenu->actions().count(), 1);
    QVERIFY(m_extension->m_contextMenu->actions().at(0) != nullptr);
    QVERIFY(!m_extension->m_contextMenu->actions().at(0)->text().isEmpty());

    m_extension->receive({{"action", "closeMenu"}});
    flushEvents();
    QTRY_VERIFY(m_extension->m_contextMenu.isNull());

    m_extension->receive({{"action", "emptyAreaContextMenu"}});
    QVERIFY(m_extension->m_contextMenu != nullptr);
    m_extension->receive({{"action", "emptyAreaContextMenu"}});
    flushEvents();
    QTRY_VERIFY(m_extension->m_contextMenu.isNull());

    m_extension->receive({{"action", "closeMenu"}});
    flushEvents();
    QTRY_VERIFY(m_extension->m_contextMenu.isNull());
}

void recentFile_test::test_MenuActionsAndOpenFile()
{
    QVERIFY(m_extension != nullptr);

    QAbstractListModel *model = recentFilesModel(m_extension);
    QVERIFY(model != nullptr);

    QVector<RecentFile> recentFiles = buildRecentFiles();
    QVERIFY(QMetaObject::invokeMethod(model,
                                      "updateRecentFiles",
                                      Qt::DirectConnection,
                                      Q_ARG(QVector<RecentFile>, recentFiles)));

    const int watcherCountBeforeReceive = pendingWatcherCount(m_extension);

    m_extension->receive({{"action", "openFile"}, {"url", "file:///tmp/direct-open.txt"}});
    QTRY_VERIFY(pendingWatcherCount(m_extension) > watcherCountBeforeReceive);
    flushEvents();

    const int watcherCountBeforePrivateOpen = pendingWatcherCount(m_extension);
    m_extension->openFile("file:///tmp/direct-private-open.txt");
    QTRY_VERIFY(pendingWatcherCount(m_extension) > watcherCountBeforePrivateOpen);
    auto watchers = m_extension->findChildren<QDBusPendingCallWatcher *>();
    QVERIFY(!watchers.isEmpty());
    Q_EMIT watchers.last()->finished(watchers.last());
    flushEvents();

    m_extension->receive({{"action", "contextMenu"}, {"url", "file:///tmp/from-menu.txt"}, {"index", 0}});
    QVERIFY(m_extension->m_contextMenu != nullptr);

    QList<QAction *> actions = m_extension->m_contextMenu->actions();
    QCOMPARE(actions.count(), 5);

    const int watcherCountBeforeTrigger = pendingWatcherCount(m_extension);
    actions.at(0)->trigger();
    QTRY_VERIFY(pendingWatcherCount(m_extension) > watcherCountBeforeTrigger);
    flushEvents();

    actions.at(1)->trigger();
    actions.at(3)->trigger();
    actions.at(4)->trigger();

    m_extension->m_contextMenu->close();
    flushEvents();
    QTRY_VERIFY(m_extension->m_contextMenu.isNull());

    m_extension->receive({{"action", "emptyAreaContextMenu"}});
    QVERIFY(m_extension->m_contextMenu != nullptr);
    QCOMPARE(m_extension->m_contextMenu->actions().count(), 1);
    m_extension->m_contextMenu->actions().at(0)->trigger();

    flushEvents();
}

void recentFile_test::test_HelperMethods()
{
    QVERIFY(m_extension != nullptr);

    QCOMPARE(m_extension->getDefaultAppForUrl("invalid://not-exist"), QString());
    QCOMPARE(m_extension->getAppIcon(QString()).isNull(), QIcon::fromTheme("unk-app-symbolic").isNull());
    m_extension->getDefaultAppForUrl(QUrl::fromLocalFile(QCoreApplication::applicationFilePath()).toString().toStdString());

    QTemporaryDir appHome;
    QVERIFY(!prepareFakeApplication(appHome).isEmpty());
    QVERIFY(!m_extension->getAppIcon(g_fakeDesktopFilePath).isNull());
    g_appInfoLookupMode = AppInfoLookupMode::UriScheme;
    QCOMPARE(m_extension->getDefaultAppForUrl("wrapped://handler/path"), g_fakeDesktopFilePath);

    g_appInfoLookupMode = AppInfoLookupMode::FileHandler;
    QCOMPARE(m_extension->getDefaultAppForUrl("file:///tmp/wrapped-handler.txt"), g_fakeDesktopFilePath);
    g_appInfoLookupMode = AppInfoLookupMode::Real;
    m_extension->getDefaultAppForUrl("");

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    QFile desktopFile(temporaryDir.filePath("test.desktop"));
    QVERIFY(desktopFile.open(QIODevice::WriteOnly | QIODevice::Text));
    const QByteArray desktopContent = "[Desktop Entry]\n"
                                      "Type=Application\n"
                                      "Name=Recent File Test\n"
                                      "Exec=/bin/true\n"
                                      "Icon=/definitely/not/exist/icon.png\n";
    QCOMPARE(desktopFile.write(desktopContent), desktopContent.size());
    desktopFile.close();

    QCOMPARE(m_extension->getAppIcon(desktopFile.fileName()).isNull(),
             QIcon::fromTheme("unk-app-symbolic").isNull());

    const QString iconFilePath = temporaryDir.filePath("test-icon.png");
    QImage iconImage(16, 16, QImage::Format_ARGB32);
    iconImage.fill(Qt::red);
    QVERIFY(iconImage.save(iconFilePath));

    QFile iconDesktopFile(temporaryDir.filePath("theme.desktop"));
    QVERIFY(iconDesktopFile.open(QIODevice::WriteOnly | QIODevice::Text));
    const QByteArray iconDesktopContent = "[Desktop Entry]\n"
                                          "Type=Application\n"
                                          "Name=Recent File Theme Test\n"
                                          "Exec=/bin/true\n"
                                          "Icon=" + iconFilePath.toUtf8() + "\n";
    QCOMPARE(iconDesktopFile.write(iconDesktopContent), iconDesktopContent.size());
    iconDesktopFile.close();

    QVERIFY(!m_extension->getAppIcon(iconDesktopFile.fileName()).isNull());

    QFile noIconDesktopFile(temporaryDir.filePath("no-icon.desktop"));
    QVERIFY(noIconDesktopFile.open(QIODevice::WriteOnly | QIODevice::Text));
    const QByteArray noIconDesktopContent = "[Desktop Entry]\n"
                                            "Type=Application\n"
                                            "Name=Recent File No Icon Test\n"
                                            "Exec=/bin/true\n";
    QCOMPARE(noIconDesktopFile.write(noIconDesktopContent), noIconDesktopContent.size());
    noIconDesktopFile.close();

    QCOMPARE(m_extension->getAppIcon(noIconDesktopFile.fileName()).isNull(),
             QIcon::fromTheme("unk-app-symbolic").isNull());
}

void recentFile_test::test_GVFSAsyncLoadPaths()
{
    QVERIFY(m_extension != nullptr);
    QVERIFY(m_extension->m_recentFileProvider != nullptr);

    QAbstractListModel *model = recentFilesModel(m_extension);
    QVERIFY(model != nullptr);

    g_gioEnumerateMode = GioEnumerateMode::Empty;
    m_extension->m_recentFileProvider->getRecentData();
    QCOMPARE(model->rowCount(), 0);

    g_gioEnumerateMode = GioEnumerateMode::Files;
    m_extension->m_recentFileProvider->getRecentData();
    QCOMPARE(model->rowCount(), 4);
    QCOMPARE(model->data(model->index(0, 0), IconRole).toString(), QString("text-plain"));
    QCOMPARE(model->data(model->index(1, 0), NameRole).toString(), QString("wrapped-newer.txt"));
    QCOMPARE(model->data(model->index(1, 0), IconRole).toString(), QString("text-plain"));
    QCOMPARE(model->data(model->index(2, 0), NameRole).toString(), QString("wrapped-unknown-icon.txt"));
    QCOMPARE(model->data(model->index(2, 0), IconRole).toString(), QString("text-plain"));
    QCOMPARE(model->data(model->index(3, 0), NameRole).toString(), QString("wrapped-older.txt"));
    QCOMPARE(model->data(model->index(3, 0), IconRole).toString(), QString("text-plain"));

    g_gioEnumerateMode = GioEnumerateMode::ParseError;
    m_extension->m_recentFileProvider->getRecentData();
    QCOMPARE(model->rowCount(), 4);

    g_gioEnumerateMode = GioEnumerateMode::Error;
    m_extension->m_recentFileProvider->getRecentData();
    QCOMPARE(model->rowCount(), 4);

    g_gioEnumerateMode = GioEnumerateMode::Empty;
    triggerMonitorChanged(G_FILE_MONITOR_EVENT_CREATED);
    QCOMPARE(model->rowCount(), 0);
    triggerMonitorChanged(G_FILE_MONITOR_EVENT_DELETED);
    triggerMonitorChanged(G_FILE_MONITOR_EVENT_CHANGED);

    GFile *savedRootDir = GVFSRecentFileData::s_recentFileRootDir;
    GVFSRecentFileData::s_recentFileRootDir = nullptr;
    m_extension->m_recentFileProvider->getRecentData();
    GVFSRecentFileData::s_recentFileRootDir = savedRootDir;

    GFileMonitor *savedMonitor = GVFSRecentFileData::s_recentFileMonitor;
    GVFSRecentFileData::s_recentFileMonitor = nullptr;
    g_monitorShouldError = true;
    GVFSRecentFileData::fileMonitor(m_extension->m_recentFileProvider);
    g_monitorShouldError = false;
    GVFSRecentFileData::s_recentFileMonitor = savedMonitor;
}

void recentFile_test::test_ProviderDataProcessAndOpenFile()
{
    QVector<RecentFile> recentFiles = buildRecentFiles();
    std::swap(recentFiles[0], recentFiles[1]);

    m_extension->m_recentFileProvider->dataProcess(recentFiles);
    QCOMPARE(recentFiles.size(), 2);
    QCOMPARE(recentFiles.first().name, QString("newer.txt"));
    QCOMPARE(recentFiles.last().name, QString("older.txt"));

    m_extension->m_recentFileProvider->getRecentData();
    flushEvents();

    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    qputenv("HOME", temporaryHome.path().toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    const QString testFilePath = temporaryHome.filePath("open-file.txt");
    QFile testFile(testFilePath);
    QVERIFY(testFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QCOMPARE(testFile.write("content"), qint64(7));
    testFile.close();

    QVERIFY(!prepareFakeApplication(temporaryHome).isEmpty());
    g_appInfoLookupMode = AppInfoLookupMode::DefaultForType;
    g_appLaunchShouldSucceed = false;

    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/mimeapps.list");
    m_extension->m_recentFileProvider->openFileByGFile(QUrl::fromLocalFile(testFilePath).toString());

    writeMimeAppsList("[Default Applications]\n");
    m_extension->m_recentFileProvider->openFileByGFile(QUrl::fromLocalFile(testFilePath).toString());

    writeMimeAppsList("[Default Applications]\ntext/plain=wrapped-handler.desktop\n");
    g_appLaunchShouldSucceed = true;
    m_extension->m_recentFileProvider->openFileByGFile(QUrl::fromLocalFile(testFilePath).toString());

    g_queryInfoMode = QueryInfoMode::FastContentType;
    g_appLaunchShouldSucceed = false;
    m_extension->m_recentFileProvider->openFileByGFile(QUrl::fromLocalFile(testFilePath).toString());

    g_queryInfoMode = QueryInfoMode::EmptyContentType;
    m_extension->m_recentFileProvider->openFileByGFile(QUrl::fromLocalFile(testFilePath).toString());

    g_fakeDesktopFilePath.clear();
    g_appInfoLookupMode = AppInfoLookupMode::DefaultForType;
    m_extension->m_recentFileProvider->openFileByGFile(QUrl::fromLocalFile(testFilePath).toString());

    g_queryInfoMode = QueryInfoMode::Real;
    g_appInfoLookupMode = AppInfoLookupMode::Real;
    g_appLaunchShouldSucceed = false;

    const QString missingFileUrl = QUrl::fromLocalFile(temporaryHome.filePath("missing-file.txt")).toString();
    m_extension->m_recentFileProvider->openFileByGFile(missingFileUrl);

    g_fileNewForUriShouldFail = true;
    m_extension->m_recentFileProvider->openFileByGFile("file:///tmp/new-for-uri-fails.txt");
    g_fileNewForUriShouldFail = false;

    g_fileDeleteShouldSucceed = true;
    GVFSRecentFileData::removeRecentFileByInfoId(QUrl::fromLocalFile(testFilePath).toString());
    g_fileDeleteShouldSucceed = false;
}

void recentFile_test::test_OpenFileByProvider()
{
    QVERIFY(m_extension != nullptr);
    QVERIFY(m_extension->m_recentFileProvider != nullptr);
    QSignalSpy openSpy(m_extension, &RecentFileExtension::openFileASync);
    Q_EMIT m_extension->openFileASync("file:///tmp/provider-open.txt");
    QTRY_COMPARE(openSpy.count(), 1);
}

void recentFile_test::test_Plugin()
{
    auto *plugin = new RecentFileExtensionPlugin;

    QCOMPARE(plugin->id(), QString("recent-file"));
    QCOMPARE(plugin->createContextMenuExtension(), nullptr);

    delete plugin;

    auto *stackPlugin = new RecentFileExtensionPlugin;
    WidgetExtension *widgetExtension = stackPlugin->createWidgetExtension();
    QVERIFY(widgetExtension != nullptr);
    QCOMPARE(widgetExtension->metadata().value(WidgetMetadata::Id), QString("recent-file"));
    auto *recentExtension = qobject_cast<RecentFileExtension *>(widgetExtension);
    QVERIFY(recentExtension != nullptr);
    GVFSRecentFileData::s_recentFileRootDir = g_file_new_for_uri("recent:///");
    GVFSRecentFileData::s_recentFileMonitor = nullptr;
    delete widgetExtension;
    GVFSRecentFileData::s_recentFileRootDir = g_file_new_for_uri("recent:///");
    GVFSRecentFileData::s_recentFileMonitor = reinterpret_cast<GFileMonitor *>(g_object_new(G_TYPE_OBJECT, nullptr));
    delete stackPlugin;
}

QTEST_MAIN(recentFile_test)

#include "recent-file-test.moc"
