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

namespace networktelnet
{

MainWindow::MainWindow(KWidget *parent)
    : KWidget(parent), m_btnTest(new QPushButton(this)), m_label(new QLabel(this)), m_lineEditIp(new QLineEdit(this)),
      m_labelPort(new QLabel(this)), m_spinBox(new QSpinBox(this)), m_process(new QProcess(this)),
      m_timer(new QTimer(this))
{
    this->setWidgetName(tr("Network Connectivity Detection Tool"));
    this->setIcon(QIcon::fromTheme("kylin-telnet")); // 设置图标
    setFixedSize(420, 244);
    this->setBackgroundRole(QPalette::Base);
    windowButtonBar()->maximumButton()->hide();
    windowButtonBar()->menuButton()->hide();

    // 居中
    QScreen *screen = QGuiApplication::primaryScreen();
    this->move(screen->geometry().center() - this->rect().center());

    textTest = tr("Test");
    textTesting = tr("Testing");
    textIP = tr("IP/domain name");
    textPort = tr("Port");
    textConnSuccess = tr("Connection successful!");
    textConnFail = tr("Connection failed!");
    textBtnOk = tr("OK");

    m_btnTest->setText(textTest);
    m_label->setText(textIP);
    m_labelPort->setText(textPort);
    m_spinBox->setRange(0, 65535);

    QGridLayout *gridlayput = new QGridLayout;
    gridlayput->addWidget(m_label, 0, 0);
    gridlayput->addWidget(m_labelPort, 1, 0);
    gridlayput->addWidget(m_lineEditIp, 0, 1, 1, 2);
    gridlayput->addWidget(m_spinBox, 1, 1, 1, 2);
    gridlayput->setVerticalSpacing(24);

    QHBoxLayout *hLayoutBtn = new QHBoxLayout;
    hLayoutBtn->addStretch();
    hLayoutBtn->addWidget(m_btnTest);
    hLayoutBtn->setSpacing(0);
    hLayoutBtn->setMargin(0);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(gridlayput);
    vLayout->addSpacing(32);
    vLayout->addLayout(hLayoutBtn);
    vLayout->setContentsMargins(24, 16, 24, 24);

    baseBar()->setLayout(vLayout);

    connect(m_btnTest, &QPushButton::clicked, this, &MainWindow::slotClickBtn);

    m_btnTest->setEnabled(false);
    connect(m_lineEditIp, &QLineEdit::textChanged, this, [=](QString str) {
        m_btnTest->setEnabled(!str.isEmpty());
    });
}

MainWindow::~MainWindow()
{
    m_process->close();
    m_timer->stop();
}

void MainWindow::slotClickBtn()
{
    if (m_btnTest->text() == textTesting) {
        m_process->close();
        m_btnTest->setText(textTest);
        return;
    }
    m_btnTest->setEnabled(false);

    QString ip = m_lineEditIp->text();
    QString port = m_spinBox->text();

    m_btnTest->setText(textTesting);
    m_lineEditIp->clearFocus();

    connect(m_process, &QProcess::readyReadStandardOutput, this, &MainWindow::slotGetOutPut, Qt::UniqueConnection);
    connect(m_process, &QProcess::readyReadStandardError, this, &MainWindow::slotGetError, Qt::UniqueConnection);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinish()),
            Qt::UniqueConnection);

    QString cmdstr = QString("telnet %1 %2").arg(ip).arg(port);
    m_process->start(cmdstr);

    connect(m_timer, &QTimer::timeout, this, &MainWindow::slotTimeOut, Qt::UniqueConnection);
    m_timer->start(10000);
}

void MainWindow::slotGetOutPut()
{
    QString output = m_process->readAllStandardOutput();
    if (output.contains("Connected to ")) {

        m_process->close();

        kdk::KMessageBox w;
        w.setCustomIcon(QIcon::fromTheme("ukui-dialog-success"));
        w.setText(textConnSuccess);
        QPushButton *btn1 = new QPushButton(textBtnOk);
        w.addButton(btn1, KMessageBox::YesRole);
        w.exec();
    }
}

void MainWindow::slotGetError()
{
    QString output = m_process->readAllStandardError();
    qDebug() << " telnet 标准错误输出 " << output;
    kdk::KMessageBox w;
    w.setCustomIcon(QIcon::fromTheme("dialog-error"));
    w.setText(textConnFail);
    QPushButton *btn1 = new QPushButton(textBtnOk);
    w.addButton(btn1, KMessageBox::YesRole);
    w.exec();
    m_process->close();
}

void MainWindow::slotProcessFinish()
{
    m_btnTest->setText(textTest);
    m_btnTest->setEnabled(true);
    m_timer->stop();
}

void MainWindow::slotTimeOut()
{
    slotProcessFinish();
    m_process->close();

    kdk::KMessageBox w;
    w.setCustomIcon(QIcon::fromTheme("dialog-error"));
    w.setText(textConnFail);
    QPushButton *btn1 = new QPushButton(textBtnOk);
    w.addButton(btn1, KMessageBox::YesRole);
    w.exec();
}

} // namespace networktelnet
