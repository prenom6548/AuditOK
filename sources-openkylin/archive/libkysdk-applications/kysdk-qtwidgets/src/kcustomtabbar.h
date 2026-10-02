/*
 * libkysdk-qtwidgets's Library
 *
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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */
#ifndef KCUSTOMBAR_H
#define KCUSTOMBAR_H

#include <QIcon>
#include <QLayoutItem>
#include <QStyleOptionTab>
#include <QTabBar>
#include <QtWidgets/qtwidgetsglobal.h>
#include <QtWidgets/qwidget.h>

class KCustomTabBarPrivate;

/**
 *  @defgroup KCustomTabBar
 *  @since 2.5.2.0
 */

class KCustomTabBar : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(Shape shape READ shape WRITE setShape)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(bool drawBase READ drawBase WRITE setDrawBase)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)
    Q_PROPERTY(bool usesScrollButtons READ usesScrollButtons WRITE setUsesScrollButtons)
    Q_PROPERTY(bool tabsClosable READ tabsClosable WRITE setTabsClosable)
    Q_PROPERTY(SelectionBehavior selectionBehaviorOnRemove READ selectionBehaviorOnRemove WRITE setSelectionBehaviorOnRemove)
    Q_PROPERTY(bool expanding READ expanding WRITE setExpanding)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
    Q_PROPERTY(bool documentMode READ documentMode WRITE setDocumentMode)
    Q_PROPERTY(bool autoHide READ autoHide WRITE setAutoHide)
    Q_PROPERTY(bool changeCurrentOnDrag READ changeCurrentOnDrag WRITE setChangeCurrentOnDrag)

public:
    explicit KCustomTabBar(QWidget *parent = nullptr);
    ~KCustomTabBar();

    enum Shape { RoundedNorth,
                 RoundedSouth,
                 RoundedWest,
                 RoundedEast,
                 TriangularNorth,
                 TriangularSouth,
                 TriangularWest,
                 TriangularEast
    };
    Q_ENUM(Shape)

    enum ButtonPosition {
        LeftSide,
        RightSide
    };

    enum SelectionBehavior {
        SelectLeftTab,
        SelectRightTab,
        SelectPreviousTab
    };

    Shape shape() const;
    void setShape(Shape shape);

    int addTab(const QString &text);
    int addTab(const QIcon &icon, const QString &text);

    int insertTab(int index, const QString &text);
    int insertTab(int index, const QIcon &icon, const QString &text);

    void removeTab(int index);
    void moveTab(int from, int to);

    bool isTabEnabled(int index) const;
    void setTabEnabled(int index, bool);

    QString tabText(int index) const;
    void setTabText(int index, const QString &text);

    void setTabContent(int index, const QString &text);

    QColor tabTextColor(int index) const;
    void setTabTextColor(int index, const QColor &color);

    QIcon tabIcon(int index) const;
    void setTabIcon(int index, const QIcon &icon);

    Qt::TextElideMode elideMode() const;
    void setElideMode(Qt::TextElideMode);

    // #ifndef QT_NO_TOOLTIP
    void setTabToolTip(int index, const QString &tip);
    QString tabToolTip(int index) const;
    // #endif

    // #if QT_CONFIG(whatsthis)
    void setTabWhatsThis(int index, const QString &text);
    QString tabWhatsThis(int index) const;
    // #endif

    void setTabData(int index, const QVariant &data);
    QVariant tabData(int index) const;

    QRect tabRect(int index) const;
    int tabAt(const QPoint &pos) const;

    int currentIndex() const;
    int count() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setDrawBase(bool drawTheBase);
    bool drawBase() const;

    QSize iconSize() const;
    void setIconSize(const QSize &size);

    bool usesScrollButtons() const;
    void setUsesScrollButtons(bool useButtons);

    bool tabsClosable() const;
    void setTabsClosable(bool closable);

    void setTabButton(int index, ButtonPosition position, QWidget *widget);
    QWidget *tabButton(int index, ButtonPosition position) const;

    SelectionBehavior selectionBehaviorOnRemove() const;
    void setSelectionBehaviorOnRemove(SelectionBehavior behavior);

    bool expanding() const;
    void setExpanding(bool enabled);

    bool isMovable() const;
    void setMovable(bool movable);

    bool documentMode() const;
    void setDocumentMode(bool set);

    bool autoHide() const;
    void setAutoHide(bool hide);

    bool changeCurrentOnDrag() const;
    void setChangeCurrentOnDrag(bool change);

    // #ifndef QT_NO_ACCESSIBILITY
    QString accessibleTabName(int index) const;
    void setAccessibleTabName(int index, const QString &name);
    // #endif

public Q_SLOTS:
    void setCurrentIndex(int index);
    //    void scrollTabs();
    void leftScrollTabs();
    void rightScrollTabs();
    void closeTab();

Q_SIGNALS:
    void currentChanged(int index);
    void tabCloseRequested(int index);
    void tabMoved(int from, int to);
    void tabBarClicked(int index);
    void tabBarDoubleClicked(int index);
    void leftBStatus(bool flag);
    void rightBStatus(bool flag);

protected:
    virtual QSize tabSizeHint(int index) const;
    virtual QSize minimumTabSizeHint(int index) const;
    virtual void tabInserted(int index);
    virtual void tabRemoved(int index);
    virtual void tabLayoutChange();

    bool event(QEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *) override;
    void changeEvent(QEvent *) override;
    void timerEvent(QTimerEvent *event) override;
    void initStyleOption(QStyleOptionTab *option, int tabIndex) const;

    // #ifndef QT_NO_ACCESSIBILITY
    //     friend class QAccessibleTabBar;
    // #endif
private:
    Q_DISABLE_COPY(KCustomTabBar)
    Q_DECLARE_PRIVATE(KCustomTabBar)
    KCustomTabBarPrivate *const d_ptr;
};

#endif // QTABBAR_H
