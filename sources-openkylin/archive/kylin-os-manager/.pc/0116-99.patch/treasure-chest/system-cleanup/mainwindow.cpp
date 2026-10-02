#include <QMessageBox>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QIcon>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

#include <kmessagebox.h>

#include "mainwindow.h"

namespace systemcleanup
{

MainWindow::MainWindow(KWidget *parent)
    : KWidget(parent)
    , m_stackedWid(new QStackedWidget(this))
    , m_homePage(new HomePage(this))
    , m_cleanupPage(new CleanupFinishPage(this))
    , m_scanFilePage(new ScanFilePage(this))
    , m_scanFileListPage(new ScanFilesListPage(this))
    , m_scanSameFileListPage(new ScanFilesListPage(this, true))
    , m_fileMd5(new FileMd5(this))
    , m_process(new QProcess(this))
    , m_thread(new QThread(this))
    , m_timer(new QTimer(this))
{
    this->setWidgetName(tr("System Cleanup"));
    this->setIcon(QIcon::fromTheme("kylin-system-cleanup")); // 设置图标
    setFixedSize(904, 568);
    this->setBackgroundRole(QPalette::Base);
    windowButtonBar()->maximumButton()->hide(); 
    windowButtonBar()->menuButton()->hide();

    // 居中
    QScreen *screen = QGuiApplication::primaryScreen();
    this->move(screen->geometry().center() - this->rect().center());
    
    m_stackedWid->addWidget(m_homePage);
    m_stackedWid->addWidget(m_scanFilePage);
    m_stackedWid->addWidget(m_scanFileListPage);
    m_stackedWid->addWidget(m_scanSameFileListPage);
    m_stackedWid->addWidget(m_cleanupPage);
    m_stackedWid->setCurrentIndex(0);
    auto *vBoxLayout = new QVBoxLayout;
    vBoxLayout->addWidget(m_stackedWid);
    vBoxLayout->setSpacing(0);
    vBoxLayout->setMargin(0);
    baseBar()->setLayout(vBoxLayout);

    m_thread->start();
    m_fileMd5->moveToThread(m_thread);

    m_timer->setInterval(100);// 使用计时器，每隔100毫秒将队列中的数据插入到表格中
    connect(m_timer, &QTimer::timeout, this, &MainWindow::slotTimeOutGetBigFileList);

    qRegisterMetaType<QHash<QByteArray,QStringList>>("QHash<QByteArray,QStringList>");//注册<QHash<QByteArray,QStringList>
    connect(m_fileMd5, &FileMd5::signalFileInfo, this, &MainWindow::slotGetSameMd5FileInfo);
    connect(m_fileMd5, &FileMd5::signalProgressValue, m_scanFilePage, &ScanFilePage::slotProcessValueChanged);
    connect(this, &MainWindow::signalFindSameFile, m_fileMd5, &FileMd5::slotGetFileInfo);
    connect(this, &MainWindow::signalCannelScanFile, m_fileMd5, &FileMd5::slotCannelGetMd5);

    connect(m_homePage, &HomePage::signalStartScanBigFile, this, &MainWindow::slotScanBigFile);
    connect(m_homePage, &HomePage::signalStartScanSameFile, this, &MainWindow::slotScanSameFile);

    connect(m_scanFilePage, &ScanFilePage::signalCannelScan, this, &MainWindow::slotCannelScanFile);
    connect(m_scanFilePage, &ScanFilePage::signalScanSuccess, this, &MainWindow::slotScanFileSucc);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &MainWindow::slotProcessReadAllOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, [=](){
        QByteArray output = m_process->readAllStandardError();
        qDebug() << " 查询失败的部分： " << QString::fromUtf8(output);
    });
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinish(int,QProcess::ExitStatus)));

    connect(m_scanFileListPage, &ScanFilesListPage::signalBtnBack, this, &MainWindow::slotBackHomePage);
    connect(m_scanFileListPage, &ScanFilesListPage::signalClearUp, this, &MainWindow::slotCleanUpFiles);
    
    connect(m_scanSameFileListPage, &ScanFilesListPage::signalBtnBack, this, &MainWindow::slotBackHomePage);
    connect(m_scanSameFileListPage, &ScanFilesListPage::signalClearUp, this, &MainWindow::slotCleanUpFiles);

    connect(m_cleanupPage, &CleanupFinishPage::signalNextPage, this, &MainWindow::slotChangePage);
    connect(m_cleanupPage, &CleanupFinishPage::signalBtnBack, this, &MainWindow::slotCleanFailBackPage);
    connect(m_cleanupPage, &CleanupFinishPage::signalBtnRetry, this, &MainWindow::slotCleanRetry);

    // 进度界面与加载大文件界面切换
    connect(this, &MainWindow::signalAddBigFileInfoFinish, m_scanFilePage, &ScanFilePage::slotProcessFinish);
    connect(this, &MainWindow::signalProcessFinishSuccess, this, &MainWindow::slotGetBigFileListFinish, Qt::QueuedConnection);
    connect(this, &MainWindow::signalGetBigFileListFinish, this, &MainWindow::slotGetBigFileListFinish, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    m_thread->quit();
}

void MainWindow::slotProcessReadAllOutput()
{
    QByteArray output = m_process->readAllStandardOutput();

    m_byteArray = m_byteArray.append(output);

    while (m_byteArray.indexOf("\n") > 0)
    {
        int index = m_byteArray.indexOf("\n");
        QString path = m_byteArray.left(index);
        qDebug() << " == " << path << " == " << ++ii;
        m_byteArray.remove(0, index + 1);

        QFile file(path);
        if (file.exists()) {
            m_queueBigFiles.enqueue(path);
        }
        if (!m_timer->isActive()) {
            m_timer->start();
        }
        
    }
}

void MainWindow::slotProcessFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        //程序正常结束
        qDebug() << " Find file程序正常结束 ";
        m_findProcessFinish=true;
        Q_EMIT signalProcessFinishSuccess();
    } else {
        //程序异常结束
        qDebug() << " Find file程序异常结束 ";
        m_findProcessFinish=true;
        Q_EMIT signalProcessFinishSuccess();
    }
}

void MainWindow::slotChangePage()
{
    m_stackedWid->setCurrentIndex(0);
}

void systemcleanup::MainWindow::slotCleanRetry()
{
    slotCleanUpFiles(m_cleanFileList);
}

void MainWindow::slotCleanFailBackPage()
{
    m_stackedWid->setCurrentIndex(2);
}

void MainWindow::slotGetSameMd5FileInfo(const QHash<QByteArray, QStringList> &hashFileInfo)
{
    for (QHash<QByteArray,QStringList>::const_iterator itr = hashFileInfo.begin(); itr != hashFileInfo.end(); ++itr) {
        if(itr.value().count()>1) {
            for (const QString &str : itr.value()) {
                m_scanSameFileListPage->addTableWidgetItem(str,itr.key());
                qApp->processEvents();
            }
        }
    }
}

void MainWindow::slotScanBigFile()
{
    // 扫描状态和扫描类型
    m_isScaningFile = true;
    m_isSameFileType = false;
    m_byteArray.clear();

    m_findProcessFinish = false;
    m_getBigFileListFinish = false;

    m_scanFilePage->setProgressValue(0); // 进度条重置
    m_scanFileListPage->clearTableWidget(); // 清除表格
    m_stackedWid->setCurrentIndex(1);

    QString comm = QString(" find /home/%1 -type f -size +100M ").arg(getenv("USER"));
    m_process->start(comm);
}

void MainWindow::slotScanSameFile()
{
    m_isScaningFile = true;
    m_isSameFileType = true;

    m_scanFilePage->setProgressValue(0, true); // 进度条重置
    m_scanSameFileListPage->clearTableWidget(); // 清除表格
    m_stackedWid->setCurrentIndex(1);

    QString path = QString("/home/%1").arg(getenv("USER"));
    // path = QString("/home/zxx/桌面/eeeeeee");
    Q_EMIT signalFindSameFile(path);
}

void MainWindow::slotCannelScanFile()
{
    m_isScaningFile = false;
    m_process->close();
    m_stackedWid->setCurrentIndex(0);
    
    if (m_timer->isActive()) {
        m_timer->stop();
        m_queueBigFiles.clear();
    }
    if (m_isSameFileType) {
        Q_EMIT signalCannelScanFile();
    }
}

void MainWindow::slotScanFileSucc()
{
    m_isScaningFile = false;
    if (m_isSameFileType) {
        m_stackedWid->setCurrentIndex(3);
    } else {
        m_process->close();
        m_stackedWid->setCurrentIndex(2);
    }
}

void MainWindow::slotBackHomePage()
{
    m_stackedWid->setCurrentIndex(0);

    if (m_timer->isActive()) {
        m_timer->stop();
        m_queueBigFiles.clear();
    }
}

void MainWindow::slotCleanUpFiles(QStringList list)
{
    m_cleanFileList = list;
    QStringList successlist;
    QStringList failList;
    _Float64 size = 0;
    qDebug() << " === " << list;
    for (int i = 0; i < list.size(); i++) {
        _Float64 fileSize = QFile(list.at(i)).size();
        // 调用remove()函数删除文件
        bool success = QFile::remove(list.at(i));
        if (success) {
            qDebug("文件删除成功！");
            successlist.append(list.at(i));
            size = size + fileSize;
            m_cleanFileList.removeOne(list.at(i));
        } else {
            qDebug("文件删除失败！");
            failList.append(list.at(i));
        }
    }

    // 计算大小
    int num = 0;
    QString suffixStr = "B";
    if (size > 1024) {
        for (int i = 0; size > 1024; i++) {
            size = size / 1024;
            num = i + 1;
            if (num == 1) {
                suffixStr = "KB";
            } else if(num == 2) {
                suffixStr = "MB";
            } else if(num == 3) {
                suffixStr = "GB";
                break;
            }
        }
    }
    QString tmpSize = QString::number(size, 'f', 1);


    m_scanFileListPage->slotCleanFileSuccess(successlist);
    if (failList.size() > 0) {
        changeCleanFailPage(failList);
    } else {
        changeCleanSuccesPage(tmpSize + suffixStr);
    }
    
}

void MainWindow::slotTimeOutGetBigFileList()
{
    if (m_queueBigFiles.isEmpty()) {
        m_timer->stop();
        m_scanFileListPage->sortTableWidget(2);
        m_getBigFileListFinish = true;
        emit signalGetBigFileListFinish();
        return;
    }
    m_getBigFileListFinish = false;
    int getCount = 10 > m_queueBigFiles.size() ? m_queueBigFiles.size() : 10;
    for (int i = 0; i < getCount; i++) {
        m_scanFileListPage->addTableWidgetItem(m_queueBigFiles.dequeue());
    }
}

void MainWindow::slotGetBigFileListFinish()
{
    // 读取队列中的文件，加载到大文件列表完成后，判断find进程是否结束
    if (!m_findProcessFinish) {
        m_getBigFileListFinish = false;
    } else {
        // find进程结束,判断加载大文件列表是否结束
        if (m_queueBigFiles.isEmpty()) {
            m_getBigFileListFinish = true;
        }
    }
    
    // 判断是否都结束
    if (m_getBigFileListFinish && m_findProcessFinish) {
        Q_EMIT signalAddBigFileInfoFinish();
    }
    return;
}

void MainWindow::changeCleanSuccesPage(QString succFilesSize)
{
    m_stackedWid->setCurrentIndex(4);
    m_cleanupPage->setCleanUpSucc(succFilesSize);
}

void MainWindow::changeCleanFailPage(QStringList failList)
{
    m_stackedWid->setCurrentIndex(4);
    m_cleanupPage->setCleanUpFail(failList);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 关闭窗口时，先判断是否正在扫描文件
    if (m_isScaningFile) {
        event->ignore();
        m_scanFilePage->slotStopScan();
    }
}
}