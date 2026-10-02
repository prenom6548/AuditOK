/*
 * Peony Intelligent Spaces Service
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: He JinQin <hejinqin@kylinos.cn>
 *
 */

#include "peony-intelligent-search-view-plugin.h"
#include "search-view-container.h"
#include <QDebug>
#include <QTranslator>
using namespace Peony;

PeonyIntelligentSearchViewPlugin::PeonyIntelligentSearchViewPlugin(QObject *parent)
{
    //加载翻译
    QTranslator *t = new QTranslator(qApp);
    t->load(":/peony-intelligent-search-view_"+QLocale::system().name());
    QApplication::installTranslator(t);
}

int PeonyIntelligentSearchViewPlugin::priority(const QString &directoryUri)
{
    qDebug() << "=======" << __func__ << directoryUri;
    m_currentUri = directoryUri;
    if (directoryUri.startsWith("search:///")) {
        return 1;
    }
    return -1;
}

DirectoryViewWidget *PeonyIntelligentSearchViewPlugin::create()
{
    return new SearchViewContainer(nullptr,m_currentUri);
}
