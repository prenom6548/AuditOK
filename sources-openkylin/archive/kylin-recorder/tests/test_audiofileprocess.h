#ifndef TEST_AUDIOFILEPROCESS_H
#define TEST_AUDIOFILEPROCESS_H

#include <QObject>
#include <QtTest/QTest>
#include "audiofileprocess.h"

class TestAudioFileProcess : public QObject
{
    Q_OBJECT

public:
    explicit TestAudioFileProcess(QObject *parent = nullptr);

private slots:
    void test_construct();
    void test_wavFile_Analy();
    void test_wavFile_Analy_NonWavFile();
    void test_wavFile_Analy_ShortDuration();
    void test_wavFile_Analy_InvalidFormat();
};

#endif // TEST_AUDIOFILEPROCESS_H