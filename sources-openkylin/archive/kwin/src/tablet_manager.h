/*
    SPDX-FileCopyrightText: 2022 KylinSoft Co., Ltd.

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWIN_TABLET_BACKGROUND_H
#define KWIN_TABLET_BACKGROUND_H

#include <QVector>
#include <QWidget>
#include <config-kwin.h>
#include <kwin_export.h>
#include <kwinglobals.h>
#include <netwm.h>

namespace KWin
{

class AbstractClient;
class EffectsHandlerImpl;
class AbstractEffectLoader;

class KWIN_EXPORT TabletBackground : public QWidget
{
    Q_OBJECT
public:
    TabletBackground();
    ~TabletBackground() override = default;

    void init();
    // only on X11
    void setState(NET::States state);

protected:
    void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void slotGeometryChanged();

private:
};

class KWIN_EXPORT SingleWindowManager : public QObject
{
    Q_OBJECT
public:
    explicit SingleWindowManager(QObject *parent = nullptr);
    ~SingleWindowManager() override = default;
    void setBlurBackground(TabletBackground *blurBackground);
    void switchTablet(bool bTabletMode);
    void showBackground();
    void hideBackground();

private Q_SLOTS:
    void handleClientAdded(AbstractClient *client);
    void handleClientRemoved(AbstractClient *client);
    void handleClientActivated(AbstractClient *client);
    void handleClientMinimizedChanged(AbstractClient *client);
    void handleScreenGeometryChanged();

private:
    template<class T>
    void addTransient(QVector<T *> &clients, T *transientClient);
    template<class T>
    bool removeTransient(QVector<T *> &clients, T *transientClient);
    void printClientInformation(const AbstractClient *client, const char *funcName = nullptr) const;
    bool isBackgroundWindow(const AbstractClient *client) const;
    bool isClientShouldBeMaximized(const AbstractClient *client) const;
    bool isWindowNeedBlurBackground(const AbstractClient *client) const;
    bool isWindowBelongToWorkingWindow(const AbstractClient *client) const;
    bool isWindowBelongToAuthenticWindow(const AbstractClient *client) const;
    bool isClientShouldNotBePlacedCenter(const AbstractClient *client) const;
    void placeWindowCenter(AbstractClient *client) const;
    bool maximizeCurrentMainWindowOnly(AbstractClient *mainWindow);
    bool maximizeWindow(AbstractClient *client);
    void updateDesktopVisibleState(AbstractClient *desktop);
    void minimizeWindowsExceptForFamilyWindow(QList<AbstractClient *> familyWindowList);

private:
    AbstractClient *m_blurHelperWindow;
    TabletBackground *m_blurBackground;
    AbstractClient *m_pcDesktop;
    AbstractClient *m_tabletDesktop;
    QStringList m_ignorePlaceCenterList;
    QStringList m_transientWindowList;
    QStringList m_authenticWindowList;
    bool m_bTabletMode;
    QVector<AbstractClient *> m_transientClients;
};

class KWIN_EXPORT TabletManager : public QObject
{
    Q_OBJECT
public:
    ~TabletManager() override;

    void toggleTabletMode();
    bool tabletMode() const
    {
        return m_bTabletMode;
    }
    bool existTabletDesktop() const
    {
        return m_existTabletDesktop;
    }
    void setBlurBehind(double radius = 4000);
    void showBackground(double radius = 4000);
    // 外部接口，隐藏时设置为4000，使平板模式下的毛玻璃足够模糊
    void hideBackground(double radius = 4000);

private Q_SLOTS:
    void init();
    void slotSwitchTabletMode(bool bSurfaceMode);

Q_SIGNALS:
    /**
     * @brief This signal is emitted when the mode of system has changed.
     *
     * @param bTabletMode
     */
    void tabletModeChanged(bool bTabletMode);

private:
    void moveWindowBetweenVDesktop(QUuid windowId, int srcDesktopIndex, int destDesktopIndex);

private:
    TabletBackground *m_background;
    SingleWindowManager *m_singleWinManager;
    bool m_bTabletMode;
    bool m_existTabletDesktop;
    QMap<QUuid, int> m_windowDesktopMap;
    EffectsHandlerImpl *m_effects;
    AbstractEffectLoader *m_effectloader;
    KWIN_SINGLETON(TabletManager)
};

}

#endif
