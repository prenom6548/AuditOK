/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "island-view.h"

#include "shared-engine-component.h"
#include "island.h"

namespace UkuiQuick {

class IslandViewPrivate
{
public:
    explicit IslandViewPrivate(IslandView *q);

    Island *island {nullptr};
    QQmlContext *rootContext {nullptr};
    WidgetContainerItem *containerItem {nullptr};
    IslandView *q = nullptr;
    IslandView::ResizeMode resizeMode {IslandView::SizeRootObjectToView};

    void onContentItemSizeChanged();
    void disconnectResizeSignal(WidgetContainerItem *item);
    void initResizeSignal();
    bool onMainViewChanged();
};

IslandViewPrivate::IslandViewPrivate(IslandView* q) : q(q)
{
}
void IslandViewPrivate::onContentItemSizeChanged()
{
    if (!containerItem) {
        return;
    }

    // 在 SizeViewToRootObject 模式下，由窗口跟随根项尺寸变化（QQuickViewPrivate::updateSize()）
    if (resizeMode == IslandView::SizeViewToRootObject) {
        QSize newSize;
        // 最小为1x1
        newSize.setWidth(containerItem->width() < 1 ? 1 : (int)containerItem->width());
        newSize.setHeight(containerItem->height() < 1 ? 1 : (int)containerItem->height());

        if (newSize.isValid() && newSize != q->size()) {
            q->resize(newSize);
        }
        return;
    }

    // 在 SizeRootObjectToView 模式下，由根项跟随当前窗口尺寸变化
    const bool needToUpdateWidth = !qFuzzyCompare(q->width(), containerItem->width());
    const bool needToUpdateHeight = !qFuzzyCompare(q->height(), containerItem->height());

    if (needToUpdateWidth && needToUpdateHeight) {
        containerItem->setSize(QSizeF(q->width(), q->height()));
    } else if (needToUpdateWidth) {
        containerItem->setWidth(q->width());
    } else if (needToUpdateHeight) {
        containerItem->setHeight(q->height());
    }
}

void IslandViewPrivate::disconnectResizeSignal(WidgetContainerItem *item)
{
    if (!item) {
        return;
    }

    QObject::disconnect(item, &QQuickItem::widthChanged, q, nullptr);
    QObject::disconnect(item, &QQuickItem::heightChanged, q, nullptr);
}

void IslandViewPrivate::initResizeSignal()
{
    if (!containerItem) {
        return;
    }

    disconnectResizeSignal(containerItem);

    if (resizeMode == IslandView::SizeViewToRootObject) {
        QObject::connect(containerItem, &QQuickItem::widthChanged, q, [this]() {
            onContentItemSizeChanged();
        });
        QObject::connect(containerItem, &QQuickItem::heightChanged, q, [this]() {
            onContentItemSizeChanged();
        });
    }

    onContentItemSizeChanged();
}

bool IslandViewPrivate::onMainViewChanged()
{
    auto cont = island->mainView();
    if (!cont) {
        return false;
    }

    auto contItem = qobject_cast<WidgetContainerItem *>(WidgetQuickItem::loadWidgetItem(cont, rootContext));
    if (!contItem) {
        qWarning() << "The main item must be WidgetContainerItem.";
        return false;
    }

    disconnectResizeSignal(containerItem);

    containerItem = contItem;
    containerItem->setParentItem(q->contentItem());

    initResizeSignal();
    return true;
}

/*!
    \class UkuiQuick::IslandView
    \inmodule UkuiQuickFramework
    \inherits QQuickWindow
    \inheaderfile island-view.h
    \brief  用于加载 \c ContainerItem 的窗口.

    \c IslandView 支持两种尺寸调整模式，可用于控制窗口尺寸与主视图根容器项尺寸之间的同步关系.
*/
IslandView::IslandView(const QString &id, const QString &app) : QQuickWindow(), d(new IslandViewPrivate(this))
{
    // TODO: 信号监听
    d->island = new Island(id, app, this);
    d->rootContext = new QQmlContext(SharedEngineComponent::sharedEngine(), this);
    d->rootContext->setContextProperty("island", this);

    // 当island指定的主View切换时，同步加载对应的主qml文件
    connect(d->island, &Island::mainViewChanged, this, [this] {
        d->onMainViewChanged();
    });
}

IslandView::~IslandView()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

/*!
    \brief 加载Container的ui.

    \deprecated 使用 \l {loadMainView(const QString &id)} 自动加载.
    \note 若需要分离 \c Container 与 \c island 层，可以从此处加载特定的 \c island 代码，并将 \c Container 嵌入到该对象.
*/
void IslandView::setSource(const QUrl &source)
{
    Q_UNUSED(source)
}

/*!
    \brief mainView UI组件的上下文
    目前的情况是 \c IslandView 拥有一个上下文，用于开发时向 \c ui 提供一些数据，
    \c Container 也有一个上下文，他们两个之间的数据并不互通，为了能让这两个上下文互通，现在 \c IslandView 的上下文会作为 \c Container 的父级存在.
 */
QQmlContext *IslandView::rootContext() const
{
    return d->rootContext;
}

/*!
    \brief 主视图的根容器项.
*/
WidgetContainerItem *IslandView::rootItem() const
{
    return d->containerItem;
}

/*!
    \enum UkuiQuick::IslandView::ResizeMode
    \brief \c IslandView 的尺寸调整模式.

    \value SizeViewToRootObject 窗口尺寸跟随主视图根容器项的尺寸变化.
    \value SizeRootObjectToView 主视图根容器项的尺寸始终与窗口尺寸保持一致.
*/

/*!
    \brief 设置 \c IslandView 的尺寸调整模式.

    当模式为 \c SizeViewToRootObject 时，窗口尺寸会跟随主视图根容器项的尺寸变化；
    当模式为 \c SizeRootObjectToView 时，主视图根容器项的尺寸始终与窗口尺寸保持一致.
*/
void IslandView::setResizeMode(ResizeMode resizeMode)
{
    if (d->resizeMode == resizeMode) {
        return;
    }

    d->resizeMode = resizeMode;
    d->initResizeSignal();
}

/*!
    \brief 获取当前 \c IslandView 的尺寸调整模式.
*/
IslandView::ResizeMode IslandView::resizeMode() const
{
    return d->resizeMode;
}

/*!
    \brief 通过插件 \c id 加载对应的主视图到当前 \c IslandView 中.

    \sa Island::loadMainView
*/
bool IslandView::loadMainView(const QString &id)
{
    return d->island->loadMainView(id);
}

/*!
    \brief 通过插件 \c id 和 \c instanceId 加载对应的主视图到当前 \c IslandView 中.

    \sa Island::loadMainView
*/
bool IslandView::loadMainView(const QString& id, int instanceId)
{
    return d->island->loadMainView(id, instanceId);
}

/*!
    \brief 在搜索路径中自动寻找该插件并加载,不加载 \c UI,配合 \c {loadMainViewItem()} 使用.

    \sa Island::loadMainView
*/
bool IslandView::loadMainViewWithoutItem(const QString &id)
{
    return d->island->loadMainView(id, false);
}
/*!
    \brief 在搜索路径中自动寻找该插件并加载,不加载 \c UI,配合 \c {loadMainViewItem()} 使用.

    \sa Island::loadMainView
*/
bool IslandView::loadMainViewWithoutItem(const QString& id, int instanceId)
{
    return d->island->loadMainView(id, instanceId, false);
}

/*!
    \brief 加载当前 \c mainview 的 \c ui .
*/
bool IslandView::loadMainViewItem()
{
    return d->onMainViewChanged();
}

/*!
    \brief 设置一个 \c Container 作为根 \c View ，一般用作异常情况调用.

    \sa Island::setMainView
*/
void IslandView::setMainView(WidgetContainer *container)
{
    d->island->setMainView(container);
}

/*!
    \brief 获取当前 \c mainview .

    \sa Island::mainView
*/
WidgetContainer *IslandView::mainView() const
{
    return d->island->mainView();
}

/*!
    \brief 获取当前 \c IslandView 的配置.

    \sa Island::config
*/
Config *IslandView::config() const
{
    return d->island->config();
}

/*!
    \brief 获取当前 \c IslandView 关联的 \c Island 对象.

    \sa Island::island
*/
Island *IslandView::island() const
{
    return d->island;
}

void IslandView::resizeEvent(QResizeEvent *event)
{
    // 同步根项尺寸，再交给 QQuickWindow 更新 contentItem 尺寸。(QQuickView::resizeEvent())
    if (d->resizeMode == SizeRootObjectToView) {
        d->onContentItemSizeChanged();
    }

    QQuickWindow::resizeEvent(event);
}
} // UkuiQuick
