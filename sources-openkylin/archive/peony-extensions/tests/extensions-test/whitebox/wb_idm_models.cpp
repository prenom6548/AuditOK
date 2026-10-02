/*
 * 白盒：SpacesFilesFetcher / FileInfoJob 等（不触发 ModelController 全局单例与忙等循环）。
 * 注意：SearchResultModel 构造会调用 ModelController::globalInstance()，在无完整服务环境下易死循环，故不在此测。
 */
#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QCoreApplication>
#include <QFile>
#include <QRandomGenerator>
#include <QSignalSpy>
#include <QThread>
#include <QUrl>

#include "model/spacesfilesfetcher.h"
#include "file/fileinfojob.h"

using namespace UKUI::IDM;

TEST(IdmModelsWhitebox, spaces_files_fetcher_temp_db)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    SpacesFilesFetcher fetcher(dir.filePath(QStringLiteral("s.db")), dir.filePath(QStringLiteral("sf.db")));
    QStringList names = fetcher.getAllSpacesName();
    Q_UNUSED(names);
    (void)fetcher.getSpaceIdByName(QStringLiteral("__no_such_space__"));
    (void)fetcher.getSpaceNameById(-1);
    (void)fetcher.getSpaceStatus(-1);
    fetcher.updateSpaceStatus(-1, false);
    (void)fetcher.getFilesInSpace(QStringLiteral("__none__"), false);
    (void)fetcher.getFilesInSpaceById(-1, false);
    (void)fetcher.getSpaceconditon(QStringLiteral("__none__"));
    (void)fetcher.getSpacelabels(QStringLiteral("__none__"));
    (void)fetcher.isUserAddFile(QStringLiteral("__none__"), QStringLiteral("x"));
}

TEST(IdmModelsWhitebox, file_info_job_from_paths_smoke)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.path() + QStringLiteral("/fj.txt");
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("a");
    f.close();

    FileInfoJob *job = FileInfoJob::fromPaths({path}, false);
    ASSERT_NE(job, nullptr);
    job->setSpaceId(0);
    job->setLabels(QStringList());
    job->setConditions({});
    job->setHandleContentCondition(false);
    job->cancel();
    delete job;

    const QString uri = QUrl::fromLocalFile(path).toEncoded();
    FileInfoJob *job2 = FileInfoJob::fromUris({uri}, false);
    ASSERT_NE(job2, nullptr);
    job2->cancel();
    delete job2;
}

TEST(IdmModelsWhitebox, spaces_files_fetcher_file_events_delayer_and_helpers)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    SpacesFilesFetcher fetcher(dir.filePath(QStringLiteral("ev.db")), dir.filePath(QStringLiteral("evf.db")));

    fetcher.handleFileCreated(QStringLiteral("/tmp/idm_wb_evt_created"));
    fetcher.handleFileDeleted(QStringLiteral("/tmp/idm_wb_evt_deleted"));
    fetcher.handleFileIndexCreated(QStringLiteral("/tmp/idm_wb_evt_idx"));
    fetcher.handleFileMoved(QStringLiteral("/tmp/idm_wb_old"), QStringLiteral("/tmp/idm_wb_new"));

    fetcher.handleFileCreated(QStringLiteral("/tmp/idm_wb_moved_target"));
    fetcher.handleFileMoved(QStringLiteral("/tmp/idm_wb_unused"), QStringLiteral("/tmp/idm_wb_moved_target"));

    fetcher.handleFileCreated(QStringLiteral("/tmp/idm_wb_del_cancel"));
    fetcher.handleFileDeleted(QStringLiteral("/tmp/idm_wb_del_cancel"));

    QThread::msleep(450);
    QCoreApplication::processEvents();

    fetcher.DeleteTargetFiles({});
    fetcher.DeleteTargetFiles({QStringLiteral("/nonexistent/path")});

    fetcher.handleAfterFilesReloaded(QStringList(), -1);
    fetcher.handleAfterFilesReloadedBycreate(QStringList(), -1);
    fetcher.handleSpaceRemoved(-1, QStringList());
}

TEST(IdmModelsWhitebox, spaces_files_fetcher_fetch_emits_fetched)
{
    /* DatabaseManager 将相对库名拼到 QStandardPaths::DataLocation；勿传绝对路径以免路径异常或阻塞。 */
    const QString uniq = QString::number(QRandomGenerator::global()->generate());
    SpacesFilesFetcher fetcher(QStringLiteral("wb_fetch_") + uniq + QStringLiteral(".db"),
                               QStringLiteral("wb_fetch_") + uniq + QStringLiteral("_f.db"));
    QSignalSpy spy(&fetcher, SIGNAL(fetched(bool)));
    fetcher.fetch();
    ASSERT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.at(0).at(0).toBool());
}
