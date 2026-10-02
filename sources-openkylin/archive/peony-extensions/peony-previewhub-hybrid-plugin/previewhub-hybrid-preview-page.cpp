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

#include "previewhub-hybrid-preview-page.h"
#include "previewhub-hybrid-hub-page.h"

#include <file-info.h>
#include <preview-page/default-preview-page/default-preview-page.h>

#include <QMimeDatabase>
#include <QUrl>

#include <cstring>

/**
 * @brief 判断给定 MIME 类型是否属于办公文档类型。
 * @param mime 待判断的 MIME 类型字符串
 * @return 属于办公文档返回 true，否则返回 false
 */
static bool isOfficeMime(const QString &mime)
{
    for (int i = 0; std::strcmp(Peony::office_mime_types[i], "end") != 0; ++i) {
        if (mime == QLatin1String(Peony::office_mime_types[i]))
            return true;
    }
    return false;
}

/**
 * @brief 判断给定 MIME 类型是否使用 previewhub 预览。
 * @param mime 待判断的 MIME 类型字符串
 * @return 支持文本、图片、音视频、PDF 及办公文档返回 true，否则返回 false
 */
bool PreviewhubHybridPreviewPage::isHubMime(const QString &mime)
{
    if (mime.isEmpty())
        return false;
    if (mime.startsWith(QLatin1String("text/")) ||
        mime.startsWith(QLatin1String("application/xml")) ||
        mime.startsWith(QLatin1String("application/json")) ||
        mime.startsWith(QLatin1String("application/javascript")) ||
        mime.startsWith(QLatin1String("application/vnd.nokia.qt.qmakeprofile")))
        return true;
    if (mime.startsWith(QLatin1String("image/")))
        return true;
    if (mime.startsWith(QLatin1String("audio/")))
        return true;
    if (mime.startsWith(QLatin1String("video/")))
        return true;
    if (mime == QLatin1String("application/pdf"))
        return true;
    if (mime.startsWith(QLatin1String("application/vnd.oasis.opendocument.")))
        return true;
    if (mime.startsWith(QLatin1String("application/vnd.openxmlformats-officedocument.")))
        return true;
    return isOfficeMime(mime);
}

/**
 * @brief 解析给定 URI 对应文件的 MIME 类型。
 * @param uri 文件的 URI
 * @return MIME 类型字符串，无法解析时返回空字符串
 *
 * 优先使用 Peony::FileInfo 提供的类型，失败时回退到 QMimeDatabase 按本地路径匹配。
 */
QString PreviewhubHybridPreviewPage::resolveMime(const QString &uri)
{
    auto info = Peony::FileInfo::fromUri(uri);
    if (info) {
        const QString fromInfo = info->mimeType();
        if (!fromInfo.isEmpty())
            return fromInfo;
    }
    const QUrl u = QUrl(uri);
    if (u.isLocalFile()) {
        const QString path = u.toLocalFile();
        if (!path.isEmpty())
            return QMimeDatabase().mimeTypeForFile(path, QMimeDatabase::MatchDefault).name();
    }
    return QString();
}

/**
 * @brief 判断文件是否适合交给 previewhub 做内容预览。
 * @param uri 文件的 URI
 * @return 可读且 MIME 支持 previewhub 时返回 true，否则返回 false
 *
 * 无读取权限或符号链接目标不可访问时回退到默认预览页，避免 previewhub
 * 显示“无法打开该文件”，由默认页展示文件默认图标和属性信息。
 */
bool PreviewhubHybridPreviewPage::canUseHubPreview(const QString &uri)
{
    auto info = Peony::FileInfo::fromUri(uri);
    if (!info)
        return false;
    if (info->isDir())
        return false;
    if (!info->canRead() || !info->isExistTargetOfSymlink())
        return false;

    return isHubMime(resolveMime(uri));
}

/**
 * @brief 构造函数，创建默认预览页与混合预览页并默认显示前者。
 * @param parent 父控件指针
 */
PreviewhubHybridPreviewPage::PreviewhubHybridPreviewPage(QWidget *parent)
    : QStackedWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    setAutoFillBackground(true);

    m_defaultPage = new Peony::DefaultPreviewPage(this);
    m_hubPage = new PreviewhubHybridHubPage(this);
    m_hubPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    addWidget(m_defaultPage);
    addWidget(m_hubPage);
    setCurrentWidget(m_defaultPage);
}

PreviewhubHybridPreviewPage::~PreviewhubHybridPreviewPage()
{
    cancel();
}

/**
 * @brief 根据文件类型确定预览模式并委派给对应预览页准备数据。
 * @param uri 目标文件的 URI
 *
 * 目录、无读取权限文件始终使用默认预览；显式类型按传入类型判定；
 * 否则依据解析出的 MIME 类型决定是否使用混合预览。
 */
void PreviewhubHybridPreviewPage::updateModeAndDelegatePrepare(const QString &uri)
{
    m_uri = uri;

    auto info = Peony::FileInfo::fromUri(uri);
    const bool isDir = info && info->isDir();
    const bool canRead = info && info->canRead() && info->isExistTargetOfSymlink();

    if (isDir || !canRead) {
        m_useHub = false;
    } else if (m_explicitType) {
        switch (m_passedType) {
        case Peony::PreviewPageIface::Text:
        case Peony::PreviewPageIface::Pictrue:
        case Peony::PreviewPageIface::PDF:
        case Peony::PreviewPageIface::OfficeDoc:
            m_useHub = canRead;
            break;
        case Peony::PreviewPageIface::Attribute:
        case Peony::PreviewPageIface::Other:
        default:
            m_useHub = false;
            break;
        }
    } else {
        m_useHub = canUseHubPreview(uri);
    }

    m_hubPage->cancel();
    m_defaultPage->cancel();

    if (m_useHub) {
        m_hubPage->prepare(m_uri);
    } else {
        if (m_explicitType)
            m_defaultPage->prepare(m_uri, m_passedType);
        else
            m_defaultPage->prepare(m_uri);
    }
}

/**
 * @brief 带显式预览类型的准备接口。
 * @param uri 目标文件的 URI
 * @param type 调用方指定的预览类型
 */
void PreviewhubHybridPreviewPage::prepare(const QString &uri, Peony::PreviewPageIface::PreviewType type)
{
    m_explicitType = true;
    m_passedType = type;
    updateModeAndDelegatePrepare(uri);
}

/**
 * @brief 不带预览类型的准备接口，由 MIME 自动判定模式。
 * @param uri 目标文件的 URI
 */
void PreviewhubHybridPreviewPage::prepare(const QString &uri)
{
    m_explicitType = false;
    m_passedType = Peony::PreviewPageIface::Other;
    updateModeAndDelegatePrepare(uri);
}

/**
 * @brief 启动预览，切换到对应预览页并触发其渲染。
 */
void PreviewhubHybridPreviewPage::startPreview()
{
    if (m_useHub) {
        setCurrentWidget(m_hubPage);
        m_hubPage->startPreview();
    } else {
        setCurrentWidget(m_defaultPage);
        m_defaultPage->startPreview();
    }
}

/**
 * @brief 取消两个预览页并回退到默认预览页。
 */
void PreviewhubHybridPreviewPage::cancel()
{
    m_defaultPage->cancel();
    m_hubPage->cancel();
    setCurrentWidget(m_defaultPage);
}

/**
 * @brief 关闭预览页，取消预览并延迟销毁自身。
 */
void PreviewhubHybridPreviewPage::closePreviewPage()
{
    cancel();
    deleteLater();
}
