#include <QtTest>
#include "lib/ky-udf-image/include/interface.h"
#include "lib/ky-burn-engine/include/burnjob.h"
#include "lib/ky-burn-engine/include/burnjobmanager.h"
#include "test_common/test_helpers.h"

using namespace ky_plugin_burn;
using namespace DFMBURN;

class EngineTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testInitUdfImage();
    void testAppendUdfImage();
    void testCreateImage();
};

void EngineTest::initTestCase()
{
    // 在所有测试开始前执行
}

void EngineTest::cleanupTestCase()
{
    // 在所有测试结束后执行
}

void EngineTest::init()
{
    // 在每个测试函数执行前执行
}

void EngineTest::cleanup()
{
    // 在每个测试函数执行后执行
}

void EngineTest::testInitUdfImage()
{
    QString file_save_path = QCoreApplication::applicationDirPath().append(QString("/../../test_data"));
    QString volume_id = "test";
    QString tmp_image_path = QCoreApplication::applicationDirPath().append(QString("/ky_udf_image.iso"));

    int handle_result = init_udf102_image(file_save_path.toStdString(), volume_id.toStdString(),tmp_image_path.toStdString());

    QVERIFY(handle_result == 0);
}

void EngineTest::testAppendUdfImage()
{
    QString file_save_path = QCoreApplication::applicationDirPath().append(QString("/../../test_data"));
    QString volume_id = "test";
    QString tmp_image_path = QCoreApplication::applicationDirPath().append(QString("/ky_udf_image.iso"));
     QString append_image_path = QCoreApplication::applicationDirPath().append(QString("/append_udf_image.iso"));

    int handle_result = init_udf102_image(file_save_path.toStdString(), volume_id.toStdString(),tmp_image_path.toStdString());
    QVERIFY(handle_result == 0);

    int ret = build_append_udf102_image(tmp_image_path.toStdString().c_str(),165536,file_save_path.toStdString().c_str(),volume_id.toStdString().c_str(),append_image_path.toStdString().c_str());
    QVERIFY(ret == 0);

}

void EngineTest::testCreateImage()
{

    //burn option
    BurnOptions opts;
    BurnJobManager::Config config;
    opts |= BurnOption::kJolietSupport;
    opts |= BurnOption::kKeepAppendable;

    config.opts = opts;
    config.volName = QString("test");

    QString destPath =  QCoreApplication::applicationDirPath().append(QString("/../../test_data"));

    QString output_path = QCoreApplication::applicationDirPath().append(QString("/createImage.iso"));


    BurnJobManager::instance()->disconnect();
    QEventLoop loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
    //start burn job
    BurnJobManager::instance()->startCreateISOImage(output_path, QUrl::fromLocalFile(destPath), config);
    loop.exec();
}




QTEST_MAIN(EngineTest)

#include "engine_test.moc"
