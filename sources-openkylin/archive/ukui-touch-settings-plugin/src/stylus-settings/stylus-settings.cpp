/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "stylus-settings.h"
#include "stylussettingpage.h"

#include <QTranslator>
#include <QApplication>

namespace StylusSettings {

StylusSettings::StylusSettings()
{
    QTranslator* translator = new QTranslator(this);
    translator->load("/usr/share/stylus-settings/translations/" + QLocale::system().name());
    QApplication::installTranslator(translator);
}

StylusSettings::~StylusSettings()
{

}

QString StylusSettings::plugini18nName()
{
    return QString(tr("Stylus"));
}
int StylusSettings::pluginTypes()
{
    return FunType::DEVICES;
}
QWidget* StylusSettings::pluginUi()
{
    if (m_pluginwidget == nullptr)
        m_pluginwidget = new StylusSettingPage();
    return m_pluginwidget;
}
bool StylusSettings::isEnable() const
{
    return true;
}
const QString StylusSettings::name() const
{
    //! \note Can not use tr(), because translation will take ukcc search error.
    return QString("Stylus");
}
bool StylusSettings::isShowOnHomePage() const
{
    return true;
}
QIcon StylusSettings::icon() const
{
    QPixmap pixmap(":/img/img/pen-symbolic.svg");
    return QIcon::fromTheme("pen-symbolic.svg", QIcon(pixmap));
}
QString StylusSettings::translationPath() const
{
    return "/usr/share/stylus-settings/translations/%1.ts";
}

} // namespace
