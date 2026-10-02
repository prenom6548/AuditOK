#include "signal-transformer-test.h"

#include <QSignalSpy>
#include <QTest>

#include "application-property.h"

#define private public
#include "signal-transformer.h"
#undef private

using namespace UkuiSearch;

namespace {

void resetTransformer(SignalTransformer *transformer)
{
    QMutexLocker locker(&SignalTransformer::s_mutex);
    transformer->m_items2BUpdate.clear();
    transformer->m_items2BUpdateAll.clear();
    transformer->m_items2BInsert.clear();
    transformer->m_items2BDelete.clear();
    transformer->m_signalType = SignalTransformer::Invalid;
}

} // namespace

void SignalTransformerTest::testSingleton()
{
    qRegisterMetaType<ApplicationInfoMap>("ApplicationInfoMap");

    SignalTransformer *transformer = SignalTransformer::getTransformer();
    QVERIFY(transformer != nullptr);
    QCOMPARE(SignalTransformer::getTransformer(), transformer);
}

void SignalTransformerTest::testBatchesInsertedItemsUntilFlush()
{
    SignalTransformer *transformer = SignalTransformer::getTransformer();
    resetTransformer(transformer);

    QSignalSpy addSpy(transformer, SIGNAL(appDBItemsAdd(QStringList)));
    QVERIFY(addSpy.isValid());

    transformer->handleItemInsert(QStringLiteral("/tmp/a.desktop"));
    transformer->handleItemInsert(QStringLiteral("/tmp/b.desktop"));
    QCOMPARE(addSpy.count(), 0);

    transformer->handleSignalTransform();

    QCOMPARE(addSpy.count(), 1);
    QCOMPARE(addSpy.first().first().toStringList(),
             QStringList({QStringLiteral("/tmp/a.desktop"), QStringLiteral("/tmp/b.desktop")}));
    QVERIFY(transformer->m_items2BInsert.isEmpty());
}

void SignalTransformerTest::testSwitchingSignalTypeFlushesPreviousBatch()
{
    SignalTransformer *transformer = SignalTransformer::getTransformer();
    resetTransformer(transformer);

    QSignalSpy addSpy(transformer, SIGNAL(appDBItemsAdd(QStringList)));
    QSignalSpy deleteSpy(transformer, SIGNAL(appDBItemsDelete(QStringList)));
    QVERIFY(addSpy.isValid());
    QVERIFY(deleteSpy.isValid());

    transformer->handleItemInsert(QStringLiteral("/tmp/a.desktop"));
    transformer->handleItemDelete(QStringLiteral("/tmp/a.desktop"));

    QCOMPARE(addSpy.count(), 1);
    QCOMPARE(addSpy.first().first().toStringList(), QStringList{QStringLiteral("/tmp/a.desktop")});
    QCOMPARE(deleteSpy.count(), 0);

    transformer->handleSignalTransform();
    QCOMPARE(deleteSpy.count(), 1);
    QCOMPARE(deleteSpy.first().first().toStringList(), QStringList{QStringLiteral("/tmp/a.desktop")});
}

void SignalTransformerTest::testSwitchingFromUpdateAllUpdateAndDeleteFlushesPreviousBatch()
{
    SignalTransformer *transformer = SignalTransformer::getTransformer();
    resetTransformer(transformer);

    QSignalSpy updateAllSpy(transformer, SIGNAL(appDBItemsUpdateAll(QStringList)));
    QSignalSpy updateSpy(transformer, SIGNAL(appDBItemsUpdate(ApplicationInfoMap)));
    QSignalSpy deleteSpy(transformer, SIGNAL(appDBItemsDelete(QStringList)));
    QVERIFY(updateAllSpy.isValid());
    QVERIFY(updateSpy.isValid());
    QVERIFY(deleteSpy.isValid());

    transformer->handleItemUpdateAll(QStringLiteral("/tmp/update-all.desktop"));

    ApplicationInfoMap appInfo;
    appInfo[QStringLiteral("/tmp/update.desktop")].insert(ApplicationProperty::LocalName, QStringLiteral("App"));
    transformer->handleItemUpdate(appInfo);

    QCOMPARE(updateAllSpy.count(), 1);
    QCOMPARE(updateAllSpy.first().first().toStringList(), QStringList{QStringLiteral("/tmp/update-all.desktop")});

    transformer->handleItemDelete(QStringLiteral("/tmp/delete.desktop"));

    QCOMPARE(updateSpy.count(), 1);
    const auto updated = qvariant_cast<ApplicationInfoMap>(updateSpy.first().first());
    QCOMPARE(updated.value(QStringLiteral("/tmp/update.desktop")).value(ApplicationProperty::LocalName).toString(),
             QStringLiteral("App"));

    transformer->handleItemInsert(QStringLiteral("/tmp/insert.desktop"));

    QCOMPARE(deleteSpy.count(), 1);
    QCOMPARE(deleteSpy.first().first().toStringList(), QStringList{QStringLiteral("/tmp/delete.desktop")});
}

void SignalTransformerTest::testUpdateItemsAreMergedByDesktopFile()
{
    SignalTransformer *transformer = SignalTransformer::getTransformer();
    resetTransformer(transformer);

    QSignalSpy updateSpy(transformer, SIGNAL(appDBItemsUpdate(ApplicationInfoMap)));
    QVERIFY(updateSpy.isValid());

    ApplicationInfoMap nameUpdate;
    nameUpdate[QStringLiteral("/tmp/a.desktop")].insert(ApplicationProperty::LocalName, QStringLiteral("App"));
    ApplicationInfoMap favoriteUpdate;
    favoriteUpdate[QStringLiteral("/tmp/a.desktop")].insert(ApplicationProperty::Favorites, 3);

    transformer->handleItemUpdate(nameUpdate);
    transformer->handleItemUpdate(favoriteUpdate);
    transformer->handleSignalTransform();

    QCOMPARE(updateSpy.count(), 1);
    const auto appInfo = qvariant_cast<ApplicationInfoMap>(updateSpy.first().first());
    QCOMPARE(appInfo.value(QStringLiteral("/tmp/a.desktop")).value(ApplicationProperty::LocalName).toString(),
             QStringLiteral("App"));
    QCOMPARE(appInfo.value(QStringLiteral("/tmp/a.desktop")).value(ApplicationProperty::Favorites).toInt(), 3);
}

void SignalTransformerTest::testFlushEmitsAllPendingSignalTypesInOrder()
{
    SignalTransformer *transformer = SignalTransformer::getTransformer();
    resetTransformer(transformer);

    QSignalSpy addSpy(transformer, SIGNAL(appDBItemsAdd(QStringList)));
    QSignalSpy updateSpy(transformer, SIGNAL(appDBItemsUpdate(ApplicationInfoMap)));
    QSignalSpy updateAllSpy(transformer, SIGNAL(appDBItemsUpdateAll(QStringList)));
    QSignalSpy deleteSpy(transformer, SIGNAL(appDBItemsDelete(QStringList)));
    QVERIFY(addSpy.isValid());
    QVERIFY(updateSpy.isValid());
    QVERIFY(updateAllSpy.isValid());
    QVERIFY(deleteSpy.isValid());

    transformer->m_items2BInsert << QStringLiteral("/tmp/add.desktop");
    transformer->m_items2BUpdate[QStringLiteral("/tmp/update-one.desktop")]
        .insert(ApplicationProperty::LocalName, QStringLiteral("Update"));
    transformer->m_items2BUpdateAll << QStringLiteral("/tmp/update.desktop");
    transformer->m_items2BDelete << QStringLiteral("/tmp/delete.desktop");

    transformer->handleSignalTransform();

    QCOMPARE(addSpy.count(), 1);
    QCOMPARE(updateSpy.count(), 1);
    QCOMPARE(updateAllSpy.count(), 1);
    QCOMPARE(deleteSpy.count(), 1);
    QVERIFY(transformer->m_items2BInsert.isEmpty());
    QVERIFY(transformer->m_items2BUpdate.isEmpty());
    QVERIFY(transformer->m_items2BUpdateAll.isEmpty());
    QVERIFY(transformer->m_items2BDelete.isEmpty());
}

void SignalTransformerTest::testDirectTransformWithEmptyCurrentBatchDoesNotEmit()
{
    SignalTransformer *transformer = SignalTransformer::getTransformer();
    resetTransformer(transformer);

    QSignalSpy addSpy(transformer, SIGNAL(appDBItemsAdd(QStringList)));
    QSignalSpy updateSpy(transformer, SIGNAL(appDBItemsUpdate(ApplicationInfoMap)));
    QSignalSpy updateAllSpy(transformer, SIGNAL(appDBItemsUpdateAll(QStringList)));
    QSignalSpy deleteSpy(transformer, SIGNAL(appDBItemsDelete(QStringList)));
    QVERIFY(addSpy.isValid());
    QVERIFY(updateSpy.isValid());
    QVERIFY(updateAllSpy.isValid());
    QVERIFY(deleteSpy.isValid());

    transformer->m_signalType = SignalTransformer::Delete;
    transformer->transform();
    transformer->m_signalType = SignalTransformer::Insert;
    transformer->transform();
    transformer->m_signalType = SignalTransformer::UpdateAll;
    transformer->transform();
    transformer->m_signalType = SignalTransformer::Update;
    transformer->transform();
    transformer->m_signalType = SignalTransformer::Invalid;
    transformer->transform();

    QCOMPARE(addSpy.count(), 0);
    QCOMPARE(updateSpy.count(), 0);
    QCOMPARE(updateAllSpy.count(), 0);
    QCOMPARE(deleteSpy.count(), 0);
}

void SignalTransformerTest::testEmptyFlushDoesNotEmitSignals()
{
    SignalTransformer *transformer = SignalTransformer::getTransformer();
    resetTransformer(transformer);

    QSignalSpy addSpy(transformer, SIGNAL(appDBItemsAdd(QStringList)));
    QSignalSpy updateSpy(transformer, SIGNAL(appDBItemsUpdate(ApplicationInfoMap)));
    QSignalSpy updateAllSpy(transformer, SIGNAL(appDBItemsUpdateAll(QStringList)));
    QSignalSpy deleteSpy(transformer, SIGNAL(appDBItemsDelete(QStringList)));

    transformer->handleSignalTransform();

    QCOMPARE(addSpy.count(), 0);
    QCOMPARE(updateSpy.count(), 0);
    QCOMPARE(updateAllSpy.count(), 0);
    QCOMPARE(deleteSpy.count(), 0);
}

QTEST_MAIN(SignalTransformerTest)
