/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "myiconlabel.h"
#include <QHBoxLayout>
#include <QIcon>
#include <QApplication>
#include "../globalsignals.h"
#include "../globalbackupinfo.h"
#include "../gsettingswrapper.h"
#include "imageutil.h"

MyIconLabel::MyIconLabel(QWidget *parent /*= nullptr*/) :
    QLabel(parent)
{
    m_iconLabel = new CircleLable("", this, 36);
    m_iconLabel->setObjectName("yhkylin-backup-tools_kybackup_CircleLable_m_iconLabel");
    m_iconSize = 16;
    //  border:1px solid black;
    // const QString greySheetStyle = "min-width: 36px; min-height: 36px;max-width:36px; max-height: 36px;border-radius: 18px; background:grey";
//    const QString greySheetStyle = "min-width: 36px; min-height: 36px;max-width:36px; max-height: 36px;border-radius: 18px; background:#F4F4F4";
//    m_iconLabel->setStyleSheet(greySheetStyle);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setBackgroundColor(QColor(COLOR_LIGHT_DARK));

    m_textLabel = new QLabel(this);
    m_textLabel->setObjectName("yhkylin-backup-tools_kybackup_QLabel_m_textLabel");
    QSizePolicy textLabelPolicy = m_textLabel->sizePolicy();
    textLabelPolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    textLabelPolicy.setVerticalPolicy(QSizePolicy::Fixed);
    m_textLabel->setSizePolicy(textLabelPolicy);
    m_textLabel->setScaledContents(true);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(8, 0, 0, 0);
    hLayout->addWidget(m_iconLabel, Qt::AlignCenter);
    hLayout->addWidget(m_textLabel);
    hLayout->addStretch();
    setLayout(hLayout);

    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, [=](){
        this->setThemeIcon(this->m_themeIconName, this->m_defaultIconName, this->m_iconSize);
    });
}

MyIconLabel::~MyIconLabel()
{}

void MyIconLabel::setThemeIcon(const QString &themeIconName, const QString &defaultIconName, int size)
{
    m_themeIconName = themeIconName;
    m_defaultIconName = defaultIconName;
    m_iconSize = size;

    if (g_GSettingWrapper.isDarkTheme())
        m_iconLabel->setPixmap(ImageUtil::loadTheme(this->m_themeIconName, this->m_defaultIconName, "white", m_iconSize));
    else
        m_iconLabel->setPixmap(ImageUtil::loadTheme(this->m_themeIconName, this->m_defaultIconName, "default", m_iconSize));
}

void MyIconLabel::setDesplayText(const QString &text)
{
    m_originalText = text;
    m_textLabel->setFixedWidth(this->width() - 40);
    QFontMetrics  fontMetrics(m_textLabel->font());
    int fontSize = fontMetrics.width(text);
    if (fontSize > m_textLabel->width()) {
        m_textLabel->setText(fontMetrics.elidedText(text, Qt::ElideRight, m_textLabel->width()));
    } else {
        m_textLabel->setText(text);
    }
}

