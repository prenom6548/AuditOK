#include "sourcedialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTextEdit>
#include <QFileDialog>
#include <QCheckBox>
#include <QScrollArea>

#include "globalinfo.h"

SourceDialog::SourceDialog(const QString& componentName, QWidget *parent) :
    QDialog(parent),
    m_componentName(componentName)
{
    setFixedSize(500, 350);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);    

    // 创建 QScrollArea
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    // 创建容器 Widget，用于存放所有内容
    QWidget *containerWidget = new QWidget;
    QVBoxLayout *containerLayout = new QVBoxLayout(containerWidget);

    // 初始化公共控件
    {
        QFormLayout *formLayout = new QFormLayout();

//        // 固定的架构列表
//        QStringList arches = {"x86_64", "i386", "arm64", "loong64", "loongarch64", "mips64", "sw64"};

//        // 创建水平布局，用于放置两个 QGroupBox
//        QHBoxLayout *hboxLayout = new QHBoxLayout;

//        // only-arches 复选框组
//        m_onlyArchesGroupBox = new QGroupBox(tr("only-arches"));
//        QGridLayout *onlyArchesLayout = new QGridLayout;
//        for (int i = 0; i < arches.size(); ++i) {
//            QCheckBox *checkBox = new QCheckBox(arches[i]);
//            onlyArchesLayout->addWidget(checkBox, i / 2, i % 2); // 每行两个复选框
//        }
//        m_onlyArchesGroupBox->setLayout(onlyArchesLayout);
//        m_onlyArchesGroupBox->setToolTip(tr("Build the component only on the selected arches"));
//        hboxLayout->addWidget(m_onlyArchesGroupBox);

//        // skip-arches 复选框组
//        m_skipArchesGroupBox = new QGroupBox(tr("skip-arches"));
//        QGridLayout *skipArchesLayout = new QGridLayout;
//        for (int i = 0; i < arches.size(); ++i) {
//            QCheckBox *checkBox = new QCheckBox(arches[i]);
//            skipArchesLayout->addWidget(checkBox, i / 2, i % 2); // 每行两个复选框
//        }
//        m_skipArchesGroupBox->setLayout(skipArchesLayout);
//        m_skipArchesGroupBox->setToolTip(tr("Ignore the selected architectures, don't build the component on them"));
//        hboxLayout->addWidget(m_skipArchesGroupBox);

//        // 将水平布局添加到表单布局中
//        formLayout->addRow(hboxLayout);

//        // Command 输入框
//        m_commandEdit = new QTextEdit;

//        m_commandEdit->setPlaceholderText(tr("A series of script commands that are executed after the source code is extracted. "
//                                             "Example: "
//                                             "mv *.cpp files \\n "
//                                             "rm -rf data "));
//        m_commandEdit->setToolTip(tr("A series of script commands that are executed after the source code is extracted \n"
//                                     "Commands are separated by newlines \n"
//                                     "Example:\n"
//                                     " mv *.cpp files \n"
//                                     " rm -rf data \n"
//                                     " install -D demo ... \n"));
//        m_commandEdit->setFixedHeight(75);
//        formLayout->addRow(tr("command"), m_commandEdit);

        // 类型选择
        m_typeCombo = new QComboBox();
        m_typeCombo->setToolTip("Source type");
        m_typeCombo->addItem("git", KMBuilderManifest::GIT);
        m_typeCombo->addItem("archive", KMBuilderManifest::ARCHIVE);
        m_typeCombo->addItem("dir", KMBuilderManifest::DIR);
        m_typeCombo->addItem("file", KMBuilderManifest::FILE);
        formLayout->addRow(tr("type") + "<font color=red>*</font>", m_typeCombo);
        m_typeCombo->setToolTip(tr("The kind of code source"));

        containerLayout->addLayout(formLayout);
    }

    // 动态表单
    m_stack = new QStackedWidget();
    createGitForm();      // 索引 0
    createArchiveForm();  // 索引 1
    createDirForm();      // 索引 2
    createFileForm();     // 索引 3

    containerLayout->addWidget(m_stack, 1);

    // 连接信号与槽
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SourceDialog::onTypeChanged);

    m_stack->setCurrentIndex(0); // 显示第一个添加的表单

    // 确认按钮
    QPushButton *btnOk = new QPushButton(tr("OK"));
    connect(btnOk, &QPushButton::clicked, this,[=](){
        QString type = m_typeCombo->currentText();
        //git
        if (type == "git")
        {

            QString url= m_fields["git_url"]->text();
            if (url.isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(containerWidget, QObject::tr("Warning"),
                                                                       QObject::tr("Please set url"),
                                                                       QObject::tr("OK"));
                return;
            }
        }

        //archive
        if (type == "archive")
        {
            QString url = m_fields["archive_url"]->text();
            if (url.isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(containerWidget, QObject::tr("Warning"),
                                                                       QObject::tr("Please set url"),
                                                                       QObject::tr("OK"));
                return;
            }

            QString sha256 = m_fields["archive_sha256"]->text();
            if (sha256.isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(containerWidget, QObject::tr("Warning"),
                                                                       QObject::tr("Please set sha256"),
                                                                       QObject::tr("OK"));
                return;
            }
        }

        //dir
        if (type == "dir")
        {
            QString path = m_fields["dir_path"]->text();
            if (path.isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(containerWidget, QObject::tr("Warning"),
                                                                       QObject::tr("Please set path"),
                                                                       QObject::tr("OK"));
                return;
            }
        }

        //file
        if (type == "file")
        {
            QString path = m_fields["file_path"]->text();
            if (path.isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(containerWidget, QObject::tr("Warning"),
                                                                       QObject::tr("Please set path"),
                                                                       QObject::tr("OK"));
                return;
            }
        }
        QDialog::accept();
    });
    containerLayout->addWidget(btnOk);

    // 将容器 Widget 设置为 QScrollArea 的内容
    scrollArea->setWidget(containerWidget);
    mainLayout->addWidget(scrollArea); // 将 QScrollArea 添加到主布局
}

KMBuilderManifest::Component::Source SourceDialog::getSourceData()  {
    KMBuilderManifest::Component::Source source; // 创建临时对象
    // 关键修正：从UI控件获取当前类型
    source.m_type = static_cast<KMBuilderManifest::SOURCEType>(
        m_typeCombo->currentData().toInt()
    );

//    // 获取 only-arches 选中的架构
//    QVector<QString> onlyArches;
//    if (m_onlyArchesGroupBox) {
//        for (QCheckBox *checkBox : m_onlyArchesGroupBox->findChildren<QCheckBox*>()) {
//            if (checkBox->isChecked()) {
//                onlyArches.append(checkBox->text()); // 将选中的架构添加到列表中
//            }
//        }
//    }
//    source.m_onlyArches = onlyArches;

//    // 获取 skip-arches 选中的架构
//    QVector<QString> skipArches;
//    if (m_skipArchesGroupBox) {
//        for (QCheckBox *checkBox : m_skipArchesGroupBox->findChildren<QCheckBox*>()) {
//            if (checkBox->isChecked()) {
//                skipArches.append(checkBox->text()); // 将选中的架构添加到列表中
//            }
//        }
//    }
//    source.m_skipArches = skipArches;

//    source.m_command = m_commandEdit->toPlainText();
    // 根据当前类型填充对应字段
    switch(source.m_type) {
    case KMBuilderManifest::GIT:
        source.m_git.m_url = m_fields["git_url"]->text();
        source.m_git.m_branch = m_fields["git_branch"]->text();
        source.m_git.m_tag = m_fields["git_tag"]->text();
        source.m_git.m_commit = m_fields["git_commit"]->text();
        break;
    case KMBuilderManifest::ARCHIVE:
//        source.m_archive.m_path = m_fields["archive_path"]->text();
        source.m_archive.m_url = m_fields["archive_url"]->text();
        source.m_archive.m_sha256 = m_fields["archive_sha256"]->text();
//        source.m_archive.m_sha512 = m_fields["archive_sha512"]->text();
//        source.m_archive.m_destFilename = m_fields["archive_destfilename"]->text();
        break;
    case KMBuilderManifest::DIR:
    {

        source.m_dir.m_path = m_fields["dir_path"]->text();

        QString skipText = m_fields["dir_skip"]->text().trimmed();
        QVector<QString> processedSkips = splitAndTrim(skipText);
        source.m_dir.m_skip = processedSkips;

        source.m_dir.m_originalPath = m_originalDirEdit->text();
        break;
    }
    case KMBuilderManifest::FILE:
        source.m_file.m_path = m_fields["file_path"]->text();
//        source.m_file.m_url = m_fields["file_url"]->text();
//        source.m_file.m_sha256 = m_fields["file_sha256"]->text();
//        source.m_file.m_sha512 = m_fields["file_sha512"]->text();
//        source.m_file.m_destFilename = m_fields["file_destfilename"]->text();

        source.m_file.m_originalPath = m_originalFileEdit->text();
        break;
    default:
        break;
    }
    return source;
}

void SourceDialog::setSourceData(const KMBuilderManifest::Component::Source &source) {
//    // 设置公共字段
//    // 设置 only-arches 复选框的状态
//    if (m_onlyArchesGroupBox) {
//        for (QCheckBox *checkBox : m_onlyArchesGroupBox->findChildren<QCheckBox*>()) {
//            // 如果复选框的文本在 source.m_onlyArches 中，则设置为选中状态
//            checkBox->setChecked(source.m_onlyArches.contains(checkBox->text()));
//        }
//    }

//    // 设置 skip-arches 复选框的状态
//    if (m_skipArchesGroupBox) {
//        for (QCheckBox *checkBox : m_skipArchesGroupBox->findChildren<QCheckBox*>()) {
//            // 如果复选框的文本在 source.m_skipArches 中，则设置为选中状态
//            checkBox->setChecked(source.m_skipArches.contains(checkBox->text()));
//        }
//    }

//    m_commandEdit->setPlainText(source.m_command);

    // 设置类型
    int typeIndex = m_typeCombo->findData(static_cast<int>(source.m_type));
    if (typeIndex != -1) {
        m_typeCombo->setCurrentIndex(typeIndex);
    }

    // 根据类型设置具体字段
    switch (source.m_type) {
    case KMBuilderManifest::GIT:
        m_fields["git_url"]->setText(source.m_git.m_url);
        m_fields["git_branch"]->setText(source.m_git.m_branch);
        m_fields["git_tag"]->setText(source.m_git.m_tag);
        m_fields["git_commit"]->setText(source.m_git.m_commit);
        break;
    case KMBuilderManifest::ARCHIVE:
//        m_fields["archive_path"]->setText(source.m_archive.m_path);
        m_fields["archive_url"]->setText(source.m_archive.m_url);
        m_fields["archive_sha256"]->setText(source.m_archive.m_sha256);
//        m_fields["archive_sha512"]->setText(source.m_archive.m_sha512);
//        m_fields["archive_destfilename"]->setText(source.m_archive.m_destFilename);
        break;
    case KMBuilderManifest::DIR: {
        QStringList skipList = source.m_dir.m_skip.toList(); // 将 QVector<QString> 转换为 QStringList
        m_fields["dir_path"]->setText(source.m_dir.m_path);
        m_fields["dir_skip"]->setText(skipList.join(", "));

        m_originalDirEdit->setText(source.m_dir.m_originalPath);
        break;
    }
    case KMBuilderManifest::FILE:
        m_fields["file_path"]->setText(source.m_file.m_path);
//        m_fields["file_url"]->setText(source.m_file.m_url);
//        m_fields["file_sha256"]->setText(source.m_file.m_sha256);
//        m_fields["file_sha512"]->setText(source.m_file.m_sha512);
//        m_fields["file_destfilename"]->setText(source.m_file.m_destFilename);

         m_originalFileEdit->setText(source.m_file.m_originalPath);
        break;
    default:
        break;
    }
}

// 类型切换槽函数
void SourceDialog::onTypeChanged(int index) {
    m_stack->setCurrentIndex(index);
}

void SourceDialog::createGitForm() {
    QWidget *form = new QWidget(m_stack);
    QFormLayout *layout = new QFormLayout(form);

    // URL 输入框
    QLineEdit *urlEdit = new QLineEdit;
    urlEdit->setPlaceholderText(tr("Git project URL. "
                                   "Example: "
                                   "https://github.com/user/repo.git"));
    urlEdit->setToolTip(tr("Git repository URL, \n"
                           "Example: \n"
                           "https://github.com/user/repo.git"));
    layout->addRow(tr("url") + "<font color=red>*</font>", urlEdit);

    // Branch 输入框
    QLineEdit *branchEdit = new QLineEdit;
    branchEdit->setPlaceholderText(tr("The branch to which a Git project belongs in the repository. "
                                      "Example: "
                                      " master"));
    branchEdit->setToolTip(tr("The branch to which a Git project belongs in the repository.\n"
                              "Example:\n"
                              "master"));
    layout->addRow(tr("branch"), branchEdit);

    // Tag 输入框
    QLineEdit *tagEdit = new QLineEdit;
    tagEdit->setPlaceholderText(tr("The tag of a git project in the repository. "
                                   "Example: "
                                   "tag_test/v1.0.0"));
    tagEdit->setToolTip(tr("The tag of a git project in the repository.\n"
                           "Example:\n"
                           "tag_test/v1.0.0"));
    layout->addRow(tr("tag"), tagEdit);

    // Commit 输入框
    QLineEdit *commitEdit = new QLineEdit;
    commitEdit->setPlaceholderText(tr("The commit to use in the git project. "
                                      "Example: "
                                      "d3b07384d113edec49eaa6238ad5ff0"));
    commitEdit->setToolTip(tr("If a branch, a tag, and a commit are set simultaneously, the priority order is commit > tag > branch.\n"
                              "Example:\n"
                              "d3b07384d113edec49eaa6238ad5ff0"));
    layout->addRow(tr("commit"), commitEdit);

    // 添加控件到堆栈和字段映射
    m_stack->addWidget(form);
    m_fields["git_url"] = urlEdit;
    m_fields["git_branch"] = branchEdit;
    m_fields["git_tag"] = tagEdit;
    m_fields["git_commit"] = commitEdit;
}

void SourceDialog::createArchiveForm() {
    QWidget *form = new QWidget(m_stack);
    QFormLayout *layout = new QFormLayout(form);

//    // Path 输入框
//    QLineEdit *pathEdit = new QLineEdit;
//    pathEdit->setPlaceholderText(tr("The path to the archive file ,which is relative path. "
//                                    "Example: "
//                                    "path: ./file.zip"));
//    pathEdit->setToolTip(tr("The path to the archive file. Note: path is the relative path to the yaml file (choose one between path and url)\n"
//                            "Example:\n"
//                            "path: ./file.zip"));
//    layout->addRow(tr("path:"), pathEdit);

    // URL 输入框
    QLineEdit *urlEdit = new QLineEdit;
    urlEdit->setPlaceholderText(tr("Remote archive address. "
                                   "Example: "
                                   "https://....../dir/file.zip"));
    urlEdit->setToolTip(tr("Remote archive address (if path is also set, url is preferred),The remote address can be http, https, ftp, or ftps\n"
                           "Example:\n"
                           "https://....../dir/file.zip"));
    layout->addRow(tr("url") + "<font color=red>*</font>", urlEdit);

    // SHA256 输入框
    QLineEdit *sha256Edit = new QLineEdit;
    sha256Edit->setPlaceholderText(tr("sha256 checksum of the file ")); //文件的sha256校验和
    sha256Edit->setToolTip(tr("sha256 checksum of the file.\n"
                              "Example:\n"
                              "962eb48e42ae130da99c7909a4c11a93c9799d62932c8531cb78f646a35f653c"));
    layout->addRow(tr("sha256") + "<font color=red>*</font>", sha256Edit);


//    // SHA512 输入框
//    QLineEdit *sha512Edit = new QLineEdit;
//    sha512Edit->setPlaceholderText(tr("sha512 checksum of the file (optional with sha256)")); //文件的sha512校验和
//    sha512Edit->setToolTip(tr("sha512 checksum of the file (optional with sha256)\n"
//                              "Example:\n"
//                              "sha512:f5cb59b56b3f9efede2f4619064ce992bbca53b094990cd84c1b844274d8de612b07929d4be9bc02bc3dfcd623c1c2d0dd6e4c92311ab8a7baaf0fff9e8decb6"));
//    layout->addRow(tr("sha512:"), sha512Edit);

//    // Dest-filename 输入框
//    QLineEdit *destEdit = new QLineEdit;
//    destEdit->setPlaceholderText(tr("Customize the downloaded file name. "
//                                    "Example: "
//                                    "dest-filename: customized filename"));
//    destEdit->setToolTip(tr("Customize the downloaded file name.\n"
//                            "The default is the basename of the url\n"
//                            "Example:\n"
//                            "dest-filename: customized filename"));
//    layout->addRow(tr("dest-filename:"), destEdit);

    // 添加控件到堆栈和字段映射
    m_stack->addWidget(form);
//    m_fields["archive_path"] = pathEdit;
    m_fields["archive_url"] = urlEdit;
    m_fields["archive_sha256"] = sha256Edit;
//    m_fields["archive_sha512"] = sha512Edit;
//    m_fields["archive_destfilename"] = destEdit;
}

void SourceDialog::createDirForm()
{
    QWidget *form = new QWidget(m_stack);
    QFormLayout *layout = new QFormLayout(form);

    m_originalDirEdit = new ChooseLineEdit;
    m_originalDirEdit->setReadOnly(true);
    layout->addRow(tr("Original Path"), m_originalDirEdit);

    // Path 输入框 (必填)
    QLineEdit *pathEdit = new QLineEdit;

    pathEdit->setReadOnly(true);
//    pathEdit->setPlaceholderText(tr("The path of the local directory to be copied. "
//                                    "Example: "
//                                    "./src"));
    pathEdit->setToolTip(tr("The path of the local directory to be copied\n"
                            "Example:\n"
                            "./src"));
    layout->addRow(tr("path") + "<font color=red>*</font>", pathEdit);

    // Skip 输入框 (可选数组)
    QLineEdit *skipEdit = new QLineEdit;
    skipEdit->setPlaceholderText(tr("Ignored files/directories when copying. "
                                    "Example: "
                                    "*.so, src"));
    skipEdit->setToolTip(tr("Ignored files/directories, Wildcards are supported, Separated with commas.\n"
                            "Example:\n"
                            "*.so, src"));
    layout->addRow(tr("skip"), skipEdit);

    m_stack->addWidget(form);
    m_fields["dir_path"] = pathEdit;
    m_fields["dir_skip"] = skipEdit; // 存储为逗号分隔字符串，使用时拆分

    connect(m_originalDirEdit, &ChooseLineEdit::choose, this, [=](){
        QFileDialog *fileDialog = new QFileDialog(this);
        fileDialog->setWindowTitle(tr("Please select source path"));
        // fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
        fileDialog->setFileMode(QFileDialog::DirectoryOnly);
        fileDialog->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
        QList<QUrl> siderUrls;
        siderUrls << QUrl::fromLocalFile(QDir::homePath());
        fileDialog->setSidebarUrls(siderUrls);

        if (fileDialog->exec() == QDialog::Accepted) {
            QStringList selectFiles = fileDialog->selectedFiles();
            if (selectFiles.size() > 0) {
                QString selectedPath = selectFiles.at(0);
                m_originalDirEdit->setText(selectedPath);
                QString projectDir =  GlobelInfo::instance().getProjectDir();

                // 检查 selectedPath 是否是 projectDir 的子目录
                if (selectedPath.startsWith(projectDir)) {
                    // 是子目录，截取相对路径
                    QString relativePath = QDir(projectDir).relativeFilePath(selectedPath);
                    pathEdit->setText(relativePath);
                } else {
                    QDir dir(selectedPath);
                    pathEdit->setText("kaiming-bundles/" + this->m_componentName + "/" + dir.dirName());
                }

                m_originalDirEdit->clearFocus();
            }
        }

        fileDialog->deleteLater();
    });
}

void SourceDialog::createFileForm()
{
    QWidget *form = new QWidget(m_stack);
    QFormLayout *layout = new QFormLayout(form);

    m_originalFileEdit = new ChooseLineEdit;
    m_originalFileEdit->setReadOnly(true);
    layout->addRow(tr("Original Path"), m_originalFileEdit);

    // Path 输入框 (path/url二选一)
    QLineEdit *pathEdit = new QLineEdit;

    pathEdit->setReadOnly(true);
//    pathEdit->setPlaceholderText(tr("The path of the local file to be copied. "
//                                    "Example: "
//                                    "./demo.sh"));
    pathEdit->setToolTip(tr("The path of the local file to be copied, read-only"));
    layout->addRow(tr("path") + "<font color=red>*</font>", pathEdit);

//    // URL 输入框 (path/url二选一)
//    QLineEdit *urlEdit = new QLineEdit;
//    urlEdit->setPlaceholderText(tr("https://..../dmeo.sh"));
//    urlEdit->setToolTip(tr("Remote file address, supporting http/https/ftp/ftps\n"
//                           "(if the path is set simultaneously, the url will be given priority).\n"
//                           "Example:\n"
//                           "url: https://..../dmeo.sh "));
//    layout->addRow(tr("url:"), urlEdit);

//    // SHA256 输入框 (sha256/sha512二选一)
//    QLineEdit *sha256Edit = new QLineEdit;
//    sha256Edit->setPlaceholderText(tr("sha256 checksum"));
//    sha256Edit->setToolTip(tr("sha256 checksumof file(choose one between this and sha512)\n"
//                              "Example:\n"
//                              "sha256: 962eb48e42ae130da99c7909a4c11a93c9799d62932c8531cb78f646a35f653c"));
//    sha256Edit->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-f0-9]{64}$"), this));
//    layout->addRow(tr("sha256:"), sha256Edit);

//    // SHA512 输入框 (sha256/sha512二选一)
//    QLineEdit *sha512Edit = new QLineEdit;
//    sha512Edit->setPlaceholderText(tr("sha512 checksum"));
//    sha512Edit->setToolTip(tr("sha512 checksum of file(choose one between this and sha256)\n"
//                              "Example:\n"
//                              "sha512:f5cb59b56b3f9efede2f4619064ce992bbca53b094990cd84c1b844274d8de612b07929d4be9bc02bc3dfcd623c1c2d0dd6e4c92311ab8a7baaf0fff9e8decb6"));
//    sha512Edit->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-f0-9]{128}$"), this));
//    layout->addRow(tr("sha512:"), sha512Edit);


    // Dest-filename 输入框
//    QLineEdit *destEdit = new QLineEdit;
//    destEdit->setPlaceholderText(tr("dest-filename"));
//    destEdit->setToolTip(tr("Customize the downloaded file name.\n"
//                            "The default is the basename of the url\n"
//                            "Example:\n"
//                            "dest-filename: filename"));
//    layout->addRow(tr("dest-filename:"), destEdit);

    connect(m_originalFileEdit, &ChooseLineEdit::choose, this, [=](){
        QFileDialog *fileDialog = new QFileDialog(this);
        fileDialog->setWindowTitle(tr("Please select source path"));
        // fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
        fileDialog->setFileMode(QFileDialog::ExistingFile);
        fileDialog->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        QList<QUrl> siderUrls;
        siderUrls << QUrl::fromLocalFile(QDir::homePath());
        fileDialog->setSidebarUrls(siderUrls);

        if (fileDialog->exec() == QDialog::Accepted) {
            QStringList selectFiles = fileDialog->selectedFiles();
            if (selectFiles.size() > 0) {
                QString selectedPath = selectFiles.at(0);
                m_originalFileEdit->setText(selectedPath);
                QString projectDir =  GlobelInfo::instance().getProjectDir();

                // 检查 selectedPath 是否是 projectDir 的子目录
                if (selectedPath.startsWith(projectDir)) {
                    // 是子目录，截取相对路径
                    QString relativePath = QDir(projectDir).relativeFilePath(selectedPath);
                    pathEdit->setText(relativePath);
                } else {
                    QDir dir(selectedPath);
                    pathEdit->setText("kaiming-bundles/" + this->m_componentName + "/" + dir.dirName());
                }

                m_originalDirEdit->clearFocus();
            }
        }

        fileDialog->deleteLater();
    });

    m_stack->addWidget(form);
    m_fields["file_path"] = pathEdit;
//    m_fields["file_url"] = urlEdit;
//    m_fields["file_sha256"] = sha256Edit;
//    m_fields["file_sha512"] = sha512Edit;
//    m_fields["file_destfilename"] = destEdit;
}

QVector<QString> SourceDialog::splitAndTrim(const QString& input) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QStringList rawList = input.split(',', Qt::SkipEmptyParts);
#else
    QStringList rawList = input.split(',', QString::SkipEmptyParts);
#endif
    QVector<QString> result;
    result.reserve(rawList.size());  // 预分配内存提升性能

    for (QString item : rawList) {
        QString trimmed = item.trimmed();
        if (!trimmed.isEmpty()) {
            result.append(trimmed);
        }
    }
    return result;
}

