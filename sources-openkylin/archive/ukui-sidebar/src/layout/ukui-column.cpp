/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
*  Copyright (C) 2025, KylinSoft Co., Ltd.
*
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** iaom <zhangpengfei@kylinos.cn>
** qiqi49 <qiqi@kylinos.cn>
****************************************************************************/

#include "ukui-column.h"

#include <qparallelanimationgroup.h>
#include <qpropertyanimation.h>
#include <qqmlinfo.h>
static const QQuickItemPrivate::ChangeTypes watchedChanges
    = QQuickItemPrivate::Geometry
    | QQuickItemPrivate::SiblingOrder
    | QQuickItemPrivate::Visibility
    | QQuickItemPrivate::Destroyed;

UkuiScale::UkuiScale(QObject *parent)
    : QQuickTransform(parent)
{
}

UkuiScale::~UkuiScale()
{
}

QVector3D UkuiScale::origin() const
{
    return m_origin;
}
void UkuiScale::setOrigin(const QVector3D &point)
{
    if (m_origin == point)
        return;
    m_origin = point;
    update();
    emit originChanged();
}

qreal UkuiScale::xScale() const
{
    return m_xScale;
}
void UkuiScale::setXScale(qreal scale)
{
    if (m_xScale == scale)
        return;
    m_xScale = scale;
    update();
    emit xScaleChanged();
    emit scaleChanged();
}

qreal UkuiScale::yScale() const
{
    return m_yScale;
}
void UkuiScale::setYScale(qreal scale)
{
    if (m_yScale == scale)
        return;
    m_yScale = scale;
    update();
    emit yScaleChanged();
    emit scaleChanged();
}

qreal UkuiScale::zScale() const
{
    return m_zScale;
}
void UkuiScale::setZScale(qreal scale)
{
    if (m_zScale == scale)
        return;
    m_zScale = scale;
    update();
    emit zScaleChanged();
    emit scaleChanged();
}

void UkuiScale::applyTo(QMatrix4x4 *matrix) const
{
    matrix->translate(m_origin);
    matrix->scale(m_xScale, m_yScale, m_zScale);
    matrix->translate(-m_origin);
}

void UkuiBasePositionerPrivate::watchChanges(QQuickItem *other)
{
    QQuickItemPrivate *otherPrivate = QQuickItemPrivate::get(other);
    otherPrivate->addItemChangeListener(this, watchedChanges);
}

void UkuiBasePositionerPrivate::unwatchChanges(QQuickItem* other)
{
    QQuickItemPrivate *otherPrivate = QQuickItemPrivate::get(other);
    otherPrivate->removeItemChangeListener(this, watchedChanges);
}


UkuiBasePositioner::PositionedItem::PositionedItem(QQuickItem *i)
    : item(i)
    , transitionableItem(nullptr)
    , index(-1)
    , isNew(false)
    , isVisible(true)
    , topPadding(0)
    , leftPadding(0)
    , rightPadding(0)
    , bottomPadding(0)
{
}

UkuiBasePositioner::PositionedItem::~PositionedItem()
{
    delete transitionableItem;
}

qreal UkuiBasePositioner::PositionedItem::itemX() const
{
    return transitionableItem ? transitionableItem->itemX() : item->x();
}

qreal UkuiBasePositioner::PositionedItem::itemY() const
{
    return transitionableItem ? transitionableItem->itemY() : item->y();
}

void UkuiBasePositioner::PositionedItem::moveTo(const QPointF &pos)
{
    if (transitionableItem) {
        transitionableItem->moveTo(pos);
    }
    else
        item->setPosition(pos);
}

void UkuiBasePositioner::PositionedItem::transitionNextReposition(QQuickItemViewTransitioner *transitioner, QQuickItemViewTransitioner::TransitionType type, bool asTarget)
{
    if (!transitioner)
        return;
    if (!transitionableItem)
        transitionableItem = new QQuickItemViewTransitionableItem(item);
    transitioner->transitionNextReposition(transitionableItem, type, asTarget);
}

bool UkuiBasePositioner::PositionedItem::prepareTransition(QQuickItemViewTransitioner *transitioner, const QRectF &viewBounds)
{
    return transitionableItem ? transitionableItem->prepareTransition(transitioner, index, viewBounds) : false;
}

void UkuiBasePositioner::PositionedItem::startTransition(QQuickItemViewTransitioner *transitioner)
{
    if (transitionableItem)
        transitionableItem->startTransition(transitioner, index);
}

void UkuiBasePositioner::PositionedItem::updatePadding(qreal lp, qreal tp, qreal rp, qreal bp)
{
    leftPadding = lp;
    topPadding = tp;
    rightPadding = rp;
    bottomPadding = bp;
}

void UkuiBasePositioner::PositionedItem::setScaleTransition(qreal start, qreal end)
{
    hasScaleTransition = true;
    startScale = start;
    endScale = end;
}

void UkuiBasePositioner::PositionedItem::applyCustomTransitions()
{
    if (!item) {
        return;
    }

    if (hasScaleTransition) {
        QPropertyAnimation *anim = new QPropertyAnimation(item, "scale");
        anim->setDuration(300);
        anim->setEasingCurve(QEasingCurve::OutBack);
        anim->setStartValue(startScale);
        anim->setEndValue(endScale);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    hasScaleTransition = false;
    startScale = 0.0;
    endScale = 0.0;
}

void UkuiBasePositioner::PositionedItem::setYScale(qreal scale)
{
    if (!item || yScale == scale) {
        return;
    }
    yScale = scale;
    clearTransformFromItem();

    UkuiScale* scaleTransform = nullptr;
    if (!scaleTransform && yScale != 1.0) {
        scaleTransform = new UkuiScale(item);
        scaleTransform->setOrigin(QVector3D(0, 0, 0));
        scaleTransform->setXScale(1.0);
        scaleTransform->setYScale(yScale);
        addTransformToItem(scaleTransform);
    }
}

void UkuiBasePositioner::PositionedItem::addTransformToItem(QQuickTransform* transform)
{
    if (!item || !transform) {
        return;
    };
    QQmlListProperty<QQuickTransform> prop = item->transform();
    prop.append(&prop, transform);
}

void UkuiBasePositioner::PositionedItem::clearTransformFromItem()
{
    if (!item) {
        return;
    };

    QQmlListProperty<QQuickTransform> prop = item->transform();
    prop.clear(&prop);
}

UkuiBasePositioner::UkuiBasePositioner(PositionerType at, QQuickItem *parent)
    : QQuickImplicitSizeItem(*(new UkuiBasePositionerPrivate), parent)
{
    Q_D(UkuiBasePositioner);
    d->init(at);
}

UkuiBasePositioner::UkuiBasePositioner(UkuiBasePositionerPrivate &dd, PositionerType at, QQuickItem *parent)
    : QQuickImplicitSizeItem(dd, parent)
{
    Q_D(UkuiBasePositioner);
    d->init(at);
}

UkuiBasePositioner::~UkuiBasePositioner()
{
    Q_D(UkuiBasePositioner);
    delete d->transitioner;
    for (int i = 0; i < positionedItems.count(); ++i)
        d->unwatchChanges(positionedItems.at(i).item);
    for (int i = 0; i < unpositionedItems.count(); ++i)
        d->unwatchChanges(unpositionedItems.at(i).item);
    clearPositionedItems(&positionedItems);
    clearPositionedItems(&unpositionedItems);
}

void UkuiBasePositioner::updatePolish()
{
    Q_D(UkuiBasePositioner);
    if (d->positioningDirty)
        prePositioning();
}

qreal UkuiBasePositioner::spacing() const
{
    Q_D(const UkuiBasePositioner);
    return d->spacing;
}

void UkuiBasePositioner::setSpacing(qreal s)
{
    Q_D(UkuiBasePositioner);
    if (s == d->spacing)
        return;
    d->spacing = s;
    d->setPositioningDirty();
    emit spacingChanged();
}

QQuickTransition *UkuiBasePositioner::populate() const
{
    Q_D(const UkuiBasePositioner);
    return d->transitioner ? d->transitioner->populateTransition : nullptr;
}

void UkuiBasePositioner::setPopulate(QQuickTransition *transition)
{
    Q_D(UkuiBasePositioner);
    if (!d->transitioner)
        d->transitioner = new QQuickItemViewTransitioner;
    if (d->transitioner->populateTransition != transition) {
        d->transitioner->populateTransition = transition;
        emit populateChanged();
    }
}

QQuickTransition *UkuiBasePositioner::move() const
{
    Q_D(const UkuiBasePositioner);
    return d->transitioner ? d->transitioner->displacedTransition : nullptr;
}

void UkuiBasePositioner::setMove(QQuickTransition *mt)
{
    Q_D(UkuiBasePositioner);
    if (!d->transitioner)
        d->transitioner = new QQuickItemViewTransitioner;
    if (mt == d->transitioner->displacedTransition)
        return;

    d->transitioner->displacedTransition = mt;
    emit moveChanged();
}

QQuickTransition *UkuiBasePositioner::add() const
{
    Q_D(const UkuiBasePositioner);
    return d->transitioner ? d->transitioner->addTransition : nullptr;
}

void UkuiBasePositioner::setAdd(QQuickTransition *add)
{
    Q_D(UkuiBasePositioner);
    if (!d->transitioner)
        d->transitioner = new QQuickItemViewTransitioner;
    if (add == d->transitioner->addTransition)
        return;

    d->transitioner->addTransition = add;
    emit addChanged();
}

QQuickTransition* UkuiBasePositioner::removeTransition() const
{
    Q_D(const UkuiBasePositioner);
    return d->transitioner ? d->transitioner->removeTransition : nullptr;
}

void UkuiBasePositioner::setRemoveTransition(QQuickTransition* transition)
{
    Q_D(UkuiBasePositioner);
    if (!d->transitioner) {
        d->transitioner = new QQuickItemViewTransitioner;
        // d->transitioner->setChangeListener(this); // TODO
    }
    if (d->transitioner->removeTransition != transition) {
        d->transitioner->removeTransition = transition;
        emit removeTransitionChanged();
    }
}

void UkuiBasePositioner::componentComplete()
{
    Q_D(UkuiBasePositioner);
    QQuickItem::componentComplete();
    if (d->transitioner)
        d->transitioner->setPopulateTransitionEnabled(true);
    positionedItems.reserve(childItems().count());
    prePositioning();
    if (d->transitioner)
        d->transitioner->setPopulateTransitionEnabled(false);
}

void UkuiBasePositioner::itemChange(ItemChange change, const ItemChangeData &value)
{
    Q_D(UkuiBasePositioner);
    if (change == ItemChildAddedChange) {
        d->setPositioningDirty();
    } else if (change == ItemChildRemovedChange) {
        QQuickItem *child = value.item;
        UkuiBasePositioner::PositionedItem posItem(child);
        int idx = positionedItems.find(posItem);
        if (idx >= 0) {
            d->unwatchChanges(child);
            removePositionedItem(&positionedItems, idx);
        } else if ((idx = unpositionedItems.find(posItem)) >= 0) {
            d->unwatchChanges(child);
            removePositionedItem(&unpositionedItems, idx);
        }
        d->setPositioningDirty();
    }

    QQuickItem::itemChange(change, value);
}

void UkuiBasePositioner::forceLayout()
{
    updatePolish();
}

void UkuiBasePositioner::prePositioning()
{
    Q_D(UkuiBasePositioner);
    // 1. 前置检查：确保组件已完成初始化且没有重入问题
    if (!isComponentComplete())
        return;

    if (d->doingPositioning)
        return;

    // 2. 状态标记：标记布局开始，清除脏标记
    d->positioningDirty = false;
    d->doingPositioning = true;

    // ===== 记录动画起始状态 =====
    for (int i = 0; i < positionedItems.count(); ++i) {
        PositionedItem& item = positionedItems[i];
        if (item.item) {
            item.startScale = item.item->scale();
        }
    }

    // 3. 获取子项列表（按创建/堆叠顺序）
    QList<QQuickItem *> children = childItems();

    // 4. 备份旧项数据：保存当前定位项和未定位项
    QPODVector<PositionedItem,8> oldItems;
    positionedItems.copyAndClear(oldItems);
    for (int ii = 0; ii < unpositionedItems.count(); ii++)
        oldItems.append(unpositionedItems[ii]);
    unpositionedItems.clear();

    int addedIndex = -1;  // 跟踪首个新增项的索引

    // 5. 遍历所有子项，构建新的定位列表
    for (int ii = 0; ii < children.count(); ++ii) {
        QQuickItem *child = children.at(ii);
        // 跳过透明项（不参与布局的项）
        if (QQuickItemPrivate::get(child)->isTransparentForPositioner())
            continue;

        QQuickItemPrivate *childPrivate = QQuickItemPrivate::get(child);
        PositionedItem posItem(child);
        int wIdx = oldItems.find(posItem);

        // 6. 处理新出现的项
        if (wIdx < 0) {
            d->watchChanges(child);
            posItem.isNew = true;

            // 6.1 无效项处理（不可见或尺寸为0）
            if (!childPrivate->explicitVisible || !child->width() || !child->height()) {
                posItem.isVisible = false;
                posItem.index = -1;
                unpositionedItems.append(posItem);  // 加入unpositionedItems未定位列表
            } else {
                // 6.2 有效新项处理
                posItem.index = positionedItems.count();
                positionedItems.append(posItem);  // 加入positionedItems定位列表

                // 处理动画过渡
                if (d->transitioner) {
                    if (addedIndex < 0)
                        addedIndex = posItem.index;
                    PositionedItem *theItem = &positionedItems[positionedItems.count()-1];
                    if (d->transitioner->canTransition(QQuickItemViewTransitioner::PopulateTransition, true))
                        theItem->transitionNextReposition(d->transitioner, QQuickItemViewTransitioner::PopulateTransition, true);
                    else if (!d->transitioner->populateTransitionEnabled())
                        theItem->transitionNextReposition(d->transitioner, QQuickItemViewTransitioner::AddTransition, true);
                }
            }
        } else {
            PositionedItem *item = &oldItems[wIdx];

            // 7.1 无效项处理（尺寸为0）
            if (!child->width() || !child->height()) {
                item->isVisible = false;
                item->index = -1;
                unpositionedItems.append(*item);
            } else if (!item->isVisible) {
                // item changed from non-visible to visible, treat it as a "new" item
                item->isVisible = childPrivate->explicitVisible;
                item->isNew = false;
                item->index = positionedItems.count();
                positionedItems.append(*item);

                // 处理添加动画
                if (d->transitioner) {
                    if (addedIndex < 0)
                        addedIndex = item->index;
                    positionedItems[positionedItems.count()-1].transitionNextReposition(d->transitioner, QQuickItemViewTransitioner::AddTransition, true);
                }
            } else {
                // 7.3 常规存在的项
                item->isNew = false;
                item->index = positionedItems.count();
                positionedItems.append(*item);
            }
        }
    }

    if (d->transitioner) {
        for (int i = 0; i < positionedItems.count(); i++) {
            if (positionedItems[i].isNew) {
                positionedItems[i].transitionNextReposition(d->transitioner, QQuickItemViewTransitioner::AddTransition, true);
            } else {
                positionedItems[i].transitionNextReposition(d->transitioner, QQuickItemViewTransitioner::MoveTransition, false);
            }
        }
    }

    // 9. 执行实际布局
    QSizeF contentSize(0,0);
    reportConflictingAnchors();
    if (!d->anchorConflict) {
        doPositioning(&contentSize);
        updateAttachedProperties();
    }

    // 10. 处理过渡动画的启动
    if (d->transitioner) {
        QRectF viewBounds(QPointF(), contentSize);
        // 准备过渡
        for (int i=0; i<positionedItems.count(); i++)
            positionedItems[i].prepareTransition(d->transitioner, viewBounds);
        // 启动过渡
        for (int i=0; i<positionedItems.count(); i++)
            positionedItems[i].startTransition(d->transitioner);
        d->transitioner->resetTargetLists();
    }

    // ===== 应用自定义Scale动画 =====
    for (int i = 0; i < positionedItems.count(); i++) {
        PositionedItem&pItem = positionedItems[i];
        QQuickItem* item = pItem.item;
        // 缩放动画（折叠模式特有）
        if (qAbs(pItem.startScale - item->scale()) > 0.01) {
            pItem.setScaleTransition(pItem.startScale, item->scale());
        }
        pItem.applyCustomTransitions();
    }

    d->doingPositioning = false;
    setImplicitSize(contentSize.width(), contentSize.height());  // 设置隐式尺寸
    emit positioningComplete();  // 发出布局完成信号
}

void UkuiBasePositioner::positionItem(qreal x, qreal y, PositionedItem *target)
{
    if ( target->itemX() != x || target->itemY() != y )
        target->moveTo(QPointF(x, y));
}

void UkuiBasePositioner::positionItemX(qreal x, PositionedItem *target)
{
    Q_D(UkuiBasePositioner);
    if (target->itemX() != x
            && (d->type == Horizontal || d->type == Both)) {
        target->moveTo(QPointF(x, target->itemY()));
    }
}

void UkuiBasePositioner::positionItemY(qreal y, PositionedItem *target)
{
    Q_D(UkuiBasePositioner);
    if (target->itemY() != y
            && (d->type == Vertical || d->type == Both)) {
        target->moveTo(QPointF(target->itemX(), y));
    }
}

/*
  Since PositionedItem values are stored by value, their internal transitionableItem pointers
  must be cleaned up when a PositionedItem is removed from a QPODVector, otherwise the pointer
  is never deleted since QPODVector doesn't invoke the destructor.
  */
void UkuiBasePositioner::removePositionedItem(QPODVector<PositionedItem,8> *items, int index)
{
    Q_ASSERT(index >= 0 && index < items->count());
    delete items->at(index).transitionableItem;
    items->remove(index);
}
void UkuiBasePositioner::clearPositionedItems(QPODVector<PositionedItem,8> *items)
{
    for (int i=0; i<items->count(); i++)
        delete items->at(i).transitionableItem;
    items->clear();
}

UkuiPositionerAttached *UkuiBasePositioner::qmlAttachedProperties(QObject *obj)
{
    return new UkuiPositionerAttached(obj);
}

void UkuiBasePositioner::updateAttachedProperties(UkuiPositionerAttached *specificProperty, QQuickItem *specificPropertyOwner) const
{
    // If this function is deemed too expensive or shows up in profiles, it could
    // be changed to run only when there are attached properties present. This
    // could be a flag in the positioner that is set by the attached property
    // constructor.
    UkuiPositionerAttached *prevLastProperty = nullptr;
    UkuiPositionerAttached *lastProperty = nullptr;

    for (int ii = 0; ii < positionedItems.count(); ++ii) {
        const PositionedItem &child = positionedItems.at(ii);
        if (!child.item)
            continue;

        UkuiPositionerAttached *property = nullptr;

        if (specificProperty) {
            if (specificPropertyOwner == child.item) {
                property = specificProperty;
            }
        } else {
            property = static_cast<UkuiPositionerAttached *>(qmlAttachedPropertiesObject<UkuiBasePositioner>(child.item, false));
        }

        if (property) {
            property->setIndex(ii);
            property->setIsFirstItem(ii == 0);

            if (property->isLastItem()) {
                if (prevLastProperty)
                    prevLastProperty->setIsLastItem(false); // there can be only one last property
                prevLastProperty = property;
            }
        }

        lastProperty = property;
    }

    if (prevLastProperty && prevLastProperty != lastProperty)
        prevLastProperty->setIsLastItem(false);
    if (lastProperty)
        lastProperty->setIsLastItem(true);

    // clear attached properties for unpositioned items
    for (int ii = 0; ii < unpositionedItems.count(); ++ii) {
        const PositionedItem &child = unpositionedItems.at(ii);
        if (!child.item)
            continue;

        UkuiPositionerAttached *property = nullptr;

        if (specificProperty) {
            if (specificPropertyOwner == child.item) {
                property = specificProperty;
            }
        } else {
            property = static_cast<UkuiPositionerAttached *>(qmlAttachedPropertiesObject<UkuiBasePositioner>(child.item, false));
        }

        if (property) {
            property->setIndex(-1);
            property->setIsFirstItem(false);
            property->setIsLastItem(false);
        }
    }
}

qreal UkuiBasePositioner::padding() const
{
    Q_D(const UkuiBasePositioner);
    return d->padding();
}

void UkuiBasePositioner::setPadding(qreal padding)
{
    Q_D(UkuiBasePositioner);
    if (qFuzzyCompare(d->padding(), padding))
        return;

    d->extra.value().padding = padding;
    d->setPositioningDirty();
    emit paddingChanged();
    if (!d->extra.isAllocated() || !d->extra->explicitTopPadding)
        emit topPaddingChanged();
    if (!d->extra.isAllocated() || !d->extra->explicitLeftPadding)
        emit leftPaddingChanged();
    if (!d->extra.isAllocated() || !d->extra->explicitRightPadding)
        emit rightPaddingChanged();
    if (!d->extra.isAllocated() || !d->extra->explicitBottomPadding)
        emit bottomPaddingChanged();
}

void UkuiBasePositioner::resetPadding()
{
    setPadding(0);
}

qreal UkuiBasePositioner::topPadding() const
{
    Q_D(const UkuiBasePositioner);
    if (d->extra.isAllocated() && d->extra->explicitTopPadding)
        return d->extra->topPadding;
    return d->padding();
}

void UkuiBasePositioner::setTopPadding(qreal padding)
{
    Q_D(UkuiBasePositioner);
    d->setTopPadding(padding);
}

void UkuiBasePositioner::resetTopPadding()
{
    Q_D(UkuiBasePositioner);
    d->setTopPadding(0, true);
}

qreal UkuiBasePositioner::leftPadding() const
{
    Q_D(const UkuiBasePositioner);
    if (d->extra.isAllocated() && d->extra->explicitLeftPadding)
        return d->extra->leftPadding;
    return d->padding();
}

void UkuiBasePositioner::setLeftPadding(qreal padding)
{
    Q_D(UkuiBasePositioner);
    d->setLeftPadding(padding);
}

void UkuiBasePositioner::resetLeftPadding()
{
    Q_D(UkuiBasePositioner);
    d->setLeftPadding(0, true);
}

qreal UkuiBasePositioner::rightPadding() const
{
    Q_D(const UkuiBasePositioner);
    if (d->extra.isAllocated() && d->extra->explicitRightPadding)
        return d->extra->rightPadding;
    return d->padding();
}

void UkuiBasePositioner::setRightPadding(qreal padding)
{
    Q_D(UkuiBasePositioner);
    d->setRightPadding(padding);
}

void UkuiBasePositioner::resetRightPadding()
{
    Q_D(UkuiBasePositioner);
    d->setRightPadding(0, true);
}

qreal UkuiBasePositioner::bottomPadding() const
{
    Q_D(const UkuiBasePositioner);
    if (d->extra.isAllocated() && d->extra->explicitBottomPadding)
        return d->extra->bottomPadding;
    return d->padding();
}

void UkuiBasePositioner::setBottomPadding(qreal padding)
{
    Q_D(UkuiBasePositioner);
    d->setBottomPadding(padding);
}

void UkuiBasePositioner::resetBottomPadding()
{
    Q_D(UkuiBasePositioner);
    d->setBottomPadding(0, true);
}

UkuiBasePositionerPrivate::ExtraData::ExtraData()
    : padding(0)
    , topPadding(0)
    , leftPadding(0)
    , rightPadding(0)
    , bottomPadding(0)
    , explicitTopPadding(false)
    , explicitLeftPadding(false)
    , explicitRightPadding(false)
    , explicitBottomPadding(false)
{
}

void UkuiBasePositionerPrivate::setTopPadding(qreal value, bool reset)
{
    Q_Q(UkuiBasePositioner);
    qreal oldPadding = q->topPadding();
    if (!reset || extra.isAllocated()) {
        extra.value().topPadding = value;
        extra.value().explicitTopPadding = !reset;
    }
    if ((!reset && !qFuzzyCompare(oldPadding, value)) || (reset && !qFuzzyCompare(oldPadding, padding()))) {
        setPositioningDirty();
        emit q->topPaddingChanged();
    }
}

void UkuiBasePositionerPrivate::setLeftPadding(qreal value, bool reset)
{
    Q_Q(UkuiBasePositioner);
    qreal oldPadding = q->leftPadding();
    if (!reset || extra.isAllocated()) {
        extra.value().leftPadding = value;
        extra.value().explicitLeftPadding = !reset;
    }
    if ((!reset && !qFuzzyCompare(oldPadding, value)) || (reset && !qFuzzyCompare(oldPadding, padding()))) {
        setPositioningDirty();
        emit q->leftPaddingChanged();
    }
}

void UkuiBasePositionerPrivate::setRightPadding(qreal value, bool reset)
{
    Q_Q(UkuiBasePositioner);
    qreal oldPadding = q->rightPadding();
    if (!reset || extra.isAllocated()) {
        extra.value().rightPadding = value;
        extra.value().explicitRightPadding = !reset;
    }
    if ((!reset && !qFuzzyCompare(oldPadding, value)) || (reset && !qFuzzyCompare(oldPadding, padding()))) {
        setPositioningDirty();
        emit q->rightPaddingChanged();
    }
}

void UkuiBasePositionerPrivate::setBottomPadding(qreal value, bool reset)
{
    Q_Q(UkuiBasePositioner);
    qreal oldPadding = q->bottomPadding();
    if (!reset || extra.isAllocated()) {
        extra.value().bottomPadding = value;
        extra.value().explicitBottomPadding = !reset;
    }
    if ((!reset && !qFuzzyCompare(oldPadding, value)) || (reset && !qFuzzyCompare(oldPadding, padding()))) {
        setPositioningDirty();
        emit q->bottomPaddingChanged();
    }
}

UkuiPositionerAttached::UkuiPositionerAttached(QObject *parent) : QObject(parent), m_index(-1), m_isFirstItem(false), m_isLastItem(false)
{
    QQuickItem *attachedItem = qobject_cast<QQuickItem *>(parent);
    if (attachedItem) {
        UkuiBasePositioner *positioner = qobject_cast<UkuiBasePositioner *>(attachedItem->parent());
        if (positioner) {
            positioner->updateAttachedProperties(this, attachedItem);
        }
    }
}

void UkuiPositionerAttached::setIndex(int index)
{
    if (m_index == index)
        return;
    m_index = index;
    emit indexChanged();
}

void UkuiPositionerAttached::setIsFirstItem(bool isFirstItem)
{
    if (m_isFirstItem == isFirstItem)
        return;
    m_isFirstItem = isFirstItem;
    emit isFirstItemChanged();
}

void UkuiPositionerAttached::setIsLastItem(bool isLastItem)
{
    if (m_isLastItem == isLastItem)
        return;
    m_isLastItem = isLastItem;
    emit isLastItemChanged();
}
UkuiColumn::UkuiColumn(QQuickItem *parent)
: UkuiBasePositioner(Vertical, parent)
{
}

void UkuiColumn::setFoldMode(bool isFoldMode)
{
    if (m_isFoldMode == isFoldMode) {
        return;
    }

    m_isFoldMode = isFoldMode;
    UkuiBasePositionerPrivate *d = static_cast<UkuiBasePositionerPrivate*>(UkuiBasePositionerPrivate::get(this));
    d->setPositioningDirty();
    Q_EMIT foldModeChanged();
}

bool UkuiColumn::isFoldMode() const
{
    return m_isFoldMode;
}

int UkuiColumn::heightOffset() const
{
    return m_heightOffset;
}

void UkuiColumn::doPositioning(QSizeF *contentSize)
{
    if (m_isFoldMode) {
        // 1. 初始化变量
        qreal voffset = topPadding();
        const qreal padding = leftPadding() + rightPadding();
        const qreal foldHeight = 8; // 折叠消息露出的高度
        contentSize->setWidth(qMax(contentSize->width(), padding));

        qreal maxHeight = 0; // 记录最新消息的高度
        QRect secondItemRect = QRect(0, 0, 0, 0); // 存储第二项的位置（用于放置后续项）

        // 2. 遍历
        int totalItems = positionedItems.count();
        for (int ii = 0; ii < totalItems; ++ii) {
            PositionedItem &child = positionedItems[ii];
            child.item->setVisible(true);
            // 3. 处理前三个项
            if (ii < 3) {
                if (ii == 0) {
                    // 3.1 最新消息（不缩放）
                    maxHeight = child.item->height();
                    child.item->setScale(1.0);
                    child.setYScale(1.0);

                    child.item->setZ(3);
                    positionItem(child.itemX() + leftPadding() - child.leftPadding, voffset, &child);
                    child.updatePadding(leftPadding(), topPadding(), rightPadding(), bottomPadding());

                    contentSize->setWidth(qMax(contentSize->width(), child.item->width() + padding));
                    voffset += child.item->height();
                    m_heightOffset = 0;
                } else {
                    // 3.2 折叠消息（缩放效果）
                    qreal baseScale = (ii == 1) ? 0.95 : 0.9;
                    qreal scaledHeight = child.item->height() * baseScale;
                    qreal yScale = 1.0;
                    // 如果缩放后高度仍大于第一条高度，则进行二次缩放
                    if ((scaledHeight > maxHeight) && maxHeight > 0) {
                        yScale = maxHeight / scaledHeight;
                        scaledHeight = maxHeight;
                    }
                    child.item->setScale(baseScale);
                    child.setYScale(yScale);

                    qreal yPos = voffset - (scaledHeight - foldHeight);
                    qreal xPos = child.itemX() + leftPadding() - child.leftPadding;

                    int z = ii == 1 ? 2 : 1;
                    child.item->setZ(z);
                    child.item->setTransformOrigin(QQuickItem::Top);
                    positionItem(xPos, yPos, &child);

                    // 更新内容尺寸（考虑缩放）
                    qreal scaledWidth = child.item->width() * baseScale;
                    contentSize->setWidth(qMax(contentSize->width(), scaledWidth + padding));
                    // 记录第二项位置，用于完全折叠项position
                    if (ii == 1) {
                        secondItemRect = QRect(xPos, yPos, scaledWidth, scaledHeight);
                    }
                    // 垂直偏移增加（只增加露出的8像素）
                    voffset += foldHeight;
                    m_heightOffset = yPos + child.item->height() - maxHeight - (foldHeight * ii);
                }

            } else {
                // 4. 处理index≥3的项（放在第三项位置并完全覆盖）
                qreal baseScale = 0.9;
                qreal scaledHeight = child.item->height() * baseScale;
                qreal yScale = 1.0;
                // 如果缩放后高度仍大于第一条高度，则进行二次缩放
                if ((scaledHeight > secondItemRect.height()) && secondItemRect.height() > 0) {
                    yScale = secondItemRect.height() / scaledHeight;
                    scaledHeight = secondItemRect.height();
                }
                child.item->setScale(baseScale);
                child.setYScale(yScale);

                child.item->setZ(0);
                child.item->setTransformOrigin(QQuickItem::Top);
                child.item->setTransformOrigin(QQuickItem::Top);
                positionItem(secondItemRect.x(), secondItemRect.y(), &child);
            }
        }
        contentSize->setHeight(voffset + bottomPadding());
    } else {
        //Precondition: All items in the positioned list have a valid item pointer and should be positioned
        qreal voffset = topPadding();
        const qreal padding = leftPadding() + rightPadding();
        contentSize->setWidth(qMax(contentSize->width(), padding));

        for (int ii = 0; ii < positionedItems.count(); ++ii) {
            PositionedItem &child = positionedItems[ii];
            child.item->setVisible(true);
            child.item->setScale(1.0);
            child.setYScale(1.0);

            positionItem(child.itemX() + leftPadding() - child.leftPadding, voffset, &child);
            child.updatePadding(leftPadding(), topPadding(), rightPadding(), bottomPadding());
            contentSize->setWidth(qMax(contentSize->width(), child.item->width() + padding));

            voffset += child.item->height();
            voffset += spacing();
        }

        if (voffset - topPadding() != 0)//If we positioned any items, undo the spacing from the last item
            voffset -= spacing();
        m_heightOffset = 0;
        contentSize->setHeight(voffset + bottomPadding());
    }
    Q_EMIT heightOffsetChanged();
}

void UkuiColumn::reportConflictingAnchors()
{
    UkuiBasePositionerPrivate *d = static_cast<UkuiBasePositionerPrivate*>(UkuiBasePositionerPrivate::get(this));
    for (int ii = 0; ii < positionedItems.count(); ++ii) {
        const PositionedItem &child = positionedItems.at(ii);
        if (child.item) {
            QQuickAnchors *anchors = QQuickItemPrivate::get(static_cast<QQuickItem *>(child.item))->_anchors;
            if (anchors) {
                QQuickAnchors::Anchors usedAnchors = anchors->usedAnchors();
                if (usedAnchors & QQuickAnchors::TopAnchor ||
                    usedAnchors & QQuickAnchors::BottomAnchor ||
                    usedAnchors & QQuickAnchors::VCenterAnchor ||
                    anchors->fill() || anchors->centerIn()) {
                    d->anchorConflict = true;
                    break;
                }
            }
        }
    }
    if (d->anchorConflict) {
        qmlWarning(this) << "Cannot specify top, bottom, verticalCenter, fill or centerIn anchors for items inside Column."
            << " Column will not function.";
    }
}
