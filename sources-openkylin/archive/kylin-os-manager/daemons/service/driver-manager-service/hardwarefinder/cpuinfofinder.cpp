#include "cpuinfofinder.h"
#include <QProcess>
#include <QDebug>
#include <QFile>

CpuInfoFinder::CpuInfoFinder(QObject *parent) : QObject(parent) {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(&timer, &QTimer::timeout, this, &CpuInfoFinder::runDmidecodeAndSave);
    timer.start(1000);
    connect(m_thread, &QThread::started, this, &CpuInfoFinder::dowork);

    connect(this, &CpuInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &CpuInfoFinder::finished, this, &CpuInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

CpuInfoFinder::~CpuInfoFinder() {

}

CpuInfo CpuInfoFinder::getCpuInfo() {
    return m_cpuInfo;
}

void CpuInfoFinder::dowork() {
    QProcess proc;
    QString cmd = "lscpu";
    proc.setEnvironment(QStringList() << "LANGUAGE=en_us");
    proc.start(cmd);
    proc.waitForFinished();
    QStringList outPut = QString(proc.readAllStandardOutput()).split("\n");
    for (int i = 0; i < outPut.length(); ++i) {
        if(QString(outPut[i]).contains("Architecture:")){
            m_cpuInfo.arch = QString(QString(outPut[i]).split("Architecture:")[1]).trimmed();
            continue;
        }

        // CPU(s)代表的是逻辑处理器的数量
        if (QString(outPut[i]).contains("CPU(s):") && !QString(outPut[i]).contains("-") && !QString(outPut[i]).contains(",")) {
            m_cpuInfo.cpus = QString(QString(outPut[i]).split("CPU(s):")[1]).trimmed();
            continue;
        }

        // 在线的逻辑处理器的数量
        if(QString(outPut[i]).contains("On-line CPU(s) list:")){
            QString res = QString(outPut[i]).split("On-line CPU(s) list:")[1];
            if(res.contains("-")){
                m_cpuInfo.OnlineCpus = QString::number(QString(QString(QString(res).split(":").last()).split("-")[1]).trimmed().toInt() + 1);
            }else{
                m_cpuInfo.OnlineCpus = 2;
            }
            continue;
        }

        if(QString(outPut[i]).contains("Core(s) per socket:")){
            m_cpuInfo.core_per_socket = QString(QString(outPut[i]).split("Core(s) per socket:")[1]).trimmed();
            continue;
        }

        if(QString(outPut[i]).contains("Socket(s):")){
            m_cpuInfo.socket = QString(QString(outPut[i]).split("Socket(s):")[1]).trimmed();
            continue;
        }

        if(QString(outPut[i]).contains("Thread(s) per core:")){
            m_cpuInfo.threadprecore = QString(QString(outPut[i]).split("Thread(s) per core:")[1]).trimmed();
            continue;
        }
        if(QString(outPut[i]).contains("CPU max MHz:")){
            m_cpuInfo.maxMHz = QString(QString(outPut[i]).split("CPU max MHz:")[1]).trimmed() + " MHz";
            continue;
        }
        if(QString(outPut[i]).contains("L1d cache:")){
            m_cpuInfo.L1dcache = QString(QString(outPut[i]).split("L1d cache:")[1]).trimmed();
            continue;
        }
        if(QString(outPut[i]).contains("L1i cache:")){
            m_cpuInfo.L1icache = QString(QString(outPut[i]).split("L1i cache:")[1]).trimmed();
            continue;
        }
        if(QString(outPut[i]).contains("L2 cache:")){
            m_cpuInfo.L2cache = QString(QString(outPut[i]).split("L2 cache:")[1]).trimmed();
            continue;
        }
        if(QString(outPut[i]).contains("L3 cache:")){
            m_cpuInfo.L3cache = QString(QString(outPut[i]).split("L3 cache:")[1]).trimmed();
            continue;
        }
        if(QString(outPut[i]).contains("L4 cache:")){
            m_cpuInfo.L4cache = QString(QString(outPut[i]).split("L4 cache:")[1]).trimmed();
            continue;
        }
    }
    proc.close();

    m_cpuInfo.cores = QString::number(m_cpuInfo.socket.toInt() * m_cpuInfo.core_per_socket.toInt());
    if (m_cpuInfo.cpus == m_cpuInfo.OnlineCpus){
        m_cpuInfo.onlinecores = m_cpuInfo.cores;
    }else{
        m_cpuInfo.onlinecores = QString::number(QString::number(m_cpuInfo.socket.toInt() * m_cpuInfo.core_per_socket.toInt()).toInt() / 2);
    }

    QProcess proc2;
    QString cmd2 = "dmidecode -t processor";
    proc2.setEnvironment(QStringList() << "LANGUAGE=en_us");
    proc2.start(cmd2);
    proc2.waitForFinished();
    QStringList outPut2 = QString(proc2.readAllStandardOutput()).split("\n");
    for (int i = 0; i < outPut2.length(); ++i) {
        if (QString(outPut2[i]).contains("Manufacturer:")) {
            m_cpuInfo.vendor = QString(QString(outPut2[i]).split("Manufacturer:")[1]).trimmed();
            continue;
        }

        if (QString(outPut2[i]).contains("Version:")) {
            m_cpuInfo.model = QString(QString(outPut2[i]).split("Version:")[1]).trimmed();
            continue;
        }
    }

    if (m_cpuInfo.cores != "" && m_cpuInfo.thread != "" && m_cpuInfo.threadprecore == ""){
        m_cpuInfo.threadprecore = QString::number(m_cpuInfo.thread.toInt() / m_cpuInfo.cores.toInt());
    }

    proc2.close();

    emit succeed();
}

void CpuInfoFinder::runDmidecodeAndSave() {
    QProcess process;
    process.start("dmidecode", QStringList() << "-t" << "processor");

    if (!process.waitForFinished()) {
        qWarning() << "dmidecode did not finish!";
        return;
    }

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (!error.isEmpty()) {
        qWarning() << "dmidecode error:" << error;
    } else {
        QFile file("/tmp/kylin-os-manager-cpu.dat");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << output;
            file.close();
        } else {
            qWarning() << "Could not open file for writing!";
        }
    }
}
