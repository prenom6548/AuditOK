#ifndef AUDIOFILEPROCESS_H
#define AUDIOFILEPROCESS_H

#include <string>
#include "stdio.h"
#include <vector>
#include <QString>
#include <QDebug>
#include <QObject>
#include <QFile>
#include <QProcess>

// ffmpeg库
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
using namespace std;
#define MAX_AUDIO_FRAME_SIZE 16000 // 1 second of 48khz 32bit audio
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 16000

class AudioFileProcess : public QObject
{

    Q_OBJECT
public:
    explicit AudioFileProcess(QObject *parent = nullptr);
    ~AudioFileProcess();

    struct WavHead // wave文件头格式
    {
        char chunkID[4];      //文档标识 固定值"RIFF"
        quint32 chunkSize;     //文件数据长度
        char format[4];       //文件格式类型 固定值为"WAVE"
        qint32 subChunk1ID;   //格式块标识   固定值为"fmt"
        qint32 subChunk1Size; //格式块长度 取决于编码格式
        qint16 audioFormat;   //编码格式代码 （PCM/非压缩格式）
        qint16 numChannels;   //声道个数
        qint32 sampleRate;    //采样频率
        qint32 byteRate;      //传输速率
        qint16 blockRate;     //数据块对齐单位
        qint16 bitsPerSample; //采样位数
        char subChunk2ID[4];
        qint64 subChunk2Size;
    } stuWavHead;

    bool isWav = true;
    QFile *file = nullptr;

public:
    int sampleCountCompare = 0;
    int wavFile_Analy(QString filePath);

signals:

    void toPaintWave(int index, int value);

private:
    QProcess *process = nullptr;
};

#endif // AUDIOFILEPROCESS_H
