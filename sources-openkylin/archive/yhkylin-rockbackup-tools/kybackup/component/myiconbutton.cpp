/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "myiconbutton.h"
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QStyleOptionButton>
#include <QApplication>
#include <QPainter>
#include "imageutil.h"
#include "../gsettingswrapper.h"
#include "../globalbackupinfo.h"

MyIconButton::MyIconButton(QWidget *parent) :
    QPushButton(parent)
{
    this->setCheckable(false);
    // 注释掉，不再使用主题，因为按钮切换会有闪烁并且主题是全灰色不适合设计要求，改为使用stylesheet方式自行设置
    // this->setProperty("useButtonPalette", true);
    m_hoverColor = pluginBtnHoverColor(true);
    m_clickColor = pluginBtnHoverColor(false);
    if (!this->isChecked())
        this->setStyleSheet(QString("QPushButton:hover{background-color:%1;border-radius: 6px;}"
                                    "QPushButton:pressed{background-color:%2;border-radius: 6px;}").arg(m_hoverColor).arg(m_clickColor));
    m_pixSize = 16;

    m_iconButton = new QPushButton(this);
    m_iconButton->setObjectName("yhkylin-backup-tools_kybackup_QPushButton_m_iconButton");
    m_iconButton->setCheckable(true);
    m_iconButton->setFixedSize(QSize(24, 24));
    m_iconButton->setFocusPolicy(Qt::NoFocus);
    m_iconButton->setFlat(true);
    m_iconButton->setProperty("useIconHighlightEffect", 0x8);
    QString iconBtnQss = QString("QPushButton:checked{border:  none;}"
                                 "QPushButton:!checked{border: none;}");
    m_iconButton->setStyleSheet(iconBtnQss);

    m_textLabel = new MyLabel(this);
    m_textLabel->setObjectName("yhkylin-backup-tools_kybackup_MyLabel_m_textLabel");
    QSizePolicy textLabelPolicy = m_textLabel->sizePolicy();
    textLabelPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    textLabelPolicy.setVerticalPolicy(QSizePolicy::Expanding);
    m_textLabel->setSizePolicy(textLabelPolicy);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(10, 0, 5, 0);
    hLayout->setSpacing(5);
    hLayout->addWidget(m_iconButton);
    hLayout->addWidget(m_textLabel);
    hLayout->setAlignment(Qt::AlignLeft);
    setLayout(hLayout);

    connect(m_iconButton, &QPushButton::clicked, this, [=]() {
        this->click();
        m_iconButton->setChecked(this->isChecked());
    });

    connect(m_iconButton, &QPushButton::toggled, this, [=] (bool checked) {
        if (checked || g_GSettingWrapper.isDarkTheme())
            m_iconButton->setIcon(ImageUtil::loadTheme(this->m_themeIconName, this->m_defaultIconName, "white", this->m_pixSize));
        else
            m_iconButton->setIcon(ImageUtil::loadTheme(this->m_themeIconName, this->m_defaultIconName, "default", this->m_pixSize));
    });

    connect(this, &MyIconButton::toggled, this, [=](bool checked) {
        m_iconButton->setChecked(this->isChecked());
        if (checked) {
            this->setStyleSheet("QPushButton:checked{background-color: palette(highlight);border-radius: 6px;}");
            m_textLabel->setStyleSheet("color:white");
        } else {
            this->setStyleSheet(QString("QPushButton:hover{background-color:%1;border-radius: 6px;}"
                                        "QPushButton:pressed{background-color:%2;border-radius: 6px;}").arg(this->m_hoverColor).arg(this->m_clickColor));
            m_textLabel->setStyleSheet("color:palette(windowText)");
        }
    });

    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, [=](bool isDark) {
        // 深浅主题切换时，因为调色板已经更换，高亮等颜色已经改变，所以要重新加载图标。
        if (this->isChecked() || isDark)
            this->m_iconButton->setIcon(ImageUtil::loadTheme(this->m_themeIconName, this->m_defaultIconName, "white", this->m_pixSize));
        else
            this->m_iconButton->setIcon(ImageUtil::loadTheme(this->m_themeIconName, this->m_defaultIconName, "default", this->m_pixSize));

        this->m_hoverColor = pluginBtnHoverColor(true);
        this->m_clickColor = pluginBtnHoverColor(false);
        if (!this->isChecked())
            this->setStyleSheet(QString("QPushButton:hover{background-color:%1;border-radius: 6px;}"
                                        "QPushButton:pressed{background-color:%2;border-radius: 6px;}").arg(this->m_hoverColor).arg(this->m_clickColor));
    });

    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::fontChanged, this, [=](int fontSize) {
        QFont font = this->m_textLabel->font();
        font.setPointSize(fontSize);
        this->m_textLabel->setFont(font);
        // 字体家族、大小变化需重绘，并且字体大小变化也可能会造成显示不全问题
        this->setDesplayText(m_originalText);
    });

}

MyIconButton::~MyIconButton()
{}

void MyIconButton::setThemeIcon(const QString &themeIconName, const QString &defaultIconName, int size)
{
    m_themeIconName = themeIconName;
    m_defaultIconName = defaultIconName;
    m_pixSize = size;

    if (g_GSettingWrapper.isDarkTheme())
        m_iconButton->setIcon(ImageUtil::loadTheme(m_themeIconName, m_defaultIconName, "white", m_pixSize));
    else
        m_iconButton->setIcon(ImageUtil::loadTheme(m_themeIconName, m_defaultIconName, "default", m_pixSize));
}

/**
 * @brief 设置显示文字
 * @param text
 * @note 先设置button大小，然后再调用此函数设置显示文字，因为显示文字label宽度要根据button宽度计算
 */
void MyIconButton::setDesplayText(const QString &text)
{
    m_originalText = text;
    // m_textLabel->setFixedWidth(this->width() - 40);
    m_textLabel->setMinimumWidth(this->width() - 40);
    if (m_textLabel->wordWrap()) {
        m_textLabel->setDeplayText(m_originalText);
    } else {
        QFontMetrics fontMetrics(m_textLabel->font());
        int fontSize = fontMetrics.width(m_originalText);
        if (fontSize > m_textLabel->width()) {
            m_textLabel->setDeplayText(fontMetrics.elidedText(m_originalText, Qt::ElideRight, m_textLabel->width()));
        } else {
            m_textLabel->setDeplayText(m_originalText);
        }
    }
}

QString MyIconButton::pluginBtnHoverColor(bool hoverFlag)
{
    QColor color1, color2;
    if (this->parent()) {
        QWidget * parent = qobject_cast<QWidget *>(this->parent());
        color1 = parent->palette().color(QPalette::Active, QPalette::Button);
        color2 = parent->palette().color(QPalette::Active, QPalette::BrightText);
    } else {
        color1 = palette().color(QPalette::Active, QPalette::Button);
        color2 = palette().color(QPalette::Active, QPalette::BrightText);
    }
    QColor color;
    qreal r,g,b,a;
    QString hoverColor;
    if ((g_GSettingWrapper.isDarkTheme() && hoverFlag) ||
        (!g_GSettingWrapper.isDarkTheme() && !hoverFlag)) {
        r = color1.redF() * 0.8 + color2.redF() * 0.2;
        g = color1.greenF() * 0.8 + color2.greenF() * 0.2;
        b = color1.blueF() * 0.8 + color2.blueF() * 0.2;
        a = color1.alphaF() * 0.8 + color2.alphaF() * 0.2;
    } else {
        r = color1.redF() * 0.95 + color2.redF() * 0.05;
        g = color1.greenF() * 0.95 + color2.greenF() * 0.05;
        b = color1.blueF() * 0.95 + color2.blueF() * 0.05;
        a = color1.alphaF() * 0.95 + color2.alphaF() * 0.05;
    }
    color = QColor::fromRgbF(r, g, b, a);
    hoverColor = QString("rgba(%1, %2, %3, %4)").arg(color.red())
                                                .arg(color.green())
                                                .arg(color.blue())
                                                .arg(color.alpha());
    return hoverColor;
}

/**
 * 废弃，改为使用qss了
 */
void MyIconButton::changePalette(bool checked)
{
    m_iconButton->setChecked(checked);
    QPalette pal = m_textLabel->palette();
    // png格式的图标会自动跟随主题，不需再手动设置像素颜色
    if (g_GSettingWrapper.isDarkTheme()) {
        if (checked) {
            pal.setColor(QPalette::ButtonText, this->palette().highlightedText().color());
        } else {
            pal.setColor(QPalette::ButtonText, this->palette().windowText().color());
        }
    } else {
        if (checked) {
            pal.setColor(QPalette::ButtonText, this->palette().highlightedText().color());
        } else {
            pal.setColor(QPalette::ButtonText, this->palette().windowText().color());
        }
    }
    m_textLabel->setPalette(pal);
}

