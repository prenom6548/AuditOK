#include "appversionerrorwarningwidget.h"
#include "util.h"

#include <QProcess>
#include <QLabel>
AppVersionErrorWarningWidget::AppVersionErrorWarningWidget(QWidget *parent)
    : QWidget(parent)
{
    //初始化布局
    initLayout();
}

void AppVersionErrorWarningWidget::initLayout()
{

    //图标
    m_iconLabel = new QLabel(this);
    m_iconLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_iconLabel");
    m_iconLabel-> setPixmap(QIcon::fromTheme("dialog-warning").pixmap(24,24));
    m_iconLabel->setFixedSize(36, 36);
    m_iconLabel->setStyleSheet("background: transparent;");
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setContentsMargins(0, 0, 0, 0);

    //提醒
    m_tipsLabel = new QLabel;
    m_tipsLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_descripLabel");
    m_tipsLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_tipsLabel->setWordWrap(true);
    m_tipsLabel->setFixedHeight(40);
    m_tipsLabel->setMinimumWidth(456);
    m_tipsLabel->setMaximumWidth(16777215);
    m_tipsLabel->setContentsMargins( 0, 0, 0, 0);
    m_tipsLabel->setText(tr("which may lead limited functionality. Please upgrade to the latest version to ensure proper usage."));
    m_tipsLabel->setStyleSheet("color: rgba(255, 125, 0, 1); background: transparent;padding-top:0px;padding-left:2px;padding-right:2px;");


    //按钮
    m_configButton = new QPushButton; //
    m_configButton->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KBorderlessButton_m_configButton");
    m_configButton->setText(tr("Go to update"));
    m_configButton->setFixedSize(QSize(88,36));
    m_configButton->setFocusPolicy(Qt::NoFocus);
    m_configButton->setStyleSheet("QPushButton {"
                                  "background-color:#FF7D00;;"
                                  "color: white;"
                                  "font-size:14px;"
                                  "border-radius: 6px;"
                                  "}"
                                  "QPushButton:hover {"
                                  "background-color:#F27700;;"
                                  "}"
                                  "QPushButton:pressed {"
                                  "background-color:#CC6400;;"
                                  "}");
    connect(m_configButton,&kdk::KBorderlessButton::clicked,this,&AppVersionErrorWarningWidget::loadSoftwareStore);

    m_mainHLayout = new QHBoxLayout();
    m_mainHLayout->setContentsMargins(8, 0, 16, 0);
    m_mainHLayout->setSpacing(0);
    m_mainHLayout->addWidget(m_iconLabel);
    m_mainHLayout->addSpacing(8);
    m_mainHLayout->addWidget(m_tipsLabel);
    m_mainHLayout->addSpacing(30);
//    m_mainHLayout->addStretch();
    m_mainHLayout->addWidget(m_configButton);
    this->setLayout(m_mainHLayout);
}

void AppVersionErrorWarningWidget::loadSoftwareStore()
{
    // 打开软件商店：V11 走关键字搜索助手；其它系统保持原菜单入口
    const QString program = QStringLiteral("/usr/bin/kylin-software-center");
    if (curSystemVersion() == SystemVersion::Version_V11) {
        QProcess::startDetached(program, QStringList{QStringLiteral("--SearchKeyword=kylin-aiassistant")});
    } else {
        QProcess::startDetached(program, QStringList{QStringLiteral("-menuid=2"), QStringLiteral("-tagid=709")});
    }
}

void AppVersionErrorWarningWidget::updateTips(QString tips)
{
    if(m_tipsLabel)
    {
        m_tipsLabel->setText(tips);
    }
}





