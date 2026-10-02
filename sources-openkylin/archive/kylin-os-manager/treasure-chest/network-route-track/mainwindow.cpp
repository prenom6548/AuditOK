#include <QMessageBox>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

#include "mainwindow.h"

namespace networkrt
{

MainWindow::MainWindow(KWidget *parent)
    : KWidget(parent)
    , m_btnStart(new QPushButton(this))
    , m_label(new QLabel(this))
    , m_lineEditIp(new QLineEdit(this))
    , m_textEditOutPut(new QTextEdit(this))
#ifdef V11_ENVIRONMENT
    , m_process(new kom::KomProcess(this))
#else
    , m_process(new QProcess(this))
#endif
{
    this->setWidgetName(tr("Network Routing Tracking Tool"));
    this->setIcon(QIcon::fromTheme("kylin-traceroute")); // 设置图标
    setFixedSize(452, 356);
    this->setBackgroundRole(QPalette::Base);
    windowButtonBar()->maximumButton()->hide();
    windowButtonBar()->menuButton()->hide();

    // 居中
    QScreen *screen = QGuiApplication::primaryScreen();
    this->move(screen->geometry().center() - this->rect().center());

    strStrat = tr("Start");
    strStop = tr("Stop");

    m_btnStart->setText(strStrat);
    m_label->setText(tr("IP/domain name"));
    m_textEditOutPut->setReadOnly(true);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_label);
    hLayout->addStretch();
    hLayout->addWidget(m_lineEditIp);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_btnStart);
    hLayout->setSpacing(0);
    hLayout->setMargin(0);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(hLayout);
    vLayout->addSpacing(16);
    vLayout->addWidget(m_textEditOutPut);
    vLayout->setContentsMargins(24, 16, 24, 24);

    baseBar()->setLayout(vLayout);

    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::slotClickBtn);
}

MainWindow::~MainWindow()
{
    m_process->close();
}

void MainWindow::slotClickBtn()
{
    if (m_btnStart->text() == strStop) {
        m_process->close();
        m_btnStart->setText(strStrat);
        return;
    }
    m_textEditOutPut->clear();
    m_btnStart->setEnabled(false);
    QString ip = m_lineEditIp->text();

    m_btnStart->setEnabled(true);
    m_btnStart->setText(strStop);
    m_lineEditIp->clearFocus();

    QString cmdstr = QString("traceroute %1").arg(ip);
#ifdef V11_ENVIRONMENT
    connect(m_process, &kom::KomProcess::readyReadStandardOutput, this, &MainWindow::slotGetOutPut, Qt::UniqueConnection);
    connect(m_process, &kom::KomProcess::readyReadStandardError, this, &MainWindow::slotGetError, Qt::UniqueConnection);
#else
    connect(m_process, &QProcess::readyReadStandardOutput, this, &MainWindow::slotGetOutPut, Qt::UniqueConnection);
    connect(m_process, &QProcess::readyReadStandardError, this, &MainWindow::slotGetError, Qt::UniqueConnection);
#endif
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinish()), Qt::UniqueConnection);
    m_process->start(cmdstr);
}

void MainWindow::slotGetOutPut()
{
    QString output = m_process->readAllStandardOutput();
    m_textEditOutPut->insertPlainText(output);
}

void MainWindow::slotGetError()
{
    QString output = m_process->readAllStandardError();
    m_textEditOutPut->insertPlainText(output);
}

void MainWindow::slotProcessFinish()
{
    m_btnStart->setText(strStrat);
}

}