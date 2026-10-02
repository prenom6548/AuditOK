#ifndef TEST_MP3ENCODER_H
#define TEST_MP3ENCODER_H

#include <QObject>
#include <QtTest/QtTest>

class Mp3Encoder;

class TestMp3Encoder : public QObject
{
    Q_OBJECT

public:
    TestMp3Encoder();

private slots:
    void initTestCase();
    void test_construct();
    void test_encode();
    void test_getLastMp3Data();
    void cleanupTestCase();

private:
    Mp3Encoder *m_mp3Encoder;
};

#endif // TEST_MP3ENCODER_H