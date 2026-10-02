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
#include "mythread.h"
#include "mainwindow.h"
#include "tools.h"
#include <unistd.h>
#include "libkydate.h"

struct WAVFILEHEADER
{
    char RiffName[4];    // RIFF 头(标志)
    uint32_t RiffLength; //从下一个开始到文件结尾的字节数
    char WavName[4];     // WAVE 文件
    char FmtName[4];     // fmt 波形格式

    uint32_t FmtLength;      // 数据格式
    uint16_t AudioFormat;    //音频格式是否是压缩的PCM编码，1无压缩，大于1有压缩
    uint16_t ChannleNumber;  //声道数量
    uint32_t SampleRate;     //采样频率
    uint32_t BytesPerSecond; // byterate(每秒字节数) = 采样率(Hz)*样本数据位数(bit)*声道数/8
    uint16_t BytesPerSample; //块对齐/帧大小 framesize = 通道数 * 数据位数
    uint16_t BitsPerSample;  //样本数据位数

    char DATANAME[4];    // catheFileName , QString wavF数据块中的块头
    uint32_t DataLength; //实际音频数据的大小
};

MyThread::MyThread(QObject *parent) : QObject(parent)
{

    qDebug() << "子线程id:" << QThread::currentThreadId();
    recordData = new QGSettings(KYLINRECORDER);


    //    tipTimer = new QTimer;//延时process,防止tip界面卡顿//不需要了
    timer = new QTimer(this);
    connect(timer , &QTimer::timeout , this , &MyThread::handleTimerout);

    process = new QProcess; //调用外部程序如：ffmpeg进行音频文件转码
    connect(process, SIGNAL(started()), this, SLOT(uploadStarted()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(audioConversionFinish(int, QProcess::ExitStatus)));
    //    connect(this,&MyThread::handling,MainWindow::mutual,&MainWindow::handlingSlot);//处理中...
    connect(this, &MyThread::listItemAddSignal, MainWindow::mutual, &MainWindow::slotListItemAdd);
}

QString MyThread::getFileName()
{
    return endFileName;
}

QString MyThread::getFilePath()
{
    return recordData->get("path").toString();
}
// pcm转换未压缩的Mp3
qint64 MyThread::toConvertWAV(QString catheFileName, QString filename)
{
    WAVFILEHEADER WavFileHeader;
    qstrcpy(WavFileHeader.RiffName, "RIFF");
    qstrcpy(WavFileHeader.WavName, "WAVE");
    qstrcpy(WavFileHeader.FmtName, "fmt ");
    qstrcpy(WavFileHeader.DATANAME, "data");
    WavFileHeader.FmtLength = 16;     //  表示 FMT 的长度
    WavFileHeader.AudioFormat = 1;    //这个表示 PCM 编码无压缩;
    WavFileHeader.BitsPerSample = 16; //每次采样得到的样本数据位数;
    // nBytesPerSample 和 nBytesPerSecond这两个值通过设置的参数计算得到;
    // 数据块对齐单位(每个采样需要的字节数 = 通道数 × 每次采样得到的样本数据位数 / 8 )
    WavFileHeader.BytesPerSample = 2;
    WavFileHeader.SampleRate = 48000; //采样频率
    // 波形数据传输速率或(每秒平均字节数) = 采样频率 × 通道数 × 每次采样得到的样本数据位数 / 8  = 采样频率 ×
    // 每个采样需要的字节数
    WavFileHeader.BytesPerSecond = 96000; // byterate(每秒字节数)=采样率(Hz)*采样位数(bit)*通道数/8
    WavFileHeader.ChannleNumber = 1;      //音频通道数平声道
    QFile cacheFile(catheFileName);
    QFile wavFile(filename);
    if (!cacheFile.open(QIODevice::ReadWrite)) {
        qDebug() << "cachefile 不允许读写";
        return -1;
    }
    if (!wavFile.open(QIODevice::WriteOnly)) {
        qDebug() << "wavfile 不允许读写";
        return -2;
    }
    int nSize = sizeof(WavFileHeader);
    qint64 nFileLen = cacheFile.bytesAvailable();

    WavFileHeader.RiffLength = static_cast<unsigned long>(nFileLen - 8 + nSize);
    // static_cast<类型>(变量);  //新式的强制转换
    WavFileHeader.DataLength = static_cast<unsigned long>(nFileLen);

    // 先将wav文件头信息写入，再将音频数据写入;
    wavFile.write((const char *)&WavFileHeader, nSize);
    wavFile.write(cacheFile.readAll());
    cacheFile.close();
    wavFile.close();
    if (nFileLen >= 0) {
        QFile::remove(catheFileName);
    }
    return nFileLen;
}

// pcm转换mp3
qint64 MyThread::toConvertMp3(QString catheFileName, QString mp3FileName)
{

    QFile cacheFile(catheFileName);
    QFile mp3File(mp3FileName);
    if (!cacheFile.open(QIODevice::ReadWrite)) {
        return -1;
    }
    if (!mp3File.open(QIODevice::WriteOnly)) {
        return -2;
    }
    qint64 nFileLen = cacheFile.bytesAvailable();
    QByteArray ba = cacheFile.readAll();

    qDebug() << "缓存长度:" << ba.size() << nFileLen;
    QTextCodec *code = QTextCodec::codecForName("gb2312"); //解决中文路径保存
    code->fromUnicode(mp3FileName).data();
    QString cmd = "ffmpeg -y -threads 2 -f s16le -ar 48k -ac 2 -i \"" + catheFileName + "\" \"" + mp3FileName + "\"";
    //    process->start(cmd);//0609暂时去掉后期复原
    cacheFile.close();
    mp3File.close();

    return nFileLen;
}

void MyThread::uploadStarted()
{
    qDebug() << "执行中!!!";
    emit handling(true);
}

void MyThread::audioConversionFinish(int isOk, QProcess::ExitStatus)
{
    if (isOk == 0) {
        qDebug() << "音频格式转换成功" << isOk;
        isSuccess = isOk;
        //MainWindow::mutual->WrrMsg->hide();
        //MainWindow::mutual->m_tipWindow->hide();

    } else {
        qDebug() << "失败" << isOk;
        isSuccess = isOk;
        //MainWindow::mutual->m_tipWindow->hide();
    }
}

QString MyThread::pathSetting()
{
#ifndef WHITE_TEST
    QString outputName; //输出的文件路径名
    QString pathStr = recordData->get("path").toString();
    qDebug() << "存储的路径是:" << pathStr;
    QString defaultStr = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/" + tr("Recorder") + "/";
    //default_Location = setDefaultPath(pathStr); //设置默认存储到录音的路径
    if(pathStr.compare("") == 0){
        qDebug() << "GSetting is null , use default ";
        recordData->set("path",defaultStr);
    }
    pathStr = recordData->get("path").toString();
    QDir dir(pathStr);
    if(!dir.exists()){
        dir.mkdir(pathStr);
        qDebug() << "Create new folder :" << pathStr;
    }

    //格式适配
    tm *timeFormat = new tm();
    QDateTime dateTime = QDateTime::currentDateTime();
    timeFormat->tm_year = dateTime.date().year();
    timeFormat->tm_mon = dateTime.date().month();
    timeFormat->tm_mday = dateTime.date().day();
    QString d = kdk_system_shortformat_transform(timeFormat);
    QString date;
    date = d.append(" hh:mm:ss");


    fileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
//    recordTime = dateTime.toString(date);

    recordTime = QDateTime::currentDateTime();


    if (recordData->get("path").toString() == "") {
        qDebug() <<"GSetting 第二次读空，有大问题";
        if (recordData->get("type").toInt() == 1) // mp3
        {
            outputName = defaultStr + "/" + fileName + ".mp3";
            format = Mp3();
        } else if (recordData->get("type").toInt() == 3) // wav
        {
            outputName = defaultStr + "/" + fileName + ".wav";
            format = Wav();
        } else {
            outputName = defaultStr + "/" + fileName + ".m4a";
            format = M4a();
        }
    } else {
        QString tempStr = recordData->get("path").toString();
        qDebug() << "尾部'/'后有字符串的路径:" << tempStr;
        if (recordData->get("type").toInt() == 1) // mp3
        {
            outputName = tempStr + "/" + fileName + ".mp3";
            format = Mp3();
        } else if (recordData->get("type").toInt() == 3) // wav
        {
            outputName = tempStr + "/" + fileName + ".wav";
            format = Wav();
        } else {
            outputName = tempStr + "/" + fileName + ".m4a";
            format = M4a();
        }
    }
    return outputName;
#endif
}

void MyThread::generateAudio()
{
#ifndef WHITE_TEST
    int i = 1;
    QStringList cacheListFiles;
    listFiles.clear();
    cacheListFiles.append(absolutionWAVPath);

    while(1){
        QString new_endFileName = QFileInfo(absolutionWAVPath).absolutePath() + "/" + QFileInfo(absolutionWAVPath).baseName() + "[" + QString::number(i) + "]." + QFileInfo(absolutionWAVPath).completeSuffix();
        if(QFileInfo(new_endFileName).exists()){
            cacheListFiles.append(new_endFileName);
            i++;
        }else{
            break;
        }     
    }

    for(int j = 0; j < cacheListFiles.length(); ++j){
        QString targetFileName = QFileInfo(endFileName).absolutePath() + "/" + QFileInfo(cacheListFiles[j]).baseName() + "." + QFileInfo(endFileName).completeSuffix();
        QString convertFormatCmd = QString("ffmpeg -i %1 %2").arg(cacheListFiles[j]).arg(targetFileName);
        listFiles.append(targetFileName);
        qDebug() << convertFormatCmd;
        QStringList args;
        args<< "-c" << convertFormatCmd;
        process->start("/bin/sh", args);
        process->waitForFinished(3000000);
        process->waitForReadyRead(3000000);
        qDebug() << process->readAllStandardError();
        qDebug() << process->readAllStandardOutput();
        process->close();
    }
#endif
}

//按压录音按钮之后触发
void MyThread::record_pressed()
{
#ifndef WHITE_TEST
    qDebug() << "开始录音:";
    /*
     * 只要按压按钮了就说明要录音开始isRecordStart = true
     * 决定此值的只有停止按钮才可以使其置为isRecordStart = false
     * 暂停不影响此值变化
     */
    isRecordStart = true;
    //开始前，count先从0开始
    outputFileName = pathSetting(); //根据配置文件更新录音文件的路径
    qDebug() << "存储的路径是:" << outputFileName;

    endFileName = isSameFileName(outputFileName); //判断是否文件名重复
    qDebug() << "录音开始";

    rawFileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".raw";
    QString str = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    absolutionPath = str + "/.cache/" + rawFileName; // raw缓存文件放在绝对路径
    qDebug() << "绝对路径:" << absolutionPath;
    file = new QFile(absolutionPath);
    bool is_open = file->open(QIODevice::WriteOnly | QIODevice::Truncate);
    if (!is_open) {
        qDebug() << "打开失败";
        exit(1);
    }

    QAudioDeviceInfo inputDevice(QAudioDeviceInfo::defaultInputDevice());
    mFormatSound = inputDevice.nearestFormat(mFormatSound); //读时用默认的8000采样率防止波形刷新过慢
    audioDev = monitorVoiceSource(recordData->get("source").toInt());
    if (audioDev.isNull()) {
        audioDev = inputDevice;
    }

    audioInputSound = new QAudioInput(audioDev, mFormatSound, this); //要在InitMonitor()中的audioInputSound->start()前面
    InitMonitor();
    //写文件时用匹配的format采样率48000
    audioInputFile = new QAudioInput(audioDev, Wav(), this);
    audioInputFile->setNotifyInterval(100);
    connect(audioInputFile, &QAudioInput::notify, this, &MyThread::updateRecordDuration);
    m_recordDurationOffsetUs = 0;

    QString m_vendorid = findVendorId();
    if(m_vendorid == " 0x14f103c9" || m_vendorid == " 0x1d179f8d" || m_vendorid == " 0x14f11f86" || m_vendorid == " 0x14f11f87" || m_vendorid == " 0x14f12008" || m_vendorid == " 0x14f120d1" || m_vendorid == " 0x1fa86186")
    {
        qDebug() << "静音";
        audioInputFile->setVolume(0); //设置录制进文件的音量大小0
    }else
    {
        qDebug() << "非静音";
        audioInputFile->setVolume(1); //设置录制进文件的音量大小1
    }

    audioInputFile->start(file);
    emit recordDurationChanged(0);
    if(audioInputFile->error() == QAudio::OpenError){
        isRecordStart = false;
        emit audioError(audioInputFile->error());
    }
    timer->start(500);
#endif
}

QString MyThread::findVendorId()
{
#ifndef WHITE_TEST
    QString basePath = "/proc/asound/";
    QDir dir(basePath);
    QStringList cardDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    qDebug() << "cardDirs ====" << cardDirs;

    //遍历每个声卡下的codec#0
    for(const QString& cardDir : cardDirs)
    {
        QString codecPath = basePath + cardDir + "/codec#0";
        QFile codecFile(codecPath);
        if(codecFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&codecFile);
            qDebug() << "stream" << stream.readLine();
            QString line;
            while(!stream.atEnd())
            {
                line = stream.readLine();
                if(line.contains("Vendor Id:"))
                {
                    vendorId = line.mid(line.indexOf(":") + 1);
                    qDebug() << "Vendor Id = " << vendorId;
                }
            }
            codecFile.close();
        }
    }
    return vendorId;
#endif
}

void MyThread::handleTimerout(){
#ifndef WHITE_TEST
    if(!ismuted){

        ismuted = true;
        audioInputFile->setVolume(1);
        qDebug() << "500毫秒已经过去" << QTime::currentTime() << audioInputFile->volume();
        timer->stop();
    }
#endif
}

// Mp3格式
QAudioFormat MyThread::Mp3()
{
    QAudioFormat format = QAudioFormat();
    format.setSampleRate(48000); /*
                                  *每秒钟对声音的采样次数，越大越精细，48000HZ(标准CD级)，
                                  *48000HZ的采样率(每秒钟采集48000个声波的点)
                                  */
    format.setChannelCount(2);   //立体声，数目为2
    format.setSampleSize(16);
    format.setCodec("audio/pcm");                    //编码器
    format.setByteOrder(QAudioFormat::LittleEndian); //低位优先
    format.setSampleType(QAudioFormat::SignedInt);   // QAudioFormat::SignedInt

    return format;
}
// Wav格式
QAudioFormat MyThread::Wav()
{
    QAudioFormat format = QAudioFormat();            //设置采样格式
    format.setSampleRate(48000);                     /*
                                                      *每秒钟对声音的采样次数，越大越精细，
                                                      *48000HZ的采样率(每秒钟采集48000个声波的点)
                                                      */
    format.setChannelCount(1);                       //设置通道数：平声道，数目为1
    format.setSampleSize(16);                        //设置采样大小，一般为8位或16位
    format.setCodec("audio/pcm");                    //编码器
    format.setByteOrder(QAudioFormat::LittleEndian); //设置字节序，低位优先
    format.setSampleType(QAudioFormat::SignedInt);   //设置样本数据类型
    return format;
}

//格式M4a格式
QAudioFormat MyThread::M4a()
{
    QAudioFormat format = QAudioFormat();
    format.setSampleRate(48000); /*
                                  *每秒钟对声音的采样次数，越大越精细，
                                  *48000HZ的采样率(每秒钟采集48000个声波的点)
                                  */
    format.setChannelCount(2);   //立体声，数目为2
    format.setSampleSize(16);
    format.setCodec("audio/pcm");                    //编码器
    format.setByteOrder(QAudioFormat::LittleEndian); //低位优先
    format.setSampleType(QAudioFormat::SignedInt);   // v10.1规定QAudioFormat::SignedInt。
    return format;
}



void MyThread::stopBtnPressed() //停止录音
{
    ismuted = false;
    if (audioInputFile != nullptr) {
        audioInputFile->stop(); //音频文件写入停止
    }
    if (audioInputSound != nullptr) {
        audioInputSound->stop(); //监听停止
    }
    if (file != nullptr) {
        file->close();
    }
    if (isRecordStart == true) {
#ifndef WHITE_TEST
        isRecordStart = false;
        absolutionWAVPath = QFileInfo(absolutionPath).absolutePath() + "/" + QFileInfo(absolutionPath).baseName() + ".wav";
//        int num = toConvertWAV(absolutionPath, endFileName.toLocal8Bit().data());
        int num = toConvertWAV(absolutionPath, absolutionWAVPath.toLocal8Bit().data());
        int i = 1;
        QFileInfo filename = QFileInfo(absolutionPath);
        do{
            QString new_filename = filename.absolutePath() + "/" + filename.baseName() + "[" + QString::number(i) + "]." + filename.completeSuffix();
            QString new_endFileName = QFileInfo(absolutionWAVPath).absolutePath() + "/" + QFileInfo(absolutionWAVPath).baseName() + "[" + QString::number(i) + "]." + QFileInfo(absolutionWAVPath).completeSuffix();
            i++;
            if(QFileInfo(new_filename).exists()){
                int num = toConvertWAV(new_filename, new_endFileName.toLocal8Bit().data());
            }else{
                break;
            }
        }while(1);
        // 拼接音频
        generateAudio();

        if (num >= 0) {
            for(int j = 0 ; j < listFiles.length(); ++j){
                FFUtil fu;
                fu.open(listFiles.at(j));
                int dur_time = fu.getDuration(listFiles.at(j));
                QString formatTime = QString(QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime(0, 0, 0, 0).addMSecs(dur_time).toString("hh:mm:ss")) ;
                emit DATABASE_INSTANCE.addRecordSignal(listFiles.at(j), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                emit listItemAddSignal(listFiles.at(j), recordTime);
            }
            //修改创建音频时间

            qDebug() << "保存成功";
        }else{
            qDebug() << "保存失败";
        }
        QTextCodec *code = QTextCodec::codecForName("gb2312"); //解决中文路径保存
        code->fromUnicode(endFileName).data();
#endif
    }
    qDebug() << "成功停止";
    audioInputFile->deleteLater();
    audioInputFile = nullptr;
    audioInputSound->deleteLater();
    audioInputSound = nullptr;
    file->deleteLater();
    file = nullptr;
    MainWindow::mutual->m_tipWindow->hide();

}

//设置默认存储路径判断中英文环境下文件路径唯一
QString MyThread::setDefaultPath(QString path)
{
    QDir *record = new QDir;
    QLocale locale = QLocale::system().name();
    QString savePath;
    bool existEn = record->exists(path + "/Recorder");
    bool existCH = record->exists(path + "/录音");
    //判断是否存在此路径,若两种路径都不存在就创建一个
    if (!existEn && !existCH) {
        //创建record文件夹
        if (locale.language() == QLocale::English) {
            //            qDebug()<<"English Environment:"<<path+"/"+tr("Recorder");
            record->mkdir(path + "/" + tr("Recorder"));
        } else if (locale.language() == QLocale::Chinese) {
            //            qDebug()<<"中文环境:"<<path+"/"+tr("Recorder");
            record->mkdir(path + "/" + tr("Recorder"));
        }
        savePath = path + "/" + tr("Recorder");
    } else {
        if (existEn) {
            savePath = path + "/Recorder";
        } else if (existCH) {
            savePath = path + "/录音";
        }
    }

    return savePath;
}


void MyThread::updateAmplitudeList(int valueArray[]) //更新检测到的音频振幅值到配置文件
{
#ifndef WHITE_TEST
    QString newStr = "";
    for (int i = 0; i < MYTHREAD_RECTANGLE_COUNT; i++) {
        QString amplitudeStr = recordData->get("amplitude").toString(); //从配置文件中读取振幅集,配置文件中初始值为""
        QString str = QString::number(valueArray[i]);
        newStr = amplitudeStr + "," + str;
        recordData->set("amplitude", newStr);
        // qDebug()<<newStr;
    }
    newStr += ";";
    recordData->set("amplitude", newStr);
    qDebug() << "当前的振幅集合:" << newStr;
#endif
}

QString MyThread::readPathCollected()
{
    QString str = "";
    if (recordData != nullptr) {
        QStringList keyList = recordData->keys();
        if (keyList.contains("recorderpath")) {
            str = recordData->get("recorderpath").toString();
        }
    }
    return str;
}

void MyThread::writePathCollected(QString filePath)
{

    recordData->set("recorderpath", filePath);
}

int MyThread::readNumList()
{
    int num = 0;
    if (recordData != nullptr) {
        QStringList keyList = recordData->keys();
        if (keyList.contains("num")) {
            num = recordData->get("num").toInt();
        }
    }
    return num;
}

void MyThread::writeNumList(int num)
{
    recordData->set("num", num);
}

void MyThread::onChangeCurrentRecordList(QString filePath)
{
    QString newFilePath = "";
    QString strSaveFilePath = readPathCollected(); //从配置文件中读取路径集,配置文件中初始值为""
    newFilePath = strSaveFilePath + "," + filePath;
    // qDebug()<<newFilePath;
    writeNumList(newFilePath.split(",").length());
    writePathCollected(newFilePath);
}

void MyThread::pauseRecord() //暂停录制
{
    if (audioInputFile != nullptr) {
        audioInputFile->suspend();
        updateRecordDuration();
    }
    if (audioInputSound != nullptr) {
        audioInputSound->suspend();
    }
}

void MyThread::playRecord() //开始录制
{
    if (audioInputFile != nullptr) {
        audioInputFile->resume();
    }
    if (audioInputSound != nullptr) {
        audioInputSound->resume();
    }
}

void MyThread::updateRecordDuration()
{
    if (audioInputFile != nullptr) {
        emit recordDurationChanged(m_recordDurationOffsetUs + audioInputFile->processedUSecs());
    }
}

void MyThread::InitMonitor()
{
#ifndef WHITE_TEST
    //判断当前的音频输入设备是否支持QAudioFormat配置，如果不支持，获取支持的最接近的配置信息

    //    qDebug()<<infoIn.deviceName()<<mFormatSound;
    // CreateAudioInput();
    //************************************************2020.9.24

    // audioInputSound->setVolume(MaxValue*0.001);//设置检测波形的音量大小

    inputDevSound = audioInputSound->start();

    QTime base_time(0, 0, 0);
    int run_time = MainWindow::mutual->get_sys_runtime(1);
    MainWindow::mutual->m_baseTime = base_time.addSecs(run_time);
    MainWindow::mutual->m_miniWindow.m_miniBaseTime = base_time.addSecs(run_time);

    connect(inputDevSound, SIGNAL(readyRead()), this, SLOT(OnReadMore()));
#endif
}

QAudioDeviceInfo MyThread::monitorVoiceSource(int i)
{
#ifndef WHITE_TEST
    qDebug() << "配置文件中的值(1.全部2.内部3.麦克风)" << i;
    if (i == 2) { //系统内部

        //当录制系统内部声音时调用此方法
        qDebug() << "内部" << i;
        QAudioDeviceInfo outputDevice(QAudioDeviceInfo::defaultOutputDevice());
        qDebug() << "当前输出设备:" << outputDevice.deviceName();
        QList<QAudioDeviceInfo> audioDeviceListI = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
        emit deviceover();
        foreach (QAudioDeviceInfo devInfo, audioDeviceListI) {
            //            qDebug() << "输入设备:" << devInfo.deviceName();
            if (devInfo.deviceName().contains(outputDevice.deviceName() + ".monitor")) {
                if(devInfo.deviceName().contains("histen_sink.monitor")){
                    continue;
                }
                qDebug() << "当前内部输入设备:" << devInfo.deviceName();
                audioDev = devInfo;
            }
        }
        qDebug() << audioDev.deviceName();
        return audioDev;

    } else { //其他

        qDebug() << "麦克风声音";
        emit deviceover();
        QAudioDeviceInfo inputDevice(QAudioDeviceInfo::defaultInputDevice());
        mFormatSound = inputDevice.nearestFormat(mFormatSound);
	qDebug() << "当前外部输入设备:" << inputDevice.deviceName() << mFormatSound; //可以判断当前输入设备
        return inputDevice;
    }
#endif
}

//调节音量大小
void MyThread::OnSliderValueChanged(int value)
{
    soundVolume = value;
    emit changeVoicePicture();
}

void MyThread::fun()
{
    canMonitor = true;
}

void MyThread::OnReadMore()
{
    if (!canMonitor) {

        return; //
    }
    canMonitor = false;
    //如果输入为空则返回空
    if (!audioInputSound) {
        qDebug() << " REISIVE VOICE";
        return;
    }

    //处理音频输入，_Buffer中保存音频数据，len是获取到的数据的长度
    QByteArray _Buffer(BufferSize, 0);
    len = audioInputSound->bytesReady();
    if (len > 4096)
        len = 4096;
    qint64 l = inputDevSound->read(_Buffer.data(), len);
    if (l <= 0) {
        return;
    }
    short *tempData = (short *)_Buffer.data();
    outdata = tempData;
    MaxValue = 0;
    for (int i = 0; i < len; i++) {
        //把样本数据转换为整型
        value = abs(useVolumeSample(
            outdata[i])); /*麦克风中的薄膜始终是在平衡位置附近value会检测到正负相间的震荡频率,加个绝对值*/
        MaxValue = MaxValue >= value ? MaxValue : value;
    }

    emit recordPaint(MaxValue); //每检测到value就发送一次绘图信号
    file->flush();
    /*
     * fsync(file->handle());
     * 将所有修改过的块缓冲区写入队列，然后就返回，并不等待实际写操作的结束
     * 同步内存中的所有已修改的文件数据到存储设备(必须加，否则断电时文件就无法正确保存)
     */
    fsync(file->handle());
}

int MyThread::useVolumeSample(qreal sample)
{
    return qMax(qMin(((sample * soundVolume) / 100), 30000.0), -30000.0); //求每次检测到的振幅组中最大值
}

//判断重复路径时后加[]
QString MyThread::isSameFileName(QString FileName)
{
#ifndef WHITE_TEST
    QFileInfoList fileList = Tools::getRecordingFileinfos();

    qDebug() << "文件路径:" << FileName;
    int fileName_addNum;
    if (fileList.contains(FileName)) {
        fileName_addNum = recordData->get("samefilenum").toInt();
        int position = FileName.lastIndexOf("."); //从后面开始查"."所在位置
        qDebug() << "从后面开始查.所在位置" << position;
        QString newFileName = FileName.insert(position, "[" + QString::number(fileName_addNum) + "]");
        qDebug() << "新文件名" << newFileName;
        fileName_addNum++;
        recordData->set("samefilenum", fileName_addNum);
        return newFileName;
    }
    return FileName;
#endif
}

//要放到此线程里防止阻塞主界面
void MyThread::OpenFolderProcess(QString openedPath)
{
    qDebug() << "打开文件路径!" << openedPath;
    //打开文件所在位置不选中
    QString path = openedPath.mid(0, openedPath.lastIndexOf("/"));
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MyThread::delAudioFile(QStringList path)
{
#ifndef WHITE_TEST

    QStringList delFilePath = path;
    QString cmd = "gio";
    QStringList arguments = QStringList()<< "trash";
    for(int index = 0; index < delFilePath.length(); index++){
        QFileInfo *fileInfo = new QFileInfo(delFilePath.at(index).toLocal8Bit().data());
        if (fileInfo->exists()) {
            qDebug() << "路径存在，删除" << delFilePath.at(index).toLocal8Bit().data();
            // m_file->moveToTrash() 这个方法在 qt5.15中可以使用，等主线支持5.15便可替换
            QString del_file_path = fileInfo->absoluteFilePath();
            arguments << del_file_path;
        }
        delete fileInfo;
    }
    if(m_processDEL == nullptr){
        m_processDEL = new QProcess(this);
    }
    m_processDEL->start(cmd, arguments);
#endif
}

void MyThread::saveAs(QString oldFileName) //右键另存为可以选择存储音频格式
{
#ifndef WHITE_TEST

    QFileDialog *fileDialog = new QFileDialog;
    QString fileType = "Mp3(*.mp3)";
    fileDialog->setNameFilter(fileType);
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    qDebug() << defaultPath;
    // QFileDialog指定父窗口,使系统弹窗在中间MainWindow::mutual->mainWid
    QString newFileName = QFileDialog::getSaveFileName(MainWindow::mutual, tr("Select a file storage directory"),
                                                       defaultPath + "/RecordFile.mp3", fileType);
    if (/*ret == QMessageBox::Ok && */ newFileName == "" || newFileName.contains(" ") || newFileName.contains("?")
        || newFileName.contains("'") || newFileName.contains("\"") || newFileName.contains("\\")) {
        QMessageBox::warning(MainWindow::mutual, tr("Warning"), tr("Do not enter illegal file name"), QMessageBox::Ok);
        qDebug() << "进来了!!";
        return;
    }
    if (oldFileName != newFileName) {
        if (newFileName.split(".").last() == "mp3") {
            qDebug() << "旧文件名:" << oldFileName << "新文件名:" << newFileName << newFileName.split(".").last();
            QString cmd = "ffmpeg -y -i \"" + oldFileName + "\" \"" + newFileName + "\"";
            process->start(cmd);
        } else if (newFileName.split(".").last() == "wav") {
        } else if (newFileName.split(".").last() == "m4a") {
        }
        QFile::copy(oldFileName, newFileName);
    } else {
        if (QMessageBox::Yes){
            qDebug() << "确认";
            QFile::copy(oldFileName, newFileName);
        }
    }
    fileDialog->deleteLater();
    //    mBox->deleteLater();
    return;
#endif
}

//振幅检测
void MyThread::processBuffer(const QAudioBuffer &buffer)
{
#ifndef WHITE_TEST

    if (!canMonitor)
        return;
    canMonitor = false;

    QVector<qreal> levels = getBufferLevels(buffer);
    //        qDebug()<<"长度:"<<levels.count();
    MaxValue = 0;
    for (int i = 0; i < levels.count(); i++) {
        //        qDebug()<<"振幅:"<<levels.at(i)*10000;
        //把样本数据转换为整型
        value = abs(useVolumeSample(
            levels.at(i) * 10000)); /*麦克风中的薄膜始终是在平衡位置附近value会检测到正负相间的震荡频率,加个绝对值*/
        MaxValue = MaxValue >= value ? MaxValue : value;
    }
    emit recordPaint(MaxValue); //每检测到value就发送一次绘图信号
#endif
}

void MyThread::reRecordSlots()
{
#ifndef WHITE_TEST

    QFileInfo filename = QFileInfo(absolutionPath);
    int i = 1; //记录文件个数
    QString new_filename;
    // 如果存在则往下一个文件里写
    do{
        new_filename = filename.absolutePath() + "/" + filename.baseName() + "[" + QString::number(i) + "]." + filename.completeSuffix();
        i++;
    }while(QFileInfo(new_filename).exists());

    if (audioInputFile != nullptr) {
        m_recordDurationOffsetUs += audioInputFile->processedUSecs();
        audioInputFile->stop(); //音频文件写入停止
    }
    if (audioInputSound != nullptr) {
        audioInputSound->stop(); //监听停止
    }

    if (file != nullptr) {
        file->close();
    }

    file = new QFile(new_filename);
    bool is_open = file->open(QIODevice::WriteOnly | QIODevice::Truncate);
    if (!is_open) {
        qDebug() << "打开失败";
        exit(1);
    }

    QAudioDeviceInfo inputDevice(QAudioDeviceInfo::defaultInputDevice());
    mFormatSound = inputDevice.nearestFormat(mFormatSound); //读时用默认的8000采样率防止波形刷新过慢
    audioDev = monitorVoiceSource(recordData->get("source").toInt());
    if (audioDev.isNull()) {
        audioDev = inputDevice;
    }

    audioInputSound = new QAudioInput(audioDev, mFormatSound, this); //要在InitMonitor()中的audioInputSound->start()前面
    inputDevSound = audioInputSound->start();
    connect(inputDevSound, SIGNAL(readyRead()), this, SLOT(OnReadMore()));
    //写文件时用匹配的format采样率48000
    audioInputFile = new QAudioInput(audioDev, Wav(), this);
    audioInputFile->setNotifyInterval(100);
    connect(audioInputFile, &QAudioInput::notify, this, &MyThread::updateRecordDuration);

    audioInputFile->setVolume(1); //设置录制进文件的音量大小
    audioInputFile->start(file);
#endif
}

//获得buffer等级
QVector<qreal> MyThread::getBufferLevels(const QAudioBuffer &buffer)
{
#ifndef WHITE_TEST
    QVector<qreal> values;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return values;

    if (buffer.format().codec() != "audio/pcm")
        return values;

    int channelCount = buffer.format().channelCount();
    values.fill(0, channelCount);
    qreal peak_value = getPeakValue(buffer.format());
    if (qFuzzyCompare(peak_value, qreal(0)))
        return values;

    switch (buffer.format().sampleType()) {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] = qAbs(values.at(i) - peak_value / 2) / (peak_value / 2);
        break;
    case QAudioFormat::Float:
        if (buffer.format().sampleSize() == 32) {
            values = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < values.size(); ++i)
                values[i] /= peak_value;
        }
        break;
    case QAudioFormat::SignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] /= peak_value;
        break;
    }

    return values;
#endif
}

//此函数用于返回给定音频格式的最大可能采样值
qreal MyThread::getPeakValue(const QAudioFormat &format)
{
#ifndef WHITE_TEST

    //注意：只支持最常见的样本格式
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
    case QAudioFormat::Unknown:
        break;
    case QAudioFormat::Float:
        if (format.sampleSize() != 32) // 其他样本格式不支持
            return qreal(0);
        return qreal(1.00003);
    case QAudioFormat::SignedInt:
        if (format.sampleSize() == 32)
            return qreal(INT_MAX);
        if (format.sampleSize() == 16)
            return qreal(SHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(CHAR_MAX);
        break;
    case QAudioFormat::UnSignedInt:
        if (format.sampleSize() == 32)
            return qreal(UINT_MAX);
        if (format.sampleSize() == 16)
            return qreal(USHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(UCHAR_MAX);
        break;
    }
#endif

    return qreal(0);
}

template <class T> QVector<qreal> MyThread::getBufferLevels(const T *buffer, int frames, int channels)
{
#ifndef WHITE_TEST

    QVector<qreal> max_values;
    max_values.fill(0, channels);

    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values.at(j))
                max_values.replace(j, value);
        }
    }

    return max_values;
#endif
}
