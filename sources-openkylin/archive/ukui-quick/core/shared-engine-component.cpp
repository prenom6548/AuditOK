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

#include "shared-engine-component.h"
#include "shared-qml-engine.h"

#include <QQmlComponent>

namespace UkuiQuick {

class SharedEngineComponentPrivate
{
public:
    // raw
    QUrl source;
    SharedEngineComponent *q {nullptr};

    // env
    QVariantMap initData;
    QQmlComponent *component {nullptr};
    QQmlContext *rootContext {nullptr};

    // generate
    QObject *rootObject {nullptr};

    // func
    explicit SharedEngineComponentPrivate(SharedEngineComponent *parent);
    void printComponentError() const;
    void beginCreateRootObject();
};

SharedEngineComponentPrivate::SharedEngineComponentPrivate(SharedEngineComponent *parent) : q(parent)
{

}

void SharedEngineComponentPrivate::printComponentError() const
{
    const QList<QQmlError> errorList = component->errors();
    for (const QQmlError &error : errorList) {
        QMessageLogger(error.url().toString().toLatin1().constData(), error.line(), nullptr).warning() << error;
    }
}

void SharedEngineComponentPrivate::beginCreateRootObject()
{
    delete component;
    component = new QQmlComponent(SharedEngineComponent::sharedEngine(), q);
    QObject::connect(component, &QQmlComponent::statusChanged, q, &SharedEngineComponent::componentStatusChanged);

    component->loadUrl(source);
//    rootObject = component->beginCreate(rootContext ? rootContext : SharedEngineComponent::sharedEngine()->rootContext());
    rootObject = component->beginCreate(rootContext);

    if (component->isReady() || component->isError()) {
        q->completedCreateRootObject();
    } else {
        QObject::connect(component, &QQmlComponent::statusChanged, q, &SharedEngineComponent::completedCreateRootObject);
    }
}

/*!
    \class UkuiQuick::SharedEngineComponent
    \inmodule UkuiQuickCore
    \inheaderfile shared-engine-component.h
    \brief 基于共享QML引擎的组件加载器.

    一个Component，用于加载并实例化一个qml文件的顶级对象，该对象为 \c rootObject。
    基本功能由 \l QQmlComponent 提供。所有通过 \l SharedEngineComponent 实例化的 \c object 都运行在同一个共享 \c engine 中。
    每个 \c object 在创建前都需要一个上下文，如果未指定那么会从共享 \c engine 根上下文创建一个。
    可以在构造 \l SharedEngineComponent 时传入初始化参数，用于初始化 \c rootObject 的属性。

    \sa QQmlComponent
 */


// ====== SharedEngineComponent ====== //

/*!
    \fn QQmlEngine* SharedEngineComponent::sharedEngine()
    获取全局共享的QML引擎实例.

    \sa SharedQmlEngine
 */
QQmlEngine *SharedEngineComponent::sharedEngine()
{
    return SharedQmlEngine::sharedQmlEngine();
}
/*!
    \fn SharedEngineComponent::SharedEngineComponent(QQmlContext *rootContext, QObject *parent)
    \brief 构造函数, \a rootContext 为组件的QML上下文, \a parent 为父对象.
*/
SharedEngineComponent::SharedEngineComponent(QQmlContext *rootContext, QObject *parent)
  : QObject(parent),
    d(new SharedEngineComponentPrivate(this))
{
    if (!rootContext) {
        rootContext = new QQmlContext(SharedEngineComponent::sharedEngine());
    }
    d->rootContext = rootContext;
}

/*!
    \fn SharedEngineComponent::SharedEngineComponent(const QVariantMap &initData, QQmlContext *rootContext, QObject *parent)
    \brief 构造函数, \a initData 作为初始化数据, \a rootContext 作为组件的QML上下文、
*/
SharedEngineComponent::SharedEngineComponent(const QVariantMap &initData, QQmlContext *rootContext, QObject *parent)
  : SharedEngineComponent(rootContext, parent)
{
    d->initData = initData;
}

SharedEngineComponent::~SharedEngineComponent()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

/*!
    \fn void SharedEngineComponent::setSource(const QUrl &source)
    \brief 设置组件的QML文件路径, \a source QML文件路径
*/
void SharedEngineComponent::setSource(const QUrl &source)
{
    if (source.isEmpty()) {
        return;
    }

    d->source = source;
    d->beginCreateRootObject();
}

/*!
    \fn QUrl SharedEngineComponent::source() const
    \brief 获取组件的QML文件路径.
*/
QUrl SharedEngineComponent::source() const
{
    return d->source;
}

/*!
    \fn QObject *SharedEngineComponent::rootObject() const
    \brief 获取组件的根对象.
 */
QObject *SharedEngineComponent::rootObject() const
{
    return d->rootObject;
}

/*!
    \fn QQmlComponent *SharedEngineComponent::component() const
    \brief 获取内部管理的QQmlComponent组件实例.
*/
QQmlComponent *SharedEngineComponent::component() const
{
    return d->component;
}

/*!
    \fn QQmlContext *SharedEngineComponent::rootContext() const
    \brief 获取组件的QML上下文.

    \sa QQmlContext
*/
QQmlContext *SharedEngineComponent::rootContext() const
{
    return d->rootContext;
}

/*!
    \fn QVariantMap SharedEngineComponent::initializeData() const
    \brief 获取组件的初始化数据.
*/
QVariantMap SharedEngineComponent::initializeData() const
{
    return d->initData;
}

void SharedEngineComponent::completedCreateRootObject()
{
    if (!d->component) {
        qWarning() << "SharedEngineComponent: Component is null, source:" << d->source;
        return;
    }

    disconnect(d->component, &QQmlComponent::statusChanged, this, &SharedEngineComponent::completedCreateRootObject);
    if (!d->component->isReady() || d->component->isError()) {
        d->printComponentError();
        return;
    }

    QVariantMap::const_iterator iterator = d->initData.constBegin();
    for (; iterator != d->initData.constEnd() ; ++iterator) {
        d->rootObject->setProperty(iterator.key().toUtf8().data(), iterator.value());
    }

    d->component->completeCreate();
}
/*!
    \fn void SharedEngineComponent::componentStatusChanged(QQmlComponent::Status status)
    \brief 此信号在组件状态发生变化时触发，\a status 为组件的新状态.
*/
} // UkuiQuick
