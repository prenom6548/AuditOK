#include "installenvpage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QDebug>

#include <component/keywordhighlighter.h>
#include <component/outputedit.h>
#include <logic/KMRef.h>
#include <logic/KMBuilderSearch.h>
#include "messageboxutils.h"
#include "common/subprocess.h"
#include "common/KMInstalledAppQuery.h"

class InstallEnvPagePrivate : public QObject
{
    Q_OBJECT

public:
    InstallEnvPagePrivate(InstallEnvPage * parent);
    virtual ~InstallEnvPagePrivate();

    void setupUi();

private:
    Q_DECLARE_PUBLIC(InstallEnvPage)
    InstallEnvPage * q_ptr;

//    QProgressBar *m_progressBar;
//    int m_progress = 0;
//    QTextEdit *m_logText;

    QLabel *m_subTitleLabel;
    OutputEdit * m_output;
    KeywordHighlighter * m_highlighter = nullptr;
    SubProcess m_subProcess;
    bool m_installationSuccess;
    QString m_baseRef;
    QString m_runtimeRef;
};


InstallEnvPage::InstallEnvPage(QWidget *parent)
    : QWizardPage(parent),
      d_ptr(new InstallEnvPagePrivate(this))
{
    Q_D(InstallEnvPage);
    d->setupUi();
    registerField("installFinished*", this);
    connect(&d->m_subProcess, &SubProcess::output, [=](QString msg) {
        if (msg.length() >2 && msg.at(0) == QChar('\r') && msg.at(1) != QChar('\n'))
        {
            // TODO：碰到回车符，认为是回到行首
            // return ;
        }

        msg.replace("[1;31m", "");
        msg.replace("[1;33m", "");
        msg.replace("[0m", "");
        d->m_output->appendText(msg);
        if (d->m_highlighter == nullptr) {
            d->m_highlighter = new KeywordHighlighter(d->m_output->document());
        }
    });

    connect(&d->m_subProcess, &SubProcess::finished, [=](bool result) {
        setField("installFinished", true);  // 设置字段值
        emit completeChanged();
        d->m_installationSuccess = result;
        if (result) {
            d->m_subTitleLabel->setText(tr("Install Success"));
            d->m_subTitleLabel->setStyleSheet("font-size: 12px; color:green;");
            MessageBoxUtils::QMESSAGE_BOX_INFORMATION(this, QObject::tr("Install Complete"),
                                                  tr("Install Complete!"),
                                                  QObject::tr("OK"));
        } else {
            QString base = field("base").toString();
            QString runtime = field("runtime").toString();
            QString errorMsg;
            // 检查基础环境
            if (!wizard()->field("isBaseInstalled").toBool() && !KMBuilderSearch::isRefFromLocal("base", base)) {
                errorMsg += tr("The basic environment was not installed successfully."
                              " Please use the command to manually install it: sudo kaiming install %1\n\n")
                              .arg(d->m_baseRef);
            }

            // 检查运行时环境
            if (!wizard()->field("isRuntimeInstalled").toBool() && !runtime.isEmpty() &&
                !KMBuilderSearch::isRefFromLocal("runtime", runtime)) {
                errorMsg += tr("The runtime environment was not installed successfully."
                              " Please use the command to manually install it: sudo kaiming install %1\n\n")
                              .arg(d->m_runtimeRef);
            }

            if (!errorMsg.isEmpty()) {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"),
                                                    errorMsg.trimmed(),
                                                    QObject::tr("OK"));
            }
        }
    });
}

InstallEnvPage::~InstallEnvPage()
{
    Q_D(InstallEnvPage);
    d->m_subProcess.stop();
}

void InstallEnvPage::initializePage()
{
    bool needBaseInstall = !wizard()->field("isBaseInstalled").toBool();
    bool needRuntimeInstall = !wizard()->field("isRuntimeInstalled").toBool();
    QString baseStr = field("base").toString();
    QString runtimeStr = field("runtime").toString();
    qDebug() << "base=" << baseStr << ",runtime=" << runtimeStr;

    startInstall(needBaseInstall ? baseStr : "", needRuntimeInstall ? runtimeStr : "");

}

void InstallEnvPage::startInstall(QString baseStr, QString runtimeStr)
{
    Q_D(InstallEnvPage);
    d->m_output->clear();
    QStringList args;
    args << "/opt/kaiming-tools/bin/kaiming";
    args << "install";
    args << "-y";
    args << "--compatibility";
    args << "--skip-signature-verification";
    if(!baseStr.isEmpty())
    {
        KMRef ref1;
        ref1.parseFromInfo(baseStr);
        d->m_baseRef = ref1.m_id + "/" + ref1.m_version + "/" + "devel" + "/" + ref1.m_channel;
        qDebug()<< "base:" << d->m_baseRef;
        args << d->m_baseRef;
    }
    if(!runtimeStr.isEmpty())
    {
        KMRef ref2;
        ref2.parseFromInfo(runtimeStr);
        d->m_runtimeRef = ref2.m_id + "/" + ref2.m_version + "/" + "devel" + "/" + ref2.m_channel;
        qDebug() << "runtime：" << d->m_runtimeRef;
        args << d->m_runtimeRef;
    }

    d->m_subProcess.start("pkexec", args);
}

bool InstallEnvPage::isComplete() const
{
    return field("installFinished").toBool();
}

bool InstallEnvPage::validatePage()
{
    Q_D(InstallEnvPage);
    QString base = field("base").toString();
    QString runtime = field("runtime").toString();

    KMInstalledAppQuery query;
    std::optional<Ref> baseRef = KMInstalledAppQuery::parsePref(base);
    QVector<Ref> bases = query.kind("base")
                              .id(baseRef->id)
                              .module("devel")
                              .version(baseRef->version)
                              .query(KMInstalledAppQuery::User);
    bool baseInstalled = false;
    if (!bases.empty())
    {
        baseInstalled = true;
    }

    KMRef ref1;
    ref1.parseFromInfo(base);
    d->m_baseRef = ref1.m_id + "/" + ref1.m_version + "/" + "devel" + "/" + ref1.m_channel;

    // 检查基础环境
    if (!baseInstalled) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"),
                                              tr("The basic environment was not installed successfully."
                                                 " Please use the command to manually install it: sudo kaiming install %1")
                                              .arg(d->m_baseRef),
                                              QObject::tr("OK"));
        return false;
    }

    if (runtime.isEmpty())
    {
        return true;
    }

    std::optional<Ref> runtimeRef = KMInstalledAppQuery::parsePref(runtime);
    QVector<Ref> runtimes = query.kind("runtime")
                                 .id(runtimeRef->id)
                                 .module("devel")
                                 .version(runtimeRef->version)
                                 .query(KMInstalledAppQuery::User);
    bool runtimeInstalled = false;
    if (!runtimes.empty())
    {
        runtimeInstalled = true;
    }

    // 检查运行时环境
    if (!runtimeInstalled) {
        KMRef ref2;
        ref2.parseFromInfo(runtime);
        d->m_runtimeRef = ref2.m_id + "/" + ref2.m_version + "/" + "devel" + "/" + ref2.m_channel;
        MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"),
                                              tr("The runtime environment was not installed successfully."
                                                 " Please use the command to manually install it: sudo kaiming install %1")
                                              .arg(d->m_runtimeRef),
                                              QObject::tr("OK"));
        return false;
    }
    return true;
}

InstallEnvPagePrivate::InstallEnvPagePrivate(InstallEnvPage * parent)
    : QObject(parent),
      q_ptr(parent),
      m_subProcess(this)
{}

InstallEnvPagePrivate::~InstallEnvPagePrivate()
{}

void InstallEnvPagePrivate::setupUi()
{
    Q_Q(InstallEnvPage);
    QLabel *globalHint = new QLabel(
        tr("💡 If the selected base\runtime environment is not installed, it will be automatically installed.")
    );
    globalHint->setStyleSheet(
        "background-color: #e3f2fd;"      // 浅蓝色背景
        "padding: 12px 15px;"            // 内边距
        "border-radius: 6px;"             // 圆角
        "border: 2px solid #90caf9;"     // 蓝色边框
        "color: #0d47a1;"
        "font-size: 14px;"
        "margin-bottom: 14px;"
        "box-shadow: 0 2px 4px rgba(0,0,0,0.1);" // 添加轻微阴影
    );
    globalHint->setAlignment(Qt::AlignCenter);

    // 创建自定义标题控件
    QLabel *titleLabel = new QLabel(tr("Environment Installation"));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    m_subTitleLabel = new QLabel(tr("Installing required dependencies..."));
    m_subTitleLabel->setStyleSheet("font-size: 12px; color: #666;");

    // 日志显示区域
    m_output = new OutputEdit;
    {
        m_output->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_output->setReadOnly(true);
    }

    // 布局管理
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(globalHint);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_subTitleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(new QLabel(tr("Installation Log:")));
    mainLayout->addWidget(m_output);
    q->setLayout(mainLayout);
}

#include "installenvpage.moc"
#include "moc_installenvpage.cpp"

