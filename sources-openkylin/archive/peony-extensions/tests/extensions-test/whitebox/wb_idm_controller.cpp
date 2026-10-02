/*
 * 白盒：ModelController / SearchResultModel（依赖 IDM_TEST_RELAX_MODEL_INIT 避免 fetch 与 initModels 死锁）。
 */
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QEventLoop>
#include <QMap>
#include <QRandomGenerator>
#include <QThread>

#include "model/modelcontroller.h"
#include "model/searchresultmodel.h"
#include "condition/condition.h"

namespace {

static void pumpMainThread(int maxRounds = 500)
{
    for (int i = 0; i < maxRounds; ++i) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        if (i % 50 == 0)
            QThread::msleep(2);
    }
}

} // namespace

TEST(IdmControllerWhitebox, model_controller_global_then_search_result_smoke)
{
    ModelController *mc = ModelController::globalInstance();
    ASSERT_NE(mc, nullptr);
    pumpMainThread();

    (void)mc->getAllSpacesName();
    (void)mc->getSpaceIdByName(QStringLiteral("__no_such_space__"));
    (void)mc->getCurrentSearchpath();
    mc->setCurrentSearchPath(QStringLiteral("/tmp"));
    mc->cancelSearch();
    (void)mc->isAisubsysteminited();
    (void)mc->getAinotAvailabletype();
    (void)mc->getIntelligentLabel(QString());
    (void)mc->getIntelligentLabel(QStringLiteral("kw"));
    (void)mc->getFileSummery(QString());
    (void)mc->getFileLabels(QString());
    (void)mc->getFilesInSpaceByName(QStringLiteral("__none__"), false);
    (void)mc->isUserAddFile(QStringLiteral("__none__"), QStringLiteral("f"));
    (void)mc->getSpaceAccesstime(QStringLiteral("__none__"));
    (void)mc->getSpaceModifytime(QStringLiteral("__none__"));
    QVariantHash h = mc->getSpacesSortConfig(-1);
    Q_UNUSED(h);
    mc->setSpacesSortConfig(-1, 0, 0, 0);

    (void)mc->getConditionsBySpaces(QStringLiteral("__none__"));
    (void)mc->updateSpacestatus(QStringLiteral("__none__"), false);
    (void)mc->getSpacestatus(QStringLiteral("__none__"));
    (void)mc->userAddFilesToSpace(QStringList(), QStringLiteral("__none__"));
    mc->spaceRemovedWithFiles(0, QStringList());
    mc->SearchContentLoad(QStringList());
    mc->fileReloadByBatch(QStringList(), true);
    mc->fileTextContentLoad({{QStringLiteral("a"), QStringLiteral("b")}});
    mc->updateSearchCondition(QStringLiteral("{}"));
    mc->spaceUpdatefinish(QStringLiteral("done"));
    mc->filesCreated(QList<QSharedPointer<UKUI::IDM::FileInfo>>(), -1);
    mc->fileDeleted(QMap<QString, QStringList>());
    mc->spaceCreateFinish(QStringLiteral("sn"), QStringList());
    (void)mc->removeSpace(-99999);

    SearchResultModel srm;
    EXPECT_FALSE(srm.getSearchInSpace());
    (void)srm.getContentMatchFiles();
    srm.setSearchInSpace(true);
    srm.setSearchInSpacePaths(QStringList() << QStringLiteral("/tmp"));
    (void)srm.getContentResult();
    srm.setSearchInSpace(false);
    srm.setCurrentSpaceId(1);
    (void)srm.getCurrentSpaceId();
    (void)srm.getSearchConditions();
    (void)srm.getReversedConditions();
    (void)srm.getNameMatchFiles();
    (void)srm.getContentMatchFiles();
    (void)srm.getContentResult();
    srm.setLabels(QStringList());
    (void)srm.getLabels();
    srm.setcurrentLabels(QStringList());
    (void)srm.getcurrentLabels();
    (void)srm.getAiSearch();
    srm.setAiSearch(true);
    (void)srm.getLoading();
    srm.setLoading(false);
    (void)srm.getHasResult();
    srm.setHasResult(false);
    srm.setSearchInSpacePaths(QStringList());
    {
        QList<std::shared_ptr<Condition>> conds;
        auto c = std::make_shared<Condition>(Condition::FileName);
        c->setFileNameCondition(true, {QStringLiteral("z")});
        conds << c;
        srm.setSearchConditions(conds);
        (void)srm.getSearchConditions();
    }
    pumpMainThread();
}

TEST(IdmControllerWhitebox, model_controller_startsearch_rescan_labels_plugin_slots)
{
    ModelController *mc = ModelController::globalInstance();
    ASSERT_NE(mc, nullptr);
    pumpMainThread();

    mc->startsearch(QStringLiteral("kw_wb"), QString(), QStringLiteral("cid_wb1"));
    QCoreApplication::processEvents();
    mc->startsearch(QStringLiteral("kw_wb2"), QStringLiteral("__none__"), QStringLiteral("cid_wb2"));
    QCoreApplication::processEvents();

    mc->reSearchWithLabels(QStringLiteral("lbl_kw"), {QStringLiteral("1")}, QString());
    QCoreApplication::processEvents();
    mc->reSearchWithLabels(QStringLiteral("lbl_kw2"), {QStringLiteral("2")}, QStringLiteral("__none__"));
    QCoreApplication::processEvents();

    mc->userAddFilesToSpaceBySpaceplugin({QStringLiteral("/tmp/wb_plugin_path")}, QStringLiteral("__none__"), true);
    mc->editSpaceConditionBySpaceplugin(QStringLiteral("file:///tmp/wb_space_uri"));
    pumpMainThread();
}

TEST(IdmControllerWhitebox, model_controller_space_create_rename_edit_remove_roundtrip)
{
    ModelController *mc = ModelController::globalInstance();
    ASSERT_NE(mc, nullptr);
    pumpMainThread();

    const QString name = QStringLiteral("wb_sp_%1").arg(QRandomGenerator::global()->generate());
    const QString cond = QStringLiteral("{}");
    if (!mc->onSpaceCreateRequest(name, cond, true)) {
        GTEST_SKIP() << "createSpace unavailable in this environment";
    }
    pumpMainThread();

    const int sid = mc->getSpaceIdByName(name);
    ASSERT_GT(sid, 0);
    const QString renamed = name + QStringLiteral("_r");
    (void)mc->renameSpace(sid, renamed);
    mc->editSpaceCondition(renamed, QStringLiteral("{}"));
    QCoreApplication::processEvents();

    EXPECT_TRUE(mc->removeSpace(sid));
    pumpMainThread();
}
