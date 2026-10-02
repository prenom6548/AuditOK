#include <QMessageBox>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QIcon>
#include <QDebug>

#include "pageserver.h"

namespace networkbt {

PageServer::PageServer(QWidget *parent)
    : QWidget(parent),
      m_btnStart(new QPushButton(this)),
      m_labelPort(new QLabel(this)),
      m_labelAgree(new QLabel(this)),
      m_spinBox(new QSpinBox(this)),
      m_radioBtnTcp(new QRadioButton(this)),
      m_radioBtnUdp(new QRadioButton(this)),
      m_textEdit(new QTextEdit(this)),
      m_process(new QProcess(this))
{
    i18n_map_ = {
        {"bind failed: Permission denied", tr("bind failed: Permission denied")}
    };

	textStart = tr("Start");
    textStop = tr("Stop");
	textPort = tr("Port");
	textAgreement = tr("Agreement");

    m_labelPort->setText(textPort);
    m_labelAgree->setText(textAgreement);
    m_btnStart->setText(textStart);
    m_textEdit->setMinimumSize(404, 216);
    m_textEdit->setReadOnly(true);
    m_spinBox->setRange(0, 65535);

    m_radioBtnTcp->setText("Tcp");
    m_radioBtnTcp->setChecked(true);
    m_radioBtnUdp->setText("Udp");

    QGridLayout *gridlayput = new QGridLayout;
    gridlayput->addWidget(m_labelPort, 0, 0);
    gridlayput->addWidget(m_spinBox, 0, 1, 1, 2);
    gridlayput->addWidget(m_labelAgree, 1, 0);
    gridlayput->addWidget(m_radioBtnTcp, 1, 1);
    gridlayput->addWidget(m_radioBtnUdp, 1, 2);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addStretch();
    hlayout->addWidget(m_btnStart);

    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->addLayout(gridlayput);
    vboxLayout->addWidget(m_textEdit);
    vboxLayout->addLayout(hlayout);

    setLayout(vboxLayout);

    connect(m_btnStart, &QPushButton::clicked, this, &PageServer::slotClickBtn);
}

PageServer::~PageServer()
{
    slotProcessStop();
}

void PageServer::slotClickBtn()
{
    if (m_btnStart->text() == textStop) {
        m_process->close();
        m_btnStart->setText(textStart);
        return;
    }
    connect(m_process, &QProcess::readyReadStandardOutput, this, &PageServer::slotGetOutPut, Qt::UniqueConnection);
    connect(m_process, &QProcess::readyReadStandardError, this, &PageServer::slotGetError, Qt::UniqueConnection);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinish()), Qt::UniqueConnection);
    m_textEdit->clear();
    m_btnStart->setText(textStop);

    QString agreementType;
    if (m_radioBtnUdp->isChecked()) {
        agreementType = "-u";
    } else {
        agreementType = "";
    }
    QString cmdstr = QString("iperf -s -p %1 -B 0.0.0.0 %2").arg(m_spinBox->text()).arg(agreementType);

    m_process->start(cmdstr);
}

void PageServer::slotGetOutPut() {
    QString output = m_process->readAllStandardOutput();
    m_textEdit->insertPlainText(output);
}

void PageServer::slotGetError() {
    QString output = m_process->readAllStandardError();
    auto iter = i18n_map_.find(output.trimmed());
    if (iter != i18n_map_.end()) {
        output = iter.value();
    }

    m_textEdit->insertPlainText(output);
}

void PageServer::slotProcessStop()
{
    m_process->close();    
}

void PageServer::slotProcessFinish()
{
    m_btnStart->setText(textStart);
}

}