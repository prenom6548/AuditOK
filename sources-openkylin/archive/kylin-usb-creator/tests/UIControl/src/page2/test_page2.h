#ifndef TEST_PAGE2_H
#define TEST_PAGE2_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCryptographicHash>
#include <QTemporaryDir>
#include "page2.h"

class TestPage2 : public QObject
{
    Q_OBJECT

public:
    TestPage2();

private slots:
    // 测试套件生命周期
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 构造函数和基本功能测试
    void testConstructor();
    void testSelectPath();
    void testGetFileSize();
    void testQStringToJsonArray();
    
    // 核心业务逻辑测试
    void testMakestar();
    void testAllClose();
    void testStartMaking();
    
    // UI状态切换测试 - 现在可以直接测试私有方法
    void testPlayLoadingGif();
    void testPlayFinishGif();
    void testPlayErrorGif();
    
    // DBus信号处理测试
    void testDealWorkingProgress();
    void testDealMakeFinish();
    
    // 主题相关测试
    void testSetThemeStyleDark();
    void testSetThemeStyleLight();
    void testSetIconchange();
    
    // 辅助函数测试 - 现在可以直接测试私有方法
    void testIsMakingSuccess();
    void testMovieRefresh();
    void testFinishEvent();
    
    // 信号测试
    void testSignals();
    
    // 私有方法直接测试（现在可以访问）
    void testGetmountpoint();
    void testCheckmd5();
    void testCheckmd5finish();
    void testReadStandardOutput();
    
    // 新增测试
    void testSelectPathWithInvalidFiles();
    void testSelectPathWithValidFiles();
    void testMovieStatusTransitions();
    void testProcessManagement();
    void testGetmountpointWithMockData();
    void testMovieRefreshWithDifferentStates();
    void testIsMakingSuccessWithFalseReturn();
    void testGetmountpointFunction();
    void testCheckmd5Function();
    void testReadStandardOutputFunction();
    void testCheckmd5finishFunction();
    void testAllCloseWithRetryButton();
    void testPlayFinishGifWithDarkTheme();
    void testMovieRefreshWithFinishState();
    void testFinishEventWithSuccess();
    void testSetIconchangeWithFinishText();
    void testReadStandardOutputWithMd5Data();
    
    // 新增测试用例以提高覆盖率
    void testMovieRefreshWithLoadingState();
    void testMovieRefreshWithErrorState();
    void testSetIconchangeWithCreationFailedText();
    void testReadStandardOutputWithMd5Output();
    void testAllCloseWithCloseButton();
    void testGetmountpointWithEmptyMountPoints();
    void testGetmountpointWithNonEmptyMountPoints();
    void testReadStandardOutputWithMd5DataNotEmpty();
    
    // 新增测试用例以提高305-335行代码的覆盖率
    void testGetmountpointWithUsbDeviceAndEmptyMountPoints();
    void testGetmountpointWithUsbDeviceAndNonEmptyMountPoints();
    void testGetmountpointWithMatchingUsbDeviceName();
    void testGetmountpointWithMediaMountPoint();
    void testGetmountpointWithNonMediaMountPoint();
    
    // 新增测试用例以进一步提高305-335行代码的覆盖率
    void testGetmountpointWithMockJsonData();
    void testGetmountpointWithUsbDeviceAndChildren();
    void testGetmountpointWithNonEmptyMountPointsAndMatchingDevice();
    
    // 新增测试用例以进一步提高覆盖率
    void testQStringToJsonArrayWithNullJsonDocument();
    void testQStringToJsonArrayWithParseError();
    void testQStringToJsonArrayWithEmptyBlockDevices();
    void testFinishEventWithMakingSuccess();
    void testFinishEventWithError();
    void testMovieRefreshWithAllStates();
    void testDealMakeFinishWithAllStatus();
    void testIsMakingSuccessWithMatchingDevice();
    void testIsMakingSuccessWithoutMatchingDevice();

private:
    Page2* m_page2;
    QTemporaryFile* m_tempFile;
    QString m_testFilePath;
    QTemporaryDir* m_tempDir;
    
    // 辅助函数
    void createTestFile(qint64 sizeMB);
    QJsonObject createMockBlockDevice(const QString& name, const QString& type, 
                                      const QString& tran, const QStringList& mountpoints);
    void setupMockMd5Test();
    void cleanupMockMd5Test();
    
    // 模拟数据
    QJsonArray createMockBlockDevicesArray();
};

#endif // TEST_PAGE2_H