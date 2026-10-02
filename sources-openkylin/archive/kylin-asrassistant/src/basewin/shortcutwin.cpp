#include "shortcutwin.h"
#include <ukuistylehelper/ukuistylehelper.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVariant>
#include <QPalette>
#include <QFont>

const int WIN_SIZE_W = 400;
const int WIN_SIZE_H = 312;
const int ICON_SIZE = 16;
const int BTN_SIZE = 30;
const int LAB_W = 176;
const int LAB_H = 40;

ShortCutWin::ShortCutWin(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setFixedSize(WIN_SIZE_W, WIN_SIZE_H);
    kdk::UkuiStyleHelper::self()->removeHeader(this);

    m_titleIcon = new QLabel(this);
    m_titleIcon->setPixmap(
        QPixmap(QIcon::fromTheme("ukui-kylin-asrassistant-symbolic").pixmap(QSize(BTN_SIZE, BTN_SIZE))));

    m_titleName = new QLabel(this);
    m_titleName->setText(tr("Asr assistant"));
    m_titleName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_exitBtn = new QPushButton(this);
    connect(m_exitBtn, &QPushButton::clicked, this, &ShortCutWin::close);
    m_exitBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_exitBtn->setToolTip(tr("Close"));
    m_exitBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_exitBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_exitBtn->setProperty("isWindowButton", 0x2);
    m_exitBtn->setProperty("useIconHighlightEffect", 0x8);
    m_exitBtn->setFlat(true);

    m_tableName = new QLabel(this);
    QFont font;
    font.setBold(true);
    font.setPointSizeF(16);
    m_tableName->setFont(font);
    m_tableName->setText(tr("ShortCut"));

    m_gaugeOutfit1 = new QLabel(this);
    m_gaugeOutfit1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_gaugeOutfit1->setText(tr("Description"));
    m_gaugeOutfit1->setFixedSize(LAB_W, LAB_H);
    m_gaugeOutfit2 = new QLabel(this);
    m_gaugeOutfit2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_gaugeOutfit2->setText(tr("Keypress"));
    m_gaugeOutfit2->setFixedSize(LAB_W, LAB_H);

    m_itemLab1 = new QLabel(this);
    m_itemLab1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_itemLab1->setText(tr("Open the asr assistant"));
    m_itemLab1->setFixedWidth(LAB_W);
    m_itemLab1->setMinimumHeight(LAB_H);
    m_itemLab1->setWordWrap(true);
    m_itemLab2 = new QLabel(this);
    m_itemLab2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_itemLab2->setText("Win + Q");
    m_itemLab2->setFixedWidth(LAB_W);
    m_itemLab2->setMinimumHeight(LAB_H);
    m_itemLab2->setWordWrap(true);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setSpacing(0);
    titleLayout->setMargin(8);
    titleLayout->addWidget(m_titleIcon);
    titleLayout->addWidget(m_titleName);
    titleLayout->addStretch();
    titleLayout->addWidget(m_exitBtn);

    QHBoxLayout *tabLayout = new QHBoxLayout;
    tabLayout->setSpacing(0);
    tabLayout->setMargin(0);
    tabLayout->addSpacing(24);
    tabLayout->addWidget(m_tableName);
    tabLayout->addStretch();

    QHBoxLayout *gaugeOutfitLayout = new QHBoxLayout;
    gaugeOutfitLayout->setSpacing(0);
    gaugeOutfitLayout->setMargin(0);
    gaugeOutfitLayout->addSpacing(24);
    gaugeOutfitLayout->addWidget(m_gaugeOutfit1);
    gaugeOutfitLayout->addWidget(m_gaugeOutfit2);
    gaugeOutfitLayout->addStretch();

    QHBoxLayout *itemLayout = new QHBoxLayout;
    itemLayout->setSpacing(0);
    itemLayout->setMargin(0);
    itemLayout->addSpacing(24);
    itemLayout->addWidget(m_itemLab1);
    itemLayout->addWidget(m_itemLab2);
    itemLayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addLayout(titleLayout);
    layout->addSpacing(16);
    layout->addLayout(tabLayout);
    layout->addSpacing(16);
    layout->addLayout(gaugeOutfitLayout);
    layout->addLayout(itemLayout);
    layout->addStretch();

    setLayout(layout);
}

void ShortCutWin::changeTheme(PublicAttributes::Theme theme)
{
    switch (theme) {
        case PublicAttributes::Theme::Light: {
            m_gaugeOutfit1->setStyleSheet("color: #6E6E6E;");
            m_gaugeOutfit2->setStyleSheet("color: #6E6E6E;");
            m_itemLab1->setStyleSheet("background-color: #F5F5F5;");
            m_itemLab2->setStyleSheet("background-color: #F5F5F5;");
            break;
        }
        case PublicAttributes::Theme::Dark: {
            m_gaugeOutfit1->setStyleSheet("color: #999999;");
            m_gaugeOutfit2->setStyleSheet("color: #999999;");
            m_itemLab1->setStyleSheet("background-color: #333333;");
            m_itemLab2->setStyleSheet("background-color: #333333;");
            break;
        }
        default:
            break;
    }
}

void ShortCutWin::changeFont(double fontSize)
{
    QFont font;
    font.setPointSizeF(fontSize);
    m_gaugeOutfit1->setFont(font);
    m_gaugeOutfit2->setFont(font);
    m_itemLab1->setFont(font);
    m_itemLab2->setFont(font);
    font.setBold(true);
    font.setPointSizeF(fontSize + 2);
    m_tableName->setFont(font);
}