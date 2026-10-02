#include "test_mp3encoder.h"
#include "mp3encoder.h"

TestMp3Encoder::TestMp3Encoder()
{
}

void TestMp3Encoder::initTestCase()
{
    // 在所有测试函数执行前执行一次
    m_mp3Encoder = new Mp3Encoder(44100, 2); // 创建一个44100Hz采样率，双声道的编码器
}

void TestMp3Encoder::test_construct()
{
    // 测试构造函数
    QVERIFY(m_mp3Encoder != nullptr);
    
    // 创建另一个编码器实例进行测试
    Mp3Encoder *encoder = new Mp3Encoder(22050, 1); // 22050Hz采样率，单声道
    QVERIFY(encoder != nullptr);
    delete encoder;
}

void TestMp3Encoder::test_encode()
{
    // 测试encode方法
    short pcm_data[1024]; // 模拟PCM数据
    char mp3_data[1024];  // 存储编码后的MP3数据
    
    // 初始化PCM数据
    for (int i = 0; i < 1024; i++) {
        pcm_data[i] = i % 256; // 简单的测试数据
    }
    
    // 执行编码
    int result = m_mp3Encoder->encode(pcm_data, sizeof(pcm_data), mp3_data, sizeof(mp3_data));
    
    // 验证编码结果
    QVERIFY(result >= 0); // 返回值应为非负数，表示编码成功的字节数
}

void TestMp3Encoder::test_getLastMp3Data()
{
    // 测试getLastMp3Data方法
    char mp3_data[1024]; // 存储最后的MP3数据
    
    // 获取最后的MP3数据
    int result = m_mp3Encoder->getLastMp3Data(mp3_data, sizeof(mp3_data));
    
    // 验证结果
    QVERIFY(result >= 0); // 返回值应为非负数，表示编码成功的字节数
}

void TestMp3Encoder::cleanupTestCase()
{
    // 在所有测试函数执行后执行一次
    delete m_mp3Encoder;
}