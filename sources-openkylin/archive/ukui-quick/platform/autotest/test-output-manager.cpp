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

#include "kywc-output-manager.h"
#include "registry.h"
#include <QTest>
#include <QSignalSpy>
#include <QDebug>
#include <QTimer>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>

namespace UkuiQuick {
namespace WaylandClient {
namespace Test {

/**
 * @brief KywcOutputManagerTest - kywc-output-manager 自动化测试类
 *
 * 测试 KywcOutputManager、KywcOutput 和 KywcOutputMode 类的功能。
 * 包含基础 API 测试和可选的 Wayland 集成测试。
 */
class KywcOutputManagerTest : public QObject
{
    Q_OBJECT

public:
    KywcOutputManagerTest() = default;
    ~KywcOutputManagerTest() override = default;

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
     * @brief 测试 KywcOutputMode 基本操作
     */
    void testKywcOutputModeBasic();

    /**
     * @brief 测试 KywcOutput 基本操作
     */
    void testKywcOutputBasic();

    /**
     * @brief 测试 KywcOutputManager 基本操作
     */
    void testKywcOutputManagerBasic();

    /**
     * @brief 测试 KywcOutput 信号发射
     */
    void testKywcOutputSignals();

    /**
     * @brief 测试 KywcOutputMode 信号发射
     */
    void testKywcOutputModeSignals();

    /**
     * @brief 测试 KywcOutputManager 信号发射
     */
    void testKywcOutputManagerSignals();

    /**
     * @brief 测试输出模式列表操作
     */
    void testSupportedModes();

    /**
     * @brief 测试主输出获取
     */
    void testPrimaryOutput();

    /**
     * @brief 测试无效状态下的操作
     */
    void testInvalidOperations();

    /**
     * @brief 集成测试 - 从 Wayland 合成器获取输出信息
     *
     * 此测试仅在 Wayland 环境下运行，
     * 需要设置环境变量 XDG_SESSION_TYPE=wayland
     */
    void testIntegrationWithCompositor();

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

    KywcOutputMode *m_outputMode = nullptr;
    KywcOutput *m_output = nullptr;
    KywcOutputManager *m_outputManager = nullptr;

    // Wayland 连接相关
    KWayland::Client::ConnectionThread *m_connection = nullptr;
    KWayland::Client::EventQueue *m_eventQueue = nullptr;
    Registry *m_registry = nullptr;
    bool m_waylandAvailable = false;
};

void KywcOutputManagerTest::init()
{
    m_outputMode = new KywcOutputMode(this);
    m_output = new KywcOutput(this);
    m_outputManager = new KywcOutputManager(this);
}

void KywcOutputManagerTest::testKywcOutputModeBasic()
{
    QVERIFY2(m_outputMode != nullptr, "Failed to create KywcOutputMode");

    QVERIFY2(!m_outputMode->isValid(), "New KywcOutputMode should be invalid");
    QVERIFY2(m_outputMode->refreshRate() == 0, "Default refresh rate should be zero");
    QVERIFY2(!m_outputMode->isPreferred(), "Default preferred should be false");
}

void KywcOutputManagerTest::testKywcOutputBasic()
{
    QVERIFY2(m_output != nullptr, "Failed to create KywcOutput");

    QVERIFY2(!m_output->isValid(), "New KywcOutput should be invalid");

    QVERIFY2(m_output->name().isEmpty(), "Default name should be empty");
    QVERIFY2(m_output->make().isEmpty(), "Default make should be empty");
    QVERIFY2(m_output->model().isEmpty(), "Default model should be empty");
    QVERIFY2(m_output->serialNumber().isEmpty(), "Default serial number should be empty");
    QVERIFY2(m_output->description().isEmpty(), "Default description should be empty");
    QVERIFY2(m_output->uuid().isEmpty(), "Default uuid should be empty");

    QVERIFY2(m_output->position() == QPoint(0, 0), "Default position should be zero");
    QVERIFY2(m_output->scale() == 1.0, "Default scale should be 1.0");
    QVERIFY2(m_output->transform() == 0, "Default transform should be 0");
    QVERIFY2(m_output->capabilities() == 0, "Default capabilities should be 0");
    QVERIFY2(!m_output->isEnabled(), "Default enabled should be false");
    QVERIFY2(m_output->power() == 0, "Default power should be 0");
    QVERIFY2(m_output->brightness() == 0, "Default brightness should be 0");
    QVERIFY2(m_output->colorTemp() == 0, "Default color temp should be 0");
}

void KywcOutputManagerTest::testKywcOutputManagerBasic()
{
    QVERIFY2(m_outputManager != nullptr, "Failed to create KywcOutputManager");

    QVERIFY2(!m_outputManager->isValid(), "New KywcOutputManager should be invalid");

    QVERIFY2(m_outputManager->eventQueue() == nullptr, "Default event queue should be null");

    QVERIFY2(m_outputManager->outputs().isEmpty(), "Default outputs list should be empty");
    QVERIFY2(m_outputManager->primaryOutput() == nullptr, "Default primary output should be null");
}

void KywcOutputManagerTest::testKywcOutputSignals()
{
    QSignalSpy nameSpy(m_output, &KywcOutput::nameChanged);
    QSignalSpy makeSpy(m_output, &KywcOutput::makeChanged);
    QSignalSpy modelSpy(m_output, &KywcOutput::modelChanged);
    QSignalSpy serialSpy(m_output, &KywcOutput::serialNumberChanged);
    QSignalSpy descSpy(m_output, &KywcOutput::descriptionChanged);
    QSignalSpy sizeSpy(m_output, &KywcOutput::physicalSizeChanged);
    QSignalSpy capSpy(m_output, &KywcOutput::capabilitiesChanged);
    QSignalSpy enabledSpy(m_output, &KywcOutput::enabledChanged);
    QSignalSpy posSpy(m_output, &KywcOutput::positionChanged);
    QSignalSpy transformSpy(m_output, &KywcOutput::transformChanged);
    QSignalSpy scaleSpy(m_output, &KywcOutput::scaleChanged);
    QSignalSpy powerSpy(m_output, &KywcOutput::powerChanged);
    QSignalSpy brightnessSpy(m_output, &KywcOutput::brightnessChanged);
    QSignalSpy colorTempSpy(m_output, &KywcOutput::colorTempChanged);
    QSignalSpy finishedSpy(m_output, &KywcOutput::finished);

    QVERIFY2(nameSpy.isValid(), "nameChanged signal spy invalid");
    QVERIFY2(makeSpy.isValid(), "makeChanged signal spy invalid");
    QVERIFY2(modelSpy.isValid(), "modelChanged signal spy invalid");
    QVERIFY2(serialSpy.isValid(), "serialNumberChanged signal spy invalid");
    QVERIFY2(descSpy.isValid(), "descriptionChanged signal spy invalid");
    QVERIFY2(sizeSpy.isValid(), "physicalSizeChanged signal spy invalid");
    QVERIFY2(capSpy.isValid(), "capabilitiesChanged signal spy invalid");
    QVERIFY2(enabledSpy.isValid(), "enabledChanged signal spy invalid");
    QVERIFY2(posSpy.isValid(), "positionChanged signal spy invalid");
    QVERIFY2(transformSpy.isValid(), "transformChanged signal spy invalid");
    QVERIFY2(scaleSpy.isValid(), "scaleChanged signal spy invalid");
    QVERIFY2(powerSpy.isValid(), "powerChanged signal spy invalid");
    QVERIFY2(brightnessSpy.isValid(), "brightnessChanged signal spy invalid");
    QVERIFY2(colorTempSpy.isValid(), "colorTempChanged signal spy invalid");
    QVERIFY2(finishedSpy.isValid(), "finished signal spy invalid");
}

void KywcOutputManagerTest::testKywcOutputModeSignals()
{
    QSignalSpy sizeSpy(m_outputMode, &KywcOutputMode::sizeChanged);
    QSignalSpy refreshSpy(m_outputMode, &KywcOutputMode::refreshRateChanged);
    QSignalSpy preferredSpy(m_outputMode, &KywcOutputMode::preferredChanged);
    QSignalSpy finishedSpy(m_outputMode, &KywcOutputMode::finished);

    QVERIFY2(sizeSpy.isValid(), "sizeChanged signal spy invalid");
    QVERIFY2(refreshSpy.isValid(), "refreshRateChanged signal spy invalid");
    QVERIFY2(preferredSpy.isValid(), "preferredChanged signal spy invalid");
    QVERIFY2(finishedSpy.isValid(), "finished signal spy invalid");
}

void KywcOutputManagerTest::testKywcOutputManagerSignals()
{
    QSignalSpy outputCreatedSpy(m_outputManager, &KywcOutputManager::outputCreated);
    QSignalSpy primaryChangedSpy(m_outputManager, &KywcOutputManager::primaryOutputChanged);
    QSignalSpy doneSpy(m_outputManager, &KywcOutputManager::done);
    QSignalSpy releasedSpy(m_outputManager, &KywcOutputManager::interfaceAboutToBeReleased);
    QSignalSpy destroyedSpy(m_outputManager, &KywcOutputManager::interfaceAboutToBeDestroyed);
    QSignalSpy removedSpy(m_outputManager, &KywcOutputManager::removed);

    QVERIFY2(outputCreatedSpy.isValid(), "outputCreated signal spy invalid");
    QVERIFY2(primaryChangedSpy.isValid(), "primaryOutputChanged signal spy invalid");
    QVERIFY2(doneSpy.isValid(), "done signal spy invalid");
    QVERIFY2(releasedSpy.isValid(), "interfaceAboutToBeReleased signal spy invalid");
    QVERIFY2(destroyedSpy.isValid(), "interfaceAboutToBeDestroyed signal spy invalid");
    QVERIFY2(removedSpy.isValid(), "removed signal spy invalid");
}

void KywcOutputManagerTest::testSupportedModes()
{
    QVERIFY2(m_output->supportedModes().isEmpty(), "Default supported modes should be empty");
    QVERIFY2(m_output->currentMode() == nullptr, "Default current mode should be null");
}

void KywcOutputManagerTest::testPrimaryOutput()
{
    QVERIFY2(m_outputManager->primaryOutput() == nullptr, "Default primary output should be null");

    QList<KywcOutput *> outputs = m_outputManager->outputs();
    QVERIFY2(outputs.isEmpty(), "Default outputs list should be empty");
}

void KywcOutputManagerTest::testInvalidOperations()
{
    m_outputMode->release();
    m_outputMode->destroy();
    QVERIFY2(!m_outputMode->isValid(), "Mode should be invalid after release/destroy");

    m_output->release();
    m_output->destroy();
    QVERIFY2(!m_output->isValid(), "Output should be invalid after release/destroy");

    m_outputManager->release();
    m_outputManager->destroy();
    QVERIFY2(!m_outputManager->isValid(), "Manager should be invalid after release/destroy");

    m_outputManager->stop();
}
// ============================================
// Wayland 集成测试相关方法实现
// ============================================

bool KywcOutputManagerTest::checkWaylandEnvironment()
{
    QString sessionType = qgetenv("XDG_SESSION_TYPE");
    qDebug() << "[INFO] 当前会话类型:" << sessionType;
    m_waylandAvailable = (sessionType == "wayland");

    if (!m_waylandAvailable) {
        qDebug() << "[SKIP] Wayland 环境不可用, 跳过集成测试";
        qDebug() << "[INFO] 当前会话类型:" << sessionType;
        qDebug() << "[INFO] 要运行集成测试, 请在 Wayland 会话中执行:";
        qDebug() << "  XDG_SESSION_TYPE=wayland ./test-output-manager";
    }

    return m_waylandAvailable;
}

bool KywcOutputManagerTest::initWaylandConnection()
{
    if (!checkWaylandEnvironment()) {
        return false;
    }
    
    qDebug() << "[INFO] 初始化 Wayland 连接...";
    
    // 创建连接
    m_connection = KWayland::Client::ConnectionThread::fromApplication(this);

    if (!m_connection) {
        qDebug() << "[ERROR] Wayland 连接失败";
        cleanupWaylandConnection();
    }

    // 创建 Registry
    m_registry = new Registry(qGuiApp);


    // 等待接口注册完成
    // Registry::setup() 是异步的，需要事件循环来接收 compositor 发送的接口公告
    // 在调用 hasInterface() 或 interface() 之前，必须等待 interfacesAnnounced 信号

    qDebug() << "[INFO] 等待接口注册...";
    QSignalSpy interfacesSpy(m_registry, &Registry::interfacesAnnounced);
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    // 设置 5 秒超时
    QEventLoop waitLoop;
    QObject::connect(&timeoutTimer, &QTimer::timeout, &waitLoop, &QEventLoop::quit, Qt::DirectConnection);
    QObject::connect(m_registry, &Registry::kywcOutputManagerAnnounced, &waitLoop, &QEventLoop::quit, Qt::QueuedConnection);

    m_registry->create(m_connection);
    m_registry->setup();
    m_connection->roundtrip();
    timeoutTimer.start(5000);
    waitLoop.exec(); // 等待信号或超时

    if (timeoutTimer.isActive()) {
        qDebug() << "[INFO] 接口注册完成";
    } else {
        qDebug() << "[WARN] 接口注册超时";
    }

    if (m_registry->hasInterface(Registry::Interface::KywcOutputManager)) {
        auto interfaceInfo = m_registry->interface(Registry::Interface::KywcOutputManager);
        qDebug() << "[INFO] kywc_output_manager_v1 接口已注册: name=" 
                 << interfaceInfo.name << " version=" << interfaceInfo.version;
    } else {
        qDebug() << "[WARN] kywc_output_manager_v1 接口未找到";
    }
    
    qDebug() << "[INFO] Wayland 连接初始化成功";
    return true;
}

void KywcOutputManagerTest::cleanupWaylandConnection()
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

void KywcOutputManagerTest::testIntegrationWithCompositor()
{

if (!TEST_WITH_WAYLAND_COMPOSITOR) {
    QSKIP("未定义 TEST_WITH_WAYLAND_COMPOSITOR, 跳过连接合成器测试");
}

    if (m_outputManager) {
        delete m_outputManager;
        m_outputManager = nullptr;
    }
    if (m_output) {
        delete m_output;
        m_output = nullptr;
    }
    if (m_outputMode) {
        delete m_outputMode;
        m_outputMode = nullptr;
    }
    // 检查 Wayland 环境
    if (!initWaylandConnection()) {
        QSKIP("Wayland 环境不可用, 跳过测试");
        return;
    }

    qDebug() << "[TEST] 开始测试 - 从合成器获取输出信息";

    // 检查 kywc_output_manager_v1 接口是否可用
    if (!m_registry->hasInterface(Registry::Interface::KywcOutputManager)) {
        qDebug() << "[WARN] kywc_output_manager_v1 接口不可用";
        QSKIP("合成器不支持 kywc_output_manager_v1 接口");
        return;
    }

    // 创建 KywcOutputManager
    QVERIFY2(m_outputManager == nullptr, "m_outputManager 应该为空");
    m_outputManager = m_registry->createKywcOutputManager(
        m_registry->interface(Registry::Interface::KywcOutputManager).name,
        m_registry->interface(Registry::Interface::KywcOutputManager).version,
        this
    );

    QVERIFY2(m_outputManager != nullptr, "Failed to create KywcOutputManager");

    // 等待输出创建信号
    QSignalSpy outputCreatedSpy(m_outputManager, &KywcOutputManager::outputCreated);
    QSignalSpy doneSpy(m_outputManager, &KywcOutputManager::done);

    // 设置超时
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(10000);  // 10秒超时

    // 等待信号
    bool receivedDone = false;
    QObject::connect(m_outputManager, &KywcOutputManager::done, this, [&]() {
        receivedDone = true;
    });

    // 事件循环
    QEventLoop eventLoop;
    QObject::connect(&timeoutTimer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    QObject::connect(m_outputManager, &KywcOutputManager::done, &eventLoop, &QEventLoop::quit);

    if (!receivedDone) {
        eventLoop.exec();
    }

    // 验证结果
    QList<KywcOutput *> outputs = m_outputManager->outputs();
    qDebug() << "[INFO] 检测到输出数量:" << outputs.size();

    for (int i = 0; i < outputs.size(); ++i) {
        KywcOutput *output = outputs[i];
        qDebug() << QString("[OUTPUT %1]").arg(i)
                 << "name:" << output->name()
                 << "make:" << output->make()
                 << "model:" << output->model()
                 << "enabled:" << output->isEnabled()
                 << "position:" << output->position()
                 << "scale:" << output->scale();

        // 测试输出属性
        QVERIFY2(!output->uuid().isEmpty(), "输出 UUID 不应为空");

        // 测试模式信息
        QList<KywcOutputMode *> modes = output->supportedModes();
        qDebug() << QString("  [MODES] 支持的模式数量: %1").arg(modes.size());

        for (int j = 0; j < modes.size(); ++j) {
            KywcOutputMode *mode = modes[j];
            qDebug() << QString("    [MODE %1] size: %2x%3, refresh: %4 Hz, preferred: %5")
                        .arg(j)
                        .arg(mode->size().width())
                        .arg(mode->size().height())
                        .arg(mode->refreshRate() / 1000.0)
                        .arg(mode->isPreferred());

            // 测试模式属性
            QVERIFY2(mode->size().width() > 0, "模式宽度应大于0");
            QVERIFY2(mode->size().height() > 0, "模式高度应大于0");
        }

        // 测试当前模式
        KywcOutputMode *currentMode = output->currentMode();
        if (currentMode) {
            qDebug() << QString("  [CURRENT MODE] size: %1x%2, refresh: %3 Hz")
                        .arg(currentMode->size().width())
                        .arg(currentMode->size().height())
                        .arg(currentMode->refreshRate() / 1000.0);
        }
    }

    // 测试主输出
    KywcOutput *primary = m_outputManager->primaryOutput();
    if (primary) {
        qDebug() << "[INFO] 主输出:" << primary->name();
    } else {
        qDebug() << "[WARN] 未设置主输出";
    }

    // 验证管理器状态
    QVERIFY2(m_outputManager->isValid(), "KywcOutputManager 应该有效");
    QVERIFY2(!m_outputManager->outputs().isEmpty() || receivedDone,
             "应该收到输出或完成信号");

    qDebug() << "[TEST] 集成测试完成";

    // 清理 Wayland 连接
    cleanupWaylandConnection();
}
void KywcOutputManagerTest::cleanup()
{
    // 先清理 Wayland 连接
    cleanupWaylandConnection();

    // 清理测试对象
    delete m_outputMode;
    m_outputMode = nullptr;

    delete m_output;
    m_output = nullptr;

    delete m_outputManager;
    m_outputManager = nullptr;
}

} // namespace Test
} // namespace WaylandClient
} // namespace UkuiQuick

QTEST_MAIN(UkuiQuick::WaylandClient::Test::KywcOutputManagerTest)

#include "test-output-manager.moc"