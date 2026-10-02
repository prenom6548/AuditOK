#include "settingpagetitle.h"
#include <QHBoxLayout>
#include <QApplication>

SettingPageTitle::SettingPageTitle(QWidget *parent)
    : QWidget(parent)
{
    s_pFuncLabel = new QLabel(this);
    s_pFuncLabel->setAccessibleName("kylin-camera_QLabel_s_pFuncLabel");
    sIcon = new QPushButton(this);
    sIcon->setAccessibleName("kylin-camera_QPushButton_sIcon");
    s_pCloseButton = new QPushButton(this);
    s_pCloseButton->setAccessibleName("kylin-camera_QPushButton_s_pCloseButton");

    s_pCloseButton->setProperty("isWindowButton", 0x2);
    s_pCloseButton->setProperty("useIconHighlightEffect",0x8);
    s_pCloseButton->setFlat(true);

    s_pFuncLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    s_pCloseButton->setFixedSize(30,30);

    s_pFuncLabel->setObjectName("whiteLabel");
    s_pCloseButton->setObjectName("closeButton");
    s_pCloseButton->setToolTip(tr("close"));

    QHBoxLayout *sLayout = new QHBoxLayout(this);
    sLayout->addWidget(sIcon);
    sLayout->addSpacing(8);
    sLayout->addWidget(s_pFuncLabel);
    sLayout->addStretch();
    sLayout->addWidget(s_pCloseButton);
    sLayout->setSpacing(0);
    sLayout->setContentsMargins(0,0,0,0);

    s_pFuncLabel->setText(tr("Camera"));
    sIcon->setIcon(QIcon::fromTheme("kylin-camera"));
    sIcon->setFixedSize(30,30);
    sIcon->setIconSize(QSize(24,24));
    sIcon->setStyleSheet("QPushButton{border:0px;background:transparent;}");
    s_pCloseButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    this->setFixedHeight(30);
    this->setLayout(sLayout);

    connect(s_pCloseButton,&QPushButton::clicked,this,&SettingPageTitle::closeButton);

}

void SettingPageTitle::closeButton(){
    this->window()->close();

}
