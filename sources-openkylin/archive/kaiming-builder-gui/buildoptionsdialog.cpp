#include "buildoptionsdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QScrollArea>

BuildOptionsDialog::BuildOptionsDialog(QWidget *parent)
    : QDialog(parent)
{
    initUI();
}

void BuildOptionsDialog::initUI()
{
    setWindowTitle(tr("BuildOptions"));
    resize(800, 800);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget *areaWidget = new QWidget;
    scrollArea->setWidget(areaWidget);
    QVBoxLayout *buildoptionsLayout = new QVBoxLayout;
    areaWidget->setLayout(buildoptionsLayout);

    mainLayout->addWidget(scrollArea);

    // 基本构建选项
    QGroupBox *basicGroup = new QGroupBox(tr("Flags Options"));
    QFormLayout *basicLayout = new QFormLayout(basicGroup);

    m_cFlagsEdit = new QLineEdit;
    basicLayout->addRow(tr("cflags"), m_cFlagsEdit);
    m_cFlagsEdit->setPlaceholderText(tr("C/C++ compiler options environment variable, multiple options separated by spaces. Example: -O2 -g"));
    m_cFlagsEdit->setToolTip(tr("Environment variable for setting C/C++ compiler options\n"
                              "Can specify various compilation options like optimization level (e.g., -O2),\n"
                              "warning level (e.g., -Wall), include paths (e.g., -I/path/to/include)\n"
                              "Common options for both C and C++ compilers"));


    m_cppFlagsEdit = new QLineEdit;
    basicLayout->addRow(tr("cppflags"), m_cppFlagsEdit);
    m_cppFlagsEdit->setPlaceholderText(tr("Options needed by preprocessor, multiple options separated by spaces. Example: -I/usr/include -DDEBUG"));
    m_cppFlagsEdit->setToolTip(tr("Options needed by preprocessor\n"
                                "Mainly for specifying include paths (e.g., -I option)\n"
                                "Preprocessor will handle these options before compilation\n"
                                "to correctly find and include required header files"));

    m_cxxFlagsEdit = new QLineEdit;
    basicLayout->addRow(tr("cxxflags"), m_cxxFlagsEdit);
    m_cxxFlagsEdit->setPlaceholderText(tr("Environment variable used in CMake, multiple options separated by spaces. Example: -std=c++11 -Wall"));
    m_cxxFlagsEdit->setToolTip(tr("Environment variable used in CMake, specifically for C++ compiler options\n"
                                "When using CMake for project building, CXXFLAGS value will be used by CMake\n"
                                "to initialize CMAKE_CXX_FLAGS cache entry\n"
                                "Can add default compilation flags for C++ files"));

    m_ldFlagsEdit = new QLineEdit;
    basicLayout->addRow(tr("ldflags"), m_ldFlagsEdit);
    m_ldFlagsEdit->setPlaceholderText(tr("Linker options environment variable, multiple options separated by spaces. Example: -L/path/to/lib -lmylib"));
    m_ldFlagsEdit->setToolTip(tr("Environment variable for setting linker options\n"
                              "1. Specify library path: -L option for linker to search library files\n"
                              "2. Specify library files: -l option to link required libraries\n"
                              "3. Other linker options: can include other linker options as needed\n"
                              "Example: -L/path/to/lib -lmylib"));

    // 路径选项
    QGroupBox *pathGroup = new QGroupBox(tr("Path Options"));
    QFormLayout *pathLayout = new QFormLayout(pathGroup);

    m_appendPathEdit = new QLineEdit;
    pathLayout->addRow(tr("append-path"), m_appendPathEdit);
    m_appendPathEdit->setPlaceholderText(tr("Append to PATH, using leading colon. Example: :/opt/bin:/usr/local/bin"));
    m_appendPathEdit->setToolTip(tr("This will be appended to PATH in build environment\n"
                                  "Format: :path1:path2 (with leading colon)\n"
                                  "Example: :/opt/bin:/usr/local/bin"));

    m_prependPathEdit = new QLineEdit;
    pathLayout->addRow(tr("prepend-path"), m_prependPathEdit);
    m_prependPathEdit->setPlaceholderText(tr("Prepend to PATH, using trailing colon. Example: /opt/bin:/usr/local/bin:"));
    m_prependPathEdit->setToolTip(tr("This will be prepended to PATH in build environment\n"
                                   "Format: path1:path2: (with trailing colon)\n"
                                   "Example: /opt/bin:/usr/local/bin:"));

    m_appendLdLibraryPathEdit = new QLineEdit;
    pathLayout->addRow(tr("append-ld-library-path"), m_appendLdLibraryPathEdit);
    m_appendLdLibraryPathEdit->setPlaceholderText(tr("Append to LD_LIBRARY_PATH, using leading colon. Example: :/usr/local/lib:/opt/lib"));
    m_appendLdLibraryPathEdit->setToolTip(tr("This will be appended to LD_LIBRARY_PATH in build environment\n"
                                           "Format: :path1:path2 (with leading colon)\n"
                                           "Example: :/usr/local/lib:/opt/lib"));

    m_prependLdLibraryPathEdit = new QLineEdit;
    pathLayout->addRow(tr("prepend-ld-library-path"), m_prependLdLibraryPathEdit);
    m_prependLdLibraryPathEdit->setPlaceholderText(tr("Prepend to LD_LIBRARY_PATH, using trailing colon. Example: /usr/local/lib:/opt/lib:"));
    m_prependLdLibraryPathEdit->setToolTip(tr("This will be prepended to LD_LIBRARY_PATH in build environment\n"
                                            "Format: path1:path2: (with trailing colon)\n"
                                            "Example: /usr/local/lib:/opt/lib:"));

    m_appendPkgConfigPathEdit = new QLineEdit;
    pathLayout->addRow(tr("append-pkg-config-path"), m_appendPkgConfigPathEdit);
    m_appendPkgConfigPathEdit->setPlaceholderText(tr("Append to PKG_CONFIG_PATH, using leading colon. Example: :/new/pkg1:/new/pkg2"));
    m_appendPkgConfigPathEdit->setToolTip(tr("This will be appended to PKG_CONFIG_PATH in build environment\n"
                                           "Format: :path1:path2 (with leading colon)\n"
                                           "Example: :/usr/local/lib/pkgconfig:/opt/lib/pkgconfig"));

    m_prependPkgConfigPathEdit = new QLineEdit;
    pathLayout->addRow(tr("prepend-pkg-config-path"), m_prependPkgConfigPathEdit);
    m_prependPkgConfigPathEdit->setPlaceholderText(tr("Prepend to PKG_CONFIG_PATH, using trailing colon. Example: /new/pkg1:/new/pkg2:"));
    m_prependPkgConfigPathEdit->setToolTip(tr("This will be prepended to PKG_CONFIG_PATH in build environment\n"
                                            "Format: path1:path2: (with trailing colon)\n"
                                            "Example: /usr/local/lib/pkgconfig:/opt/lib/pkgconfig:"));

    // 环境变量和make参数
    QGroupBox *advancedGroup = new QGroupBox(tr("Advanced Options"));
    QFormLayout *advancedLayout = new QFormLayout(advancedGroup);

    // Environment variables section
    m_envContainer = new QWidget();
    m_envContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *envLayout = new QVBoxLayout(m_envContainer);
    envLayout->setContentsMargins(0, 0, 0, 0);

    m_envTableWidget = new QTableWidget();
    envLayout->addWidget(m_envTableWidget);

    m_envTableWidget->setColumnCount(3); // 新增一列用于删除按钮
    m_envTableWidget->setHorizontalHeaderLabels({"Key", "Value", ""}); // 表头
    // 设置列宽
    m_envTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); // Key 列
    m_envTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // Value 列
    m_envTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // 删除按钮列
    // 设置：允许表格自动扩展
    m_envTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_envTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 设置最小高度仅显示表头
    QTimer::singleShot(0, this, [this](){
        m_envTableWidget->setMinimumHeight(m_envTableWidget->horizontalHeader()->height());
    });

    m_addEnvButton = new QPushButton(tr("Add env"));
    connect(m_addEnvButton, &QPushButton::clicked, this, &BuildOptionsDialog::createEnvRow);

    advancedLayout->addRow(tr("env"), m_envContainer);
    advancedLayout->addRow("",m_addEnvButton);

    m_makeArgsEdit = new QTextEdit;
    m_makeArgsEdit->setPlaceholderText(tr("Enter one make argument per line, example:\n"
                                       "-j4\n"
                                       "CFLAGS=\"-O2\"\n"
                                       "DESTDIR=/tmp/pkg\n"
                                       "VERBOSE=1"));
    advancedLayout->addRow(tr("make-args"), m_makeArgsEdit);

    m_makeInstallArgsEdit = new QTextEdit;
    m_makeInstallArgsEdit->setPlaceholderText(tr("Enter one make install argument per line, example:\n"
                                              "INSTALL_ROOT=/mnt\n"
                                              "strip\n"
                                              "-i4"));
    advancedLayout->addRow(tr("make-install-args"), m_makeInstallArgsEdit);

    // 按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 添加到主布局
    buildoptionsLayout->addWidget(basicGroup);
    buildoptionsLayout->addWidget(pathGroup);
    buildoptionsLayout->addWidget(advancedGroup);
    mainLayout->addWidget(buttonBox);
}

KMBuilderManifest::Component::BuildOptions BuildOptionsDialog::getBuildOptionsData() const
{
    KMBuilderManifest::Component::BuildOptions options;

    options.m_cFlags = m_cFlagsEdit->text().trimmed();
    options.m_cppFlags = m_cppFlagsEdit->text().trimmed();
    options.m_cxxFlags = m_cxxFlagsEdit->text().trimmed();
    options.m_ldFlags = m_ldFlagsEdit->text().trimmed();
    options.m_appendPath = m_appendPathEdit->text().trimmed();
    options.m_prependPath = m_prependPathEdit->text().trimmed();
    options.m_appendLdLibraryPath = m_appendLdLibraryPathEdit->text().trimmed();
    options.m_prependLdLibraryPath = m_prependLdLibraryPathEdit->text().trimmed();
    options.m_appendPkgConfigPath = m_appendPkgConfigPathEdit->text().trimmed();
    options.m_prependPkgConfigPath = m_prependPkgConfigPathEdit->text().trimmed();

    // 提取 Env
    QMap<QString, QString> envs;
    for (int i = 0; i < m_envTableWidget->rowCount(); ++i) {
        QTableWidgetItem *keyItem = m_envTableWidget->item(i, 0);
        QTableWidgetItem *valueItem = m_envTableWidget->item(i, 1);
        if (keyItem && valueItem) {
            QString key = keyItem->text().trimmed();
            QString value = valueItem->text().trimmed();
            if (!key.isEmpty() && !value.isEmpty()) {
                envs[key] = value;
            }
        }
    }
    options.m_env = envs;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    for (const QString &arg : m_makeArgsEdit->toPlainText().split('\n', Qt::SkipEmptyParts)) {
        options.m_makeArgs.append(arg.trimmed());
    }

    for (const QString &arg : m_makeInstallArgsEdit->toPlainText().split('\n', Qt::SkipEmptyParts)) {
        options.m_makeInstallArgs.append(arg.trimmed());
    }
#else
    for (const QString &arg : m_makeArgsEdit->toPlainText().split('\n', QString::SkipEmptyParts)) {
        options.m_makeArgs.append(arg.trimmed());
    }

    for (const QString &arg : m_makeInstallArgsEdit->toPlainText().split('\n', QString::SkipEmptyParts)) {
        options.m_makeInstallArgs.append(arg.trimmed());
    }
#endif

    return options;
}

void BuildOptionsDialog::setBuildOptionsData(const KMBuilderManifest::Component::BuildOptions &buildOptions)
{
    m_cFlagsEdit->setText(buildOptions.m_cFlags);
    m_cppFlagsEdit->setText(buildOptions.m_cppFlags);
    m_cxxFlagsEdit->setText(buildOptions.m_cxxFlags);
    m_ldFlagsEdit->setText(buildOptions.m_ldFlags);
    m_appendPathEdit->setText(buildOptions.m_appendPath);
    m_prependPathEdit->setText(buildOptions.m_prependPath);
    m_appendLdLibraryPathEdit->setText(buildOptions.m_appendLdLibraryPath);
    m_prependLdLibraryPathEdit->setText(buildOptions.m_prependLdLibraryPath);
    m_appendPkgConfigPathEdit->setText(buildOptions.m_appendPkgConfigPath);
    m_prependPkgConfigPathEdit->setText(buildOptions.m_prependPkgConfigPath);

    // 加载环境变量
    for (auto it = buildOptions.m_env.begin(); it != buildOptions.m_env.end(); ++it) {
        // 调用 createEnvRow() 创建新行
        createEnvRow();

        // 获取最后一行
        const int row = m_envTableWidget->rowCount() - 1;

        // 设置 Key 和 Value
        QTableWidgetItem *keyItem = new QTableWidgetItem(it.key());
        QTableWidgetItem *valueItem = new QTableWidgetItem(it.value());
        m_envTableWidget->setItem(row, 0, keyItem);
        m_envTableWidget->setItem(row, 1, valueItem);
    }

    // 设置make参数
    m_makeArgsEdit->setText(buildOptions.m_makeArgs.join('\n'));
    qDebug() << "Make Install Args:" << buildOptions.m_makeInstallArgs;
    // 设置make install参数
    m_makeInstallArgsEdit->setText(buildOptions.m_makeInstallArgs.join('\n'));
}

void BuildOptionsDialog::createEnvRow()
{
    const int row = m_envTableWidget->rowCount();
    m_envTableWidget->insertRow(row); // 插入新行

    // 添加 Key 和 Value 的输入框
    QTableWidgetItem *keyItem = new QTableWidgetItem();
    QTableWidgetItem *valueItem = new QTableWidgetItem();
    m_envTableWidget->setItem(row, 0, keyItem);
    m_envTableWidget->setItem(row, 1, valueItem);

    // 添加删除按钮
    QPushButton *deleteButton = new QPushButton("×");
    m_envTableWidget->setCellWidget(row, 2, deleteButton);

    // 自动调整容器高度
    const int rowHeight = 40; // 根据实际行高调整
    m_envContainer->setFixedHeight(m_envTableWidget->rowCount() * rowHeight + 35);

    // 绑定删除按钮的点击事件
    connect(deleteButton, &QPushButton::clicked, this, [this, deleteButton]() {
        // 获取当前按钮所在的行号
        for (int i = 0; i < m_envTableWidget->rowCount(); ++i) {
            if (m_envTableWidget->cellWidget(i, 2) == deleteButton) {
                m_envTableWidget->removeRow(i); // 删除当前行
                break;
            }
        }

        // 调整容器高度
        const int rowHeight = 40;
        m_envContainer->setFixedHeight(m_envTableWidget->rowCount() * rowHeight + 35);

    });
}
