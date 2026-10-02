#ifndef PAGESERVER_H
#define PAGESERVER_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>
#include <QSpinBox>
#include <QProcess>
#include <QHash>

namespace networkbt
{

class PageServer : public QWidget
{
    Q_OBJECT

public:
    PageServer(QWidget *parent = nullptr);
    ~PageServer();

private:
    QPushButton *m_btnStart = nullptr;
    QLabel *m_labelPort = nullptr;
    QLabel *m_labelAgree = nullptr;
    QSpinBox *m_spinBox = nullptr;
    QRadioButton *m_radioBtnTcp = nullptr;
    QRadioButton *m_radioBtnUdp = nullptr;
    QTextEdit *m_textEdit = nullptr;	
    QProcess *m_process = nullptr;

    QString textStart;
    QString textStop;
    QString textPort;
    QString textAgreement;
    QHash<QString, QString> i18n_map_;

public slots:
    void slotClickBtn();
    void slotGetOutPut();
    void slotGetError();
    void slotProcessStop();
    void slotProcessFinish();
};

}

#endif