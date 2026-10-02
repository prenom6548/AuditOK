#include <QtTest>
#include "common/trackingmanager.h"
#include "test_common/test_helpers.h"

class TrackingManagerTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试单例模式
    void testSingleton() {
        TrackingManager* instance1 = TrackingManager::instance();
        TrackingManager* instance2 = TrackingManager::instance();


        QVERIFY(instance1 != nullptr);
        QVERIFY(instance2 != nullptr);
        QCOMPARE(instance1, instance2); // 验证是否为同一实例
    }

    // 测试PageType枚举值
    void testPageTypeEnum() {
        QCOMPARE(TrackingManager::BurnDataPage, 0);
        QCOMPARE(TrackingManager::BurnImagePage, 1);
        QCOMPARE(TrackingManager::CopyImagePage, 2);
    }

    // 测试埋点开关功能
    void testTrackingEnabled() {
        TrackingManager* manager = TrackingManager::instance();
        
        // 保存原始状态
        bool originalState = manager->isTrackingEnabled();
        
        // 测试启用埋点
        manager->setTrackingEnabled(true);
        QVERIFY(manager->isTrackingEnabled());
        
        // 测试禁用埋点
        manager->setTrackingEnabled(false);
        QVERIFY(!manager->isTrackingEnabled());
        
        // 恢复原始状态
        manager->setTrackingEnabled(originalState);
    }

    // 测试记录页面停留时间功能
    void testRecordPageDwellTime() {
        TrackingManager* manager = TrackingManager::instance();
        
        // 保存原始状态
        bool originalState = manager->isTrackingEnabled();
        
        // 启用埋点以便测试
        manager->setTrackingEnabled(true);
        
        // 测试记录不同页面的停留时间
        manager->recordPageDwellTime(TrackingManager::BurnDataPage, 1000);
        manager->recordPageDwellTime(TrackingManager::BurnImagePage, 2000);
        manager->recordPageDwellTime(TrackingManager::CopyImagePage, 3000);
        
        // 测试无效页面类型（不会崩溃）
        // 这里我们不直接测试私有方法，而是通过公共接口验证功能

        manager->flushTrackingData();

        
        QVERIFY(true); // 如果没有崩溃则测试通过
    }

    // 测试TrackingEvent结构体
    void testTrackingEventStruct() {
        TrackingManager::TrackingEvent event;
        event.pageType = TrackingManager::BurnDataPage;
        event.dwellTime = 1000;
        event.eventName = "test_event";
        event.eventDesc = "test_description";
        event.propertyKey = "test_key";
        event.propertyValue = "test_value";
        
        QCOMPARE(event.pageType, TrackingManager::BurnDataPage);
        QCOMPARE(event.dwellTime, 1000);
        QCOMPARE(event.eventName, QString("test_event"));
        QCOMPARE(event.eventDesc, QString("test_description"));
        QCOMPARE(event.propertyKey, QString("test_key"));
        QCOMPARE(event.propertyValue, QString("test_value"));
    }


    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

QTEST_MAIN(TrackingManagerTest)
#include "trackingmanager_test.moc"
