#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QRect>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      network_card_label_(new QLabel(this)),
      protocol_label_(new QLabel(this)),
      port_label_(new QLabel(this)),
      port_filter_label_(new QLabel(this)),
      host_label_(new QLabel(this)),
      host_filter_label_(new QLabel(this)),
      network_card_line_edit_(new QLineEdit(this)),
      protocol_unspecified_radio_btn_(new QRadioButton(this)),
      protocol_tcp_radio_btn_(new QRadioButton(this)),
      protocol_udp_radio_btn_(new QRadioButton(this)),
      port_spin_box_(new QSpinBox(this)),
      port_filter_unspecified_radio_btn_(new QRadioButton(this)),
      port_filter_dst_radio_btn_(new QRadioButton(this)),
      port_filter_src_radio_btn_(new QRadioButton(this)),
      host_line_edit_(new QLineEdit(this)),
      host_filter_unspecified_radio_btn_(new QRadioButton(this)),
      host_filter_dst_radio_btn_(new QRadioButton(this)),
      host_filter_src_radio_btn_(new QRadioButton(this)),
      content_(new QTextEdit(this)),
      packet_capture_btn_(new QPushButton(this)),
      is_running_(false)
{
    setWindowIcon(QIcon::fromTheme("kylin-tcpdump"));
    setWindowTitle(tr("Kylin Packet Capture Tool"));
    setMinimumSize(452, 636);

    network_card_label_->setText(tr("Network card"));
    protocol_label_->setText(tr("Protocol"));
    port_label_->setText(tr("Port"));
    port_filter_label_->setText(tr("Port filter"));
    host_label_->setText(tr("Host"));
    host_filter_label_->setText(tr("Host filter"));
    auto *label_vbox_layout = new QVBoxLayout;
    label_vbox_layout->setContentsMargins(0, 0, 0, 0);
    label_vbox_layout->setSpacing(0);
    label_vbox_layout->addWidget(network_card_label_);
    label_vbox_layout->addWidget(protocol_label_);
    label_vbox_layout->addWidget(port_label_);
    label_vbox_layout->addWidget(port_filter_label_);
    label_vbox_layout->addWidget(host_label_);
    label_vbox_layout->addWidget(host_filter_label_);

    network_card_line_edit_->setPlaceholderText(tr("Please enter the name of the network card"));
    port_spin_box_->setMaximum(65535);
    host_line_edit_->setPlaceholderText(tr("Please enter the host address"));

    auto *protocol_btn_group = new QButtonGroup(this);
    protocol_btn_group->addButton(protocol_unspecified_radio_btn_);
    protocol_btn_group->addButton(protocol_tcp_radio_btn_);
    protocol_btn_group->addButton(protocol_udp_radio_btn_);
    protocol_unspecified_radio_btn_->setText(tr("Unspecified"));
    protocol_unspecified_radio_btn_->setChecked(true);
    protocol_tcp_radio_btn_->setText("Tcp");
    protocol_udp_radio_btn_->setText("Udp");
    auto *protocol_radio_btn_hbox_layout = new QHBoxLayout;
    protocol_radio_btn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    protocol_radio_btn_hbox_layout->setSpacing(0);
    protocol_radio_btn_hbox_layout->addWidget(protocol_unspecified_radio_btn_);
    protocol_radio_btn_hbox_layout->addWidget(protocol_tcp_radio_btn_);
    protocol_radio_btn_hbox_layout->addWidget(protocol_udp_radio_btn_);

    auto *port_filter_btn_group = new QButtonGroup(this);
    port_filter_btn_group->addButton(port_filter_unspecified_radio_btn_);
    port_filter_btn_group->addButton(port_filter_dst_radio_btn_);
    port_filter_btn_group->addButton(port_filter_src_radio_btn_);
    port_filter_unspecified_radio_btn_->setText(tr("Unspecified"));
    port_filter_unspecified_radio_btn_->setChecked(true);
    port_filter_dst_radio_btn_->setText(tr("Destination port"));
    port_filter_src_radio_btn_->setText(tr("Source port"));
    auto *port_filter_radio_btn_hbox_layout = new QHBoxLayout;
    port_filter_radio_btn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    port_filter_radio_btn_hbox_layout->setSpacing(0);
    port_filter_radio_btn_hbox_layout->addWidget(port_filter_unspecified_radio_btn_);
    port_filter_radio_btn_hbox_layout->addWidget(port_filter_dst_radio_btn_);
    port_filter_radio_btn_hbox_layout->addWidget(port_filter_src_radio_btn_);

    auto *host_filter_btn_group = new QButtonGroup(this);
    host_filter_btn_group->addButton(host_filter_unspecified_radio_btn_);
    host_filter_btn_group->addButton(host_filter_dst_radio_btn_);
    host_filter_btn_group->addButton(host_filter_src_radio_btn_);
    host_filter_unspecified_radio_btn_->setText(tr("Unspecified"));
    host_filter_unspecified_radio_btn_->setChecked(true);
    host_filter_dst_radio_btn_->setText(tr("Destination host"));
    host_filter_src_radio_btn_->setText(tr("Source host"));
    auto *host_filter_radio_btn_hbox_layout = new QHBoxLayout;
    host_filter_radio_btn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    host_filter_radio_btn_hbox_layout->setSpacing(0);
    host_filter_radio_btn_hbox_layout->addWidget(host_filter_unspecified_radio_btn_);
    host_filter_radio_btn_hbox_layout->addWidget(host_filter_dst_radio_btn_);
    host_filter_radio_btn_hbox_layout->addWidget(host_filter_src_radio_btn_);

    auto *edit_vbox_layout = new QVBoxLayout;
    edit_vbox_layout->setContentsMargins(0, 0, 0, 0);
    edit_vbox_layout->setSpacing(0);
    edit_vbox_layout->addWidget(network_card_line_edit_);
    edit_vbox_layout->addSpacing(8);
    edit_vbox_layout->addLayout(protocol_radio_btn_hbox_layout);
    edit_vbox_layout->addSpacing(8);
    edit_vbox_layout->addWidget(port_spin_box_);
    edit_vbox_layout->addSpacing(8);
    edit_vbox_layout->addLayout(port_filter_radio_btn_hbox_layout);
    edit_vbox_layout->addSpacing(8);
    edit_vbox_layout->addWidget(host_line_edit_);
    edit_vbox_layout->addSpacing(8);
    edit_vbox_layout->addLayout(host_filter_radio_btn_hbox_layout);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setContentsMargins(0, 0, 0, 0);
    hbox_layout->setSpacing(0);
    hbox_layout->addLayout(label_vbox_layout, 84);
    hbox_layout->addSpacing(32);
    hbox_layout->addLayout(edit_vbox_layout, 288);

    packet_capture_btn_->setText(tr("Start packet capture"));
    connect(packet_capture_btn_, &QPushButton::clicked, this, &MainWindow::on_ClickPacketCaptureBtn);
    auto *packet_capture_btn_hbox_layout = new QHBoxLayout;
    packet_capture_btn_hbox_layout->setContentsMargins(0, 0, 0, 0);
    packet_capture_btn_hbox_layout->setSpacing(0);
    packet_capture_btn_hbox_layout->addStretch();
    packet_capture_btn_hbox_layout->addWidget(packet_capture_btn_);

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(24, 16, 24, 24);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(hbox_layout);
    vbox_layout->addSpacing(16);
    vbox_layout->addWidget(content_);
    vbox_layout->addSpacing(32);
    vbox_layout->addLayout(packet_capture_btn_hbox_layout);
    setLayout(vbox_layout);

#ifdef V11_ENVIRONMENT
    connect(&process_, &kom::KomProcess::readyReadStandardOutput, this, &MainWindow::on_ReadTcpdumpStandardOutput);
    connect(&process_, &kom::KomProcess::readyReadStandardError, this, &MainWindow::on_ReadTcpdumpStandardError);
#else
    connect(&process_, &QProcess::readyReadStandardOutput, this, &MainWindow::on_ReadTcpdumpStandardOutput);
    connect(&process_, &QProcess::readyReadStandardError, this, &MainWindow::on_ReadTcpdumpStandardError);
#endif

    // 移动到屏幕中间
    auto primary_screen = QGuiApplication::primaryScreen()->geometry();
    auto x = primary_screen.width() / 2 - this->width() / 2;
    auto y = primary_screen.height() / 2 - this->height() / 2;
    move(x, y);
}

MainWindow::~MainWindow() {
    StopPacketCapture();
}

void MainWindow::on_ClickPacketCaptureBtn() {
    is_running_ = !is_running_;

    if (is_running_) {
        StartPacketCapture();
        packet_capture_btn_->setText(tr("Stop packet capture"));
    } else {
        StopPacketCapture();
        packet_capture_btn_->setText(tr("Start packet capture"));
    }
}

void MainWindow::StartPacketCapture() {
    QStringList args;
    args.push_back("tcpdump");

    // 网卡
    args.push_back("-i");
    auto network_card = network_card_line_edit_->text();
    if (!network_card.isEmpty()) {
        args.push_back(network_card);
    } else {
        args.push_back("any");
    }

    // 协议（可以不指定）
    if (protocol_tcp_radio_btn_->isChecked()) {
        args.push_back("tcp");
    } else if (protocol_udp_radio_btn_->isChecked()) {
        args.push_back("udp");
    }

    // 端口（可以不指定）
    if (port_spin_box_->value() != 0) {
        if (port_filter_dst_radio_btn_->isChecked()) {
            args.push_back("dst");
        } else if (port_filter_src_radio_btn_->isChecked()) {
            args.push_back("src");
        }
        args.push_back("port");
        args.push_back(port_spin_box_->text());
    }

    // 主机（可以不指定）
    if (!host_line_edit_->text().isEmpty()) {
        if (port_spin_box_->value() != 0) {
            args.push_back("and");
        }

        if (host_filter_dst_radio_btn_->isChecked()) {
            args.push_back("dst");
        } else if (host_filter_src_radio_btn_->isChecked()) {
            args.push_back("src");
        }
        args.push_back("host");
        args.push_back(host_line_edit_->text());
    }

    args.push_back("-nn");
    args.push_back("-X");
    args.push_back("-S");
    args.push_back("-l");
    args.push_back("--immediate-mode");

    qInfo() << "[MainWindow][StartPacketCapture] run command: " << args;

    process_.start("pkexec", args);
}

void MainWindow::StopPacketCapture() {
#ifdef V11_ENVIRONMENT
   process_.close(); 
#else
    auto pid = process_.processId();
    if (pid == 0) {
        return;
    }

    QProcess p;
    p.setProgram("pkexec");
    QStringList args;
    args.push_back("kill");
    args.push_back("-9");
    args.push_back(QString::number(pid));
    p.setArguments(args);
    p.start();
    p.waitForFinished();
#endif
}

void MainWindow::on_ReadTcpdumpStandardOutput() {
    content_->insertPlainText(process_.readAllStandardOutput());
}

void MainWindow::on_ReadTcpdumpStandardError() {
    content_->insertPlainText(process_.readAllStandardError());
}
