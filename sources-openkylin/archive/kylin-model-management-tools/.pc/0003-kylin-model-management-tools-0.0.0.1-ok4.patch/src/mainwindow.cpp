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
    modelFileToDownloadLayout_.reset(new QHBoxLayout());
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
    tutorialButton_.reset(new QPushButton("工具使用方法"));
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

    modelFileToDownloadLayout_->addWidget(modelFileToDownloadLabel_.get());
    modelFileToDownloadLayout_->addWidget(modelFileToDownloadComboBox_.get());

    buttonLayout_->addWidget(downloadButton_.get());
    buttonLayout_->addWidget(quitButton_.get());

    terminalLayout_->addWidget(terminalTextEdit_.get());

    mainLayout_->addWidget(tutorialButton_.get());
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
    connect(tutorialButton_.get(), &QPushButton::clicked, this, [this]() {
        QWidget *w = new QWidget();
        QLabel *label = new QLabel(w);
        label->setText(
            " - speech 模型(共4个)下载：\n1. Model Type 选择 speech；\n2. "
            "Model Name 共 4 个，都需要下载，依次选择进行下载；\n\n"
            " - nlp 模型(共1个)下载：\n1. Model Type 选择 nlp；\n2. "
            "其他不用修改，点击 Download Model 按钮下载；\n\n"
            "注意事项：\n1. 第一次下载时，会安装依赖，可能时间较长，"
            "如果遇到下载失败，可以尝试切换网络后重试；\n2. "
            "如果窗口日志中有“依赖下载完成”，则表示依赖已经下载完成。模型下载时"
            "使用可以访问 https://www.modelscope.cn/ 的网络进行下载；\n3. "
            "下载完模型后，工具会将模型链接到系统目录，此时会有弹窗要求输入密码"
            "；\n4. 下载完一个模型后，再下载其他模型。");
        label->setAlignment(Qt::AlignLeft);
        label->setStyleSheet("font-size: 16px;");
        label->setWordWrap(true);
        label->setMinimumSize(300, 500);
        // 使 label 的内容可复制
        w->setWindowTitle("工具使用方法");
        w->resize(300, 500);
        w->show();
    });
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
