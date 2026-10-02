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
****************************************************************************/

#include "ukui-flow.h"

#include <qqmlinfo.h>
static const QQuickItemPrivate::ChangeTypes watchedChanges
    = QQuickItemPrivate::Geometry
    | QQuickItemPrivate::SiblingOrder
    | QQuickItemPrivate::Visibility
    | QQuickItemPrivate::Destroyed;

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
    if (!isComponentComplete())
        return;

    if (d->doingPositioning)
        return;

    d->positioningDirty = false;
    d->doingPositioning = true;
    //Need to order children by creation order modified by stacking order
    QList<QQuickItem *> children = childItems();

    QPODVector<PositionedItem,8> oldItems;
    positionedItems.copyAndClear(oldItems);
    for (int ii = 0; ii < unpositionedItems.count(); ii++)
        oldItems.append(unpositionedItems[ii]);
    unpositionedItems.clear();
    int addedIndex = -1;

    for (int ii = 0; ii < children.count(); ++ii) {
        QQuickItem *child = children.at(ii);
        if (QQuickItemPrivate::get(child)->isTransparentForPositioner())
            continue;
        QQuickItemPrivate *childPrivate = QQuickItemPrivate::get(child);
        PositionedItem posItem(child);
        int wIdx = oldItems.find(posItem);
        if (wIdx < 0) {
            d->watchChanges(child);
            posItem.isNew = true;
            if (!childPrivate->explicitVisible || !child->width() || !child->height()) {
                posItem.isVisible = false;
                posItem.index = -1;
                unpositionedItems.append(posItem);
            } else {
                posItem.index = positionedItems.count();
                positionedItems.append(posItem);

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
            // Items are only omitted from positioning if they are explicitly hidden
            // i.e. their positioning is not affected if an ancestor is hidden.
            if (!childPrivate->explicitVisible || !child->width() || !child->height()) {
                item->isVisible = false;
                item->index = -1;
                unpositionedItems.append(*item);
            } else if (!item->isVisible) {
                // item changed from non-visible to visible, treat it as a "new" item
                item->isVisible = true;
                item->isNew = true;
                item->index = positionedItems.count();
                positionedItems.append(*item);

                if (d->transitioner) {
                    if (addedIndex < 0)
                        addedIndex = item->index;
                    positionedItems[positionedItems.count()-1].transitionNextReposition(d->transitioner, QQuickItemViewTransitioner::AddTransition, true);
                }
            } else {
                item->isNew = false;
                item->index = positionedItems.count();
                positionedItems.append(*item);
            }
        }
    }

    if (d->transitioner) {
        for (int i=0; i<positionedItems.count(); i++) {
            if (!positionedItems[i].isNew) {
                if (addedIndex >= 0) {
                    positionedItems[i].transitionNextReposition(d->transitioner, QQuickItemViewTransitioner::AddTransition, false);
                } else {
                    // just queue the item for a move-type displace - if the item hasn't
                    // moved anywhere, it won't be transitioned anyway
                    positionedItems[i].transitionNextReposition(d->transitioner, QQuickItemViewTransitioner::MoveTransition, false);
                }
            }
        }
    }

    QSizeF contentSize(0,0);
    reportConflictingAnchors();
    if (!d->anchorConflict) {
        doPositioning(&contentSize);
        updateAttachedProperties();
    }

    if (d->transitioner) {
        QRectF viewBounds(QPointF(), contentSize);
        for (int i=0; i<positionedItems.count(); i++)
            positionedItems[i].prepareTransition(d->transitioner, viewBounds);
        for (int i=0; i<positionedItems.count(); i++)
            positionedItems[i].startTransition(d->transitioner);
        d->transitioner->resetTargetLists();
    }

    d->doingPositioning = false;

    //Set implicit size to the size of its children
    setImplicitSize(contentSize.width(), contentSize.height());

    emit positioningComplete();
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

class UkuiFlowPrivate : public UkuiBasePositionerPrivate
{
    Q_DECLARE_PUBLIC(UkuiFlow)

public:
    UkuiFlowPrivate()
        : UkuiBasePositionerPrivate(), flow(UkuiFlow::LeftToRight), lineHorizontalCenterAlignment(false)
    {}

    void effectiveLayoutDirectionChange() override
    {
        Q_Q(UkuiFlow);
        // Don't postpone, as it might be the only trigger for visible changes.
        q->prePositioning();
        emit q->effectiveLayoutDirectionChanged();
    }

    UkuiFlow::Flow flow;
    bool lineHorizontalCenterAlignment;
    QHash<int, QPointF> posCache;
};

UkuiFlow::UkuiFlow(QQuickItem *parent)
: UkuiBasePositioner(*(new UkuiFlowPrivate), Both, parent)
{
    Q_D(UkuiFlow);
    // Flow layout requires relayout if its own size changes too.
    d->addItemChangeListener(d, QQuickItemPrivate::Geometry);
}

UkuiFlow::Flow UkuiFlow::flow() const
{
    Q_D(const UkuiFlow);
    return d->flow;
}

void UkuiFlow::setFlow(Flow flow)
{
    Q_D(UkuiFlow);
    if (d->flow != flow) {
        d->flow = flow;
        prePositioning();
        emit flowChanged();
    }
}

Qt::LayoutDirection UkuiFlow::layoutDirection() const
{
    Q_D(const UkuiFlow);
    return d->layoutDirection;
}

void UkuiFlow::setLayoutDirection(Qt::LayoutDirection layoutDirection)
{
    Q_D(UkuiFlow);
    if (d->layoutDirection != layoutDirection) {
        d->layoutDirection = layoutDirection;
        emit layoutDirectionChanged();
        d->effectiveLayoutDirectionChange();
    }
}

Qt::LayoutDirection UkuiFlow::effectiveLayoutDirection() const
{
    return UkuiBasePositionerPrivate::getEffectiveLayoutDirection(this);
}

bool UkuiFlow::lineHorizontalCenterAlignment() const
{
    Q_D(const UkuiFlow);
    return d->lineHorizontalCenterAlignment;
}

void UkuiFlow::setLineHorizontalCenterAlignment(bool lineHorizontalCenterAlignment)
{
    Q_D(UkuiFlow);
    if (d->lineHorizontalCenterAlignment != lineHorizontalCenterAlignment) {
        d->lineHorizontalCenterAlignment = lineHorizontalCenterAlignment;
        d->setPositioningDirty();
        emit lineHorizontalCenterAlignmentChanged();
    }
}

int UkuiFlow::moveCurrentIndexLeft(int currentIndex)
{
    if (effectiveLayoutDirection() == Qt::LayoutDirection::LeftToRight) {
        if (currentIndex > 0) {
            return currentIndex - 1;
        } else {
            return positionedItems.count() - 1;
        }
    } else {
        if (currentIndex < positionedItems.count() - 1) {
            return currentIndex + 1;
        } else {
            return 0;
        }
    }
}

int UkuiFlow::moveCurrentIndexRight(int currentIndex)
{
    if (effectiveLayoutDirection() == Qt::LayoutDirection::LeftToRight) {
        if (currentIndex < positionedItems.count() - 1) {
            return currentIndex + 1;
        } else {
            return 0;
        }
    } else {
        if (currentIndex > 0) {
            return currentIndex - 1;
        } else {
            return positionedItems.count() - 1;
        }
    }
}

int UkuiFlow::moveCurrentIndexUp(int currentIndex)
{
    // 只有一行时走焦不移动
    if (positionedItems[0].itemY() == positionedItems[positionedItems.count() - 1].itemY()) {
        return currentIndex;
    }

    PositionedItem &currentChild = positionedItems[currentIndex];
    qreal currentRowY = currentChild.itemY();
    int upRowStartIndex = 0;
    int upRowEndIndex = positionedItems.count() - 1;

    // 计算当前获取键盘焦点的窗口需要移动至哪一行
    if (currentChild.itemY() == positionedItems[0].itemY()) {
        // 当前键盘焦点窗口位于第一行
        currentRowY = positionedItems[positionedItems.count() - 1].itemY();
        upRowEndIndex = positionedItems.count() - 1;
    } else {
        for (int i = positionedItems.count() - 1; i >= 0; i--) {
            if (positionedItems[i].itemY() < currentChild.itemY()) {
                currentRowY = positionedItems[i].itemY();
                upRowEndIndex = i;
                break;
            }
        }
    }

    for (int i = upRowEndIndex; i >= 0; i--) {
        if (positionedItems[i].itemY() < currentRowY) {
            upRowStartIndex = i + 1;
            break;
        }
    }

    // 查找应该跳转到上一行的哪个窗口
    return findUpDownItemIndex(currentIndex, upRowStartIndex, upRowEndIndex);
}

int UkuiFlow::moveCurrentIndexDown(int currentIndex)
{
    // 只有一行时走焦不移动
    if (positionedItems[0].itemY() == positionedItems[positionedItems.count() - 1].itemY()) {
        return currentIndex;
    }

    PositionedItem &currentChild = positionedItems[currentIndex];
    qreal currentRowY = currentChild.itemY();
    int downRowStartIndex = 0;
    int downRowEndIndex = positionedItems.count() - 1;

    // 计算当前获取键盘焦点的窗口需要移动至哪一行
    if (currentChild.itemY() == positionedItems[positionedItems.count() - 1].itemY()) {
        // 当前键盘焦点窗口位于最后一行
        currentRowY = positionedItems[0].itemY();
        downRowStartIndex = 0;
    } else {
        for (int i = 0; i < positionedItems.count(); i++) {
            if (positionedItems[i].itemY() > currentChild.itemY()) {
                currentRowY = positionedItems[i].itemY();
                downRowStartIndex = i;
                break;
            }
        }
    }

    for (int i = downRowStartIndex; i < positionedItems.count(); i++) {
        if (positionedItems[i].itemY() > currentRowY) {
            downRowEndIndex = i - 1;
            break;
        }
    }

    // 查找应该跳转到下一行的哪个窗口
    return findUpDownItemIndex(currentIndex, downRowStartIndex, downRowEndIndex);
}

int UkuiFlow::findUpDownItemIndex(int currentIndex, int startIndex, int endIndex)
{
    PositionedItem &currentChild = positionedItems[currentIndex];
    // 获取当前键盘焦点窗口的leftX rightX
    qreal currentIndexLeftX = currentChild.itemX();
    qreal currentIndexRightX = currentChild.itemX() + currentChild.item->width();

    // 判断应该跳转到上/下一行的哪个窗口
    int resultIndex = startIndex;
    qreal result = 0;
    for (int i = startIndex; i <= endIndex; i++) {
        qreal temp = 0;
        if (effectiveLayoutDirection() == Qt::LayoutDirection::LeftToRight) {
            if ((positionedItems[i].itemX() + positionedItems[i].item->width()) < currentIndexLeftX) {
                resultIndex = i;
                continue;
            }
            if (positionedItems[i].itemX()> currentIndexRightX) {
                break;
            }

        } else {
            if ((positionedItems[i].itemX() + positionedItems[i].item->width()) < currentIndexLeftX) {
                break;
            }
            if (positionedItems[i].itemX()> currentIndexRightX) {
                resultIndex = i;
                continue;
            }
        }

        if (positionedItems[i].itemX() <= currentIndexLeftX) {
            if ((positionedItems[i].itemX() + positionedItems[i].item->width())  <= currentIndexRightX) {
                temp = (positionedItems[i].itemX() + positionedItems[i].item->width())  - currentIndexLeftX;
            } else {
                temp = currentIndexRightX - currentIndexLeftX;
            }
        } else  {
            if ((positionedItems[i].itemX() + positionedItems[i].item->width())  <= currentIndexRightX) {
                temp = positionedItems[i].item->width();
            } else {
                temp = currentIndexRightX - positionedItems[i].itemX();
            }
        }

        if (temp > result) {
            result = temp;
            resultIndex = i;
        }
    }
    return resultIndex;
}

void UkuiFlow::doPositioning(QSizeF *contentSize)
{
    //Precondition: All items in the positioned list have a valid item pointer and should be positioned
    Q_D(UkuiFlow);

    qreal hoffset1 = leftPadding();
    qreal hoffset2 = rightPadding();
    if (!d->isLeftToRight())
        qSwap(hoffset1, hoffset2);
    qreal hoffset = hoffset1;
    const qreal voffset1 = topPadding();
    qreal voffset = voffset1;
    qreal linemax = 0;
    contentSize->setWidth(qMax(contentSize->width(), hoffset1 + hoffset2));
    contentSize->setHeight(qMax(contentSize->height(), voffset + bottomPadding()));

    int lineFirstIndex = 0;
    for (int i = 0; i < positionedItems.count(); ++i) {
        PositionedItem &child = positionedItems[i];

        if (d->flow == LeftToRight)  {
            if (widthValid() && hoffset != hoffset1 && hoffset + child.item->width() + hoffset2 > width()) {
                //居中处理
                if (d->lineHorizontalCenterAlignment) {
                    PositionedItem &lineLastItem = positionedItems[i - 1];
                    qreal moveOffSet = (width() - (d->posCache.value(lineLastItem.index).x() + lineLastItem.item->width() - d->posCache.value(lineFirstIndex).x())) / 2;
                    for (int j = lineFirstIndex; j < i; ++j) {
                        PositionedItem &lineChild = positionedItems[j];
                        qreal posX = d->posCache.value(j).x() + moveOffSet;
                        if (!d->isLeftToRight()) {
                            posX = width() - lineChild.item->width() - posX;
                        }
                        positionItem(posX, d->posCache.value(j).y(), &lineChild);
                        lineChild.updatePadding(leftPadding(), topPadding(), rightPadding(), bottomPadding());
                    }
                    lineFirstIndex = i;
                }
                hoffset = hoffset1;
                voffset += linemax + spacing();
                linemax = 0;
            }
        } else {
            if (heightValid() && voffset != voffset1 && voffset + child.item->height() + bottomPadding() > height()) {
                voffset = voffset1;
                hoffset += linemax + spacing();
                linemax = 0;
            }
        }

        if (d->isLeftToRight()) {
            if(d->lineHorizontalCenterAlignment) {
                d->posCache.insert(child.index, {hoffset, voffset});
            } else {
                positionItem(hoffset, voffset, &child);
            }
        } else {
            if(d->lineHorizontalCenterAlignment) {
                d->posCache.insert(child.index, {hoffset, voffset});
            } else {
                positionItemY(voffset, &child);
                child.topPadding = topPadding();
                child.bottomPadding = bottomPadding();
            }
        }

        contentSize->setWidth(qMax(contentSize->width(), hoffset + child.item->width() + hoffset2));
        contentSize->setHeight(qMax(contentSize->height(), voffset + child.item->height() + bottomPadding()));

        if (d->flow == LeftToRight)  {
            hoffset += child.item->width();
            hoffset += spacing();
            linemax = qMax(linemax, child.item->height());
        } else {
            voffset += child.item->height();
            voffset += spacing();
            linemax = qMax(linemax, child.item->width());
        }
    }
    //最后一行
    if (widthValid() && d->lineHorizontalCenterAlignment && lineFirstIndex < positionedItems.count()) {
        PositionedItem &lastItem = positionedItems[positionedItems.count() - 1];
        if (d->flow == LeftToRight) {
            qreal moveOffSet = (width() - (d->posCache.value(lastItem.index).x() + lastItem.item->width() - d->posCache.value(lineFirstIndex).x())) / 2;
            for (int j = lineFirstIndex; j < positionedItems.count(); ++j) {
                PositionedItem &lineChild = positionedItems[j];
                qreal posX = d->posCache.value(j).x() + moveOffSet;
                if (!d->isLeftToRight()) {
                    posX = width() - lineChild.item->width() - posX;
                }
                positionItem(posX, d->posCache.value(j).y(), &lineChild);
                lineChild.updatePadding(leftPadding(), topPadding(), rightPadding(), bottomPadding());
            }
        }
    }
    if(d->lineHorizontalCenterAlignment && d->flow == LeftToRight) {
        contentSize->setWidth(width());
    }
    d->posCache.clear();
}

void UkuiFlow::reportConflictingAnchors()
{
    Q_D(UkuiFlow);
    for (int ii = 0; ii < positionedItems.count(); ++ii) {
        const PositionedItem &child = positionedItems.at(ii);
        if (child.item) {
            QQuickAnchors *anchors = QQuickItemPrivate::get(static_cast<QQuickItem *>(child.item))->_anchors;
            if (anchors && (anchors->usedAnchors() || anchors->fill() || anchors->centerIn())) {
                d->anchorConflict = true;
                break;
            }
        }
    }
    if (d->anchorConflict)
        qmlWarning(this) << "Cannot specify anchors for items inside Flow." << " Flow will not function.";
}
