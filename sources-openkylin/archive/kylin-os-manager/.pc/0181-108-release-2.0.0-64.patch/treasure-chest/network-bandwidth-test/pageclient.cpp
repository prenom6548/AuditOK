#include <QMessageBox>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QIcon>
#include "pageclient.h"

namespace networkbt
{


PageClient::PageClient(QWidget *parent)
    : QWidget(parent),
      m_labelIp(new QLabel(this)),
      m_labelPort(new QLabel(this)),
      m_labelAgree(new QLabel(this)),
      m_lineEdie(new QLineEdit(this)),
      m_spinBox(new QSpinBox(this)),
      m_radioBtnTcp(new QRadioButton(this)),
      m_radioBtnUdp(new QRadioButton(this)),
      m_textEdit(new QTextEdit(this)),
      m_btnConn(new QPushButton(this)),
      m_process(new QProcess(this))
{
	textIpAddres = tr("IP");
	textServerPort = tr("Port");
	textAgreement = tr("Agreement");
	textConnect = tr("Connect");
    textStop = tr("Stop");

    m_labelIp->setText(textIpAddres);
    m_labelPort->setText(textServerPort);
    m_labelAgree->setText(textAgreement);
    m_btnConn->setText(textConnect);

    m_radioBtnTcp->setText("Tcp");
    m_radioBtnTcp->setChecked(true);
    m_radioBtnUdp->setText("Udp");

    m_textEdit->setMinimumSize(404, 216);
    m_textEdit->setReadOnly(true);
    m_spinBox->setRange(0, 65535);

    QGridLayout *gridlayput = new QGridLayout;
    gridlayput->addWidget(m_labelIp, 0, 0);
    gridlayput->addWidget(m_labelPort, 1, 0);
    gridlayput->addWidget(m_labelAgree, 2, 0);
    gridlayput->addWidget(m_lineEdie, 0, 1, 1, 2);
    gridlayput->addWidget(m_spinBox, 1, 1, 1, 2);
    gridlayput->addWidget(m_radioBtnTcp, 2, 1);
    gridlayput->addWidget(m_radioBtnUdp, 2, 2);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addStretch();
    hlayout->addWidget(m_btnConn);

    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->addLayout(gridlayput);
    vboxLayout->addWidget(m_textEdit);
    vboxLayout->addLayout(hlayout);
    setLayout(vboxLayout);

    connect(m_btnConn, &QPushButton::clicked, this, &PageClient::slotClickBtn);
}

PageClient::~PageClient()
{
}
void PageClient::slotClickBtn()
{
    if (m_btnConn->text() == textStop) {
        m_process->close();
        m_btnConn->setText(textConnect);
        return;
    }

    QRegExp regExpIp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    if (!regExpIp.exactMatch(m_lineEdie->text())) {
        QMessageBox::information(this, "提示", "ip格式错误");
        return;
    }

    connect(m_process, &QProcess::readyReadStandardOutput, this, &PageClient::slotGetOutPut, Qt::UniqueConnection);
    connect(m_process, &QProcess::readyReadStandardError, this, &PageClient::slotGetError, Qt::UniqueConnection);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinish()), Qt::UniqueConnection);

    m_textEdit->clear();
    m_btnConn->setText(textStop);
    
    QString agreementType;
    if (m_radioBtnUdp->isChecked()) {
        agreementType = "-u";
    } else {
        agreementType = "";
    }
    QString cmdstr = QString("iperf -c %1 -p %2 -B 0.0.0.0 -f M %3").arg(m_lineEdie->text()).arg(m_spinBox->text()).arg(agreementType);

    m_process->start(cmdstr);
}

void PageClient::slotGetOutPut()
{
    QString output = m_process->readAllStandardOutput();
    m_textEdit->insertPlainText(output);
}

void PageClient::slotGetError()
{
    QString output = m_process->readAllStandardError();
    m_textEdit->insertPlainText(output);
}

void PageClient::slotProcessStop()
{
    m_process->close();    
}

void PageClient::slotProcessFinish()
{
    m_btnConn->setText(textConnect);
}

}