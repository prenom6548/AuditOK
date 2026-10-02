/*
    Copyright (C) 2012  Dan Vratil <dvratil@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef QMLOUTPUT_H
#define QMLOUTPUT_H

#include "../utilsofdisplay.h"

#include <QQuickItem>
#include <KF5/KScreen/kscreen/mode.h>

class QStandardItemModel;
class QAbstractItemModel;

class ModesProxyModel;
class QMLScreen;

class QMLOutput : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(KScreen::Output *output
               READ output
               NOTIFY outputChanged)

    Q_PROPERTY(KScreen::OutputPtr outputPtr
               READ outputPtr
               WRITE setOutputPtr
               NOTIFY outputChanged)

    Q_PROPERTY(bool isCloneMode
               READ isCloneMode
               WRITE setIsCloneMode
               NOTIFY isCloneModeChanged)

    Q_PROPERTY(bool isCloneModeShow
               READ isCloneModeShow
               WRITE setIsCloneMode
               NOTIFY isCloneModeChanged)

    Q_PROPERTY(QMLScreen* screen
               READ screen
               WRITE setScreen
               NOTIFY screenChanged)

    Q_PROPERTY(QMLOutput* cloneOf
               READ cloneOf
               WRITE setCloneOf
               NOTIFY cloneOfChanged)

    Q_PROPERTY(int currentOutputHeight
               READ currentOutputHeight
               NOTIFY currentOutputSizeChanged)

    Q_PROPERTY(int currentOutputWidth
               READ currentOutputWidth
               NOTIFY currentOutputSizeChanged)

    /* Workaround for possible QML bug when calling output.pos.y = VALUE works,
     * but output.pos.x = VALUE has no effect */
    Q_PROPERTY(int outputX
               READ outputX
               WRITE setOutputX
               NOTIFY outputXChanged)

    Q_PROPERTY(int outputY
               READ outputY
               WRITE setOutputY
               NOTIFY outputYChanged)

    Q_PROPERTY(bool allowResetSize
               READ allowResetSize
               WRITE setAllowResetSize
               NOTIFY allowResetSizeChanged)

    Q_PROPERTY(QString outputName
               READ friendlyOutputName)

    Q_PROPERTY(bool isDarkMode
               READ isDarkMode
               WRITE setIsDarkMode
               NOTIFY isDarkModeChanged)
    Q_PROPERTY(bool dragEnable
               READ dragEnable
               WRITE setDragEnable
               NOTIFY dragEnableChanged)

    Q_PROPERTY(int isClicked
               READ isClicked
               WRITE setIsClicked
               NOTIFY isClickedChanged)

    // 点选选中态；与 Tab 的 activeFocus 分离，避免 Tab 只画框却变成切换屏
    Q_PROPERTY(bool isSelected
               READ isSelected
               WRITE setIsSelected
               NOTIFY isSelectedChanged)
public:
    enum {        
        ModeRole = Qt::UserRole,
        ModeIdRole,
        SizeRole,
        RefreshRateRole
    };

    explicit QMLOutput(QQuickItem *parent = nullptr);

    KScreen::Output *output() const; // For QML

    KScreen::OutputPtr outputPtr() const;
    void setOutputPtr(const KScreen::OutputPtr &output);

    QMLScreen *screen() const;
    void setScreen(QMLScreen *screen);

    Q_INVOKABLE bool collidesWithOutput(QObject *other);

    void setCloneOf(QMLOutput *other);
    QMLOutput *cloneOf() const;

    int currentOutputHeight() const;
    int currentOutputWidth() const;

    int outputX() const;
    void setOutputX(int x);

    int outputY() const;
    void setOutputY(int y);

    bool allowResetSize() const;
    void setAllowResetSize(bool t_allowResetSize);

    bool dragEnable() const;
    void setDragEnable(bool dragStatus);

    void setIsCloneMode(bool isCloneMode, bool cloneModeShow = false);
    bool isCloneMode() const;
    bool isCloneModeShow() const;

    QString friendlyOutputName();
    void setIsDarkMode(bool isDarkMode);
    bool isDarkMode() const;

    bool isClicked();
    void setIsClicked(bool isClicked);

    bool isSelected() const;
    void setIsSelected(bool selected);

public Q_SLOTS:
    void updateRootProperties();

Q_SIGNALS:
    void changed();

    void moved(const QString &self);

    /* Property notifications */
    void outputChanged();
    void screenChanged();
    void cloneOfChanged();
    void currentOutputSizeChanged();

    void outputYChanged();
    void outputXChanged();
    void allowResetSizeChanged();

    void isCloneModeChanged();
    void isDarkModeChanged();
    void dragEnableChanged();
    void isClickedChanged();
    void isSelectedChanged();

private Q_SLOTS:
    void currentModeIdChanged();

private:
    /**
     * Returns the biggest resolution available assuming it's the preferred one
     */
    KScreen::ModePtr bestMode() const;

    KScreen::OutputPtr m_output;
    QMLScreen *m_screen;

    QMLOutput *m_cloneOf;

    bool m_isCloneMode;
    bool m_cloneModeShow;
    bool m_dragEnable;
    int m_allowResetSize;

    QString m_outputName;
    bool m_isDarkMode;
    bool m_isClicked;
    bool m_isSelected;
};

#endif // QMLOUTPUT_H
