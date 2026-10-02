#ifndef PAGECLIENT_H
#define PAGECLIENT_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

#ifdef V11_ENVIRONMENT
	#include "kom_process.h"
#else
	#include <QProcess>
#endif

namespace networkbt
{

class PageClient : public QWidget
{
	Q_OBJECT

public:
	PageClient(QWidget *parent = nullptr);
	~PageClient();

private:
	QLabel *m_labelIp = nullptr;
	QLabel *m_labelPort = nullptr;
	QLabel *m_labelAgree = nullptr;
	QLineEdit *m_lineEdie = nullptr;
	QSpinBox *m_spinBox = nullptr;
	QRadioButton *m_radioBtnTcp = nullptr;
	QRadioButton *m_radioBtnUdp = nullptr;
	QTextEdit *m_textEdit = nullptr;
	QPushButton *m_btnConn = nullptr;
#ifdef V11_ENVIRONMENT
	kom::KomProcess *m_process = nullptr;
#else
	QProcess *m_process = nullptr;
#endif

	QString textIpAddres;
	QString textServerPort;
	QString textAgreement;
	QString textConnect;
    QString textStop;

public slots:
    void slotClickBtn();
    void slotGetOutPut();
    void slotGetError();
    void slotProcessStop();
    void slotProcessFinish();
};

}

#endif