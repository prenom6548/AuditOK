/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KWIN_TABBOX_SWITCHERITEM_H
#define KWIN_TABBOX_SWITCHERITEM_H

#include <QGSettings>
#include <QObject>
#include <QRect>

class QAbstractItemModel;

namespace KWin
{
namespace TabBox
{

class SwitcherItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model NOTIFY modelChanged)
    Q_PROPERTY(QRect screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)
    Q_PROPERTY(bool visible READ isVisible NOTIFY visibleChanged)
    Q_PROPERTY(QString theme READ nowTheme NOTIFY themeChanged)
    Q_PROPERTY(int themeRadius READ getThemeRadius NOTIFY themeRadiusChanged)
    Q_PROPERTY(bool allDesktops READ isAllDesktops NOTIFY allDesktopsChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(bool noModifierGrab READ noModifierGrab NOTIFY noModifierGrabChanged)
    Q_PROPERTY(bool compositing READ compositing NOTIFY compositingChanged)

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QObject *item READ item WRITE setItem NOTIFY itemChanged)

    Q_CLASSINFO("DefaultProperty", "item")
public:
    SwitcherItem(QObject *parent = nullptr);
    ~SwitcherItem() override;

    QAbstractItemModel *model() const;
    QRect screenGeometry() const;
    bool isVisible() const;
    bool isAllDesktops() const;
    int currentIndex() const;
    QString nowTheme() const;
    void setCurrentIndex(int index);
    QObject *item() const;
    void setItem(QObject *item);
    bool noModifierGrab() const
    {
        return m_noModifierGrab;
    }
    bool compositing();
    int getThemeRadius();
    Q_INVOKABLE bool isLayoutLeftToRight() const;

    // for usage from outside
    void setModel(QAbstractItemModel *model);
    void setAllDesktops(bool all);
    void setVisible(bool visible);
    void setNoModifierGrab(bool set);
    Q_INVOKABLE void selectWindow();

Q_SIGNALS:
    void visibleChanged();
    void themeChanged();
    void themeRadiusChanged();
    void currentIndexChanged(int index);
    void modelChanged();
    void allDesktopsChanged();
    void screenGeometryChanged();
    void itemChanged();
    void noModifierGrabChanged();
    void compositingChanged();

private:
    void initLayoutDirection();

    QAbstractItemModel *m_model;
    QObject *m_item;
    bool m_visible;
    bool m_allDesktops;
    int m_currentIndex;
    QString m_theme;
    QGSettings *m_themeSettings;
    QMetaObject::Connection m_selectedIndexConnection;
    bool m_noModifierGrab = false;
    bool m_isLayoutLeftToRight = true;
};

inline QAbstractItemModel *SwitcherItem::model() const
{
    return m_model;
}

inline bool SwitcherItem::isVisible() const
{
    return m_visible;
}

inline bool SwitcherItem::isAllDesktops() const
{
    return m_allDesktops;
}

inline int SwitcherItem::currentIndex() const
{
    return m_currentIndex;
}

inline QString SwitcherItem::nowTheme() const
{
    return m_theme;
}

inline QObject *SwitcherItem::item() const
{
    return m_item;
}

inline bool SwitcherItem::isLayoutLeftToRight() const
{
    return m_isLayoutLeftToRight;
}

} // TabBox
} // KWin

#endif // KWIN_TABBOX_SWITCHERITEM_H
