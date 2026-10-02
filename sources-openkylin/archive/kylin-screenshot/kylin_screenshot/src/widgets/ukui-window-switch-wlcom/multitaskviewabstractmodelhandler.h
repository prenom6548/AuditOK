/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef MultitaskViewAbstractModelHandler_H
#define MultitaskViewAbstractModelHandler_H

#include <QObject>

class WlcomWindow
{
    Q_GADGET
    Q_PROPERTY(int id READ id)
    Q_PROPERTY(QString caption READ caption)
    Q_PROPERTY(int x READ x)
    Q_PROPERTY(int y READ y)
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
    Q_PROPERTY(bool minimized READ minimized)
    Q_PROPERTY(QString uuid READ uuid)
    Q_PROPERTY(int desktopId READ desktopId)
    Q_PROPERTY(int screenId READ screenId)

public:
    WlcomWindow() = default;
    WlcomWindow(int id, QString uuid, const QString& caption, int x, int y, int width, int height, bool minimized)
        : m_id(id),
          m_uuid(uuid),
          m_caption(caption),
          m_x(x),
          m_y(y),
          m_width(width),
          m_height(height),
          m_isMinimized(minimized) {}
    int id() const { return m_id; }
    QString uuid() const { return m_uuid; }
    const QString& caption() const { return m_caption; }
    int x() const { return m_x; }
    int y() const { return m_y; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    bool minimized() const { return m_isMinimized; }
    int desktopId() const { return m_desktopId; }
    int screenId() const { return m_screenId; }
    void setDesktopId(const int desktopId) {
        m_desktopId = desktopId;
    }
    void setScreenId(const int screenId) {
        m_screenId = screenId;
    }

private:
    int m_id = 0;
    QString m_uuid;
    QString m_caption;
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
    bool m_isMinimized = false;
    int m_desktopId = 1;
    int m_screenId = 0;
};
Q_DECLARE_METATYPE(WlcomWindow)


class WlcomScreen
{
    Q_GADGET
    Q_PROPERTY(int index READ index)
    Q_PROPERTY(QList<WlcomWindow> windowList READ windowList)

public:
    WlcomScreen() = default;
    WlcomScreen(int index, const QList<WlcomWindow>& windowList)
        : m_index(index),
          m_windowList(windowList) {}

    int index() const { return m_index; }
    const QList<WlcomWindow>& windowList() const { return m_windowList; }

private:
    int m_index = 0;
    QList<WlcomWindow> m_windowList;
};
Q_DECLARE_METATYPE(WlcomScreen)


class Desktop
{
    Q_GADGET
    Q_PROPERTY(QString uuid READ uuid)
    Q_PROPERTY(QList<WlcomScreen> screenList READ screenList)

public:
    enum DesktopType {
        WorkSpace = 0,
        NewDesktop
    };

    Desktop() = default;
    Desktop(const QList<WlcomScreen>& screenList)
        : m_screenList(screenList) {}

    QString uuid() const { return m_uuid; }
    const QList<WlcomScreen>& screenList() { return m_screenList; }
    DesktopType type() const { return m_type; };

    void setUuid(const QString uuid) {
        m_uuid = uuid;
    }
    void setScreenList(const QList<WlcomScreen>& screenList) {
        m_screenList = screenList;
    }
    void setDesktopType(DesktopType type) {
        m_type = type;
    }

    QString customName() const { return m_customName; }

    void setCustomName(const QString name) {
        m_customName = name;
    }

private:
    QString m_uuid;
    QString m_customName;
    QList<WlcomScreen> m_screenList;
    DesktopType m_type = DesktopType::WorkSpace;
};
Q_DECLARE_METATYPE(Desktop)

class MultitaskViewAbstractModelHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MultitaskViewAbstractModelHandler)

public:
    explicit MultitaskViewAbstractModelHandler() = default;
    ~MultitaskViewAbstractModelHandler() = default;

    virtual void removeAppWindow(QString windowId) = 0;
    virtual void moveAppWindowToDesktop(QString windowId, int srcDesktopInex, int destDesktopInex) = 0;
    virtual void activateAppWindow(QString windowId) = 0;
    virtual QVariant getWindowIcon(QString windowId) = 0;
    virtual void removeDesktop(int desktopIndex) = 0;
    virtual void appendDesktop() = 0;
    virtual void moveDesktop(int srcDesktopIndex, int destDesktopIndex) = 0;
    virtual int getCurrentDesktop() = 0;
    virtual bool isCurrentDesktopShowingDesktop() = 0;
    virtual void setCurrentDesktop(int desktopIndex) = 0;
    virtual QRect getFullScreenGeometry(int desktopIndex, int screenIndex) =0;
    virtual bool hasModalChildWindow(QString windowId) = 0;
    virtual bool isExtensionMode() = 0;
    virtual int logicalScreenCount() = 0;
    virtual QList<WlcomWindow> createWindowList(int desktopIndex, int screenIndex) = 0;
    virtual int getNumberOfDesktops() = 0;
    virtual QString getDesktopUuid(int desktopIndex) = 0;
    virtual QString getOutputUuid(int outputIndex) = 0;
    virtual void setDesktopName(const int desktopIndex, const QString name) = 0;

signals:
    void appWindowCreated();
    void appWindowAdd(const WlcomWindow &window);
    void appWindowRemoved(QString windowId);
    void appWindowActivated();
    void appWindowDesktopChanged(QString windowId, int oldDesktopIndex,int newDesktopIndex);
    void desktopRemoved(int desktopIndex);
    void desktopAppended(int desktopIndex);
    void desktopMoved(int oldDesktopIndex, int newDesktopIndex);
    void currentDesktopChanged(int oldCurrentDesktopIndex, int newCurrentDesktopIndex);
    void screenCountChanged();
    void screenSizeChanged();
    void desktopNumberChanged();
    void tabletModeChanged(bool tabletMode);

};


#endif // MultitaskViewAbstractModelHandler_H
