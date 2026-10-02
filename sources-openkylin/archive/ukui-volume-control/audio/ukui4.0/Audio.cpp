/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "Audio.h"
#include <QDebug>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QLabel>

#include "../MainWidget.h"
#include "../Ukui4MainWidget.h"
#include "../ui_audio.h"

Audio::Audio() : m_firstLoad(true)
{
#ifndef QT_NO_TRANSLATION
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *pTranslator = new QTranslator();
    if (pTranslator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(pTranslator);
#endif
    QTranslator *translator = new QTranslator(this);
    translator->load("/usr/share/ukui-media/translations/audio/audio_" + QLocale::system().name());
    QApplication::installTranslator(translator);
    m_pluginName = tr("Audio");
    m_pluginType = SYSTEM;

    this->installEventFilter(this);
}


QString Audio::plugini18nName()
{
    return m_pluginName;
}

int Audio::pluginTypes()
{
    return m_pluginType;
}

QWidget* Audio::pluginUi()
{
    if (m_firstLoad) {
        m_firstLoad = false;
        m_pWidget = new MainWidget();
    }
    return m_pWidget;
}

bool Audio::isEnable() const
{
    return true;
}

const QString Audio::name() const
{
    return QStringLiteral("Audio");
}

bool Audio::isShowOnHomePage() const
{
    return true;
}

QIcon Audio::icon() const
{
    return QIcon::fromTheme("audio-volume-high-symbolic");
}

QString Audio::translationPath() const
{
    return "/usr/share/ukui-media/translations/audio/audio_%1.ts";
}

void Audio::initSearchText()
{
    //~ contents_path /UkccPlugin/UkccPlugin
    tr("UkccPlugin");
    //~ contents_path /UkccPlugin/ukccplugin test
    tr("ukccplugin test");
}

bool Audio::eventFilter(QObject *watched, QEvent *event)
{
    if (UkccArgEvent::TYPE == event->type()) {
        UkccArgEvent *arg = dynamic_cast<UkccArgEvent* >(event);
        if (arg != nullptr) {
            if (arg->getPluginName() == "appcontrol") {
                MainWidget* mainWidget = qobject_cast<MainWidget*>(m_pWidget);
                if (mainWidget) {
                    mainWidget->showAppControlWidget();
                }
                return true;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

