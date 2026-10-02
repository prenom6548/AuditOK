#ifndef SCANFILEPAGE_H
#define SCANFILEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QTimer>
#include "component/text_label.h"
#include <kwidget.h>
#include "kprogressbar.h"
#include "scancanneldialog.h" 

namespace systemcleanup
{

class ScanFilePage : public QWidget
{
    Q_OBJECT

public:
    ScanFilePage(QWidget *parent = nullptr);
    ~ScanFilePage();
    void setProgressValue(int value, bool isScanSameFile = false);

private:
    QLabel *m_label = nullptr;
    kdk::KProgressBar *m_progress = nullptr;
    QPushButton *m_btnCannelScan = nullptr;
    QTimer *m_timer = nullptr;
    ScanCannelDialog *m_dialog = nullptr;

    QString m_strLabel;
    QString m_strBtn;
    int timeNum = 0;
    bool isProcessFinish = false;
    bool m_isScanSameFile = false;

public Q_SLOTS:
    void slotContinueScan();
    void slotCannelScan();
    void slotStopScan();
    void slotProcessValue(int value);
    void slotProcessFinish();
    // 扫描相同文件时的进度条
    void slotProcessValueChanged(int current,int total);
    void slotProcessValueChangedMax();
Q_SIGNALS:
    void signalCannelScan();
    void signalScanSuccess();
};

}

#endif 