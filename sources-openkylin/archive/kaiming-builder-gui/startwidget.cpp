#include "startwidget.h"

#include <component/clicklabel.h>
#include <debianconvert/debianconvertdialog.h>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QDialog>
#include <QGridLayout>
#include <QPixmap>
#include <QFont>
#include <QPainter>

#include "messageboxutils.h"

StartWidget::StartWidget(QWidget *parent)
    : QWidget(parent)
{
    m_labelProject = new QLabel(tr("Projects"));
    // 创建 QFont 对象并设置字体属性
    QFont font = m_labelProject->font();
    font.setPointSize(16);
    font.setBold(true);     // 设置加粗

    // 将字体应用到 QLabel
    m_labelProject->setFont(font);

    // 创建按钮
    m_newButton = new QPushButton(tr("New"));
    m_openButton = new QPushButton(tr("Open"));
    m_debianConvertButton = new QPushButton(tr("Debian Source Convert"));
    m_newButton->setStyleSheet("color: white; background-color: gray;");
    m_openButton->setStyleSheet("color: white; background-color: gray;");
    m_debianConvertButton->setStyleSheet("color: white; background-color: gray;");

    // 连接按钮点击事件
    connect(m_newButton, &QPushButton::clicked, this, &StartWidget::onNewClicked);
    connect(m_openButton, &QPushButton::clicked, this, &StartWidget::onOpenClicked);
    connect(m_debianConvertButton, &QPushButton::clicked, this, &StartWidget::onDebianConvertClicked);

    // 创建垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    //上
    mainLayout->addStretch();
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    QLabel *logo = new QLabel(this);
    logo->setFixedSize(36, 36);
    QIcon titleIcon = QIcon::fromTheme("kaiming-builder-gui", QIcon(":/images/kaiming-builder-gui.png"));
    logo->setPixmap(titleIcon.pixmap(titleIcon.actualSize({36,36})));

    m_titleBig = new QLabel(tr("kaiming-builder-gui"));
//    QPalette paletteTextColor;
//    paletteTextColor.setColor(QPalette::WindowText, Qt::white);
//    m_titleBig->setPalette(paletteTextColor);
    m_titleBig->setStyleSheet("color: white;");
    QFont fontBig;
    fontBig.setBold(true);
    fontBig.setPixelSize(36);
    m_titleBig->setFont(fontBig);

    hLayout->addStretch();
    hLayout->addWidget(logo);
    hLayout->addWidget(m_titleBig);
    hLayout->addStretch();

    mainLayout->addLayout(hLayout);

    m_description = new QLabel(tr("kaiming-builder-gui is a graphical version of kaiming-builder, providing developers with a more convenient way to write build manifest files and build source code into Kaiming packages"));
//    m_description->setPalette(paletteTextColor);
    m_description->setStyleSheet("color: white;");
    m_description->setFixedWidth(500);
    m_description->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_description->setWordWrap(true);
    m_description->setAlignment(Qt::AlignLeft);
    m_description->adjustSize();
    QHBoxLayout *descriptionLayout = new QHBoxLayout(this);
    descriptionLayout->addStretch();
    descriptionLayout->addWidget(m_description);
    descriptionLayout->addStretch();
    mainLayout->addLayout(descriptionLayout);

    m_fileListLayout = new QVBoxLayout;
    m_fileListLayout->setAlignment(Qt::AlignLeft);
    // 添加文件列表布局
    QHBoxLayout *fileListContainerLayout = new QHBoxLayout;
    fileListContainerLayout->addStretch(); // 左侧拉伸
    fileListContainerLayout->addLayout(m_fileListLayout);
    fileListContainerLayout->addStretch(); // 右侧拉伸
//    mainLayout->addLayout(fileListContainerLayout);

    //下
    // 添加按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_newButton);
    buttonLayout->addWidget(m_openButton);
    buttonLayout->addWidget(m_debianConvertButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    // 设置主布局
    setLayout(mainLayout);

    // 读取历史记录并展示
//    loadAndDisplayHistory();
}

/**
 * @brief : 校验appid是否合法
 * @param : [in] appName, appid
 * @param : [out] errMsg, 错误信息
 */
bool StartWidget::isValidAppName(const QString &appName, QString &errMsg)
{
    if (appName.length() > 255)
    {
        errMsg = tr("Id can't be longer than 255 characters");
        return false;
    }

//    static const QRegularExpression regexStr(R"abc((\w+\.)+(\w+\.)+([\w-]+)$)abc");
    static const QRegularExpression regexStr(R"abc(([\w-]+\.)+([\w-]+\.)+([\w-]+)$)abc");
    if (!regexStr.isValid())
    {
        errMsg = "Invalid regex:" + regexStr.errorString();
        return false;
    }

    QRegularExpressionMatch matchResult = regexStr.match(appName);
    if (matchResult.hasMatch())
    {
//        QRegularExpression regexDigit(R"(^\d.*)");
//        if (!regexDigit.isValid())
//        {
//            errMsg = "Invalid digit regex:" + regexDigit.errorString();
//            return false;
//        }

        // 遍历所有捕获组（原始代码中的 element 对应捕获组内容）
        // Qt 的捕获组索引从 1 开始，第0组是整个匹配
//        for (int i = 1; i <= matchResult.lastCapturedIndex(); ++i)
//        {
//            QString str = matchResult.captured(i);
//            // 检查是否以数字开头
//            if (regexDigit.match(str).hasMatch())
//            {
//                errMsg = tr("Id is wrong! Elements may not begin with a digit.");
//                return false;
//            }
//        }
    }
    else
    {
        errMsg = tr("Id is wrong! Notes:\n"
                    "    a）Ids are composed of 3 or more elements separated by a period('.') character.\n"
                    "    b) Each element must only contain the ASCII characters '[A-Z][a-z][0-9]_-'.\n"
//                    "    c) All elements must contain at least one character.\n"
//                    "       Elements may not begin with a digit.\n"
//                    "       Additionally '-' is only allowed in the last element."
                    );
        return false;
    }

    return true;
}


void StartWidget::onNewClicked() {
    // 创建自定义对话框
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Please select project path"));
    dialog->setFixedSize(600,200);

    // 创建布局
    QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);

    QFormLayout *formLayout = new QFormLayout;

    // Project ID 输入框
    QLabel *idLabel = new QLabel(tr("Project ID"), dialog);
    QLineEdit *idEdit = new QLineEdit(dialog);
    idEdit->setPlaceholderText(tr("Such as : top.openkylin.demo"));
    formLayout->addRow(idLabel, idEdit);

    // Path 输入框和浏览按钮
    QLabel *pathLabel = new QLabel(tr("Create Path"), dialog);
    QLineEdit *pathEdit = new QLineEdit(dialog);
    QPushButton *browseButton = new QPushButton(tr("Browse"), dialog);
    QHBoxLayout *pathLayout = new QHBoxLayout;
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);
    formLayout->addRow(pathLabel, pathLayout);

    dialogLayout->addLayout(formLayout);

    // 添加确认和取消按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *confirmButton = new QPushButton(tr("Confirm"), dialog);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), dialog);
    buttonLayout->addStretch();
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    dialogLayout->addLayout(buttonLayout);

    // 连接浏览按钮点击事件
    connect(browseButton, &QPushButton::clicked, [=]() {
        // 创建文件夹选择对话框
        QFileDialog *folderDialog = new QFileDialog(this);
        folderDialog->setWindowTitle(tr("Please select project path"));
        folderDialog->setFileMode(QFileDialog::DirectoryOnly);
        folderDialog->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

        // 设置侧边栏路径
        QList<QUrl> sidebarUrls;
        sidebarUrls << QUrl::fromLocalFile(QDir::homePath());
        folderDialog->setSidebarUrls(sidebarUrls);

        // 显示对话框并处理结果
        if (folderDialog->exec() == QDialog::Accepted) {
            QStringList selectedFiles = folderDialog->selectedFiles();
            if (!selectedFiles.isEmpty()) {
                pathEdit->setText(selectedFiles.at(0));
            }
        }

        folderDialog->deleteLater(); // 释放对话框内存
    });

    connect(confirmButton, &QPushButton::clicked, [dialog, pathEdit, idEdit, this]() {
        QString selectedPath = pathEdit->text();
        QString projectID = idEdit->text().trimmed();

        // 检查输入是否为空
        if (selectedPath.isEmpty() || projectID.isEmpty()) {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(dialog, QObject::tr("Warning"), tr("Project ID and Path cannot be empty."), QObject::tr("OK"));
            return;
        }

        QString errMsg;
        if (!isValidAppName(projectID, errMsg)) {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(dialog, QObject::tr("Warning"), errMsg, QObject::tr("OK"));
            return;
        }

        // 拼接完整目录路径
        m_selectedPath = selectedPath + "/" + projectID;

        // 检查目录是否已经存在
        QDir dir(m_selectedPath);
        if (dir.exists()) {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(dialog, QObject::tr("Warning"), tr("The directory already exists. Please clean it up manually."), QObject::tr("OK"));
            return; // 如果目录已存在，直接退出
        }

        // 尝试创建目录
        if (QDir().mkpath(m_selectedPath)) {
            emit folderSelected(m_selectedPath, projectID);
        } else {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(dialog, QObject::tr("Warning"), tr("Failed to create the directory. Please check the path."), QObject::tr("OK"));
        }

        dialog->accept(); // 关闭对话框
    });

    // 连接取消按钮点击事件
    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

    // 显示对话框
    dialog->exec();

    // 释放对话框内存
    dialog->deleteLater();
}

void StartWidget::onOpenClicked() {
    // 创建文件选择对话框
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle(tr("Please select YAML file"));
    fileDialog->setFileMode(QFileDialog::ExistingFile); // 仅选择现有文件
    fileDialog->setNameFilter(tr("YAML Files (*.km.yaml *.km.yml)")); // 过滤 YAML 文件

    // 设置侧边栏路径
    QList<QUrl> sidebarUrls;
    sidebarUrls << QUrl::fromLocalFile(QDir::homePath()); // 添加主目录到侧边栏
    fileDialog->setSidebarUrls(sidebarUrls);

    // 显示对话框并处理结果
    if (fileDialog->exec() == QDialog::Accepted) {
        QStringList selectedFiles = fileDialog->selectedFiles();
        if (!selectedFiles.isEmpty()) {
            m_selectedPath = selectedFiles.at(0);
            emit fileSelected(m_selectedPath); // 发射信号
        }
    }

    fileDialog->deleteLater(); // 释放对话框内存
}

void StartWidget::onDebianConvertClicked()
{
    DebianConvertDialog dialog(this);
    // 对话框信号转发到主窗口
    connect(&dialog, &DebianConvertDialog::fileSelected,
            this, &StartWidget::handleFileSelected);

    dialog.exec();
}

void StartWidget::handleFileSelected(const QString &filePath)
{
    qDebug() << "Received file path:" << filePath;
    emit fileSelected(filePath); // 发射信号
}

void StartWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.drawPixmap(rect(),
                QPixmap(":/images/bg.png").scaled(
                size(),
                Qt::IgnoreAspectRatio,
                Qt::SmoothTransformation));
}

void StartWidget::clearLayout(QLayout *layout) {
    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item->layout()) {
            clearLayout(item->layout()); // 递归清空子布局
        } else if (item->widget()) {
            delete item->widget(); // 删除控件
        }
        delete item; // 删除布局项
    }
}

void StartWidget::loadProject(const QString& yamlPath) {

    m_selectedPath = yamlPath;
    emit fileSelected(m_selectedPath); // 发射信号
    qDebug() << "Loading project from:" << yamlPath;
}

void StartWidget::loadAndDisplayHistory() {
    clearLayout(m_fileListLayout);

    QVector<QString> history = m_historyManager.getHistory();

    // 遍历历史记录并添加到界面
    for (const QString& record : history) {
        // 将记录按逗号分隔为项目名称和路径
        QStringList parts = record.split(",");
        if (parts.size() == 2) {
            QString projectName = parts[0];
            QString yamlPath = parts[1];

            QVBoxLayout *itemLayout = new QVBoxLayout;
            // 创建项目名称标签（可点击）
            ClickLabel *nameLabel = new ClickLabel(projectName);
            QFont nameFont = nameLabel->font();
            nameFont.setPointSize(12);
            nameFont.setBold(true);
            nameLabel->setFont(nameFont);
            nameLabel->setStyleSheet(
                "ClickLabel { color: #0078d7;text-decoration: none;}"
                "ClickLabel:hover { color: #005bb5; background-color: #e0f3ff; border-radius: 5px; }"
            );

            connect(nameLabel, &ClickLabel::clicked, this, [this, yamlPath]() {
                QFile file(yamlPath);
                if(file.exists())
                {
                    loadProject(yamlPath);
                }else
                {
                    m_historyManager.moveRecordToEnd(yamlPath);
                    QMessageBox::warning(this, tr("Warning"),
                                               tr("The project file does not exist."));
                }
            });

            // 创建路径标签
            QLabel *pathLabel = new QLabel(yamlPath);
            QFont pathFont = pathLabel->font();
            pathFont.setPointSize(10);
            pathLabel->setFont(pathFont);
            pathLabel->setStyleSheet("color: #666666;");

            itemLayout->addWidget(nameLabel);
            itemLayout->addWidget(pathLabel);
            m_fileListLayout->addLayout(itemLayout);
        }
    }
}


