/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "ukui-workspace-manager.h"
#include "registry.h"
#include <QTest>
#include <QSignalSpy>
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>

namespace UkuiQuick {
namespace WaylandClient {
namespace Test {

/**
 * @brief UkuiWorkspaceManagerTest - ukui-workspace-manager 自动化测试类
 *
 * 测试 UkuiWorkspaceManager 和 UkuiWorkspace 类的功能。
 * 包含基础 API 测试和可选的 Wayland 集成测试。
 */
class UkuiWorkspaceManagerTest : public QObject
{
    Q_OBJECT

public:
    UkuiWorkspaceManagerTest() = default;
    ~UkuiWorkspaceManagerTest() override = default;

private slots:
    /**
     * @brief 测试用例初始化
     */
    void init();

    /**
     * @brief 测试用例清理
     */
    void cleanup();

    /**
     * @brief 测试 UkuiWorkspace 基本操作
     */
    void testUkuiWorkspaceBasic();

    /**
     * @brief 测试 UkuiWorkspaceManager 基本操作
     */
    void testUkuiWorkspaceManagerBasic();

    /**
     * @brief 测试 UkuiWorkspace 信号发射
     */
    void testUkuiWorkspaceSignals();

    /**
     * @brief 测试 UkuiWorkspaceManager 信号发射
     */
    void testUkuiWorkspaceManagerSignals();

    /**
     * @brief 测试工作区列表操作
     */
    void testWorkspacesList();

    /**
     * @brief 测试无效状态下的操作
     */
    void testInvalidOperations();

    /**
     * @brief 集成测试 - 从 Wayland 合成器获取工作区信息
     *
     * 此测试仅在 Wayland 环境下运行，
     * 需要设置环境变量 XDG_SESSION_TYPE=wayland
     */
#if TEST_WITH_WAYLAND_COMPOSITOR
    void testIntegrationWithCompositor();
#endif

private:
    /**
     * @brief 检查是否有 Wayland 环境
     */
    bool checkWaylandEnvironment();

    /**
     * @brief 初始化 Wayland 连接
     */
    bool initWaylandConnection();

    /**
     * @brief 清理 Wayland 连接
     */
    void cleanupWaylandConnection();

    UkuiWorkspace *m_workspace = nullptr;
    UkuiWorkspaceManager *m_workspaceManager = nullptr;

    // Wayland 连接相关
    KWayland::Client::ConnectionThread *m_connection = nullptr;
    KWayland::Client::EventQueue *m_eventQueue = nullptr;
    Registry *m_registry = nullptr;
    bool m_waylandAvailable = false;
};

void UkuiWorkspaceManagerTest::init()
{
    qRegisterMetaType<uint32_t>("uint32_t");
    m_workspace = new UkuiWorkspace(this);
    m_workspaceManager = new UkuiWorkspaceManager(this);
}

void UkuiWorkspaceManagerTest::testUkuiWorkspaceBasic()
{
    QVERIFY2(m_workspace != nullptr, "Failed to create UkuiWorkspace");

    QVERIFY2(!m_workspace->isValid(), "New UkuiWorkspace should be invalid");

    QVERIFY2(m_workspace->name().isEmpty(), "Default name should be empty");
    QVERIFY2(m_workspace->position() == 0, "Default position should be zero");
    QVERIFY2(!m_workspace->isActivated(), "Default activated should be false");
    QVERIFY2(m_workspace->uuid().isEmpty(), "Default uuid should be empty");
    QVERIFY2(!m_workspace->isValid(), "Default valid should be false");
}

void UkuiWorkspaceManagerTest::testUkuiWorkspaceManagerBasic()
{
    QVERIFY2(m_workspaceManager != nullptr, "Failed to create UkuiWorkspaceManager");

    QVERIFY2(!m_workspaceManager->isValid(), "New UkuiWorkspaceManager should be invalid");
    QVERIFY2(!m_workspaceManager->isInitialized(), "New UkuiWorkspaceManager should not be initialized");
    QVERIFY2(m_workspaceManager->eventQueue() == nullptr, "Default event queue should be null");

    QVERIFY2(m_workspaceManager->workspaces().isEmpty(), "Default workspaces list should be empty");
    QCOMPARE(m_workspaceManager->workspaceCount(), 0U);
    QCOMPARE(m_workspaceManager->workspace(QStringLiteral("missing-workspace")), nullptr);
}

void UkuiWorkspaceManagerTest::testUkuiWorkspaceSignals()
{
    QSignalSpy nameSpy(m_workspace, &UkuiWorkspace::nameChanged);
    QSignalSpy positionSpy(m_workspace, &UkuiWorkspace::positionChanged);
    QSignalSpy activatedSpy(m_workspace, &UkuiWorkspace::activated);
    QSignalSpy deactivatedSpy(m_workspace, &UkuiWorkspace::deactivated);
    QSignalSpy removedSpy(m_workspace, &UkuiWorkspace::removed);

    QVERIFY2(nameSpy.isValid(), "nameChanged signal spy invalid");
    QVERIFY2(positionSpy.isValid(), "positionChanged signal spy invalid");
    QVERIFY2(activatedSpy.isValid(), "activated signal spy invalid");
    QVERIFY2(deactivatedSpy.isValid(), "deactivated signal spy invalid");
    QVERIFY2(removedSpy.isValid(), "removed signal spy invalid");
}

void UkuiWorkspaceManagerTest::testUkuiWorkspaceManagerSignals()
{
    QSignalSpy workspaceCreatedSpy(m_workspaceManager, &UkuiWorkspaceManager::workspaceCreated);
    QSignalSpy doneSpy(m_workspaceManager, &UkuiWorkspaceManager::done);
    QSignalSpy releasedSpy(m_workspaceManager, &UkuiWorkspaceManager::interfaceAboutToBeReleased);
    QSignalSpy destroyedSpy(m_workspaceManager, &UkuiWorkspaceManager::interfaceAboutToBeDestroyed);
    QSignalSpy initializedSpy(m_workspaceManager, &UkuiWorkspaceManager::initialized);
    QSignalSpy invalidatedSpy(m_workspaceManager, &UkuiWorkspaceManager::invalidated);
    QSignalSpy removedSpy(m_workspaceManager, &UkuiWorkspaceManager::removed);

    QVERIFY2(workspaceCreatedSpy.isValid(), "workspaceCreated signal spy invalid");
    QVERIFY2(doneSpy.isValid(), "done signal spy invalid");
    QVERIFY2(releasedSpy.isValid(), "interfaceAboutToBeReleased signal spy invalid");
    QVERIFY2(destroyedSpy.isValid(), "interfaceAboutToBeDestroyed signal spy invalid");
    QVERIFY2(initializedSpy.isValid(), "initialized signal spy invalid");
    QVERIFY2(invalidatedSpy.isValid(), "invalidated signal spy invalid");
    QVERIFY2(removedSpy.isValid(), "removed signal spy invalid");
}

void UkuiWorkspaceManagerTest::testWorkspacesList()
{
    QVERIFY2(m_workspaceManager->workspaces().isEmpty(), "Default workspaces list should be empty");
    QCOMPARE(m_workspaceManager->workspaceCount(), 0U);
    QCOMPARE(m_workspaceManager->workspace(QStringLiteral("missing-workspace")), nullptr);
}

void UkuiWorkspaceManagerTest::testInvalidOperations()
{
    m_workspace->release();
    m_workspace->destroy();
    QVERIFY2(!m_workspace->isValid(), "Workspace should be invalid after release/destroy");

    QSignalSpy removedSpy(m_workspaceManager, &UkuiWorkspaceManager::removed);
    m_workspaceManager->release();
    m_workspaceManager->destroy();
    QCOMPARE(removedSpy.count(), 0);
    QVERIFY2(m_workspaceManager->workspaces().isEmpty(), "Manager workspaces should stay empty after release/destroy");

    QTest::ignoreMessage(QtWarningMsg, "UkuiWorkspaceManager::stop: can not send requests");
    QVERIFY2(!m_workspaceManager->stop(), "Unbound manager should not send stop request");
    QTest::ignoreMessage(QtWarningMsg, "UkuiWorkspaceManager::requestCreateWorkspace: can not send requests");
    QVERIFY2(!m_workspaceManager->requestCreateWorkspace(QStringLiteral("test"), 0),
             "Unbound manager should not send create request");
    QTest::ignoreMessage(QtWarningMsg, "UkuiWorkspaceManager::requestRemoveWorkspace: can not send requests");
    QVERIFY2(!m_workspaceManager->requestRemoveWorkspace(QStringLiteral("missing-workspace")),
             "Unbound manager should not send remove request");

    QTest::ignoreMessage(QtWarningMsg, "UkuiWorkspace::setName called when invalidated");
    QVERIFY2(!m_workspace->setName(QStringLiteral("Renamed")),
             "Invalid workspace should not send rename request");
    QTest::ignoreMessage(QtWarningMsg, "failed: UkuiWorkspace::setPosition called when invalidated");
    QVERIFY2(!m_workspace->setPosition(1),
             "Invalid workspace should not send move request");
    QTest::ignoreMessage(QtWarningMsg, "UkuiWorkspace::activate called when invalidated");
    QVERIFY2(!m_workspace->activate(),
             "Invalid workspace should not send activate request");
    QTest::ignoreMessage(QtWarningMsg, "UkuiWorkspace::remove called when invalidated");
    QVERIFY2(!m_workspace->remove(),
             "Invalid workspace should not send remove request");
}

// ============================================
// Wayland 集成测试相关方法实现
// ============================================

bool UkuiWorkspaceManagerTest::checkWaylandEnvironment()
{
    QString sessionType = qgetenv("XDG_SESSION_TYPE");
    qDebug() << "[INFO] 当前会话类型:" << sessionType;
    m_waylandAvailable = (sessionType == "wayland");

    if (!m_waylandAvailable) {
        qDebug() << "[SKIP] Wayland 环境不可用, 跳过集成测试";
        qDebug() << "[INFO] 当前会话类型:" << sessionType;
        qDebug() << "[INFO] 要运行集成测试, 请在 Wayland 会话中执行:";
        qDebug() << "  XDG_SESSION_TYPE=wayland ./test-workspace-manager";
    }

    return m_waylandAvailable;
}

bool UkuiWorkspaceManagerTest::initWaylandConnection()
{
    if (!checkWaylandEnvironment()) {
        return false;
    }

    qDebug() << "[INFO] 初始化 Wayland 连接...";

    m_connection = KWayland::Client::ConnectionThread::fromApplication(this);

    if (!m_connection) {
        qDebug() << "[ERROR] Wayland 连接失败";
        cleanupWaylandConnection();
        return false;
    }

    m_registry = new Registry(qGuiApp);

    qDebug() << "[INFO] 等待接口注册...";
    QSignalSpy interfacesSpy(m_registry, &Registry::interfacesAnnounced);
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    QEventLoop waitLoop;
    QObject::connect(&timeoutTimer, &QTimer::timeout, &waitLoop, &QEventLoop::quit, Qt::DirectConnection);
    QObject::connect(m_registry, &Registry::ukuiWorkspaceManagerAnnounced, &waitLoop, &QEventLoop::quit, Qt::QueuedConnection);

    m_registry->create(m_connection);
    m_registry->setup();
    m_connection->roundtrip();
    timeoutTimer.start(5000);
    waitLoop.exec();

    if (timeoutTimer.isActive()) {
        qDebug() << "[INFO] 接口注册完成";
    } else {
        qDebug() << "[WARN] 接口注册超时";
    }

    if (m_registry->hasInterface(Registry::Interface::UkuiWorkspaceManager)) {
        auto interfaceInfo = m_registry->interface(Registry::Interface::UkuiWorkspaceManager);
        qDebug() << "[INFO] kywc_workspace_manager_v1 接口已注册: name="
                 << interfaceInfo.name << " version=" << interfaceInfo.version;
    } else {
        qDebug() << "[WARN] kywc_workspace_manager_v1 接口未找到";
    }

    qDebug() << "[INFO] Wayland 连接初始化成功";
    return true;
}

void UkuiWorkspaceManagerTest::cleanupWaylandConnection()
{
    if (m_registry) {
        m_registry->deleteLater();
        m_registry = nullptr;
    }

    if (m_eventQueue) {
        m_eventQueue->deleteLater();
        m_eventQueue = nullptr;
    }

    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }

    m_waylandAvailable = false;
}

#if TEST_WITH_WAYLAND_COMPOSITOR
void UkuiWorkspaceManagerTest::testIntegrationWithCompositor()
{
    if (m_workspaceManager) {
        delete m_workspaceManager;
        m_workspaceManager = nullptr;
    }
    if (m_workspace) {
        delete m_workspace;
        m_workspace = nullptr;
    }

    if (!initWaylandConnection()) {
        QSKIP("Wayland 环境不可用, 跳过测试");
        return;
    }

    qDebug() << "[TEST] 开始测试 - 从合成器获取工作区信息";

    if (!m_registry->hasInterface(Registry::Interface::UkuiWorkspaceManager)) {
        qDebug() << "[WARN] kywc_workspace_manager_v1 接口不可用";
        QSKIP("合成器不支持 kywc_workspace_manager_v1 接口");
        return;
    }

    QVERIFY2(m_workspaceManager == nullptr, "m_workspaceManager 应该为空");
    m_workspaceManager = m_registry->createUkuiWorkspaceManager(
        m_registry->interface(Registry::Interface::UkuiWorkspaceManager).name,
        m_registry->interface(Registry::Interface::UkuiWorkspaceManager).version,
        this
    );

    QVERIFY2(m_workspaceManager != nullptr, "Failed to create UkuiWorkspaceManager");

    QSignalSpy workspaceCreatedSpy(m_workspaceManager, &UkuiWorkspaceManager::workspaceCreated);
    QSignalSpy doneSpy(m_workspaceManager, &UkuiWorkspaceManager::done);

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(10000);

    bool receivedDone = false;
    QObject::connect(m_workspaceManager, &UkuiWorkspaceManager::done, this, [&]() {
        receivedDone = true;
    });

    QEventLoop eventLoop;
    QObject::connect(&timeoutTimer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    QObject::connect(m_workspaceManager, &UkuiWorkspaceManager::done, &eventLoop, &QEventLoop::quit);

    if (!receivedDone) {
        eventLoop.exec();
    }

    QList<UkuiWorkspace *> workspaces = m_workspaceManager->workspaces();
    qDebug() << "[INFO] 检测到工作区数量:" << workspaces.size();

    for (int i = 0; i < workspaces.size(); ++i) {
        UkuiWorkspace *ws = workspaces[i];
        qDebug() << QString("[WORKSPACE %1]").arg(i)
                 << "name:" << ws->name()
                 << "position:" << ws->position()
                 << "activated:" << ws->isActivated()
                 << "uuid:" << ws->uuid();

        QVERIFY2(!ws->uuid().isEmpty(), "工作区 UUID 不应为空");
    }

    QVERIFY2(!m_workspaceManager->workspaces().isEmpty() || receivedDone,
             "应该收到工作区或完成信号");

    qDebug() << "[TEST] 集成测试完成";

    cleanupWaylandConnection();
}
#endif

void UkuiWorkspaceManagerTest::cleanup()
{
    cleanupWaylandConnection();

    delete m_workspace;
    m_workspace = nullptr;

    delete m_workspaceManager;
    m_workspaceManager = nullptr;
}

} // namespace Test
} // namespace WaylandClient
} // namespace UkuiQuick

QTEST_MAIN(UkuiQuick::WaylandClient::Test::UkuiWorkspaceManagerTest)

#include "test-workspace-manager.moc"
