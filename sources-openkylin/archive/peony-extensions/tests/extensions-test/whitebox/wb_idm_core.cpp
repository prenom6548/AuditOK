/*
 * 白盒：peony-intelligent-data-management-service 中文件、数据库、搜索与 AI 辅助类的基础冒烟用例。
 */
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRandomGenerator>

#include "file/fileinfo.h"
#include "file/fileinfomanager.h"
#include "condition/condition.h"
#include "database/databasemanager.h"
#include "search/searchtask.h"
#include "ai/ailabelhelper.h"
#include "ai/aiconditionshelper.h"
#include "ai/aipreviewhelper.h"
#include "ai/aisearchtask.h"

using namespace UKUI::IDM;

TEST(IdmCoreWhitebox, file_info_from_path_tmp_file)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.path() + QStringLiteral("/probe.txt");
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    auto info = FileInfo::fromPath(path, false);
    ASSERT_FALSE(info.isNull());
    EXPECT_FALSE(info->uri().isEmpty());
    (void)info->isValid();
    (void)info->path();
    (void)info->parentPath();
    (void)info->displayName();
    (void)info->iconName();
    (void)info->createDateString();
    (void)info->modifyDateString();
    (void)info->openDateString();
    (void)info->dateString();
    (void)info->version();
    (void)info->mimeType();
    (void)info->fileType();
    (void)info->timeModifiedMsec();
    (void)info->timeAccessMsec();
    (void)info->timeRecentModified();
    (void)info->size();
    (void)info->sizeFormat();
    (void)info->openCount();
    (void)info->labelColors();
    info->setChecked(true);
    (void)info->isChecked();
    info->setChecked(false);

    const QUrl url = QUrl::fromLocalFile(path);
    auto info2 = FileInfo::fromUrl(url, false);
    ASSERT_FALSE(info2.isNull());
    EXPECT_FALSE(info2->uri().isEmpty());
}

TEST(IdmCoreWhitebox, database_manager_temp_databases)
{
    const QString suffix = QString::number(QRandomGenerator::global()->generate());
    DatabaseManager mgr(QStringLiteral("wb_idm_") + suffix + QStringLiteral(".db"),
                        QStringLiteral("wb_idm_") + suffix + QStringLiteral("_f.db"));
    QString err;
    (void)mgr.getAllSpaceNames(&err);
    QString rn;
    const QString space = QStringLiteral("WBSpace_") + suffix;
    EXPECT_TRUE(mgr.createSpace(space, QStringList(), &err, &rn));
    (void)mgr.getSpaceIdByName(rn.isEmpty() ? space : rn);
    (void)mgr.getSpaceCondition(space, &err);
    (void)mgr.getSpacestatus(space);
    (void)mgr.updateSpacestatus(space, true);
    (void)mgr.getAllFilesFromSpaces(false, &err);
    (void)mgr.getAllFilesWithSpaceIds(false, &err);
    (void)mgr.getAllFilesFromSpacesBySpaceIds(false, &err);

    const QString resolved = rn.isEmpty() ? space : rn;
    const int sid = mgr.getSpaceIdByName(resolved, &err);
    ASSERT_GT(sid, 0);
    QTemporaryDir fdir;
    ASSERT_TRUE(fdir.isValid());
    const QString fp = fdir.filePath(QStringLiteral("member.txt"));
    {
        QFile f(fp);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("m");
    }
    (void)mgr.addFileToSpace(fp, sid, false, &err);
    (void)mgr.addFileToSpace(fp, resolved, true, &err);
    (void)mgr.getFileIdByPath(fp, false, &err);
    (void)mgr.updateSpaceStatusDescription(sid, 1, 2);
    (void)mgr.getSpaceStatusDescription(sid);
    (void)mgr.changeSpaceConditions(resolved, {QStringLiteral("{}")}, &err);
    (void)mgr.removeFileFromSpace(fp, sid, false, &err);
    (void)mgr.removeFileFromSpace(fp, resolved, true, &err);
    (void)mgr.removeFileFromAllSpace(QStringLiteral("/nonexistent/wb_rm_all"), &err);
    (void)mgr.removeSpace(resolved, &err);
}

TEST(IdmCoreWhitebox, search_task_construct)
{
    SearchTask t;
    t.setSpaceId(3);
    t.setSearchPath(QStringLiteral("/tmp"));
    t.setNeedSearchFile(false);
    (void)t.getConditionsDescription();
    (void)t.getLabels();
    t.updateSearchDirs();
    t.handleCancelWithSpaceId(999);
    {
        QJsonObject root;
        root[QStringLiteral("conditions count")] = 0;
        t.setConditionsDescription(QJsonDocument(root).toVariant());
    }
    {
        QJsonObject cond;
        cond[QStringLiteral("condition type")] = int(Condition::SearchLocation);
        cond[QStringLiteral("orCondition")] = false;
        cond[QStringLiteral("isContains")] = true;
        cond[QStringLiteral("path")] = QStringLiteral("/tmp");
        QJsonObject root;
        root[QStringLiteral("conditions count")] = 1;
        root[QStringLiteral("0")] = cond;
        t.setConditionsDescription(QJsonDocument(root).toVariant());
    }
    {
        auto fs = std::make_shared<Condition>(Condition::FileSize);
        ASSERT_TRUE(fs->setFileSizeCondition(true, 1));
        auto dr = std::make_shared<Condition>(Condition::DateRange);
        ASSERT_TRUE(dr->setDateRangeCondition(int(Condition::TODAY), 0, QDateTime::currentMSecsSinceEpoch()));
        auto fc = std::make_shared<Condition>(Condition::FileContent);
        ASSERT_TRUE(fc->setFileContentCondition(QStringLiteral("wb_content")));
        fc->setIsAndCondition(true);
        auto mr = std::make_shared<Condition>(Condition::ConditionMatchRule);
        mr->setConditionMatchRule(Condition::NameOrContent);
        const QVariant v = Condition::pharseConditions({fs, dr, fc, mr});
        t.setConditionsDescription(v);
    }
    t.cancel();
}

TEST(IdmCoreWhitebox, search_task_start_no_search_emits_finished)
{
    SearchTask t;
    t.setSpaceId(-1);
    QSignalSpy spy(&t, &SearchTask::finished);
    t.start();
    ASSERT_FALSE(spy.isEmpty());
}

TEST(IdmCoreWhitebox, file_info_manager_watcher_smoke)
{
    auto *mgr = UKUI::IDM::FileInfoManager::globalInstance();
    ASSERT_NE(mgr, nullptr);
    mgr->addFileToWatcher(QStringLiteral("file:///tmp/wb_fim_probe"));
    mgr->removeFileFromWatcher(QStringLiteral("file:///tmp/wb_fim_probe"));
    (void)mgr->addFilesToWatcher(QStringList());
    (void)mgr->removeFilesFromWatcher(QStringList());
}

TEST(IdmCoreWhitebox, ai_helpers_construct)
{
    AILabelHelper lh;
    AIConditionsHelper ch;
    AIPreviewHelper ph;
    AISearchTask st(QString(), nullptr);
    (void)lh.isValid();
    bool ok = false;
    (void)lh.getTopLabels(3, &ok);
    (void)lh.getSimilarLabels(QStringLiteral("nope"), &ok);
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString fpath = dir.filePath(QStringLiteral("ai.txt"));
    {
        QFile f(fpath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("x");
    }
    (void)lh.getFileLabels(fpath, &ok);
    (void)lh.getFilesByLabels(QStringList(), &ok);

    (void)ch.isValid();
    (void)ch.pharseConditions(QStringLiteral("{}"), &ok);
    (void)ch.isFileMatchedWithDescriptionCondition(fpath, QStringLiteral("{}"), &ok);
    (void)ch.isFileMatchedWithLables(fpath, QStringList(), &ok);

    (void)ph.isValid();
    (void)ph.getFileLabels(fpath, &ok);
    (void)ph.getFileSummary(fpath, &ok);

    st.setPharseConditions(false);
    st.setSpaceId(0);
    QVariant v;
    st.setFilterConditionDescription(v);
    st.cancel();
}
