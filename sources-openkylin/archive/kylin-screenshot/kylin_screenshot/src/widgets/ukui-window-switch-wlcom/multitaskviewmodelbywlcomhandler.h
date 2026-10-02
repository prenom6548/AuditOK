/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

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

#ifndef MULTITASKVIEWMODELBYWLCOMHANDLER_H
#define MULTITASKVIEWMODELBYWLCOMHANDLER_H
#include "multitaskviewabstractmodelhandler.h"
#include <QObject>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QRect>
#include <QVariantList>
#include "context.h"
//ukui-search
//#include <ukui-search/application-info.h>

class MultitaskViewModelByWlcomHandler : public MultitaskViewAbstractModelHandler
{
    Q_OBJECT

public:
    explicit MultitaskViewModelByWlcomHandler(QObject *parent = nullptr);
    ~MultitaskViewModelByWlcomHandler();

    void removeAppWindow(QString windowId) override;
    void moveAppWindowToDesktop(QString windowId, int srcDesktopInex, int destDesktopInex) override;
    void activateAppWindow(QString windowId) override;
    QVariant getWindowIcon(QString windowId) override;
    void removeDesktop(int desktopIndex) override;
    void moveDesktop(int srcDesktopIndex, int destDesktopIndex) override;
    int getNumberOfDesktops() override;
    void appendDesktop() override;
    int getCurrentDesktop() override;
    bool isCurrentDesktopShowingDesktop() override;
    void setCurrentDesktop(int desktopIndex) override;
    QRect getFullScreenGeometry(int desktopIndex, int screenIndex) override;
    bool hasModalChildWindow(QString windowId) override;
    bool isExtensionMode() override;
    int logicalScreenCount() override;
    QList<WlcomWindow> createWindowList(int desktopIndex, int screenIndex) override;
    QString getDesktopUuid(int desktopIndex) override;
    QString getOutputUuid(int outputIndex) override;
    void setDesktopName(const int desktopIndex, const QString name) override;

//    static UkuiSearch::ApplicationInfo *getApplicationInfo();
    static QString desktopFileFromPid(uint pid);
    WlcomWindow getActiveWindow(int desktopIndex, int screenIndex);
private:
    bool shouldAcceptWindow(Toplevel *toplevel);
    void createContext();
    void connectSignals();
    Toplevel *getToplevel(QString windowId);
    WlcomWindow windowFromToplevel(Toplevel *toplevel);
    Context *m_context = nullptr;
    QList<Toplevel *> m_allWindowList;
    QList<Workspace *> m_workspaceList;
    QList<Output *> m_outputList;

signals:
    void workspaceHasCreated();

private slots:
    void onWindowAdded(Toplevel *toplevel);
    void onOutputAdded(Output *output);
    void onWindowDeleted();
    void onWindowActivated(Toplevel *toplevel);
    void onWindowUpdate(Toplevel::Masks mask);
    void onWorkSpaceAdded(Workspace *workspace);
    void onWorkSpaceDeleted();
   // void onWorkSpaceActivated(int windowId);
    void onCurrentDesktopChanged(Workspace::Masks mask);
    void onDesktopPresenceChanged(Toplevel *toplevel, int oldDesktopIndex ,int newDesktopIndex);

};

#endif // MULTITASKVIEWMODELBYWLCOMHANDLER_H
