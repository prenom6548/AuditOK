#include "generateprojectpage.h"
#include "messageboxutils.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QTimer>
#include <QScrollBar>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>

GenerateProjectPage::GenerateProjectPage(QWidget *parent)
    : QWizardPage(parent)
{
    QLabel *titleLabel = new QLabel(tr("Kaiming Project Generation"), this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    QLabel *subTitleLabel = new QLabel(tr("Generating kaiming project..."), this);
    subTitleLabel->setStyleSheet("font-size: 12px; color: #666;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(subTitleLabel);
    mainLayout->addSpacing(10);

    // 日志显示区域
    m_output = new OutputEdit;
    {
        m_output->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_output->setReadOnly(true);
    }
    mainLayout->addWidget(new QLabel(tr("Conversion Log:")));
    mainLayout->addWidget(m_output);
    setLayout(mainLayout);

    registerField("convertFinished*", this);
    connect(&m_subProcess, &SubProcess::output, [=](QString msg) {
        if (msg.length() >2 && msg.at(0) == QChar('\r') && msg.at(1) != QChar('\n'))
        {
            // TODO：碰到回车符，认为是回到行首
            // return ;
        }

        msg.replace("[1;31m", "");
        msg.replace("[1;33m", "");
        msg.replace("[0m", "");
        m_output->appendText(msg);
        if (m_highlighter == nullptr) {
            m_highlighter = new KeywordHighlighter(m_output->document());
        }
    });

    connect(&m_subProcess, &SubProcess::finished, [=]() {
        setField("convertFinished", true);  // 设置字段值
        emit completeChanged();
    });
}

void GenerateProjectPage::initializePage()
{
    wizard()->setButtonText(QWizard::FinishButton, tr("next"));
    m_output->clear();
    // 获取向导中先前页面设置的值
    m_projectPath = field("projectPath").toString();
    m_projectID = field("projectID").toString();
    m_debianSourcePath = field("debianSourcePath").toString();
    m_base = field("base").toString();
    m_runtime = field("runtime").toString();
    qDebug() << "工作目录："  << m_projectPath
             << "项目ID"  <<  m_projectID
             << "debian 源码路径"  << m_debianSourcePath;
    if(!m_projectPath.isEmpty() && !m_projectID.isEmpty() && !m_debianSourcePath.isEmpty())
    {
        startConvert();
    }
}

void GenerateProjectPage::startConvert()
{

    m_subProcess.setWorkingDirectory(m_projectPath);
    QStringList args;
    args << "create-from-debsource";
    args << "--base=" + m_base;
    args << "--runtime=" + m_runtime;
    args << "--gui" ;
    args << m_projectID;
    args << m_debianSourcePath;
    m_subProcess.start("kaiming-builder", args);
}

bool GenerateProjectPage::isComplete() const
{
    return field("convertFinished").toBool();
}

bool GenerateProjectPage::validatePage()
{
    // 检查目录中是否存在.km.yaml文件
    QString yamlPath = m_projectPath + "/kaiming";
    QDir workDir(yamlPath);
    QStringList yamlFiles = workDir.entryList(QStringList() << "*.km.yaml",
                                           QDir::Files | QDir::NoDotAndDotDot);

    if (yamlFiles.isEmpty()) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), tr("Conversion process failed."), QObject::tr("OK"));
        return false;
    }

    // 构建完整文件路径并发射信号
    QString yamlFilePath = workDir.absoluteFilePath(yamlFiles.first());
    emit fileSelected(yamlFilePath);

    return true;
}
