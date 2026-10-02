
#include "outputwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

#include "common/subprocess.h"
#include "component/keywordhighlighter.h"
#include "component/outputedit.h"
#include <logic/BuildSettingManager.h>

class OutputWidgetPrivate : public QObject
{
    Q_OBJECT

public:
    OutputWidgetPrivate(OutputWidget * parent);
    virtual ~OutputWidgetPrivate();

    void setupUi();

private:
    Q_DECLARE_PUBLIC(OutputWidget)
    OutputWidget * q_ptr;

    QVBoxLayout * m_outputLayout;

    // 功能区
    QPushButton * m_btnBack;
    QPushButton * m_btnBackHome;
    QPushButton * m_btnStop;

    QPushButton * m_btnOpenFile;
    OutputEdit * m_output;

    KeywordHighlighter * m_highlighter = nullptr;

    SubProcess m_subProcess;
};

OutputWidget::OutputWidget(QWidget *parent)
    : QWidget(parent),
      d_ptr(new OutputWidgetPrivate(this))
{
    Q_D(OutputWidget);
    d->setupUi();

    connect(d->m_btnStop, &QPushButton::clicked, [=]() {
        d->m_subProcess.stop();
        d->m_btnStop->setEnabled(false);
        d->m_btnBack->setEnabled(true);
        d->m_btnBackHome->setEnabled(true);
    });

    connect(d->m_btnOpenFile, &QPushButton::clicked, [=](){
        // 获取当前工作目录
        QString workDir = d->m_subProcess.workingDirectory();

        // 按优先级检查文件管理器
        const QStringList fileManagers = {"/usr/bin/peony", "/usr/bin/nautilus"};
        QString foundManager;

        for(const auto &manager : fileManagers) {
            if(QFile::exists(manager)) {
                foundManager = manager;
                break;
            }
        }

        if(!foundManager.isEmpty()) {
            QProcess::startDetached(foundManager, {workDir});
        } else {
            QMessageBox::critical(this,
                                tr("Error"),
                                tr("No supported file manager found (peony/nautilus)"),
                                QMessageBox::Ok);
        }

    });

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
        d->m_btnStop->setEnabled(false);
        d->m_btnBack->setEnabled(true);
        d->m_btnBackHome->setEnabled(true);

        // 添加构建完成提示框
        if(result) {
            QMessageBox::information(this,
                                   tr("Build Complete"),
                                   tr("The build process has completed successfully!"),
                                   QMessageBox::Ok);
        } else {
            QMessageBox::critical(this,
                                tr("Build Failed"),
                                tr("The build process has failed!"),
                                QMessageBox::Ok);
        }
    });
}

OutputWidget::~OutputWidget()
{
}

void OutputWidget::startBuild(QString yamlPath)
{
    Q_D(OutputWidget);

    d->m_btnBack->setEnabled(false);
    d->m_btnStop->setEnabled(true);
    d->m_btnBackHome->setEnabled(false);
    d->m_output->setPlainText("");

    QFileInfo fileInfo(yamlPath);
    QString workDir = fileInfo.path(); // 获取文件夹路径
    QString yaml = fileInfo.fileName(); // 获取文件名
    d->m_subProcess.setWorkingDirectory(workDir);

    QStringList args;
    args << "build";
    BuildSettingManager manager;
    QStringList cmd = manager.getCommandString();
    if(!cmd.isEmpty())
    {
        args += cmd;
    }
    args << yaml;
    d->m_subProcess.start("kaiming-builder", args);
}

OutputWidgetPrivate::OutputWidgetPrivate(OutputWidget * parent)
    : QObject(parent),
      q_ptr(parent),
      m_subProcess(this)
{}

OutputWidgetPrivate::~OutputWidgetPrivate()
{}

void OutputWidgetPrivate::setupUi()
{
    Q_Q(OutputWidget);

    m_outputLayout = new QVBoxLayout;
    q->setLayout(m_outputLayout);
    m_outputLayout->setContentsMargins(24, 0, 24, 24);

    // 日志显示区域
    m_output = new OutputEdit;
    {
        m_output->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_output->setReadOnly(true);
    }

    // 操作按钮组
    QHBoxLayout * operationLayout = new QHBoxLayout;
    {
        m_btnBack = new QPushButton;
        m_btnBack->setText(tr("Back"));
        connect(m_btnBack, &QPushButton::clicked, q, &OutputWidget::back);

        m_btnStop = new QPushButton;
        m_btnStop->setText(tr("Stop"));
        m_btnOpenFile = new QPushButton(tr("Open project directory"));

        m_btnBackHome = new QPushButton(tr("Home Page"));
        connect(m_btnBackHome, &QPushButton::clicked, q, &OutputWidget::backHomeOutputClicked);

        operationLayout->addStretch();
        operationLayout->addWidget(m_btnBack);
        operationLayout->addWidget(m_btnStop);
        operationLayout->addWidget(m_btnOpenFile);
        operationLayout->addWidget(m_btnBackHome);
//        operationLayout->addStretch();
        operationLayout->setSpacing(8);
    }

    m_outputLayout->addWidget(m_output);
    m_outputLayout->addLayout(operationLayout);
}

#include "outputwidget.moc"
#include "moc_outputwidget.cpp"

