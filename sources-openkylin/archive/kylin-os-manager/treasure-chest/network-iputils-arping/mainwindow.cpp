#include "mainwindow.h"
#include <functional>
#include <QMessageBox>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QEventLoop>
#include <QTimer>
#include <kmessagebox.h>

#ifdef V11_ENVIRONMENT
    #include "kom_process.h"
#else
    #include <QProcess>
#endif

namespace networkipa {

class ScopeGuard {
public:
    ScopeGuard(std::function<void()> constructorCallback, std::function<void()> destructorCallback)
        : m_constructorCallback(std::move(constructorCallback)),
          m_destructorCallback(std::move(destructorCallback)) {
        if (m_constructorCallback) {
            m_constructorCallback();
        }
    }
    ~ScopeGuard() {
        if (m_destructorCallback) {
            m_destructorCallback();
        }
    }

private:
    std::function<void()> m_constructorCallback;
    std::function<void()> m_destructorCallback;
};

MainWindow::MainWindow(KWidget *parent)
    : KWidget(parent),
      m_label(new QLabel(this)),
      m_lineEditIp(new QLineEdit(this)),
      m_btnStart(new QPushButton(this)) {
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

void MainWindow::slotClickBtn() {
    ScopeGuard scopeGuard([this]() {
                              m_btnStart->setEnabled(false);
                              m_btnStart->setText(textTesting);
                          },
                          [this]() {
                              m_btnStart->setText(textTest);
                              m_btnStart->setEnabled(true);
                          }
    );

    QString ip = m_lineEditIp->text();
    m_lineEditIp->clearFocus();
    QRegExp regExpIp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    if (!regExpIp.exactMatch(ip)) {
        kdk::KMessageBox::critical(this, "", textMsgFailIPError);
        return;
    }

    QEventLoop eventLoop; 

    bool    isTimeout{false};
    QString standardOutput;
    QString standardError;

#ifdef V11_ENVIRONMENT
    kom::KomProcess process;
#else
    QProcess process;
#endif

    connect(&process,
#ifdef V11_ENVIRONMENT
            &kom::KomProcess::readyReadStandardOutput,
#else
            &QProcess::readyReadStandardOutput,
#endif
            &process,
            [&]() {
                standardOutput.append(QString::fromUtf8(process.readAllStandardOutput()));
            }
    );
    connect(&process,
#ifdef V11_ENVIRONMENT
            &kom::KomProcess::readyReadStandardError,
#else
            &QProcess::readAllStandardError,
#endif
            &process,
            [&]() {
                standardError.append(QString::fromUtf8(process.readAllStandardError()));
            }
    );
    connect(&process,
#ifdef V11_ENVIRONMENT
            static_cast<void (kom::KomProcess::*)(int, QProcess::ExitStatus)>(&kom::KomProcess::finished),
#else
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
#endif
            &process,
            [&]() {
                if (eventLoop.isRunning()) {
                    eventLoop.exit();
                }
            }
    );
    QTimer::singleShot(10 * 1000, &eventLoop, [&]() {
        if (eventLoop.isRunning()) {
            isTimeout = true;
            process.close();
            eventLoop.exit();
        }
    });

    auto command = QString("arping %1 -c 3").arg(ip);
    process.start(command);

    eventLoop.exec();

    qInfo("standard output:\n%s", qUtf8Printable(standardOutput));
    qInfo("standard error:\n%s", qUtf8Printable(standardError));

    if (isTimeout) {
        kdk::KMessageBox::critical(this, "", tr("Timeout detection"));
        return;
    }
    if (standardOutput.contains("NETLINK_ROUTE unexpected iov element")) {
        kdk::KMessageBox::critical(this, "", textMsgFailNetWorkError);
        return;
    }
    if (standardOutput.contains("is not ARPable")) {
        kdk::KMessageBox::critical(this, "", textMsgFailLocalIP);
        return;
    }

    const auto lines = standardOutput.split('\n');
    for (const auto &l : lines) {
        if (l.contains("Received")) {
            auto e = l.split(' ');
            e.removeAll("");
            if (e.count() < 2) {
                continue;
            }
            auto receivedNumStr = e.at(1);
            bool ok{false};
            auto receivedNum = receivedNumStr.toInt(&ok);
            if (!ok) {
                qCritical() << "received package number convert int error";
                continue;
            }
            if (receivedNum == 0) {
                // 没有机器回应，ip 不冲突
                kdk::KMessageBox::success(this, "", textMsgSuccess);
                return;
            } else {
                kdk::KMessageBox::critical(this, "", textMsgFail);
                return;
            }
        }
    }

    // 未识别的情况
    qWarning() << "unsupported scenarios";
    kdk::KMessageBox::critical(this, "", tr("Error detection"));
}

}
