/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "quick-look-preview-factory.h"
#include "quick-look-window.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

QuickLookPreviewFactory::QuickLookPreviewFactory(QObject *parent)
    : QObject(parent)
{
    QTranslator *translator = new QTranslator(this);
    const QString path = QStringLiteral(":/translations/peony-quick-look-plugin_") + QLocale::system().name();
    if (translator->load(path))
        QApplication::installTranslator(translator);
}

const QString QuickLookPreviewFactory::name()
{
    return tr("Quick Look");
}

const QString QuickLookPreviewFactory::description()
{
    return tr("Press Space to preview the selected file or folder in a popup window.");
}

const QIcon QuickLookPreviewFactory::icon()
{
    return QIcon::fromTheme(QStringLiteral("ukui-preview-file-symbolic"));
}

void QuickLookPreviewFactory::setEnable(bool enable)
{
    m_enable = enable;
}

bool QuickLookPreviewFactory::isEnable()
{
    return m_enable;
}

Peony::QuickLookWindowIface *QuickLookPreviewFactory::createQuickLookWindow()
{
    return new QuickLookWindow();
}
