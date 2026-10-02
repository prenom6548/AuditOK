#ifndef SCANCANNELDIALOG_H
#define SCANCANNELDIALOG_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QSpinBox>
#include <QProcess>
#include <QTimer>
#include "component/text_label.h"
#include <kdialog.h>
#include "kprogressbar.h"
using namespace kdk;
namespace systemcleanup
{

class ScanCannelDialog : public KDialog
{
    Q_OBJECT

public:
    ScanCannelDialog(QWidget *parent = nullptr);
    ~ScanCannelDialog();
    void setProgressValue(int value);

private:
    K::TextLabel *m_labelSummary = nullptr;
    K::TextLabel *m_labelInfo = nullptr;
    QPushButton *m_btnIcon = nullptr;
    QPushButton *m_btnContinu = nullptr;
    QPushButton *m_btnCannelScan = nullptr;

    QString m_strTitle;
    QString m_strInfo;
    QString m_strContinue;
    QString m_strCannel;

public Q_SLOTS:
    void slotBtnContinue();
    void slotBtnCannelScan();
Q_SIGNALS:
    void signalCannelScan();
    void signalContinueScan();
};

}

#endif 