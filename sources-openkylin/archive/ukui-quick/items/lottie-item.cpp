/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

/*!
    \qmltype LottieItem
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 提供 Lottie 动画显示组件.

    LottieItem 继承自 QQuickItem，并通过场景图渲染 Lottie JSON 或 dotLottie
    动画。设置 \l source 后，可使用 \l Lottie 附加属性控制动画播放、循环、
    速度和当前帧。

    LottieItem 是受支持的两个 Lottie 宿主之一；另一个是 Lottie 源的 \l Icon 。
    加载、主题和栅格化通过异步 runtime 完成，GUI 线程只接收已提交的 CPU 图像。
    在加载或替换 source 期间，\l Lottie::status 为 \c Loading ；失败时组件保持
    透明，详细 backend 诊断不会通过 \l Lottie::errorString 暴露。

    LottieItem is one of the two supported Lottie hosts. Its runtime work is
    asynchronous and its failure presentation is transparent; use the \c Lottie
    attached properties for status, metadata, playback intent, completion, and
    theme preference.

    \section1 使用示例

    \qml
    import QtQuick
    import org.ukui.quick.items 1.0

    LottieItem {
        width: 64
        height: 64
        source: "qrc:/animations/loading.lottie"
        Lottie.playing: true
        Lottie.loop: true
    }
    \endqml

    \sa Lottie, Icon
*/

/*!
    \qmlproperty string LottieItem::source
    \brief 动画源.

    设置要渲染的 Lottie JSON 或 dotLottie 定位符。可以使用本地文件
    路径、qrc URL 或相对当前 QML 文件的路径。不带路径的 \c *-motion
    和 \c *-motion.lottie 会优先在系统 motion-icon 目录中查找，
    未找到时再使用 QML 相对路径。显式路径、file/qrc URL 和 JSON
    文件名不会被当作 motion-icon 别名。路径搜索与文件读取均在内部
    asset worker 中完成。
*/

/*!
    \qmlproperty enumeration LottieItem::mode
    \brief 为动画选择普通或高亮背景下的自动主题。

    \value LottieItem.Normal 跟随系统背景选择 \c light 或 \c dark 主题。
    \value LottieItem.Highlight 使用适合品牌色等深色高亮背景的 \c dark 主题。
    \value LottieItem.AutoHighlight 跟随系统背景，行为与自动主题保持一致。
    \value LottieItem.ForceHighlight 强制使用高亮背景主题。

    该属性只选择动画资源已有的标准主题，不会给动画像素重新着色，也不会
    合成 highlight 主题。显式设置 \l Lottie::theme 后，手动主题优先；重新
    启用 \l Lottie::followHostTheme 后，当前 mode 才会重新生效。
*/

#include "lottie-item.h"

#include "lottie-attached.h"
#include "lottie-host-renderability_p.h"
#include "ukui/dt-theme.h"

#include <QImage>
#include <QPointer>
#include <QSGImageNode>
#include <QSGTexture>
#include <QQuickWindow>
#include <qqml.h>

#include <memory>

namespace UkuiQuick {

namespace {

QImage transparentImage()
{
    QImage image(QSize(1, 1), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    return image;
}

bool usesHighlightedLottieTheme(LottieItem::HighlightMode mode)
{
    return mode.testFlag(LottieItem::Highlight) &&
        !mode.testFlag(LottieItem::AutoHighlight);
}

} // namespace

class LottieItemHostAdapter : public QObject
{
public:
    LottieItemHostAdapter(LottieItem *host, LottieAttached *controller)
        : QObject()
        , m_host(host)
        , m_controller(controller)
    {
        const QPointer<LottieAttached> guardedController(m_controller);
        QObject::connect(m_controller, &LottieAttached::frameImageChanged,
                         this, [host = m_host, guardedController] {
            host->acceptCommittedLottieImage(
                guardedController ? guardedController->committedImage() : QImage());
        });
        QObject::connect(m_controller, &QObject::destroyed, this, [host = m_host](QObject *) {
            host->notifyLottieChanged();
        });

        const auto syncTheme = [this] {
            if (m_controller) {
                m_controller->setHostTheme(automaticLottieThemeCandidate(
                    usesHighlightedLottieTheme(m_host->mode())));
            }
        };
        const auto syncSource = [this, syncTheme] {
            if (!m_controller) {
                return;
            }
            syncTheme();
            m_controller->setSource(m_host->source());
        };

        QObject::connect(m_host, &LottieItem::sourceChanged, this, syncSource);
        QObject::connect(m_host, &LottieItem::modeChanged, this, syncTheme);
        QObject::connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::styleColorChanged,
                         this, syncTheme);
        QObject::connect(ThemeGlobalConfig::instance(), &ThemeGlobalConfig::themeNameChanged,
                         this, syncTheme);

        syncSource();
        m_renderability =
            std::make_unique<LottieHostRenderabilityTracker>(m_host, m_controller);
    }

    LottieAttached *controller() const
    {
        return m_controller;
    }

    void syncRenderConfiguration()
    {
        if (m_renderability) {
            m_renderability->sync();
        }
    }

    void syncRenderConfigurationForWindow(QQuickWindow *window)
    {
        if (m_renderability) {
            m_renderability->syncForWindow(window);
        }
    }

private:
    LottieItem *m_host;
    QPointer<LottieAttached> m_controller;
    std::unique_ptr<LottieHostRenderabilityTracker> m_renderability;
};

class LottieItemPrivate
{
public:
    QString source;
    LottieItem::HighlightMode mode {LottieItem::Normal};
    std::unique_ptr<LottieItemHostAdapter> hostAdapter;
    QImage committedImage;
    bool textureChanged = true;
};

LottieItem::LottieItem(QQuickItem *parent)
    : QQuickItem(parent)
    , d(new LottieItemPrivate)
{
    setFlag(ItemHasContents);
    connect(this, &QQuickItem::widthChanged, this, &LottieItem::handleSizeChanged);
    connect(this, &QQuickItem::heightChanged, this, &LottieItem::handleSizeChanged);
}

LottieItem::~LottieItem()
{
    delete d;
    d = nullptr;
}

QString LottieItem::source() const
{
    return d->source;
}

void LottieItem::setSource(const QString &source)
{
    if (d->source == source) {
        return;
    }

    d->source = source;
    if (!source.isEmpty()) {
        ensureLottieController();
    }
    Q_EMIT sourceChanged();
    markTextureChanged();
}

LottieItem::HighlightMode LottieItem::mode() const
{
    return d->mode;
}

void LottieItem::setMode(HighlightMode mode)
{
    if (d->mode == mode) {
        return;
    }

    d->mode = mode;
    Q_EMIT modeChanged();
}

LottieAttached *LottieItem::lottieAttached() const
{
    return qobject_cast<LottieAttached *>(
        qmlAttachedPropertiesObject<LottieAttached>(this, false));
}

void LottieItem::attachLottieController(LottieAttached *attached)
{
    if (d->hostAdapter) {
        Q_ASSERT(d->hostAdapter->controller() == attached);
        return;
    }
    d->hostAdapter = std::make_unique<LottieItemHostAdapter>(this, attached);
}

void LottieItem::ensureLottieController()
{
    if (d->hostAdapter) {
        return;
    }
    qmlAttachedPropertiesObject<LottieAttached>(this, true);
}

void LottieItem::syncLottieHostAdapter()
{
    if (d->hostAdapter) {
        d->hostAdapter->syncRenderConfiguration();
    }
}

void LottieItem::syncLottieHostAdapter(QQuickWindow *window)
{
    if (d->hostAdapter) {
        d->hostAdapter->syncRenderConfigurationForWindow(window);
    }
}

void LottieItem::acceptCommittedLottieImage(const QImage &image)
{
    d->committedImage = image;
    markTextureChanged();
}

void LottieItem::notifyLottieChanged()
{
    markTextureChanged();
}

QSGNode *LottieItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *data)
{
    Q_UNUSED(data)

    if (!window()) {
        delete node;
        return nullptr;
    }

    QSGImageNode *imageNode = static_cast<QSGImageNode *>(node);
    if (!imageNode) {
        imageNode = window()->createImageNode();
        imageNode->setOwnsTexture(true);
        imageNode->setFiltering(QSGTexture::Linear);
        node = imageNode;
        d->textureChanged = true;
    }

    if (d->textureChanged) {
        QImage image = d->committedImage;
        if (image.isNull()) {
            image = transparentImage();
        }

        imageNode->setTexture(window()->createTextureFromImage(image));
        imageNode->setSourceRect(QRectF(QPointF(), QSizeF(image.size())));
        d->textureChanged = false;
    }

    imageNode->setRect(boundingRect());
    return node;
}

void LottieItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == ItemDevicePixelRatioHasChanged) {
        handleSizeChanged();
    } else if (change == ItemSceneChange) {
        const bool sceneActive = value.window != nullptr;
        d->textureChanged = true;
        syncLottieHostAdapter(value.window);
        if (sceneActive) {
            update();
        }
    }

    QQuickItem::itemChange(change, value);
}

void LottieItem::handleSizeChanged()
{
    syncLottieHostAdapter();
    markTextureChanged();
}

void LottieItem::markTextureChanged()
{
    if (!d) {
        return;
    }

    d->textureChanged = true;
    update();
}

} // namespace UkuiQuick
