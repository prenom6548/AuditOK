/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
*  Copyright (C) 2025, KylinSoft Co., Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

#ifndef UKUI_FLOW_H
#define UKUI_FLOW_H

#include <private/qquickimplicitsizeitem_p.h>
#include <private/qquickitemviewtransition_p.h>
#include <private/qpodvector_p.h>
#include <private/qquickimplicitsizeitem_p_p.h>
#include <private/qquickitemchangelistener_p.h>

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

class UkuiBasePositionerPrivate;

class UkuiPositionerAttached : public QObject
{
    Q_OBJECT

public:
    UkuiPositionerAttached(QObject *parent);

    Q_PROPERTY(int index READ index NOTIFY indexChanged)
    Q_PROPERTY(bool isFirstItem READ isFirstItem NOTIFY isFirstItemChanged)
    Q_PROPERTY(bool isLastItem READ isLastItem NOTIFY isLastItemChanged)

    int index() const { return m_index; }
    void setIndex(int index);

    bool isFirstItem() const { return m_isFirstItem; }
    void setIsFirstItem(bool isFirstItem);

    bool isLastItem() const { return m_isLastItem; }
    void setIsLastItem(bool isLastItem);

    Q_SIGNALS:
        void indexChanged();
    void isFirstItemChanged();
    void isLastItemChanged();

private:
    int m_index;
    bool m_isFirstItem;
    bool m_isLastItem;
};

class UkuiBasePositioner : public QQuickImplicitSizeItem
{
    Q_OBJECT

    Q_PROPERTY(qreal spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)
    Q_PROPERTY(QQuickTransition *populate READ populate WRITE setPopulate NOTIFY populateChanged)
    Q_PROPERTY(QQuickTransition *move READ move WRITE setMove NOTIFY moveChanged)
    Q_PROPERTY(QQuickTransition *add READ add WRITE setAdd NOTIFY addChanged)

    Q_PROPERTY(qreal padding READ padding WRITE setPadding RESET resetPadding NOTIFY paddingChanged)
    Q_PROPERTY(qreal topPadding READ topPadding WRITE setTopPadding RESET resetTopPadding NOTIFY topPaddingChanged)
    Q_PROPERTY(qreal leftPadding READ leftPadding WRITE setLeftPadding RESET resetLeftPadding NOTIFY leftPaddingChanged)
    Q_PROPERTY(qreal rightPadding READ rightPadding WRITE setRightPadding RESET resetRightPadding NOTIFY rightPaddingChanged)
    Q_PROPERTY(qreal bottomPadding READ bottomPadding WRITE setBottomPadding RESET resetBottomPadding NOTIFY bottomPaddingChanged)

public:
    enum PositionerType { None = 0x0, Horizontal = 0x1, Vertical = 0x2, Both = 0x3 };

    UkuiBasePositioner(PositionerType, QQuickItem *parent);
    ~UkuiBasePositioner();

    qreal spacing() const;
    void setSpacing(qreal);

    QQuickTransition *populate() const;
    void setPopulate(QQuickTransition *);

    QQuickTransition *move() const;
    void setMove(QQuickTransition *);

    QQuickTransition *add() const;
    void setAdd(QQuickTransition *);

    static UkuiPositionerAttached *qmlAttachedProperties(QObject *obj);

    void updateAttachedProperties(UkuiPositionerAttached *specificProperty = nullptr, QQuickItem *specificPropertyOwner = nullptr) const;

    qreal padding() const;
    void setPadding(qreal padding);
    void resetPadding();

    qreal topPadding() const;
    void setTopPadding(qreal padding);
    void resetTopPadding();

    qreal leftPadding() const;
    void setLeftPadding(qreal padding);
    void resetLeftPadding();

    qreal rightPadding() const;
    void setRightPadding(qreal padding);
    void resetRightPadding();

    qreal bottomPadding() const;
    void setBottomPadding(qreal padding);
    void resetBottomPadding();

    Q_INVOKABLE void forceLayout();

protected:
    UkuiBasePositioner(UkuiBasePositionerPrivate &dd, PositionerType at, QQuickItem *parent);
    void componentComplete() override;
    void itemChange(ItemChange, const ItemChangeData &) override;

    void updatePolish() override;

Q_SIGNALS:
    void spacingChanged();
    void populateChanged();
    void moveChanged();
    void addChanged();
    void paddingChanged();
    void topPaddingChanged();
    void leftPaddingChanged();
    void rightPaddingChanged();
    void bottomPaddingChanged();
    void positioningComplete();

protected Q_SLOTS:
    void prePositioning();

protected:
    virtual void doPositioning(QSizeF *contentSize)=0;
    virtual void reportConflictingAnchors()=0;

    class PositionedItem
    {
    public :
        PositionedItem(QQuickItem *i);
        ~PositionedItem();
        bool operator==(const PositionedItem &other) const { return other.item == item; }

        qreal itemX() const;
        qreal itemY() const;

        void moveTo(const QPointF &pos);

        void transitionNextReposition(QQuickItemViewTransitioner *transitioner, QQuickItemViewTransitioner::TransitionType type, bool asTarget);
        bool prepareTransition(QQuickItemViewTransitioner *transitioner, const QRectF &viewBounds);
        void startTransition(QQuickItemViewTransitioner *transitioner);

        void updatePadding(qreal lp, qreal tp, qreal rp, qreal bp);

        QQuickItem *item;
        QQuickItemViewTransitionableItem *transitionableItem;
        int index;
        bool isNew;
        bool isVisible;

        qreal topPadding;
        qreal leftPadding;
        qreal rightPadding;
        qreal bottomPadding;
    };

    QPODVector<PositionedItem,8> positionedItems;
    QPODVector<PositionedItem,8> unpositionedItems;//Still 'in' the positioner, just not positioned

    void positionItem(qreal x, qreal y, PositionedItem *target);
    void positionItemX(qreal, PositionedItem *target);
    void positionItemY(qreal, PositionedItem *target);

    void removePositionedItem(QPODVector<PositionedItem,8> *items, int index);
    void clearPositionedItems(QPODVector<PositionedItem,8> *items);

private:
    Q_DISABLE_COPY(UkuiBasePositioner)
    Q_DECLARE_PRIVATE(UkuiBasePositioner)
};

class UkuiBasePositionerPrivate : public QQuickImplicitSizeItemPrivate, public QQuickItemChangeListener
{
    Q_DECLARE_PUBLIC(UkuiBasePositioner)

public:
    struct ExtraData {
        ExtraData();

        qreal padding;
        qreal topPadding;
        qreal leftPadding;
        qreal rightPadding;
        qreal bottomPadding;
        bool explicitTopPadding : 1;
        bool explicitLeftPadding : 1;
        bool explicitRightPadding : 1;
        bool explicitBottomPadding : 1;
    };
    QLazilyAllocated<ExtraData> extra;

    UkuiBasePositionerPrivate()
        : spacing(0), type(UkuiBasePositioner::None)
        , transitioner(0), positioningDirty(false)
        , doingPositioning(false), anchorConflict(false), layoutDirection(Qt::LeftToRight)

    {
    }

    void init(UkuiBasePositioner::PositionerType at)
    {
        type = at;
    }

    qreal spacing;

    UkuiBasePositioner::PositionerType type;
    QQuickItemViewTransitioner *transitioner;

    void watchChanges(QQuickItem *other);
    void unwatchChanges(QQuickItem* other);
    void setPositioningDirty() {
        Q_Q(UkuiBasePositioner);
        if (!positioningDirty) {
            positioningDirty = true;
            q->polish();
        }
    }

    bool positioningDirty : 1;
    bool doingPositioning : 1;
    bool anchorConflict : 1;

    Qt::LayoutDirection layoutDirection;

    void mirrorChange() override {
        effectiveLayoutDirectionChange();
    }
    bool isLeftToRight() const {
        if (type == UkuiBasePositioner::Vertical)
            return true;
        else
            return effectiveLayoutMirror ? layoutDirection == Qt::RightToLeft : layoutDirection == Qt::LeftToRight;
    }

    void itemSiblingOrderChanged(QQuickItem* other) override
    {
        Q_UNUSED(other);
        setPositioningDirty();
    }

    void itemGeometryChanged(QQuickItem *, QQuickGeometryChange change, const QRectF &) override
    {
        if (change.sizeChange())
            setPositioningDirty();
    }

    void itemVisibilityChanged(QQuickItem *) override
    {
        setPositioningDirty();
    }

    void itemDestroyed(QQuickItem *item) override
    {
        Q_Q(UkuiBasePositioner);
        int index = q->positionedItems.find(UkuiBasePositioner::PositionedItem(item));
        if (index >= 0)
            q->removePositionedItem(&q->positionedItems, index);
    }

    static Qt::LayoutDirection getLayoutDirection(const UkuiBasePositioner *positioner)
    {
        return positioner->d_func()->layoutDirection;
    }

    static Qt::LayoutDirection getEffectiveLayoutDirection(const UkuiBasePositioner *positioner)
    {
        if (positioner->d_func()->effectiveLayoutMirror)
            return positioner->d_func()->layoutDirection == Qt::RightToLeft ? Qt::LeftToRight : Qt::RightToLeft;
        else
            return positioner->d_func()->layoutDirection;
    }

    virtual void effectiveLayoutDirectionChange()
    {
    }

    inline qreal padding() const { return extra.isAllocated() ? extra->padding : 0.0; }
    void setTopPadding(qreal value, bool reset = false);
    void setLeftPadding(qreal value, bool reset = false);
    void setRightPadding(qreal value, bool reset = false);
    void setBottomPadding(qreal value, bool reset = false);
};

class UkuiFlowPrivate;
class UkuiFlow : public UkuiBasePositioner
{
    Q_OBJECT
    Q_PROPERTY(Flow flow READ flow WRITE setFlow NOTIFY flowChanged)
    Q_PROPERTY(Qt::LayoutDirection layoutDirection READ layoutDirection WRITE setLayoutDirection NOTIFY layoutDirectionChanged)
    Q_PROPERTY(Qt::LayoutDirection effectiveLayoutDirection READ effectiveLayoutDirection NOTIFY effectiveLayoutDirectionChanged)
    Q_PROPERTY(bool lineHorizontalCenterAlignment READ lineHorizontalCenterAlignment WRITE setLineHorizontalCenterAlignment NOTIFY lineHorizontalCenterAlignmentChanged)
public:
    UkuiFlow(QQuickItem *parent=nullptr);

    enum Flow { LeftToRight, TopToBottom };
    Q_ENUM(Flow)
    Flow flow() const;
    void setFlow(Flow);

    Qt::LayoutDirection layoutDirection() const;
    void setLayoutDirection (Qt::LayoutDirection);
    Qt::LayoutDirection effectiveLayoutDirection() const;

    bool lineHorizontalCenterAlignment() const;
    void setLineHorizontalCenterAlignment(bool);

public Q_SLOTS:
    int moveCurrentIndexLeft(int currentIndex);
    int moveCurrentIndexRight(int currentIndex);
    int moveCurrentIndexUp(int currentIndex);
    int moveCurrentIndexDown(int currentIndex);
    int findUpDownItemIndex(int currentIndex, int startIndex, int endIndex);

Q_SIGNALS:
    void flowChanged();
    void layoutDirectionChanged();
    void effectiveLayoutDirectionChanged();
    void lineHorizontalCenterAlignmentChanged();

protected:
    UkuiFlow(UkuiFlowPrivate &dd, QQuickItem *parent);
    void doPositioning(QSizeF *contentSize) override;
    void reportConflictingAnchors() override;
private:
    Q_DISABLE_COPY(UkuiFlow)
    Q_DECLARE_PRIVATE(UkuiFlow)
};
QML_DECLARE_TYPEINFO(UkuiPositionerAttached, QML_HAS_ATTACHED_PROPERTIES)


#endif //UKUI_FLOW_H
