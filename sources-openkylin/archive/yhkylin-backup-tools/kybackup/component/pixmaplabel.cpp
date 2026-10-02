/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "pixmaplabel.h"
#include <QIcon>
#include "imageutil.h"
#include "../globalsignals.h"
#include "../globalbackupinfo.h"
#include "../gsettingswrapper.h"
#include "../../common/mydefine.h"

PixmapLabel::PixmapLabel(QWidget *parent) :
    ClickLabel(parent)
{
    setAlignment(Qt::AlignCenter);
}

PixmapLabel::~PixmapLabel()
{}

// 应用场景一：设置深浅主题分别展示的图片
void PixmapLabel::setLightAndDarkPixmap(const QString &light, const QString &dark)
{
    m_light = light;
    m_dark = dark;

    on_styleNameChanged(g_GSettingWrapper.isDarkTheme());

    disconnect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, &PixmapLabel::on_styleNameChanged);
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, &PixmapLabel::on_styleNameChanged);
}

void PixmapLabel::on_styleNameChanged(bool isDark)
{
    if (isDark) {
        QPixmap pixmap(m_dark);
        setPixmap(pixmap);
    } else {
        QPixmap pixmap(m_light);
        setPixmap(pixmap);
    }
}

// 应用场景二：设置跟随主题风格图标，不随深浅背景色变化
void PixmapLabel::setUkuiIconSchema(const QString &schema, QSize size)
{
    m_iconTheme = schema;
    m_iconSize = size;
    m_iconThemeDefault = "";
    m_autoChangeColor = false;

    on_themeIconChanged();

    if (!m_onlyFlush) {
        disconnect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &PixmapLabel::on_themeIconChanged);
        connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &PixmapLabel::on_themeIconChanged);
    }
}

// 应用场景三：设置跟随主题风格图标（主题中也可能不存在该图标），随深浅色背景色变化
void PixmapLabel::setUkuiIconSchema(const QString &schema, const QString &schemaDefault, QSize size)
{
    m_iconTheme = schema;
    m_iconThemeDefault = schemaDefault;
    m_iconSize = size;
    m_autoChangeColor = true;

    on_themeIconChanged();

    if (!m_onlyFlush) {
        disconnect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &PixmapLabel::on_themeIconChanged);
        connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &PixmapLabel::on_themeIconChanged);

        disconnect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, &PixmapLabel::on_themeIconChanged);
        connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, &PixmapLabel::on_themeIconChanged);
    }
}

// 应用场景四：设置跟随主题风格图标（主题中也可能不存在该图标），随主体风格：寻光、启典、合印等图标风格变化而变换，不随深浅背景色变化
void PixmapLabel::setThemeIconSchema(const QString &schema, const QString &schemaDefault, QSize size)
{
    m_iconTheme = schema;
    m_iconThemeDefault = schemaDefault;
    m_iconSize = size;
    m_autoChangeColor = false;

    on_themeIconChanged();

    if (!m_onlyFlush) {
        disconnect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &PixmapLabel::on_themeIconChanged);
        connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &PixmapLabel::on_themeIconChanged);
    }
}

void PixmapLabel::on_themeIconChanged()
{
    if (m_iconThemeDefault.isEmpty()) {
        QIcon titleIcon = QIcon::fromTheme(m_iconTheme);
        setPixmap(titleIcon.pixmap(titleIcon.actualSize(m_iconSize)));
    } else if (!m_autoChangeColor) {
        QIcon titleIcon = QIcon::fromTheme(m_iconTheme, QIcon(m_iconThemeDefault));
        setPixmap(titleIcon.pixmap(titleIcon.actualSize(m_iconSize)));
    } else {
        if (g_GSettingWrapper.isDarkTheme())
            setPixmap(ImageUtil::loadTheme(m_iconTheme, m_iconThemeDefault, "white", m_iconSize));
        else
            setPixmap(ImageUtil::loadTheme(m_iconTheme, m_iconThemeDefault, "default", m_iconSize));
    }
}

