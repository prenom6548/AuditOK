#include "speechinput.h"
#include <QAudioDeviceInfo>
#include <QDebug>

const int TIME_INTERVAL = 160;
const int BUFSIZE = 5120;

SpeechInput::SpeechInput(QObject *parent) : QObject(parent)
{
    connect(this, &SpeechInput::read, this, &SpeechInput::onReadInfo);
    connect(this, &SpeechInput::startAudio, this, &SpeechInput::onStartAudio);
    connect(this, &SpeechInput::stopAudio, this, &SpeechInput::onStopAudio);
}

SpeechInput::~SpeechInput()
{
    if (m_audioInput != nullptr) {
        m_audioInput->stop();
        delete m_audioInput;
    }

#if TEST
    m_file.close();
#endif
}

void SpeechInput::start()
{
#if TEST
    m_file.setFileName("/home/hh/work/test.raw"); //语音原始文件
    m_file.open(QIODevice::WriteOnly | QIODevice::Truncate);
#endif
    m_audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    m_audioFormat.setChannelCount(1);
    m_audioFormat.setCodec("audio/pcm");
    m_audioFormat.setSampleRate(16000);
    m_audioFormat.setSampleSize(16);
    m_audioFormat.setSampleType(QAudioFormat::SignedInt);

    // 判断设备，查看是否存在
    QAudioDeviceInfo devInfo = QAudioDeviceInfo::defaultInputDevice();
    if (devInfo.isNull()) {
        qWarning() << "No recording device was found!";
    }
    // 不支持格式，使用最接近格式
    if (!devInfo.isFormatSupported(m_audioFormat)) { // 当前使用设备是否支持
        qWarning() << "Automatic conversion is made to the nearest format!";
        m_audioFormat = devInfo.nearestFormat(m_audioFormat); // 转换为最接近格式
    }

    m_audioInput = new QAudioInput(devInfo, m_audioFormat, this);
    connect(m_audioInput, &QAudioInput::stateChanged, this, &SpeechInput::handleStateChanged);
}

void SpeechInput::onReadInfo()
{
    if (m_audioInfo == nullptr) {
        qCritical() << "Audio recording is not turned on!";
        return;
    }
    char buf[BUFSIZE] = {0};
    m_audioInfo->readInfo(buf, BUFSIZE);
    Q_EMIT sigSpeechInput(QByteArray(buf, BUFSIZE));
#if TEST
    m_file.write(buf, BUFSIZE);
#endif
    QThread::msleep(TIME_INTERVAL);
}


void SpeechInput::onStartAudio()
{
    qInfo() << "audio start ...";
    m_audioInfo = new AudioInfo(m_audioFormat, this);
    connect(m_audioInfo, &AudioInfo::updateLevel, this, &SpeechInput::updateLevel);
    m_audioInfo->open(QIODevice::ReadWrite | QIODevice::Truncate);
    m_audioInput->start(m_audioInfo);
    QThread::msleep(TIME_INTERVAL);
    onReadInfo();
}

void SpeechInput::onStopAudio()
{
    qInfo() << "audio stop ...";
    m_audioInput->stop();
    if (m_audioInfo != nullptr) {
        m_audioInfo->close();
        m_audioInfo->deleteLater();
        m_audioInfo = nullptr;
    }
}

void SpeechInput::handleStateChanged(QAudio::State state)
{
    switch (state) {
    case QAudio::StoppedState: {
        if (m_audioInput->error() != QAudio::NoError) {
            qCritical() << "Sound recording error!";
        }
    } break;
    case QAudio::ActiveState: {
        qInfo() << "Start recording the sound...";
    } break;
    default:
        break;
    }
}
