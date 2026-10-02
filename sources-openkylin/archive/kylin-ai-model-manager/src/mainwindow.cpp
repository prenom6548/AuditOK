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
    advancedOptionsLayout_.reset(new QHBoxLayout());
    advancedOptionsLayout_->addStretch();
    modelTypeLayout_.reset(new QHBoxLayout());
    modelNameLayout_.reset(new QHBoxLayout());
    modelRevisionLayout_.reset(new QHBoxLayout());
    cacheDirLayout_.reset(new QHBoxLayout());
    modelFileToDownloadLayout_.reset(new QHBoxLayout());
    buttonLayout_.reset(new QHBoxLayout());
    terminalLayout_.reset(new QVBoxLayout());
}

void MainWindow::initLabel() {
    modelTypeLabel_.reset(new QLabel("模型类型"));     // Model Type:
    modelNameLabel_.reset(new QLabel("模型名称"));     // Model Name:
    modelRevisionLabel_.reset(new QLabel("模型版本")); // Model Revision:
    cacheDirLabel_.reset(new QLabel("缓存目录"));      // Cache Directory:
    modelFileToDownloadLabel_.reset(
        new QLabel("要下载的单个模型文件")); // Model File To Download:
    modelFileToDownloadInfoLabel_.reset(
        new QLabel("优先选用 q5_k_m 模型")); // Prefer the q5_k_m model.
    modelFileToDownloadInfoLabel_->setStyleSheet(
        "font-size: 12px; color: gray;");
}

void MainWindow::initLineEdit() {
    modelRevisionLineEdit_.reset(new QLineEdit());
    modelRevisionLineEdit_->setPlaceholderText("可以不填，没用");

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
    onClickDownloadButton_.reset(new QPushButton("一键下载"));
    downloadButton_.reset(new QPushButton("下载模型")); // Download Model
    quitButton_.reset(new QPushButton("退出"));         // Quit
    checkButton_.reset(new QPushButton("检查本地模型文件"));

    advancedOptionsButton_.reset(new QToolButton());
    advancedOptionsButton_->setArrowType(Qt::DownArrow); // 设置为下箭头
    advancedOptionsButton_->setCheckable(true); // 设置可选中/取消选中
    advancedOptionsButton_->setChecked(false);  // 初始状态为收起
}

void MainWindow::initTerminalTextEdit() {
    terminalTextEdit_.reset(new QPlainTextEdit());
    terminalTextEdit_->setReadOnly(true);
}

void MainWindow::loadLayout() {
    advancedOptionsLayout_->addWidget(advancedOptionsButton_.get());
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

    mainLayout_->addWidget(onClickDownloadButton_.get());
    mainLayout_->addLayout(advancedOptionsLayout_.get());
    mainLayout_->addLayout(modelTypeLayout_.get());
    mainLayout_->addLayout(modelNameLayout_.get());
    // 版本和缓存路径暂时不显示
    // mainLayout_->addLayout(modelRevisionLayout_.get());
    // mainLayout_->addLayout(cacheDirLayout_.get());
    mainLayout_->addLayout(modelFileToDownloadLayout_.get());
    mainLayout_->addLayout(buttonLayout_.get());
    mainLayout_->addWidget(checkButton_.get());
    mainLayout_->addLayout(terminalLayout_.get());

    setLayout(mainLayout_.get());

    resize(600, 600);

    {
        modelTypeLabel_->setVisible(false);
        modelNameLabel_->setVisible(false);
        modelFileToDownloadLabel_->setVisible(false);

        modelTypeComboBox_->setVisible(false);
        modelNameComboBox_->setVisible(false);
        modelFileToDownloadComboBox_->setVisible(false);

        downloadButton_->setVisible(false);
        quitButton_->setVisible(false);
    }
}

void MainWindow::connectSignals() {
    connect(advancedOptionsButton_.get(), &QAbstractButton::clicked, this,
            &MainWindow::onAdvancedOptionsClicked);
    connect(onClickDownloadButton_.get(), &QPushButton::clicked, downloader_,
            &Downloader::downloadAllModels);
    connect(downloadButton_.get(), &QPushButton::clicked, this,
            &MainWindow::onDownloadButtonClicked);
    connect(quitButton_.get(), &QPushButton::clicked, this, &MainWindow::close);
    connect(checkButton_.get(), &QPushButton::clicked, this,
            &MainWindow::onCheckButtonClicked);

    connect(downloader_, &Downloader::downloadOutput, this,
            &MainWindow::onDownloadOutput);
    connect(downloader_, &Downloader::downloadStart, this,
            [this]() { terminalTextEdit_->clear(); });
    connect(downloader_, &Downloader::downloadSucceed, this,
            &MainWindow::onDownloadSucceed);
    connect(downloader_, &Downloader::downloadFailed, this,
            &MainWindow::onDownloadFailed);

    connect(downloader_, &Downloader::allModelsDownloaded, this, [this]() {
        QMessageBox::information(this, "模型下载工具", "所有模型下载完成");
    });

    connect(this, &MainWindow::destroyed, downloader_,
            &Downloader::quitProcess);
}

void MainWindow::onAdvancedOptionsClicked() {
    bool isChecked = advancedOptionsButton_->isChecked();

    modelTypeLabel_->setVisible(isChecked);
    modelNameLabel_->setVisible(isChecked);
    modelFileToDownloadLabel_->setVisible(isChecked);

    modelTypeComboBox_->setVisible(isChecked);
    modelNameComboBox_->setVisible(isChecked);
    modelFileToDownloadComboBox_->setVisible(isChecked);

    downloadButton_->setVisible(isChecked);
    quitButton_->setVisible(isChecked);

    // 改变箭头方向，表示展开/收起
    if (isChecked) {
        advancedOptionsButton_->setArrowType(Qt::UpArrow);
    } else {
        advancedOptionsButton_->setArrowType(Qt::DownArrow);
    }
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
        QMessageBox::warning(this, "模型下载工具", "已经有模型在下载了");
        return;
    }

    setDownloadParams();

    startDownloading();
}

void MainWindow::onDownloadOutput(QByteArray output) {
    if (!terminalTextEdit_) {
        return;
    }
    terminalTextEdit_->appendPlainText(output);
}

void MainWindow::onDownloadSucceed() {
    // QMessageBox::information(this, "模型下载工具", "模型下载完成！");
    if (!terminalTextEdit_) {
        return;
    }
    terminalTextEdit_->appendPlainText("模型下载完成！");
}

void MainWindow::onDownloadFailed() {
    QMessageBox::critical(this, "模型下载工具",
                          "模型下载失败！可以尝试重新下载。");
    if (!terminalTextEdit_) {
        return;
    }
    terminalTextEdit_->appendPlainText("模型下载失败！");
}

void MainWindow::modelTypeComboBoxChanged(int index) {
    QString selectedModelType = modelTypeComboBox_->itemText(index);

    modelNameComboBox_->clear();

    auto modelNames = downloader_->getModelNames(selectedModelType);
    for (auto modelName : modelNames) {
        modelNameComboBox_->addItem(modelName);
    }
}

void MainWindow::modelNameComboBoxChanged(int index) {
    QString selectedModelName = modelNameComboBox_->itemText(index);

    modelFileToDownloadComboBox_->clear();

    auto modelFiles = downloader_->getModelFiles(selectedModelName);
    for (auto modelFile : modelFiles) {
        modelFileToDownloadComboBox_->addItem(modelFile);
    }

    modelFileToDownloadComboBox_->setCurrentIndex(
        modelFileToDownloadComboBox_->findText(
            "q5_k_m", static_cast<Qt::MatchFlags>(Qt::MatchContains)));
}

void MainWindow::onCheckButtonClicked() {
    terminalTextEdit_->clear();
    downloader_->checkModelFiles();
}
