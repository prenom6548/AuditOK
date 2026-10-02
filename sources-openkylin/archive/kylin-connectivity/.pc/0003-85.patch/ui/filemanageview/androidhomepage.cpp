#include "androidhomepage.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

AndroidHomePage::AndroidHomePage(QWidget *parent) : QWidget(parent)
{
    m_multiMediaWin = new MultiMediaWin(this);
    connect(m_multiMediaWin, &MultiMediaWin::sigBtnClicked, this, &AndroidHomePage::sigBtnClicked);
    m_multiMediaWin->setAppItem();

    m_title = new QLabel(this);
    m_title->setText(tr("Mobile Storage"));
    QFont font;
    font.setBold(true);
    m_title->setFont(font);

    m_storageItem = new AndroidItem(AndroidItem::Type::File, this);
    connect(m_storageItem, &AndroidItem::sigBtnClicked, this, &AndroidHomePage::sigBtnClicked);

    QHBoxLayout *titleHLayout = new QHBoxLayout;
    titleHLayout->setSpacing(0);
    titleHLayout->setMargin(0);
    titleHLayout->addSpacing(26);
    titleHLayout->addWidget(m_title);
    titleHLayout->addStretch();

    QHBoxLayout *itemHLayout = new QHBoxLayout;
    itemHLayout->setSpacing(0);
    itemHLayout->setMargin(0);
    itemHLayout->addSpacing(16);
    itemHLayout->addWidget(m_storageItem);
    itemHLayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_multiMediaWin);
    layout->addSpacing(40);
    layout->addLayout(titleHLayout);
    layout->addSpacing(17);
    layout->addLayout(itemHLayout);
    layout->addStretch();
    setLayout(layout);
}

void AndroidHomePage::setItemCount(AndroidItem::Type type, int num)
{
    m_multiMediaWin->setItemCount(type, num);
}
void AndroidHomePage::setTheme(PublicAttributes::Theme theme)
{
    m_multiMediaWin->setTheme(theme);
    m_storageItem->setTheme(theme);
}

void AndroidHomePage::changeFontSize(double fontSize)
{
    m_multiMediaWin->changeFontSize(fontSize);
    m_storageItem->changeFontSize(fontSize);
    QFont font;
    font.setPointSizeF(fontSize);
    font.setBold(true);
    m_title->setFont(font);
}