/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "pixmapbutton.h"
#include "imageutil.h"
#include "../gsettingswrapper.h"
#include "../globalbackupinfo.h"

PixmapButton::PixmapButton(QWidget *parent) :
    QPushButton(parent)
{
    this->setCheckable(false);
    this->setProperty("useButtonPalette", true);
    m_hoverColor = pluginBtnHoverColor(true);
    m_clickColor = pluginBtnHoverColor(false);
    this->setStyleSheet(QString("QPushButton:!hover:!pressed{background-color:palette(base);border-radius: 6px;}"
                                "QPushButton:hover{background-color:%1;border-radius: 6px;}"
                                "QPushButton:pressed{background-color:%2;border-radius: 6px;}").arg(m_hoverColor).arg(m_clickColor));

    m_iconSize = 16;

    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, [=](bool isDark) {
        // 深浅主题切换时，因为调色板已经更换，高亮等颜色已经改变，所以要重新加载图标。
        if (isDark)
            this->setIcon(ImageUtil::loadTheme(this->m_iconTheme, this->m_defaultIconName, "white", this->m_iconSize));
        else
            this->setIcon(ImageUtil::loadTheme(this->m_iconTheme, this->m_defaultIconName, "default", this->m_iconSize));

        this->m_hoverColor = pluginBtnHoverColor(true);
        this->m_clickColor = pluginBtnHoverColor(false);
        this->setStyleSheet(QString("QPushButton:!hover:!pressed{background-color:palette(base);border-radius: 6px;}"
                                    "QPushButton:hover{background-color:%1;border-radius: 6px;}"
                                    "QPushButton:pressed{background-color:%2;border-radius: 6px;}").arg(m_hoverColor).arg(m_clickColor));
    });
}

void PixmapButton::setThemeIcon(const QString &themeIconName, const QString &defaultIconName, int size)
{
    m_iconTheme = themeIconName;
    m_defaultIconName = defaultIconName;
    m_iconSize = size;

    if (g_GSettingWrapper.isDarkTheme())
        this->setIcon(ImageUtil::loadTheme(m_iconTheme, m_defaultIconName, "white", m_iconSize));
    else
        this->setIcon(ImageUtil::loadTheme(m_iconTheme, m_defaultIconName, "default", m_iconSize));
}

PixmapButton::~PixmapButton()
{}

QString PixmapButton::pluginBtnHoverColor(bool hoverFlag)
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

