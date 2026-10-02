#ifndef SPEECHINPUT_H
#define SPEECHINPUT_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioInput>
#include "audioinfo.h"

#define TEST 0

class SpeechInput : public QObject
{
    Q_OBJECT
public:
    SpeechInput(QObject *parent = nullptr);
    ~SpeechInput();

Q_SIGNALS:
    void updateLevel(double);
    void sigSpeechInput(const QByteArray &);
    void read();
    void startAudio();
    void stopAudio();

public Q_SLOTS:
    void onStartAudio();
    void onStopAudio();
    void handleStateChanged(QAudio::State state);
    void start();
    void onReadInfo();

private:
    QAudioInput *m_audioInput = nullptr; //录音对象
    AudioInfo *m_audioInfo = nullptr;
    QAudioFormat m_audioFormat;
#if TEST
    QFile m_file;
#endif
};

#endif // SPEECHINPUT_H
