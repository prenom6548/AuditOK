#include <QMessageBox>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#include <kmessagebox.h>

#include "mainwindow.h"

namespace networkipa
{

MainWindow::MainWindow(KWidget *parent)
    : KWidget(parent)
    , m_label(new QLabel(this))
    , m_lineEditIp(new QLineEdit(this))
    , m_btnStart(new QPushButton(this))
    , m_process(new QProcess(this))
{
    this->setWidgetName(tr("IP Conflict Detection Tool"));
    this->setIcon(QIcon::fromTheme("kylin-iputils-arping")); // 设置图标
    setFixedSize(420, 184);
    this->setBackgroundRole(QPalette::Base);
    windowButtonBar()->maximumButton()->hide(); 
    windowButtonBar()->menuButton()->hide();

    // 居中
    QScreen *screen = QGuiApplication::primaryScreen();
    this->move(screen->geometry().center() - this->rect().center());


    // 设置样式
    textTest = tr("Test");
    textTesting = tr("Testing");
    textMsgSuccess = tr("The IP address does not conflict !");
    textMsgFail = tr("IP address conflict !");
    textMsgFailLocalIP = tr("The tested IP is the local IP !");
    textMsgFailIPError = tr("IP format error !");
    textMsgFailNetWorkError = tr("IP detection failed, please check the network status"); // IP检测失败，请检查网络状态
    textMsgBtn = tr("OK");

    m_btnStart->setText(textTest);
    m_label->setText(tr("IP"));
    
    // 布局
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_label);
    hLayout->addSpacing(32);
    hLayout->addWidget(m_lineEditIp);
    hLayout->setSpacing(0);
    hLayout->setMargin(0);

    QHBoxLayout *hLayoutBtn = new QHBoxLayout;
    hLayoutBtn->addStretch();
    hLayoutBtn->addWidget(m_btnStart);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(hLayout);
    vLayout->addSpacing(16);
    vLayout->addLayout(hLayoutBtn);
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
    if (m_btnStart->text() == textTesting) {
        m_process->close();
        m_btnStart->setText(textTest);
        return;
    }
    QString ip = m_lineEditIp->text();

    QRegExp regExpIp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    if (!regExpIp.exactMatch(ip)) {
        kdk::KMessageBox w;
        w.setCustomIcon(QIcon::fromTheme("dialog-error"));
        w.setText(textMsgFailIPError);
        QPushButton *btn1 = new QPushButton(textMsgBtn);
        w.addButton(btn1,KMessageBox::YesRole);
        w.exec();
        
        return;
    }

    m_btnStart->setText(textTesting);
    m_btnStart->setEnabled(false);
    m_lineEditIp->clearFocus();

    connect(m_process, &QProcess::readyReadStandardOutput, this, &MainWindow::slotGetOutPut, Qt::UniqueConnection);
    connect(m_process, &QProcess::readyReadStandardError, this, &MainWindow::slotGetError, Qt::UniqueConnection);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinish()), Qt::UniqueConnection);

    QString cmdstr = QString("arping %1 -c 3").arg(ip);
    m_process->start(cmdstr);
}

void MainWindow::slotGetOutPut()
{
    QString output = m_process->readAllStandardOutput();
    QStringList lines = output.split("\n");
    foreach (const QString& line, lines) {
        qDebug() << "arping 命令标准输出 : " << output;
        kdk::KMessageBox w(this);
        QPushButton *btn1 = new QPushButton(textMsgBtn);
        w.addButton(btn1,KMessageBox::YesRole);
        if (line.contains("is not ARPable")) {
            w.setCustomIcon(QIcon::fromTheme("dialog-error"));
            w.setText(textMsgFailLocalIP);
            w.exec();
            return;
        } 
        if (line.contains("NETLINK_ROUTE unexpected iov element")) {
            w.setCustomIcon(QIcon::fromTheme("dialog-error"));
            w.setText(textMsgFailNetWorkError);
            w.exec();
            return;
        }
        if (!line.contains("Received")) {
            continue;
        }
        QStringList list = line.split(" ");
        if (list[1].toInt() > 0) {
            w.setCustomIcon(QIcon::fromTheme("dialog-error"));
            w.setText(textMsgFail);
            w.exec();
            
        } else {
            w.setCustomIcon(QIcon::fromTheme("ukui-dialog-success"));
            w.setText(textMsgSuccess);
            w.exec();
        }
    }
}

void MainWindow::slotGetError()
{
    QString output = m_process->readAllStandardError();
    qDebug() << "arping 命令标准错误 : " << output;
    kdk::KMessageBox w(this);
    w.setCustomIcon(QIcon::fromTheme("dialog-error"));
    QPushButton *btn1 = new QPushButton(textMsgBtn);
    w.addButton(btn1,KMessageBox::YesRole);
    if (output.contains("NETLINK_ROUTE unexpected iov element")) {
        w.setText(textMsgFailNetWorkError);
    } else {
        w.setText(textMsgFail);
    }
    w.exec();
}

void MainWindow::slotProcessFinish()
{
    m_btnStart->setText(textTest);
    m_btnStart->setEnabled(true);
}

}