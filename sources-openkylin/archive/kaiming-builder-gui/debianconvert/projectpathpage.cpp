#include "projectpathpage.h"
#include "messageboxutils.h"
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QFormLayout>
#include <QRegularExpression>

ProjectPathPage::ProjectPathPage(QWidget *parent)
    : QWizardPage(parent)
{
    // 创建自定义标题控件
    QLabel *titleLabel = new QLabel(tr("Project Location"), this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    QLabel *subTitleLabel = new QLabel(tr("This wizard is used to convert Debian source code to Kaiming project."), this);
    subTitleLabel->setStyleSheet("font-size: 12px; color: #666;");

    //表单
    QFormLayout *formLayout = new QFormLayout;
    // Project ID 输入框
    QLabel *idLabel = new QLabel(tr("Project ID") + "<font color=red>*</font>", this);
    m_idEdit = new QLineEdit(this);
    m_idEdit->setPlaceholderText(tr("Such as : top.openkylin.demo"));
    formLayout->addRow(idLabel, m_idEdit);

    // 不删除是为了少改原代码，后面重构时建议去掉
    m_projectPathEdit = new QLineEdit(this);
    m_projectPathEdit->setVisible(false);

    //Debian Source 输入框和浏览按钮
    QLabel *debianSourceLabel = new QLabel(tr("Debian Source") + "<font color=red>*</font>", this);
    m_debianSourceEdit = new QLineEdit(this);
    m_debianSourceEdit->setReadOnly(true);
    m_debianBrowseBtn = new QPushButton(tr("Browse"));
    QHBoxLayout *debianSourceLayout = new QHBoxLayout;
    debianSourceLayout->addWidget(m_debianSourceEdit);
    debianSourceLayout->addWidget(m_debianBrowseBtn);
    formLayout->addRow(debianSourceLabel, debianSourceLayout);

    //主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(subTitleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(formLayout);
    setLayout(mainLayout);

    // 连接浏览按钮点击事件
    connect(m_debianBrowseBtn, &QPushButton::clicked, [=]() {
        // 创建文件夹选择对话框
        QFileDialog *folderDialog = new QFileDialog(this);
        folderDialog->setWindowTitle(tr("Please select debian source path"));
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
                m_debianSourceEdit->setText(selectedFiles.at(0));
                m_projectPathEdit->setText(selectedFiles.at(0));
            }
        }

        folderDialog->deleteLater(); // 释放对话框内存
    });


    //展开
    // 注册必填字段（用于向导的完成按钮状态）
    registerField("projectID*", m_idEdit);
    registerField("projectPath*", m_projectPathEdit);
    registerField("debianSourcePath*", m_debianSourceEdit);
}

/**
 * @brief : 校验appid是否合法
 * @param : [in] appName, appid
 * @param : [out] errMsg, 错误信息
 */
bool ProjectPathPage::isValidAppName(const QString &appName, QString &errMsg)
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

// 展开
bool ProjectPathPage::validatePage()
{
    QString projectID = m_idEdit->text().trimmed();
    QString debianSourcePath = m_debianSourceEdit->text().trimmed();
    QString projectPath = debianSourcePath;

    // 检查输入是否非空
    if (projectID.isEmpty() || projectPath.isEmpty() || debianSourcePath.isEmpty()) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), tr("Project ID and Path cannot be empty."), QObject::tr("OK"));
        return false;
    }
    // ID校验
    QString errMsg;
    if (!isValidAppName(projectID, errMsg)) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), errMsg, QObject::tr("OK"));
        return false;
    }

    //项目路径校验
    {
        // 验证路径是否可写
        QFile testFile(debianSourcePath + "/.tmp_test");
        if (!testFile.open(QIODevice::ReadWrite)) {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), tr("You do not have write permission in this directory!"), QObject::tr("OK"));
            return false;
        }
        testFile.remove();
    }

    return true;
}

