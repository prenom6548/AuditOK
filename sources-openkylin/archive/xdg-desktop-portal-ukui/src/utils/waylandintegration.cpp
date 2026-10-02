/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "waylandintegration.h"
#include <QGuiApplication>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/xdgforeign.h>

namespace {
const char *windowParentHandlePropertyName = "waylandintegration-parentHandle";
}

WaylandIntegration::WaylandIntegration(QObject *parent)
    : QObject{parent}
{
    initWayland();
}

void WaylandIntegration::setParentWindow(QWindow *window, const QString &parentHandle)
{
    if (!m_xdgImporter) {
        return;
    }

    if (window->isVisible()) {
        auto *importedParent = m_xdgImporter->importTopLevel(parentHandle, window);
        auto *surface = KWayland::Client::Surface::fromWindow(window);
        importedParent->setParentOf(surface);
    }

    window->setProperty(windowParentHandlePropertyName, parentHandle);
    window->installEventFilter(this);
}

void WaylandIntegration::initWayland()
{
    auto connection = KWayland::Client::ConnectionThread::fromApplication(QGuiApplication::instance());
    if (!connection) {
        return;
    }

    m_registry = new KWayland::Client::Registry(this);
    connect(m_registry, &KWayland::Client::Registry::importerUnstableV2Announced,
            this, [this](quint32 name, quint32 version) {
        m_xdgImporter = m_registry->createXdgImporter(name, std::min(version, quint32(1)), this);
    });

    m_registry->create(connection);
    m_registry->setup();
}
