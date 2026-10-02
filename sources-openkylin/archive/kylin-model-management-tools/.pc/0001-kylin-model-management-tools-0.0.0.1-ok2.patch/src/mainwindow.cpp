#include "mainwindow.h"

#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(Downloader *downloader, QWidget *parent)
    : QWidget(parent), downloader_(downloader) {
    initUi();
    connectSignals();
}

MainWindow::~MainWindow() {}

void MainWindow::initUi() {
    initLayout();
    initLabel();
    initLineEdit();
    initComboBox();
    initButton();
    initTerminalTextEdit();
    loadLayout();
}

void MainWindow::initLayout() {
    mainLayout_.reset(new QVBoxLayout());
    modelTypeLayout_.reset(new QHBoxLayout());
    modelNameLayout_.reset(new QHBoxLayout());
    modelRevisionLayout_.reset(new QHBoxLayout());
    cacheDirLayout_.reset(new QHBoxLayout());
    fileLayout_.reset(new QHBoxLayout());
    // infoLayout_.reset(new QHBoxLayout());
    modelFileToDownloadLayout_.reset(new QVBoxLayout());
    buttonLayout_.reset(new QHBoxLayout());
    terminalLayout_.reset(new QVBoxLayout());
}

void MainWindow::initLabel() {
    modelTypeLabel_.reset(new QLabel("Model Type:"));
    modelNameLabel_.reset(new QLabel("Model Name:"));
    modelRevisionLabel_.reset(new QLabel("Model Revision:"));
    cacheDirLabel_.reset(new QLabel("Cache Directory:"));
    modelFileToDownloadLabel_.reset(new QLabel("Model File To Download:"));
    modelFileToDownloadInfoLabel_.reset(new QLabel("Prefer the q5_k_m model."));
    modelFileToDownloadInfoLabel_->setStyleSheet(
        "font-size: 12px; color: gray;");
}

void MainWindow::initLineEdit() {
    modelRevisionLineEdit_.reset(new QLineEdit());
    modelRevisionLineEdit_->setPlaceholderText(
        "leave this blank. 可以不填，没用");

    cacheDirLineEdit_.reset(new QLineEdit());
    cacheDirLineEdit_->setPlaceholderText("模型下载后的保存路径，可以不填");
}

void MainWindow::initComboBox() {
    modelTypeComboBox_.reset(new QComboBox());
    for (auto &type : downloader_->getModelTypes()) {
        modelTypeComboBox_->addItem(type);
    }

    modelNameComboBox_.reset(new QComboBox());

    modelFileToDownloadComboBox_.reset(new QComboBox());

    // Type 和 Name 绑定
    connect(modelTypeComboBox_.get(),
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::modelTypeComboBoxChanged);

    // 将两个 combobox 绑定起来。例如当模型名称显示为 7b 时，
    // 模型文件仅显示 7b 模型的文件。
    connect(modelNameComboBox_.get(),
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::modelNameComboBoxChanged);

    modelTypeComboBox_->setCurrentIndex(1);
}

void MainWindow::initButton() {
    downloadButton_.reset(new QPushButton("Download Model"));
    quitButton_.reset(new QPushButton("Quit"));
}

void MainWindow::initTerminalTextEdit() {
    terminalTextEdit_.reset(new QPlainTextEdit());
    terminalTextEdit_->setReadOnly(true);
}

void MainWindow::loadLayout() {
    modelTypeLayout_->addWidget(modelTypeLabel_.get());
    modelTypeLayout_->addWidget(modelTypeComboBox_.get());

    modelNameLayout_->addWidget(modelNameLabel_.get());
    modelNameLayout_->addWidget(modelNameComboBox_.get());

    modelRevisionLayout_->addWidget(modelRevisionLabel_.get());
    modelRevisionLayout_->addWidget(modelRevisionLineEdit_.get());

    cacheDirLayout_->addWidget(cacheDirLabel_.get());
    cacheDirLayout_->addWidget(cacheDirLineEdit_.get());

    fileLayout_->addWidget(modelFileToDownloadLabel_.get());
    fileLayout_->addWidget(modelFileToDownloadComboBox_.get());
    // infoLayout_->addStretch();
    // infoLayout_->addWidget(modelFileToDownloadInfoLabel_.get());
    modelFileToDownloadLayout_->addLayout(fileLayout_.get());
    // modelFileToDownloadLayout_->addLayout(infoLayout_.get());

    buttonLayout_->addWidget(downloadButton_.get());
    buttonLayout_->addWidget(quitButton_.get());

    terminalLayout_->addWidget(terminalTextEdit_.get());

    mainLayout_->addLayout(modelTypeLayout_.get());
    mainLayout_->addLayout(modelNameLayout_.get());
    // 版本和缓存路径暂时不显示
    // mainLayout_->addLayout(modelRevisionLayout_.get());
    // mainLayout_->addLayout(cacheDirLayout_.get());
    mainLayout_->addLayout(modelFileToDownloadLayout_.get());
    mainLayout_->addLayout(buttonLayout_.get());
    mainLayout_->addLayout(terminalLayout_.get());

    setLayout(mainLayout_.get());

    resize(600, 600);
}

void MainWindow::connectSignals() {
    connect(downloadButton_.get(), &QPushButton::clicked, this,
            &MainWindow::onDownloadButtonClicked);
    connect(quitButton_.get(), &QPushButton::clicked, this, &MainWindow::close);

    connect(downloader_, &Downloader::downloadOutput, this,
            &MainWindow::onDownloadOutput);
    connect(downloader_, &Downloader::downloadSucceed, this,
            &MainWindow::onDownloadSucceed);
    connect(downloader_, &Downloader::downloadFailed, this,
            &MainWindow::onDownloadFailed);
}

void MainWindow::setDownloadParams() {
    downloader_->setDownloadParams(
        modelTypeComboBox_->currentText(), modelNameComboBox_->currentText(),
        modelFileToDownloadComboBox_->currentText(),
        modelRevisionLineEdit_->text(), cacheDirLineEdit_->text());
}

void MainWindow::startDownloading() {
    terminalTextEdit_->clear();

    downloader_->download();
}

void MainWindow::onDownloadButtonClicked() {
    if (downloader_->isDownloading()) {
        QMessageBox::warning(this, "Warning", "Download is already running.");
        return;
    }

    setDownloadParams();

    startDownloading();
}

void MainWindow::onDownloadOutput(QByteArray output) {
    terminalTextEdit_->appendPlainText(output);
}

void MainWindow::onDownloadSucceed() {
    QMessageBox::information(this, "Success", "Download succeed.");
    terminalTextEdit_->appendPlainText("模型下载完成！");
}

void MainWindow::onDownloadFailed() {
    QMessageBox::critical(this, "Error", "Download failed.");
    terminalTextEdit_->appendPlainText("模型下载失败！");
}

void MainWindow::modelTypeComboBoxChanged(int index) {
    QString selectedModelType = modelTypeComboBox_->itemText(index);
    qDebug() << "MMMMMM selectedModelType: " << selectedModelType;

    modelNameComboBox_->clear();

    auto modelNames = downloader_->getModelNames(selectedModelType);
    for (auto modelName : modelNames) {
        modelNameComboBox_->addItem(modelName);
    }
}

void MainWindow::modelNameComboBoxChanged(int index) {
    QString selectedModelName = modelNameComboBox_->itemText(index);
    qDebug() << "MMMMMM selectedModelName: " << selectedModelName;

    modelFileToDownloadComboBox_->clear();

    auto modelFiles = downloader_->getModelFiles(selectedModelName);
    for (auto modelFile : modelFiles) {
        modelFileToDownloadComboBox_->addItem(modelFile);
    }

    modelFileToDownloadComboBox_->setCurrentIndex(
        modelFileToDownloadComboBox_->findText(
            "q5_k_m", static_cast<Qt::MatchFlags>(Qt::MatchContains)));
}
