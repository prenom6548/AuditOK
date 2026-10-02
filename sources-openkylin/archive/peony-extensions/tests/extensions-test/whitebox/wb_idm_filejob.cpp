/*
 * 白盒：FileInfoJob 异步 run、AISearchTask 取消/短路径、QtLockedFile 锁接口。
 */
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QList>
#include <memory>

#include "file/fileinfojob.h"
#include "file/fileinfo.h"
#include "condition/condition.h"
#include "ai/aisearchtask.h"
#include "3rd-parties/qtlockedfile.h"

using namespace UKUI::IDM;
using QtLPFile = QtLP_Private::QtLockedFile;

TEST(IdmFileJobWhitebox, file_info_job_start_finished_get_infos)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("job_ok.txt"));
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("ok");
    f.close();

    FileInfoJob *job = FileInfoJob::fromPaths({path}, false);
    ASSERT_NE(job, nullptr);
    QSignalSpy spy(job, &FileInfoJob::finished);
    job->start();
    ASSERT_TRUE(spy.wait(15000));
    const auto infos = job->getInfos();
    EXPECT_FALSE(infos.isEmpty());
    delete job;
}

TEST(IdmFileJobWhitebox, file_info_job_cancel_before_start_emits_cancelled)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("cancel_early.txt"));
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    FileInfoJob *job = FileInfoJob::fromPaths({path}, false);
    QSignalSpy spyCancelled(job, &FileInfoJob::cancelled);
    job->cancel();
    job->start();
    ASSERT_TRUE(spyCancelled.wait(15000));
    EXPECT_TRUE(job->getInfos().isEmpty());
    delete job;
}

TEST(IdmFileJobWhitebox, file_info_job_handle_cancel_with_space_id)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("space_cancel.txt"));
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    FileInfoJob *job = FileInfoJob::fromPaths({path}, false);
    job->setSpaceId(42);
    job->handleCancelWithSpaceId(0);
    QSignalSpy spyFinished(job, &FileInfoJob::finished);
    job->start();
    ASSERT_TRUE(spyFinished.wait(15000));
    delete job;

    FileInfoJob *job2 = FileInfoJob::fromPaths({path}, false);
    job2->setSpaceId(42);
    job2->handleCancelWithSpaceId(42);
    QSignalSpy spyCancelled(job2, &FileInfoJob::cancelled);
    job2->start();
    ASSERT_TRUE(spyCancelled.wait(15000));
    delete job2;
}

TEST(IdmFileJobWhitebox, file_info_job_from_infos_and_conditions)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("from_infos.txt"));
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    auto info = FileInfo::fromPath(path, false);
    ASSERT_FALSE(info.isNull());

    auto nameCond = std::make_shared<Condition>(Condition::FileName);
    ASSERT_TRUE(nameCond->setFileNameCondition(true, {QStringLiteral("from_infos")}));

    FileInfoJob *job = FileInfoJob::fromInfos({info}, false);
    job->setConditions({nameCond});
    QSignalSpy spy(job, &FileInfoJob::finished);
    job->start();
    ASSERT_TRUE(spy.wait(15000));
    const auto out = job->getInfos();
    EXPECT_FALSE(out.isEmpty());
    delete job;
}

TEST(IdmFileJobWhitebox, aisearchtask_cancel_with_space_id_then_do_query)
{
    AISearchTask task(QStringLiteral("probe"));
    task.setSpaceId(9);
    task.cancelWithSpaceId(9);

    using Fin = void (AISearchTask::*)(bool, const QList<QString> &, const QJsonDocument &, const QString &);
    Fin sig = &AISearchTask::finished;
    QSignalSpy spy(&task, sig);
    task.doQuery();
    ASSERT_FALSE(spy.isEmpty());
    const QList<QVariant> args = spy.takeFirst();
    ASSERT_GE(args.size(), 2);
    EXPECT_FALSE(args.at(0).toBool());
    const QString err = args.at(3).toString();
    EXPECT_FALSE(err.isEmpty());
}

TEST(IdmFileJobWhitebox, aisearchtask_cancel_with_space_id_mismatch_no_pre_cancel)
{
    AISearchTask task(QStringLiteral("probe2"));
    task.setSpaceId(1);
    task.cancelWithSpaceId(2);
    using Fin = void (AISearchTask::*)(bool, const QList<QString> &, const QJsonDocument &, const QString &);
    QSignalSpy spy(&task, static_cast<Fin>(&AISearchTask::finished));
    task.doQuery();
    ASSERT_FALSE(spy.isEmpty());
}

TEST(IdmFileJobWhitebox, qt_locked_file_lock_unlock_smoke)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString lockPath = dir.filePath(QStringLiteral("lock.bin"));
    QtLPFile lf(lockPath);
    ASSERT_TRUE(lf.open(QIODevice::ReadWrite));
    EXPECT_TRUE(lf.lock(QtLPFile::WriteLock, true));
    EXPECT_TRUE(lf.isLocked());
    EXPECT_EQ(lf.lockMode(), QtLPFile::WriteLock);
    EXPECT_TRUE(lf.lock(QtLPFile::WriteLock, true));
    EXPECT_TRUE(lf.unlock());
    EXPECT_FALSE(lf.isLocked());
    EXPECT_TRUE(lf.lock(QtLPFile::ReadLock, true));
    EXPECT_TRUE(lf.unlock());
}

TEST(IdmFileJobWhitebox, qt_locked_file_lock_without_open_fails)
{
    QtLPFile lf;
    EXPECT_FALSE(lf.lock(QtLPFile::ReadLock, false));
}

TEST(IdmFileJobWhitebox, qt_locked_file_open_truncate_rejected)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString p = dir.filePath(QStringLiteral("trunc.txt"));
    QFile::remove(p);
    QtLPFile lf(p);
    EXPECT_FALSE(lf.open(QIODevice::WriteOnly | QIODevice::Truncate));
}
