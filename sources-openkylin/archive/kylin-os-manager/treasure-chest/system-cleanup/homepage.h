#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QSpinBox>
#include <QProcess>
#include <QTimer>
#include "component/text_label.h"
#include <kwidget.h>
#include "component/radio_widget.h"

namespace systemcleanup
{

class HomePage : public QWidget
{
    Q_OBJECT

public:
    HomePage(QWidget *parent = nullptr);
    ~HomePage();

private:
    K::TextLabel *m_labelSummary = nullptr;
    K::TextLabel *m_labelInfo = nullptr;
    RadioWidget *m_radioBigFile = nullptr;
    RadioWidget *m_radioSameFile = nullptr;
    QPushButton *m_btnScan = nullptr;

    QString m_strSummary;
    QString m_strInfo;
    QString m_strBtnScan;
    QString m_strBigFile;
    QString m_strSameFile;

    bool m_boolChooseBigFile = false;
    bool m_boolChooseSameFile = false;

public Q_SLOTS:
    void slotClickBtn();
    void slotClickBigFileRadioBtn();
    void slotCanncelClickBigFileRadioBtn();
    void slotClickSameFileRadioBtn();
    void slotCanncelClickSameFileRadioBtn();
Q_SIGNALS:
    void signalStartScanBigFile();
    void signalStartScanSameFile();
};

}

#endif 