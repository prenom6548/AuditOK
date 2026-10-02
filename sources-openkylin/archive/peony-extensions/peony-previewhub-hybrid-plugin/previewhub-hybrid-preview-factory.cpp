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

#include "previewhub-hybrid-preview-factory.h"
#include "previewhub-hybrid-preview-page.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

PreviewhubHybridPreviewFactory::PreviewhubHybridPreviewFactory(QObject *parent)
    : QObject(parent)
{
    QTranslator *translator = new QTranslator(this);
    const QString path = QStringLiteral(":/translations/peony-previewhub-hybrid-plugin_") + QLocale::system().name();
    if (translator->load(path))
        QApplication::installTranslator(translator);
}

PreviewhubHybridPreviewFactory::~PreviewhubHybridPreviewFactory() = default;

/**
 * @brief 返回预览插件的显示名称。
 */
const QString PreviewhubHybridPreviewFactory::name()
{
    return tr("Hybrid Preview (PreviewHub)");
}

/**
 * @brief 返回预览插件的功能描述。
 */
const QString PreviewhubHybridPreviewFactory::description()
{
    return tr("Folders and generic files use Peony default preview; text, images, audio, video, PDF, and office documents use ukui-previewhub above with the same file property information as the default preview below.");
}

/**
 * @brief 返回预览插件的图标。
 */
const QIcon PreviewhubHybridPreviewFactory::icon()
{
    return QIcon::fromTheme(QStringLiteral("ukui-preview-file-symbolic"));
}

/**
 * @brief 设置插件启用状态。
 * @param enable 是否启用
 */
void PreviewhubHybridPreviewFactory::setEnable(bool enable)
{
    m_enable = enable;
}

/**
 * @brief 查询插件是否启用。
 * @return 已启用返回 true，否则返回 false
 */
bool PreviewhubHybridPreviewFactory::isEnable()
{
    return m_enable;
}

/**
 * @brief 创建一个混合预览页面实例。
 * @return 新建的预览页接口指针，所有权移交调用方
 */
Peony::PreviewPageIface *PreviewhubHybridPreviewFactory::createPreviewPage()
{
    return new PreviewhubHybridPreviewPage;
}
