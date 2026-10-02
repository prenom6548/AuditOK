/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijincheng <baijincheng@kylinos.cn>
 */
#ifndef MYTHREAD_H
#define MYTHREAD_H
//===================
#include <QWidget>
#include <QObject>
#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QThread>

#include <QMultimedia>
#include <QFile>
#include <QDebug>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QTimer>
#include <QTime>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QAudio>
#include <QGSettings>

#include <QtMultimedia> //使用多媒体时要引用这个并且在pro中加QT += multimedia multimediawidgets
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QFile>
#include <QIODevice>
//文件监视器
#include <QFileSystemWatcher>

#include "settings.h"
#include "save.h"
#include "mp3encoder.h"
#include "tipwindow.h"
#include "ffutil.h"
//==================

#define BufferSize 35280
#define MYTHREAD_RECTANGLE_COUNT 130 //检测时矩形条个数实时更新
class MyThread : public QObject
{
    Q_OBJECT

    friend class TestMainWindow;
public:
    explicit MyThread(QObject *parent = nullptr);

    int isSuccess = 1; //默认音频转码1不成功，
    QProcess *process = nullptr;
    QProcess *m_processDEL = nullptr;         //删除时调用命令
    QMessageBox *WrrMsg = nullptr;
    int soundVolume = 65; //初始音量为70
    Settings set;
    Save saveas;
    QString fileName;
    QString daultfileName;
    QString default_Location;
    QStringList listFiles;    // 用于记录多段录音的文件名
    QGSettings *recordData = nullptr;
    int i = 0;

    bool isRecordStart = false;

    QString readPathCollected();
    int readNumList();
    void writePathCollected(QString filePath); //更新配置文件
    void writeNumList(int num);
    void onChangeCurrentRecordList(QString filePath);
    QString endFileName;
    QString isSameFileName(QString endFileName);

    void updateAmplitudeList(int valueArray[]); //更新振幅列表
    void saveAs(QString oldFileName);           //右键另存为
    void OpenFolderProcess(QString openedPath); //打开文件夹的命令

    // 删除
    void delAudioFile(QStringList path);

    QString absolutionPath; // raw存放的绝对路径
    QString absolutionWAVPath;
    QString outputFileName; //输出的文件名
    QDateTime recordTime;
    QString rawFileName = ""; // raw临时文件名

    //    QTimer *tipTimer = nullptr;//延时执行process

    QAudioProbe *audioProbe = nullptr;

    int x = 0;
    qint64 toConvertWAV(QString catheFileName, QString wavFileName);

    QString getFileName();
    QString getFilePath();   //获取根目录地址
signals:
    void recordPaint(int);
    void stopRecord();
    void changeVoicePicture();
//    void listItemAddSignal(QString fileName, QString recordTime); //更新检测到的音频振幅值到配置文件
    void listItemAddSignal(QString fileName, QDateTime recordTime);
    void handling(bool isOk);
    void deviceover();
    void audioError(QAudio::Error error);
    void recordDurationChanged(qint64 durationUs);

private:
    int beishu = 1;  //倍数
    int quzhi = 0;   //取值
    int xianzhi = 0; //限制为110个
    // int valueArray[110];//主队列
    QList<int> tmpArray2; //副队列

    qint64 toConvertMp3(QString catheFileName, QString mp3FileName);
    QAudioFormat Mp3();
    QAudioFormat Wav();
    QAudioFormat M4a();

    QAudioFormat format;
    QIODevice *inputDevSound = nullptr;
    QAudioInput *audioInputSound = nullptr; // 负责监听声音

    QFile *file = nullptr;
    QAudioInput *audioInputFile = nullptr;   // 1将音频输入文件
    qint64 m_recordDurationOffsetUs = 0;
    QAudioOutput *audioOutputFile = nullptr; // 1
    short *outdata = nullptr;

    qint64 len;
    QAudioFormat mFormatSound;

    QAudioOutput *audioOutputSound = nullptr;
    QAudioDeviceInfo audioDev;
    void handleTimerout();
    QString  findVendorId();
    QString vendorId;

    int inputSource = 1;
    QTimer *timer = nullptr;
    bool ismuted = false;

    // mp3

    QAudioInput *ainput = nullptr;
    Mp3Encoder *encoder = nullptr;
    char *mp3_data = nullptr;
    QFile *fl = nullptr;


    bool canMonitor = true;
    int value;

    int MaxValue;

    QIODevice *outputDevSound = nullptr;
    int useVolumeSample(qreal iSample);
    int NUM = 0;

    QAudioDeviceInfo monitorVoiceSource(int i);

    QString setDefaultPath(QString path); //设置默认存储路径解决中英文路径冲突问题

    QString pathSetting();
    void generateAudio();

private slots:


    void InitMonitor();
    void OnReadMore();

    void uploadStarted(); //加载tips界面

public slots:

    void fun();
    void OnSliderValueChanged(int value);
    void record_pressed(); //开始录制音频
    void stopBtnPressed(); //停止保存录音
    void playRecord();
    void pauseRecord();
    void updateRecordDuration();
    void audioConversionFinish(int isOk, QProcess::ExitStatus);
    void processBuffer(const QAudioBuffer &buffer);
    void reRecordSlots();

private:
    QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);
    qreal getPeakValue(const QAudioFormat &format);
    template <class T> QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);
};

#endif // MYTHREAD_H
