#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDBusConnection>
#include <QDBusMessage>
#include <kmessagebox.h>
#if 0
#include <kballontip.h>
#endif

namespace source_update {

MainWindow::MainWindow(QWidget *parent)
    : kdk::KWidget(parent),
      desc_label_(new K::TextLabel(this)),
      exec_btn_(new QPushButton(this))
{
    setWindowIcon(QIcon::fromTheme("kylin-source-update-tool"));
    setWindowTitle(tr("Kylin Source Update Tool"));
    setIcon("kylin-source-update-tool");
    setWidgetName(tr("Kylin Source Update Tool"));
    windowButtonBar()->menuButton()->hide();
    windowButtonBar()->maximumButton()->hide();
    setFixedSize(380, 170);

    desc_label_->SetText(tr("Do you want to perform the source update operation?"));
    auto *desc_hbox_layout = new QHBoxLayout;
    desc_hbox_layout->setSpacing(0);
    desc_hbox_layout->setContentsMargins(0, 0, 0, 0);
    desc_hbox_layout->addWidget(desc_label_);
    desc_hbox_layout->addStretch();

    exec_btn_->setText(tr("Execute"));
    exec_btn_->resize(96, 36);
    connect(exec_btn_, &QPushButton::clicked, this, &MainWindow::on_Exec);
    auto *exec_btn_hbox_layout = new QHBoxLayout;
    exec_btn_hbox_layout->setSpacing(0);
    exec_btn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    exec_btn_hbox_layout->addStretch();
    exec_btn_hbox_layout->addWidget(exec_btn_);

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(24, 16, 24, 24);
    vbox_layout->addLayout(desc_hbox_layout);
    vbox_layout->addSpacing(32);
    vbox_layout->addLayout(exec_btn_hbox_layout);

    baseBar()->setLayout(vbox_layout);
}

void MainWindow::on_Exec() {
    exec_btn_->setText(tr("Executing..."));
    exec_btn_->setDisabled(true);

    auto message = QDBusMessage::createMethodCall("com.kylin-os-manager",
                                   "/com/KylinOsManager/ToolBox",
                                   "com.KylinOsManager.ToolBox",
                                   "UpdateSource");
    QDBusConnection::systemBus().connect("com.kylin-os-manager",
                                         "/com/KylinOsManager/ToolBox",
                                         "com.KylinOsManager.ToolBox",
                                         "UpdateSourceFinish",
                                         this,
                                         SLOT(on_UpdateSourceFinish(int)));
    auto reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Source update call d-bus interface fail: " << reply.errorMessage();
        BubblePopup(false);
    }
}

void MainWindow::BubblePopup(bool ok) {
    hide();

#if 0
    auto *popup = new kdk::KBallonTip;
    popup->setWindowFlag(Qt::FramelessWindowHint);
    popup->setAttribute(Qt::WA_TranslucentBackground);
    popup->setTipTime(2000);
    popup->setContentsMargins(20, 20, 20, 20);
    if (ok) {
        popup->setTipType(kdk::TipType::Normal);
        popup->setText(tr("Execution successful"));
    } else {
        popup->setTipType(kdk::TipType::Error);
        popup->setText(tr("Execution fail"));
    }
    popup->showInfo();

    // 移动到屏幕中间
    auto width = QApplication::desktop()->width();
    auto height = QApplication::desktop()->height();
    int x = width / 2 - popup->width() / 2;
    int y = height / 2 - popup->height() / 2;
    popup->move(x, y);

    // 启动定时器，5 秒后结束进程
    QTimer::singleShot(5000, qApp, &QApplication::quit);
#endif

    // 改用对话框形式的弹窗
    kdk::KMessageBox box;
    box.addButton(kdk::KMessageBox::StandardButton::Ok);
    if (ok) {
        box.setCustomIcon(QIcon::fromTheme("ukui-dialog-success"));
        box.setText(tr("Execution successful"));
    } else {
        box.setCustomIcon(QIcon::fromTheme("dialog-error"));
        box.setText(tr("Execution fail"));
    }

    // 移动到屏幕中间
    // 因为 MessageBox 此时还没有显示出来，所以获取不到正确的大小，而 MessageBox 的大小是固定的，所以此处使用固定值来计算
    auto width = QApplication::desktop()->width();
    auto height = QApplication::desktop()->height();
    int x = width / 2 - 424 / 2;
    int y = height / 2 - 156 / 2;
    box.move(x, y);

    box.exec();

    // 对话框关闭后，结束掉进程
    QTimer::singleShot(0, qApp, &QApplication::quit);
}

void MainWindow::on_UpdateSourceFinish(int exit_code) {
    BubblePopup(!exit_code);
}

}
