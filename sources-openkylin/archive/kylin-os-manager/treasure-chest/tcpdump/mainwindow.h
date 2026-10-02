#ifndef KYLIN_OS_MANAGER_TREASURE_CHEST_TCPDUMP_MAINWINDOW_H
#define KYLIN_OS_MANAGER_TREASURE_CHEST_TCPDUMP_MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>

#ifdef V11_ENVIRONMENT
    #include "kom_process.h"
#else
    #include <QProcess>
#endif

class MainWindow: public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    void on_ClickPacketCaptureBtn();
    void on_ReadTcpdumpStandardOutput();
    void on_ReadTcpdumpStandardError();

private:
    void StartPacketCapture();
    void StopPacketCapture();

#ifdef V11_ENVIRONMENT
    kom::KomProcess process_;
#else
    QProcess process_;
#endif
    QLabel *network_card_label_;
    QLabel *protocol_label_;
    QLabel *port_label_;
    QLabel *port_filter_label_;
    QLabel *host_label_;
    QLabel *host_filter_label_;
    QLineEdit *network_card_line_edit_;
    QRadioButton *protocol_unspecified_radio_btn_;
    QRadioButton *protocol_tcp_radio_btn_;
    QRadioButton *protocol_udp_radio_btn_;
    QSpinBox *port_spin_box_;
    QRadioButton *port_filter_unspecified_radio_btn_;
    QRadioButton *port_filter_dst_radio_btn_;
    QRadioButton *port_filter_src_radio_btn_;
    QLineEdit *host_line_edit_;
    QRadioButton *host_filter_unspecified_radio_btn_;
    QRadioButton *host_filter_dst_radio_btn_;
    QRadioButton *host_filter_src_radio_btn_;
    QTextEdit *content_;
    QPushButton *packet_capture_btn_;
    bool is_running_;
};

#endif
