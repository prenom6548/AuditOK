#include "homepage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QPalette>
#include "highlight-effect.h"

const int HOMEPAGEICON_W = 400;
const int HOMEPAGEICON_H = 400;
const int ICON_W = 24;
const int ICON_H = 24;

HomePage::HomePage(QWidget *parent) : QWidget(parent)
{
    m_homePageLab = new QLabel(this);
    m_titleLab = new QLabel(this);
    m_describeLab = new QLabel(this);
    m_deviceCodeLab = new QLabel(this);
    m_codeLab = new QLabel(this);

    m_folderIconLab = new QLabel(this);
    m_shareIconLab = new QLabel(this);
    m_crossIconLab = new QLabel(this);
    m_folderLab = new QLabel(this);
    m_shareLab = new QLabel(this);
    m_crossLab = new QLabel(this);

    m_connectBtn = new QPushButton(this);
    connect(m_connectBtn, &QPushButton::clicked, this, &HomePage::sigConnectBtnClicked);

    m_homePageLab->setPixmap(QPixmap(":/homepage/homepage.png").scaled(400, 400));

    QFont font;
    font.setPointSizeF(24);
    font.setBold(true);
    m_titleLab->setFont(font);

    QPalette palette = m_titleLab->palette();
    palette.setColor(QPalette::Text, QColor("#3690F8"));
    m_titleLab->setPalette(palette);
    m_titleLab->setText(tr("Multiterminal"));  // Multiterminal

    QFont fonts;
    fonts.setBold(true);
    m_describeLab->setAlignment(Qt::AlignTop);
    m_describeLab->setText(
        tr("Cross equipment and cross system collaboration. "
           "It is more convenient to share resources and screens, "
           "and more convenient and efficient to work!"));  // Cross system interaction between devices
    m_describeLab->setFixedWidth(342);
    m_describeLab->setWordWrap(true);
    m_describeLab->setFont(fonts);

    m_deviceCodeLab->setText(tr("Device Code:"));

    QPalette palettes = m_codeLab->palette();
    palettes.setColor(QPalette::Text, QColor("#3690F8"));
    m_codeLab->setPalette(palettes);
    QFont font1;
    font1.setBold(true);
    m_codeLab->setFont(font1);
    m_codeLab->setText("");

    m_folderIconLab->setPixmap(QPixmap(":/homepage/folder.svg").scaled(ICON_W, ICON_H));
    m_folderLab->setText(tr("Folder"));  //文件浏览
    m_folderLab->setAlignment(Qt::AlignCenter);

    m_shareIconLab->setPixmap(QPixmap(":/homepage/screen-share.svg").scaled(ICON_W, ICON_H));
    m_shareLab->setText(tr("Share"));  //共享屏幕
    m_shareLab->setAlignment(Qt::AlignCenter);

    m_crossIconLab->setPixmap(QPixmap(":/homepage/remote.svg").scaled(ICON_W, ICON_H));
    m_crossLab->setText(tr("Cross"));  //跨屏操控
    m_crossLab->setAlignment(Qt::AlignCenter);

    m_connectBtn->setText(tr("Connect Now"));  // Connect
    m_connectBtn->setFixedSize(256, 56);
    m_connectBtn->setProperty("isImportant", true);
    m_connectBtn->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *titleHLayout = new QHBoxLayout;
    titleHLayout->setMargin(0);
    titleHLayout->setSpacing(0);
    titleHLayout->addWidget(m_titleLab);
    titleHLayout->addStretch();

    QHBoxLayout *describeHLayout = new QHBoxLayout;
    describeHLayout->setMargin(0);
    describeHLayout->setSpacing(0);
    describeHLayout->addWidget(m_describeLab);
    describeHLayout->addStretch();

    QHBoxLayout *deviceCodeHLayout = new QHBoxLayout;
    deviceCodeHLayout->setMargin(0);
    deviceCodeHLayout->setSpacing(0);
    deviceCodeHLayout->addWidget(m_deviceCodeLab);
    deviceCodeHLayout->addSpacing(8);
    deviceCodeHLayout->addWidget(m_codeLab);
    deviceCodeHLayout->addStretch();

    QHBoxLayout *folderLayout = new QHBoxLayout;
    folderLayout->setMargin(0);
    folderLayout->setSpacing(0);
    folderLayout->addWidget(m_folderIconLab);
    folderLayout->addSpacing(8);
    folderLayout->addWidget(m_folderLab);
    folderLayout->addStretch();

    QHBoxLayout *shareLayout = new QHBoxLayout;
    shareLayout->setMargin(0);
    shareLayout->setSpacing(0);
    shareLayout->addWidget(m_shareIconLab);
    shareLayout->addSpacing(8);
    shareLayout->addWidget(m_shareLab);
    shareLayout->addStretch();

    QHBoxLayout *crossLayout = new QHBoxLayout;
    crossLayout->setMargin(0);
    crossLayout->setSpacing(0);
    crossLayout->addWidget(m_crossIconLab);
    crossLayout->addSpacing(8);
    crossLayout->addWidget(m_crossLab);
    crossLayout->addStretch();

    QVBoxLayout *iconLayout = new QVBoxLayout;
    iconLayout->setMargin(0);
    iconLayout->setSpacing(0);
    iconLayout->addLayout(folderLayout);
    iconLayout->addSpacing(11);
    iconLayout->addLayout(shareLayout);
    iconLayout->addSpacing(11);
    iconLayout->addLayout(crossLayout);
    iconLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addStretch();
    vLayout->addLayout(titleHLayout);
    vLayout->addSpacing(24);
    vLayout->addLayout(describeHLayout);
    vLayout->addSpacing(16);
    vLayout->addLayout(deviceCodeHLayout);
    vLayout->addSpacing(30);
    vLayout->addLayout(iconLayout);
    vLayout->addSpacing(24);
    vLayout->addWidget(m_connectBtn);
    vLayout->addStretch();

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->addStretch();
    hLayout->addWidget(m_homePageLab);
    hLayout->addSpacing(77);
    hLayout->addLayout(vLayout);
    hLayout->addStretch();

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    mainVLayout->setMargin(0);
    mainVLayout->setSpacing(0);
    mainVLayout->addStretch();
    mainVLayout->addLayout(hLayout);
    mainVLayout->addStretch();

    this->setLayout(mainVLayout);
}

void HomePage::setTheme(PublicAttributes::Theme theme)
{
    switch (theme) {
        case PublicAttributes::Theme::Light:
            m_homePageLab->setPixmap(QPixmap(":/homepage/homepage.png").scaled(400, 400));
            break;
        case PublicAttributes::Theme::Dark:
            m_homePageLab->setPixmap(QPixmap(":/homepage/homepage-dark.png").scaled(400, 400));
            break;
    }
}

void HomePage::setDeviceCode(QString code)
{
    if (code.isEmpty()) {
        m_codeLab->setText(tr("No network"));
        return;
    }
    m_codeLab->setText(code);
}

void HomePage::changeFontSize(double fontSize)
{
    QFont font;
    font.setPointSizeF(fontSize + 14);
    font.setBold(true);

    m_titleLab->setFont(font);
}
