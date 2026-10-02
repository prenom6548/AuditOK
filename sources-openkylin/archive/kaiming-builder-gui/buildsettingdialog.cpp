#include <buildsettingdialog.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>

class BuildSettingDialogPrivate : public QObject
{
     Q_OBJECT
public:
    BuildSettingDialogPrivate(BuildSettingDialog *parent, BuildSettingManager *buildSettingManager);
    virtual ~BuildSettingDialogPrivate();

    void initUI();
    void loadSettings();
    void saveSettings();
    QString generateCommandString();

private:
    Q_DECLARE_PUBLIC(BuildSettingDialog)
    BuildSettingDialog * q_ptr;

    BuildSettingManager *m_buildSettingManager;
    QCheckBox *m_buildOnlyCheck;
    QCheckBox *m_finishOnlyCheck;
    QCheckBox *m_exportOnlyCheck;
    QCheckBox *m_forceCleanCheck;
    QCheckBox *m_keepSourceCheck;

    QPushButton * m_btnConfirm;
    QPushButton * m_btnCancel;

    QStringList m_commands;

    void updateExclusiveOptions();

};

BuildSettingDialog::BuildSettingDialog(QWidget *parent, BuildSettingManager *buildSettingManager)
    : QDialog(parent),
      d_ptr(new BuildSettingDialogPrivate(this, buildSettingManager))
{
    Q_D(BuildSettingDialog);
    setFixedSize(550, 500);
    d->initUI();
    connect(d->m_btnConfirm, &QPushButton::clicked, this, [=]() {
        d->saveSettings();
        accept();
    });

    connect(d->m_btnCancel, &QPushButton::clicked, this, &BuildSettingDialog::reject);

}
BuildSettingDialog::~BuildSettingDialog()
{}

QString BuildSettingDialog::getCommandString()
{
    Q_D(BuildSettingDialog);
    return d->generateCommandString();
}

BuildSettingDialogPrivate::BuildSettingDialogPrivate(BuildSettingDialog * parent,BuildSettingManager *buildSettingManager)
    : QObject(parent),
      q_ptr(parent),
      m_buildSettingManager(buildSettingManager)
{}

BuildSettingDialogPrivate::~BuildSettingDialogPrivate()
{}

void BuildSettingDialogPrivate::initUI()
{
    Q_Q(BuildSettingDialog);

    QGroupBox *settingBox = new QGroupBox(tr("Build Setting"));
    QVBoxLayout *settingLayout = new QVBoxLayout();
    // 创建复选框
    m_buildOnlyCheck = new QCheckBox(tr("Only execute the build and clean phases"));
    m_finishOnlyCheck = new QCheckBox(tr("Only execute the finish and export packaging stages"));
    m_exportOnlyCheck = new QCheckBox(tr("Only execute the export packaging phase"));
    m_forceCleanCheck = new QCheckBox(tr("Clear the last build cache first, and then rebuild"));
    m_keepSourceCheck = new QCheckBox(tr("Keep the source code for debugging after the construction is completed"));

    QLabel *buildCmdLabel = new QLabel(tr("Build command:"));
    QLabel *cmdDisplayLabel = new QLabel();
    cmdDisplayLabel->setWordWrap(true);
    cmdDisplayLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #f8f9fa;"  // 浅灰色背景
        "   border: 1px solid #dee2e6;"  // 浅边框
        "   border-radius: 4px;"         // 圆角
        "   padding: 8px 12px;"         // 内边距
        "   color: #495057;"            // 深灰色文字
        "   font-family: 'Consolas', 'Courier New', monospace;" // 等宽字体
        "   font-size: 13px;"
        "   min-height: 20px;"
        "   selection-background-color: #e9ecef;" // 选中时的背景色
        "}"
        "QLabel:hover {"
        "   border-color: #adb5bd;"      // 悬停时边框变深
        "   background-color: #e9ecef;"  // 悬停时背景变亮
        "}"
    );

    settingLayout->addWidget(m_buildOnlyCheck);
    settingLayout->addWidget(m_finishOnlyCheck);
    settingLayout->addWidget(m_exportOnlyCheck);
    settingLayout->addWidget(m_forceCleanCheck);
    settingLayout->addWidget(m_keepSourceCheck);
    settingLayout->addWidget(buildCmdLabel);
    settingLayout->addWidget(cmdDisplayLabel); // 添加到布局
    settingBox->setLayout(settingLayout);
    settingBox->setLayout(settingLayout);

    // 创建按钮
    m_btnConfirm = new QPushButton(tr("Save"));
    m_btnCancel = new QPushButton(tr("Cancel"));

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_btnConfirm);
    buttonLayout->addWidget(m_btnCancel);

    mainLayout->addWidget(settingBox);
    mainLayout->addLayout(buttonLayout);
    q->setLayout(mainLayout);

    auto updateCmdDisplay = [=]() {
        QString cmd = generateCommandString();
        cmdDisplayLabel->setText("kaiming-builder build " + cmd + " xxx.km.yaml"); // 处理空命令的情况
    };

    // 添加互斥逻辑连接
    connect(m_buildOnlyCheck, &QCheckBox::stateChanged, this, &BuildSettingDialogPrivate::updateExclusiveOptions);
    connect(m_finishOnlyCheck, &QCheckBox::stateChanged, this, &BuildSettingDialogPrivate::updateExclusiveOptions);
    connect(m_exportOnlyCheck, &QCheckBox::stateChanged, this, &BuildSettingDialogPrivate::updateExclusiveOptions);
    connect(m_forceCleanCheck, &QCheckBox::stateChanged, this, &BuildSettingDialogPrivate::updateExclusiveOptions);


    connect(m_buildOnlyCheck, &QCheckBox::stateChanged, updateCmdDisplay);
    connect(m_finishOnlyCheck, &QCheckBox::stateChanged, updateCmdDisplay);
    connect(m_exportOnlyCheck, &QCheckBox::stateChanged, updateCmdDisplay);
    connect(m_forceCleanCheck, &QCheckBox::stateChanged, updateCmdDisplay);
    connect(m_keepSourceCheck, &QCheckBox::stateChanged, updateCmdDisplay);

    loadSettings();
    updateCmdDisplay();

}

// 新增的互斥逻辑处理函数
void BuildSettingDialogPrivate::updateExclusiveOptions()
{
    QCheckBox *senderCheck = qobject_cast<QCheckBox*>(sender());
    if (!senderCheck || !senderCheck->isChecked()) return;

    if (senderCheck == m_buildOnlyCheck) {
        m_finishOnlyCheck->setChecked(false);
        m_exportOnlyCheck->setChecked(false);
        m_forceCleanCheck->setChecked(false);
    }
    else if (senderCheck == m_finishOnlyCheck) {
        m_buildOnlyCheck->setChecked(false);
        m_exportOnlyCheck->setChecked(false);
        m_forceCleanCheck->setChecked(false);
    }
    else if (senderCheck == m_exportOnlyCheck) {
        m_buildOnlyCheck->setChecked(false);
        m_finishOnlyCheck->setChecked(false);
        m_forceCleanCheck->setChecked(false);
    }
    else if (senderCheck == m_forceCleanCheck) {
        m_buildOnlyCheck->setChecked(false);
        m_finishOnlyCheck->setChecked(false);
        m_exportOnlyCheck->setChecked(false);
    }
}

void BuildSettingDialogPrivate::loadSettings()
{

    m_buildOnlyCheck->setChecked(m_buildSettingManager->m_buildOnly);
    m_finishOnlyCheck->setChecked(m_buildSettingManager->m_finishOnly);
    m_exportOnlyCheck->setChecked(m_buildSettingManager->m_exportOnly);
    m_forceCleanCheck->setChecked(m_buildSettingManager->m_forceClean);
    m_keepSourceCheck->setChecked(m_buildSettingManager->m_keepSource);
}

void BuildSettingDialogPrivate::saveSettings()
{
    m_buildSettingManager->m_buildOnly = m_buildOnlyCheck->isChecked();
    m_buildSettingManager->m_finishOnly = m_finishOnlyCheck->isChecked();
    m_buildSettingManager->m_exportOnly = m_exportOnlyCheck->isChecked();
    m_buildSettingManager->m_forceClean = m_forceCleanCheck->isChecked();
    m_buildSettingManager->m_keepSource = m_keepSourceCheck->isChecked();
    m_buildSettingManager->saveConfig();
}

QString BuildSettingDialogPrivate::generateCommandString()
{
    m_commands.clear();  // 清空之前的命令

    if (m_buildOnlyCheck->isChecked()) m_commands << "--build-only";
    if (m_finishOnlyCheck->isChecked()) m_commands << "--finish-only";
    if (m_exportOnlyCheck->isChecked()) m_commands << "--export-only";
    if (m_forceCleanCheck->isChecked()) m_commands << "-f";
    if (m_keepSourceCheck->isChecked()) m_commands << "-k";

    return m_commands.join(" ");
}

#include "buildsettingdialog.moc"
#include "moc_buildsettingdialog.cpp"
