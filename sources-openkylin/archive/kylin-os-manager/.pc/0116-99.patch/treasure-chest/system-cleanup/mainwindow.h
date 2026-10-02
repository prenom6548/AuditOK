#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QSpinBox>
#include <QProcess>
#include <QTimer>
#include <kwidget.h>
#include <QStackedWidget>
#include <QThread>
#include <QQueue>
#include "homepage.h"
#include "cleanupfinishpage.h"
#include "scanfilepage.h"
#include "scanfileslistpage.h"
#include "filemd5.h"

using namespace kdk;
namespace systemcleanup

{

class MainWindow : public KWidget
{
    Q_OBJECT

public:
    MainWindow(KWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *m_stackedWid = nullptr;
    HomePage *m_homePage = nullptr;
    CleanupFinishPage *m_cleanupPage = nullptr;
    ScanFilePage *m_scanFilePage = nullptr;
    ScanFilesListPage *m_scanFileListPage = nullptr;
    ScanFilesListPage *m_scanSameFileListPage = nullptr;
    FileMd5 *m_fileMd5 = nullptr;

    QProcess *m_process = nullptr;
    QThread *m_thread = nullptr;
    QTimer *m_timer = nullptr;

    QQueue<QString> m_queueBigFiles;

    QStringList m_cleanFileList;

    QString m_filePath = "/tmp/komt-sys-cleanup.txt";
    QByteArray m_byteArray;
    int ii = 0;
    void changeCleanSuccesPage(QString succFilesSize);
    void changeCleanFailPage(QStringList failList);
    bool m_isScaningFile = false;
    bool m_isSameFileType = false;
    bool m_findProcessFinish = false;
    bool m_getBigFileListFinish = false;

protected:
    void closeEvent(QCloseEvent *event);

public Q_SLOTS:
    void slotScanBigFile();
    void slotScanSameFile();
    void slotCannelScanFile();
    void slotScanFileSucc();
    void slotCleanUpFiles(QStringList list);
    void slotTimeOutGetBigFileList(); // 使用定时器，从队列中获取文件列表
    void slotGetBigFileListFinish(); // 获取文件列表结束

    // qprocess 获取数据
    void slotProcessReadAllOutput();
    void slotProcessFinish(int exitCode, QProcess::ExitStatus exitStatus);
    // 界面切换
    void slotBackHomePage();
    void slotChangePage();
    void slotCleanRetry();
    void slotCleanFailBackPage();
    // 扫描文件Md5
    void slotGetSameMd5FileInfo(const QHash<QByteArray, QStringList> &hashFileInfo);
Q_SIGNALS:
    void signalProcessFinishSuccess();
    void signalProcessFinishFail();
    void signalFindSameFile(QString path);
    void signalCannelScanFile();
    void signalGetBigFileListFinish(); // 获取文件列表结束
    void signalAddBigFileInfoFinish(); // 获取文件列表
};

}

#endif