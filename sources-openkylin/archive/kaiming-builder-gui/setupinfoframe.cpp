#include "setupinfoframe.h"
#include <QLineEdit>
#include <QScrollArea>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QLabel>
#include <QHeaderView>
#include <QTimer>

static QStringList parseCmdline(QString command)
{
    command = command.trimmed();

    QStringList cmds;
    bool inQuote = false;
    QString arg;
    for (int i = 0; i < command.length(); ++i) {
        QChar c = command.at(i);

        if (c == QChar('"')) {
            inQuote = !inQuote;
            continue;
        }

        if (c == QChar(' ')) {
            if (!inQuote) {
                if (!arg.isEmpty()) {
                    cmds.append(arg);
                    arg = "";
                }
                continue;
            }
        }

        arg += c;
    }
    if (!arg.isEmpty()) {
        cmds.append(arg);
    }

    return cmds;
}

#define INIT_HBOXLAYOUT \
    QHBoxLayout *hLayout = new QHBoxLayout; \
    hLayout->setSpacing(20);


#define INIT_LINE_EDIT(labelText, widget) \
    { \
        QLabel *label = new QLabel(labelText); \
        label->setFixedWidth(150); \
        widget = new QLineEdit(); \
        widget->setFixedHeight(35); \
        widget->setPlaceholderText(labelText); \
    }
#define INIT_TEXT_EDIT(labelText, widget) \
    { \
        QLabel *label = new QLabel(labelText); \
        label->setFixedWidth(150); \
        widget = new QTextEdit(); \
        widget->setFixedHeight(75); \
        widget->setPlaceholderText(labelText); \
    }

SetupInfoFrame::SetupInfoFrame(QWidget *parent)
    : QFrame(parent)
{
    setupUI();
}

SetupInfoFrame::~SetupInfoFrame()
{
}

void SetupInfoFrame::setupUI()
{
    // Layout for the entire frame
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    {
        QFormLayout *formLayout = new QFormLayout;
        INIT_LINE_EDIT("channel", m_channelLineEdit);
        m_channelLineEdit->setText("stable"); // 直接填入默认值 "stable"
        formLayout->addRow(tr("channel") + "<font color = red>*</font>", m_channelLineEdit);
        m_channelLineEdit->setToolTip(tr("Each developer can create their own channels.\n"
                                         "All the software packages installed will belong to this channel.\n"
                                         "Example:\n"
                                         "stable"));

        INIT_LINE_EDIT("command", m_commandLineEdit);
        formLayout->addRow(tr("command") + "<font color = red>*</font>", m_commandLineEdit);
        m_commandLineEdit->setPlaceholderText(tr("An executable file called by the software package. "
                                                 "with parameters added later. "
                                                 "If a parameter contains spaces, enclose it in quotation marks. "
                                                 "Eexample: "
                                                 "cmd arg1 arg2 \"arg3.1 arg3.2\" arg4"));

        m_commandLineEdit->setToolTip(tr("An executable file called by the software package\n "
                                         "while it is running, with parameters added later. \n"
                                         "When kind is set to app, the command must be configured.If a parameter contains spaces, enclose it in quotation marks.\n"
                                         "Eexample:\n"
                                         "cmd arg1 arg2 \"arg3.1 arg3.2\" arg4"));


        INIT_TEXT_EDIT(tr("Files to be deleted after the software package is builded. "
                          "Wildcard characters are supported. Separated by newlines. "
                          "Example: "
                          " /path/a \\n"
                          " *.so \\n"
                          " /path/b "), m_cleanupTextEdit);
        formLayout->addRow(tr("cleanup"), m_cleanupTextEdit);
        m_cleanupTextEdit->setToolTip(tr("Files to be deleted after the software package is builded.\n "
                                         "Wildcard characters are supported. Separated by newlines.\n"
                                         "Example:\n"
                                         " /path/a \n"
                                         " *.so \n"
                                         " /path/b "));

        mainLayout->addLayout(formLayout);
    }

    // Modules section
    {
        QLabel *modulesLabel = new QLabel(tr("modules"));
        modulesLabel->setToolTip(tr("After the completion of the construction of the need to export the module, there are two types of binary,devel \n"
                                    "binary for running, devel for development.depend only has devel module, app only has binary module. \n"
                                    "Example :\n"
                                    " binary:\".*\""));
        QLabel *annotationModulesLabel = new QLabel(tr("There are two types of modules,including binary and devel. It is binary by default."
                                                    "Binary for running, devel for development, depend only has devel module, app only has binary module.  "
                                                    "Example: "
                                                    "binary:\".*\"."));
        annotationModulesLabel->setWordWrap(true);
        annotationModulesLabel->setStyleSheet("color: gray");
        mainLayout->addWidget(modulesLabel);
        mainLayout->addWidget(annotationModulesLabel);

        m_modulesWidget = new QWidget();
        mainLayout->addWidget(m_modulesWidget);

        m_modulesLayout = new QVBoxLayout(m_modulesWidget);
        // 调整容器和布局的尺寸策略
        m_modulesWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        m_modulesLayout->setAlignment(Qt::AlignTop);

        //添加module按钮
        m_addModuleButton = new QPushButton(tr("Add module"));
        connect(m_addModuleButton, &QPushButton::clicked, this, &SetupInfoFrame::addModule);
        mainLayout->addWidget(m_addModuleButton);
    }

    // Environment variables section
    QLabel *envLabel = new QLabel(tr("env"));

    envLabel->setToolTip(tr("Add environment variables.\n"
                            "Key is the name of the environment variable, and value is the value of environment variable.\n"
                            "Example: \n"
                            " PATH: /usr/local/bin"));
    QLabel *annotationEnvLabel = new QLabel(tr("Add environment variables. "
                                               "Key is the name of the environment variable, and value is the value of environment variable. "
                                               "Example:  "
                                               "PATH: /usr/local/bin."));
    annotationEnvLabel->setWordWrap(true);
    annotationEnvLabel->setStyleSheet("color: gray");
    // 创建容器窗口并设置布局策略
    m_envContainer = new QWidget();
    m_envContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *envLayout = new QVBoxLayout(m_envContainer);
    envLayout->setContentsMargins(0, 0, 0, 0);

    envTableWidget = new QTableWidget();
    envLayout->addWidget(envTableWidget);

    envTableWidget->setColumnCount(3); // 新增一列用于删除按钮
    envTableWidget->setHorizontalHeaderLabels({"Key", "Value", ""}); // 表头
    // 设置列宽
    envTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); // Key 列
    envTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // Value 列
    envTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // 删除按钮列
    // 设置：允许表格自动扩展
    envTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    envTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 设置最小高度仅显示表头
    QTimer::singleShot(0, this, [this](){
        envTableWidget->setMinimumHeight(envTableWidget->horizontalHeader()->height());
    });

    addEnvButton = new QPushButton(tr("Add env"));
    connect(addEnvButton, &QPushButton::clicked, this, &SetupInfoFrame::createEnvRow);

    mainLayout->addWidget(envLabel);
    mainLayout->addWidget(annotationEnvLabel);
    mainLayout->addWidget(m_envContainer);
    mainLayout->addWidget(addEnvButton);

    // 设置整个窗口的尺寸策略
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void SetupInfoFrame::addModule()
{
    // Create new module row with QComboBox, QTextEdit, and delete button
    QWidget *moduleRow = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout();

    QComboBox *modulesCombo = new QComboBox();
    modulesCombo->addItems({tr("binary"), tr("devel")});
    layout->addWidget(modulesCombo);

    QTextEdit *moduleInput = new QTextEdit();
    QString placeholderText = ".*";
    moduleInput->setPlaceholderText(placeholderText);
    layout->addWidget(moduleInput);

    QPushButton *removeButton = new QPushButton("×", moduleRow);
    removeButton->setFixedSize(24, 24);
    layout->addWidget(removeButton);
    moduleRow->setLayout(layout);

    m_modulesLayout->addWidget(moduleRow);

    // 绑定删除事件
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        removeModule(moduleRow);
    });
}

void SetupInfoFrame::removeModule(QWidget *moduleRow)
{
    m_modulesLayout->removeWidget(moduleRow);
    delete moduleRow;
}

// 实现方法
void SetupInfoFrame::updateGeometry() {
    // 更新容器高度
    m_modulesWidget->adjustSize();

    // 更新整个窗口
    adjustSize();
    if (parentWidget()) {
        parentWidget()->adjustSize();
    }
}


void SetupInfoFrame::createEnvRow()
{
    const int row = envTableWidget->rowCount();
    envTableWidget->insertRow(row); // 插入新行

    // 添加 Key 和 Value 的输入框
    QTableWidgetItem *keyItem = new QTableWidgetItem();
    QTableWidgetItem *valueItem = new QTableWidgetItem();
    envTableWidget->setItem(row, 0, keyItem);
    envTableWidget->setItem(row, 1, valueItem);

    // 添加删除按钮
    QPushButton *deleteButton = new QPushButton("×");
    envTableWidget->setCellWidget(row, 2, deleteButton);

    // 自动调整容器高度
    const int rowHeight = 40; // 根据实际行高调整
    m_envContainer->setFixedHeight(envTableWidget->rowCount() * rowHeight + 35);

    // 绑定删除按钮的点击事件
    connect(deleteButton, &QPushButton::clicked, this, [this, deleteButton]() {
        // 获取当前按钮所在的行号
        for (int i = 0; i < envTableWidget->rowCount(); ++i) {
            if (envTableWidget->cellWidget(i, 2) == deleteButton) {
                envTableWidget->removeRow(i); // 删除当前行
                break;
            }
        }

        // 调整容器高度
        const int rowHeight = 40;
        m_envContainer->setFixedHeight(envTableWidget->rowCount() * rowHeight + 35);

        QTimer::singleShot(0, this, [this](){
            adjustSize();
            if(parentWidget()) parentWidget()->adjustSize();
        });

    });
}

void SetupInfoFrame::getData()
{
    // 提取 Command
    QString command = m_commandLineEdit->text().trimmed();
    QVector<QString> commands;
    QStringList cmds = parseCmdline(command);
    for (auto& cmd : cmds) {
        commands.append(cmd);
    }

    // 提取 Cleanup
    QString cleanupText = m_cleanupTextEdit->toPlainText();
    QVector<QString> cleanups;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    for (const QString &line : cleanupText.split("\n", Qt::SkipEmptyParts)) {
        cleanups.append(line.trimmed());
    }
#else
    for (const QString &line : cleanupText.split("\n", QString::SkipEmptyParts)) {
        cleanups.append(line.trimmed());
    }
#endif

    // 提取 Modules
    QMap<QString, QString> modules;
    for (int i = 0; i < m_modulesLayout->count(); ++i) {
        QWidget *moduleRow = m_modulesLayout->itemAt(i)->widget();
        if (moduleRow) {
            QComboBox *combo = moduleRow->findChild<QComboBox *>();
            QTextEdit *textEdit = moduleRow->findChild<QTextEdit *>();
            if (combo && textEdit) {
                QString moduleType = combo->currentText();
                QString modulePath = textEdit->toPlainText().trimmed();
                modules[moduleType] = modulePath;
            }
        }
    }

    // 提取 Environment Variables
    QMap<QString, QString> envs;
    for (int i = 0; i < envTableWidget->rowCount(); ++i) {
        QTableWidgetItem *keyItem = envTableWidget->item(i, 0);
        QTableWidgetItem *valueItem = envTableWidget->item(i, 1);
        if (keyItem && valueItem) {
            QString key = keyItem->text().trimmed();
            QString value = valueItem->text().trimmed();
            if (!key.isEmpty() && !value.isEmpty()) {
                envs[key] = value;
            }
        }
    }

    KMBuilderManifest * manifest = GlobelInfo::instance().getManifest();
    QString channel = m_channelLineEdit->text().trimmed();
    manifest->m_channel = channel.isEmpty() ? "stable" : channel;
    manifest->m_commands.clear();
    manifest->m_commands.append(commands);
    manifest->m_cleanups.clear();
    manifest->m_cleanups.append(cleanups);
    manifest->m_modules = modules;
    manifest->m_envs = envs;
}

void SetupInfoFrame::clear()
{
    m_channelLineEdit->setText("stable"); // 直接填入默认值 "stable"
    m_commandLineEdit->clear();
    m_cleanupTextEdit->clear();

    // 清理模块列表
    while (QLayoutItem *item = m_modulesLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater(); // 删除模块行
        }
        delete item; // 删除布局项
    }

    // 清理环境变量表格
    envTableWidget->setRowCount(0);
    m_envContainer->setFixedHeight(envTableWidget->horizontalHeader()->height());
}

void SetupInfoFrame::load(const QString &channel, const QVector<QString> &commands, const QVector<QString> &cleanups,const QMap<QString, QString> &modules, const QMap<QString, QString> &envs)
{
    m_channelLineEdit->setText(channel);

    // 加载 commands
    if (!commands.isEmpty()) {
        QStringList commandList;
        for (const QString &command : commands) {
            commandList.append(command);
        }
        m_commandLineEdit->setText(commandList.join(" ")); // 使用 QStringList 的 join 方法
    }

    // 加载 cleanups
    if (!cleanups.isEmpty()) {
        QStringList cleanupList;
        for (const QString &cleanup : cleanups) {
            cleanupList.append(cleanup);
        }
        m_cleanupTextEdit->setText(cleanupList.join("\n")); // 使用 QStringList 的 join 方法
    }

    // 加载 modules
     for (auto it = modules.begin(); it != modules.end(); ++it) {
         // 调用 addModule() 添加新行
         addModule();

         // 获取最后一行
         QWidget *moduleRow = m_modulesLayout->itemAt(m_modulesLayout->count() - 1)->widget();
         if (moduleRow) {
             // 找到 QComboBox 并设置模块类型
             QComboBox *modulesCombo = moduleRow->findChild<QComboBox *>();
             if (modulesCombo) {
                 modulesCombo->setCurrentText(it.key());
             }

             // 找到 QTextEdit 并设置模块路径
             QTextEdit *moduleInput = moduleRow->findChild<QTextEdit *>();
             if (moduleInput) {
                 moduleInput->setPlainText(it.value());
             }
         }
     }

    // 加载环境变量
    for (auto it = envs.begin(); it != envs.end(); ++it) {
        // 调用 createEnvRow() 创建新行
        createEnvRow();

        // 获取最后一行
        const int row = envTableWidget->rowCount() - 1;

        // 设置 Key 和 Value
        QTableWidgetItem *keyItem = new QTableWidgetItem(it.key());
        QTableWidgetItem *valueItem = new QTableWidgetItem(it.value());
        envTableWidget->setItem(row, 0, keyItem);
        envTableWidget->setItem(row, 1, valueItem);
    }
}

