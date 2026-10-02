/*
 * UKUI Intelligent Data Manager
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
 * Authors: JinQin He <hejinqin@kylinos.cn>
 *
 */

#include "space-view-container.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include<file-launch-manager.h>
using namespace Peony;

SpaceViewContainer::SpaceViewContainer(QWidget *parent,QString uri,QString Spacename)
    :m_currentUri(uri),m_Spacename(Spacename)
{
    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setObjectName("_spaceview_layout");
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    m_view = new Spaceview(this,m_currentUri,m_Spacename);
    layout->addWidget(m_view);
    setLayout(layout);
}

SpaceViewContainer::~SpaceViewContainer()
{

}

const QString SpaceViewContainer::getDirectoryUri()
{
    return m_currentUri;
}

void SpaceViewContainer::bindModel(FileItemModel *model, FileItemProxyFilterSortModel *proxyModel)
{
    auto layout = findChild<QVBoxLayout *>("_spaceview_layout");
    bool ok = false;
    if (parentWidget()) {
        int statusBarHeight = parentWidget()->property("statusBarHeight").toInt(&ok);
        if (ok) {
            layout->setContentsMargins(0, 0, 0, statusBarHeight);
        }
    }
}

void SpaceViewContainer::setDirectoryUri(const QString &uri)
{
    if (m_currentUri != uri) {
        m_currentUri = uri;
        m_view->setDirectoryUri(uri);
    }
}

void SpaceViewContainer::beginLocationChange()
{
    Q_EMIT viewDirectoryChanged();
}

void SpaceViewContainer::stopLocationChange()
{
    Q_EMIT viewDirectoryChanged();
}

void SpaceViewContainer::contextMenuEvent(QContextMenuEvent *event)
{
    Q_EMIT menuRequest(event->globalPos());
}
