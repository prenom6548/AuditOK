#include "selectenvpage.h"
#include "basedialog.h"
#include "messageboxutils.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDebug>
#include <QMessageBox>
#include <QFormLayout>


SelectEnvPage::SelectEnvPage(QWidget *parent)
    : QWizardPage(parent)
{
    // 创建自定义标题控件
    QLabel *titleLabel = new QLabel(tr("Environment Selection"), this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    QLabel *subTitleLabel = new QLabel(tr("Please select the build environment for your project."), this);
    subTitleLabel->setStyleSheet("font-size: 12px; color: #666;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(subTitleLabel);
    mainLayout->addSpacing(10);

    {
        // Base 部分
        QHBoxLayout *baseLayout = new QHBoxLayout;
        m_baseInput = new QLineEdit(this);
        m_baseInput->setPlaceholderText(tr("Format: {channel}:{id}/{version}."
                                        "Example:  "
                                        "stable:top.openkylin.base/1.0.0"));
        m_baseInput->setToolTip(tr("The basic environment needed to build the package.\n"
                                   "Format: {channel}:{id}/{version}\n"
                                   "Example:\n"
                                   "stable:top.openkylin.base/1.0.0"));
        baseLayout->addWidget(m_baseInput);
        m_baseSearchButton = new QPushButton("🔍");
        baseLayout->addWidget(m_baseSearchButton);
        connect(m_baseSearchButton, &QPushButton::clicked, this, [this]() {
            openSearchDialog("base","","");
        });

        // Runtime 部分
        QHBoxLayout *runtimeLayout = new QHBoxLayout;
        m_runtimeInput = new QLineEdit(this);
        m_runtimeInput->setPlaceholderText(tr("Format: {channel}:{id}/{version}."
                                           "Example: "
                                           "stable:top.openkylin.ukui/1.0.0"));
        m_runtimeInput->setToolTip(tr("Specific environments that complement the base environment to build package,"
                                      "such as cmake,qmake, etc\n"
                                      "Format: {channel}:{id}/{version}\n"
                                      "Example:\n"
                                      "stable:top.openkylin.ukui/1.0.0"));
        runtimeLayout->addWidget(m_runtimeInput);
        m_runtimeSearchButton = new QPushButton("🔍");
        runtimeLayout->addWidget(m_runtimeSearchButton);
        connect(m_runtimeSearchButton, &QPushButton::clicked, this, [this]() {
            openSearchDialog("runtime","","");
        });

        QFormLayout *formLayout = new QFormLayout;
        formLayout->addRow(tr("base") + "<font color=red>*</font>", baseLayout);
        formLayout->addRow(tr("runtime"), runtimeLayout);
        mainLayout->addLayout(formLayout);
    }
    setLayout(mainLayout);
    registerField("base*", m_baseInput);
    registerField("runtime", m_runtimeInput);
    registerField("isBaseInstalled", this, "isBaseInstalled");
    registerField("isRuntimeInstalled", this, "isRuntimeInstalled");
}

// 初始化页面
void SelectEnvPage::initializePage() {

//    m_baseInput->setText("stable:top.openkylin.base/1.0.0");
//    m_runtimeInput->setText("stable:top.openkylin.ukui/1.0.0");

    emit m_baseInput->textChanged(m_baseInput->text());
    emit m_runtimeInput->textChanged(m_runtimeInput->text());
}

// 验证
bool SelectEnvPage::validatePage()
{
    QString base = m_baseInput->text().trimmed();
    QString runtime = m_runtimeInput->text().trimmed();
    // 检查输入是否非空
    if (base.isEmpty()) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), tr("base cannot be empty."), QObject::tr("OK"));
        return false;
    }

    //  校验格式
    QRegularExpression formatRegex("^[^:]+:[^/]+/[^/]+$"); // channel:id/version

    // 校验 base 格式
    if (!formatRegex.match(base).hasMatch()) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this,
            tr("Invalid Format"),
            tr("Base format must be: {channel}:{id}/{version}\n"
               "Example: stable:top.openkylin.base/1.0.0"),
            tr("OK"));
        m_baseInput->setFocus();
        return false;
    }

    // 校验 runtime 格式
    if (!runtime.isEmpty() && !formatRegex.match(runtime).hasMatch()) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this,
            tr("Invalid Format"),
            tr("Runtime format must be: {channel}:{id}/{version}\n"
               "Example: stable:top.openkylin.ukui/1.0.0"),
            tr("OK"));
        m_runtimeInput->setFocus();
        return false;
    }

    // 获取是否安装(展开)
    m_isBaseInstalled = KMBuilderSearch::isRefFromLocal("base", base);
    if(!runtime.isEmpty())
    {
        m_isRuntimeInstalled = KMBuilderSearch::isRefFromLocal("runtime", runtime);

    }else
    {
        m_isRuntimeInstalled = true;
    }

    emit propertyChanged("isBaseInstalled");
    emit propertyChanged("isRuntimeInstalled");
    return true;
}

int SelectEnvPage::nextId() const
{
   return (m_isBaseInstalled && m_isRuntimeInstalled) ? 3 : 2;
}

void SelectEnvPage::openSearchDialog(const QString &kind, const QString &base, const QString &runtime)
{
    BaseDialog dialog;
    dialog.loadData(kind, base, runtime);
    if (dialog.exec() == QDialog::Accepted) {
        KMRef selectedRef = dialog.getSelectedRef();
        if (kind == "base")
        {
            QString selectedText = KMBuilderStringUtils::buildRef(selectedRef);
            m_baseInput->setText(selectedText);
        } else if (kind == "runtime")
        {
            QString selectedText = KMBuilderStringUtils::buildRef(selectedRef);
            m_runtimeInput->setText(selectedText);
        }
    }
}
