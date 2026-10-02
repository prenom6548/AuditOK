#pragma once

#include "downloader.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(Downloader *downloader, QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initUi();
    void initLayout();
    void initLabel();
    void initLineEdit();
    void initComboBox();
    void initButton();
    void initTerminalTextEdit();
    void loadLayout();
    void connectSignals();

    void setDownloadParams();
    void startDownloading();

private slots:
    void onDownloadButtonClicked();
    void onDownloadOutput(QByteArray output);
    void onDownloadSucceed();
    void onDownloadFailed();
    void modelTypeComboBoxChanged(int index);
    void modelNameComboBoxChanged(int index);
    void onCheckButtonClicked();

private:
    std::unique_ptr<QVBoxLayout> mainLayout_;
    std::unique_ptr<QHBoxLayout> modelTypeLayout_;
    std::unique_ptr<QHBoxLayout> modelNameLayout_;
    std::unique_ptr<QHBoxLayout> modelRevisionLayout_;
    // TODO cache dir 没什么意义
    std::unique_ptr<QHBoxLayout> cacheDirLayout_;
    std::unique_ptr<QHBoxLayout> modelFileToDownloadLayout_;
    std::unique_ptr<QHBoxLayout> buttonLayout_;
    std::unique_ptr<QVBoxLayout> terminalLayout_;

    std::unique_ptr<QLabel> modelTypeLabel_;
    std::unique_ptr<QLabel> modelNameLabel_;
    std::unique_ptr<QLabel> modelRevisionLabel_;
    std::unique_ptr<QLabel> cacheDirLabel_;
    std::unique_ptr<QLabel> modelFileToDownloadLabel_;
    std::unique_ptr<QLabel> modelFileToDownloadInfoLabel_;

    std::unique_ptr<QLineEdit> modelRevisionLineEdit_;
    std::unique_ptr<QLineEdit> cacheDirLineEdit_;

    std::unique_ptr<QComboBox> modelTypeComboBox_;
    std::unique_ptr<QComboBox> modelNameComboBox_;
    std::unique_ptr<QComboBox> modelFileToDownloadComboBox_;

    std::unique_ptr<QPushButton> tutorialButton_;
    std::unique_ptr<QPushButton> downloadButton_;
    std::unique_ptr<QPushButton> quitButton_;
    std::unique_ptr<QPushButton> checkButton_;

    std::unique_ptr<QPlainTextEdit> terminalTextEdit_;

    Downloader *downloader_ = nullptr;
};
