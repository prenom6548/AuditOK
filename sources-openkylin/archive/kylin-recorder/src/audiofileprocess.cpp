#include "audiofileprocess.h"
#include "mainwindow.h"
#include "clipwidget.h"

AudioFileProcess::AudioFileProcess(QObject *parent) : QObject(parent)
{
    process = new QProcess;
    //    datanum = 0;
    //    datalength = 0;
    //    totalsample = 0;
    //    bitpersample = 0;
}

int AudioFileProcess::wavFile_Analy(QString fileWav)
{
    if (!QFile(fileWav).exists()) {
        qDebug() << "文件不存在";
        return -2; //不存在
    }
    // 判断是否是wav格式文件，或者是否存在隐藏的wav文件
    QString realWavFile = QFileInfo(fileWav).absolutePath() + "/" + QFileInfo(fileWav).baseName() + ".wav";
    QString hideWavFile = QFileInfo(fileWav).absolutePath() + "/." + QFileInfo(fileWav).baseName() + ".wav";
    if(QFileInfo(fileWav).completeSuffix() != "wav" && !QFileInfo(hideWavFile).exists()){
        realWavFile = QFileInfo(fileWav).absolutePath() + "/." + QFileInfo(fileWav).baseName() + ".wav";
        // 如果要求的格式不是.wav文件，而且同目录下也不存在隐藏的wav格式文件，那么就需要创建一个隐藏的wav格式文件出来
        QString convertFormatCmd = QString("ffmpeg -i %1 %2").arg(fileWav).arg(realWavFile);
        qDebug() << convertFormatCmd;
        QStringList args2;
        args2<< "-c" << convertFormatCmd;
        process->start("/bin/sh", args2);
        process->waitForFinished(30000000);
        process->waitForReadyRead(30000000);
        qDebug() << process->readAllStandardError();
        qDebug() << process->readAllStandardOutput();
        process->close();
    }
    if(QFileInfo(hideWavFile).exists()){
        realWavFile = hideWavFile;
    }

    FFUtil fu;
    fu.open(realWavFile);
    int dur_time = fu.getDuration(realWavFile);
    qDebug() << "dur_time(ms): " << dur_time;
    //fu.close();
    int minute = dur_time / 60000 ;
    if (minute >= 600) {
        QFile file(realWavFile);
        qint64 fileSize;
        fileSize = file.size();
        // byterate(每秒字节数或单位时间内生成的文件大小) = 采样率(Hz)*样本数据位数(bit)*声道数/8
        minute = (fileSize / (stuWavHead.sampleRate * stuWavHead.bitsPerSample * stuWavHead.numChannels / 8) / 60);
    }
    qDebug() << "文件时长(秒):" << dur_time / 1000;
    if (dur_time / 1000 < 1) {
        return 0; //时间太短
    }
    if (minute == 0) {
        minute = 1;
    }
    qDebug() << minute << "分钟"
             << "采样率:" << stuWavHead.sampleRate << "样本数据位数:" << stuWavHead.bitsPerSample
             << "声道数:" << stuWavHead.numChannels;
    if((realWavFile.endsWith(".wav", Qt::CaseInsensitive)) && QFile(realWavFile).exists()){

        FILE *fp;
        if ((fp = fopen(realWavFile.toUtf8().toStdString().c_str(), "rb")) != nullptr) {

            fread(&stuWavHead, sizeof(stuWavHead), 1, fp);
            //只支持声道数为1，2的非压缩格式（PCM）
            qDebug() << stuWavHead.numChannels << stuWavHead.audioFormat;
            if ((stuWavHead.numChannels != 1 && stuWavHead.numChannels != 2) || stuWavHead.audioFormat != 1) {
                return -1; //不能分析非录音产生的音频
            }
            quint32 dataSize = stuWavHead.chunkSize - sizeof(stuWavHead) - 8;
            qDebug() << "stuWavHead.chunkSize:" << stuWavHead.chunkSize << "sizeof(stuWavHead):" << sizeof(stuWavHead);
            //样本总数
            quint32 sampleCount = 0;
            int linePerSample = 0;
            if (stuWavHead.bitsPerSample / 8 != 0) {
                sampleCount = dataSize / (stuWavHead.bitsPerSample / 8);
            }
            qDebug() << "样本个数:" << sampleCount << "dur_time:/minute" << dur_time / 1000 / 60
                     << "n秒字节数:" << dur_time / 1000 * 96000;
            //只去左声道样本
            if (sampleCount < 16777216 && sampleCount > 0) {
                sampleCount = sampleCount / stuWavHead.numChannels;

            } else if (sampleCount >= 16777216) {
                sampleCount = sampleCount / stuWavHead.numChannels / minute;
            } else {
                return 0;
            }
            //每条竖线的样本数
            linePerSample = sampleCount / 100;
            if (stuWavHead.bitsPerSample == 16) {
                qint16 data;
                qint16 min = 32767;
                qint16 max = min + 1;
                int x = 0;
                for (int i = 1; i <= sampleCount; i++) {

                    fread(&data, 2, 1, fp);
                    if (min > data) {
                        min = data;
                    }

                    if (max < data) {
                        max = data;
                    }

                    if (i % linePerSample == 0) {
                        //                        qDebug()<<"MAX:"<<abs(max)<<"MIN:"<<abs(min);
                        emit toPaintWave(x, abs(max));
                        min = 32767;
                        max = min + 1;
                        x++;
                    }
                }
                qDebug() << "x:" << x;
            }
        }
        fclose(fp);
        return 1;
    }else{
        return -1; //不能分析非录音产生的音频
    }
    return 0;
}

AudioFileProcess::~AudioFileProcess() {}
