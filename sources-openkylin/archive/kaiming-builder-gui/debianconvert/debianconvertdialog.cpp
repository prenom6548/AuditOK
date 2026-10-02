#include "debianconvertdialog.h"
#include "projectpathpage.h"
#include "selectenvpage.h"
#include "installenvpage.h"
#include "generateprojectpage.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QIcon>

DebianConvertDialog::DebianConvertDialog(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Debian Source Conversion"));
    setWizardStyle(QWizard::ModernStyle);
    resize(800,450);

    // 禁用默认的标题区域
    setOption(QWizard::NoDefaultButton, false);
    setOption(QWizard::NoBackButtonOnStartPage, true);

    // 创建左侧步骤指示器
    QWidget *sideWidget = new QWidget;
    QVBoxLayout *sideLayout = new QVBoxLayout(sideWidget);

    QStringList steps = {
        tr("Project Location"),
        tr("Select Env"),
        tr("Install Env"),
        tr("Generate Project")
    };

    // 创建步骤
    for (int i = 0; i < steps.size(); ++i) {
        QWidget *stepContainer = new QWidget;
        stepContainer->setObjectName("stepWidget");

        QHBoxLayout *stepLayout = new QHBoxLayout(stepContainer);
        stepLayout->setContentsMargins(2, 2, 2, 2);

        // 图标标签
        QLabel *iconLabel = new QLabel;
        iconLabel->setObjectName("iconLabel");
        iconLabel->setFixedSize(16, 16);
        stepLayout->addWidget(iconLabel);

        // 文本标签
        QLabel *textLabel = new QLabel(steps[i]);
        textLabel->setObjectName("textLabel");
        stepLayout->addWidget(textLabel);

        sideLayout->addWidget(stepContainer);
    }

    sideLayout->addStretch();
    setSideWidget(sideWidget);

    // 添加页面
    setPage(Page_ProjectPath, new ProjectPathPage);
    setPage(Page_SelectEnv, new SelectEnvPage);
    setPage(Page_InstallEnv, new InstallEnvPage);
    setPage(Page_GenerateProject, new GenerateProjectPage);

    // 获取按钮并设置大小（使用QAbstractButton*）
    button(QWizard::BackButton)->setMinimumSize(120, 30);
    button(QWizard::NextButton)->setMinimumSize(120, 30);
    button(QWizard::FinishButton)->setMinimumSize(120, 30);
    button(QWizard::CancelButton)->setMinimumSize(80, 30);

    // 页面切换信号连接
    connect(this, &QWizard::currentIdChanged, this, &DebianConvertDialog::updateSidebarHighlight);
    // 获取生成页面指针
    if (auto genPage = qobject_cast<GenerateProjectPage*>(page(Page_GenerateProject))) {
        // 直接转发页面信号
        connect(genPage, &GenerateProjectPage::fileSelected,
                this, &DebianConvertDialog::fileSelected);
    }

}

void DebianConvertDialog::updateSidebarHighlight(int currentPageId)
{
    QWidget *side = sideWidget();
    if (!side) return;

    QList<QWidget*> stepWidgets = side->findChildren<QWidget*>("stepWidget");

    for (int i = 0; i < stepWidgets.size(); ++i) {
        QWidget *container = stepWidgets[i];
        QLabel *iconLabel = container->findChild<QLabel*>("iconLabel");
        QLabel *textLabel = container->findChild<QLabel*>("textLabel");

        if (!iconLabel || !textLabel) continue;

        bool isCurrent = (i == currentPageId);
        // 设置图标
        if (isCurrent) {
            iconLabel->setPixmap(QIcon::fromTheme("go-next",
                                QIcon(":/images/arrow-right.png")).pixmap(16, 16));
            textLabel->setStyleSheet("color: #2a82da; font-weight: bold;");
        } else {
            iconLabel->clear();
            textLabel->setStyleSheet("color: black; font-weight: normal;");
        }
    }
}
