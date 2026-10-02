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

#include "multitaskviewmodelbywlcomhandler.h"
#include <algorithm>

#include <QtDBus>
#include <QGSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QtX11Extras/QX11Info>
#include <QString>
#include <QEventLoop>
#include <iostream>
#include <KWindowSystem>
#include <libkywc.h>
#include <QDebug>
//#include "multitaskviewmanagerbywlcom.h"

using namespace std;

//static UkuiSearch::ApplicationInfo *applicationInfo = nullptr;
static std::once_flag flag;
//UkuiSearch::ApplicationInfo *MultitaskViewModelByWlcomHandler::getApplicationInfo()
//{
//    std::call_once(flag, [ & ] {
//        applicationInfo = new UkuiSearch::ApplicationInfo();
//    });
//    return applicationInfo;
//}

QString MultitaskViewModelByWlcomHandler::desktopFileFromPid(uint pid)
{
    QString desktopFile;
//    getApplicationInfo()->tranPidToDesktopFp(pid, desktopFile);
    return desktopFile;
}

WlcomWindow MultitaskViewModelByWlcomHandler::getActiveWindow(int desktopIndex, int screenIndex)
{
//    QList<WlcomWindow> windowList;
    WlcomWindow result;
    const auto wnds = m_allWindowList;

    for (auto const wnd: wnds) {
        if (!shouldAcceptWindow(wnd)) {
            continue;
        }
        bool isOnDesktop = false;
//        qDebug() << "--------------" << wnd->title() << wnd;
        int workspacesize = wnd->workspaces().size();
        for (int i = 0; i < workspacesize; i++) {
//            qDebug() << wnd->workspaces()[i];
//            qDebug() <<m_workspaceList[desktopIndex - 1]->uuid();
            if (wnd->workspaces()[i] == (QString)m_workspaceList[desktopIndex - 1]->uuid()){
                isOnDesktop = true;
                break;
            }
        }
        if ((workspacesize != 0) && !isOnDesktop){
            continue;
        }

        if (!wnd->isActivated())
        {
            continue;
        }

        QRect wndrect(wnd->point().x(),
                      wnd->point().y(),
                      wnd->size().width(),
                      wnd->size().height());
        if (!(QGuiApplication::screens().at(screenIndex)->geometry().contains(wndrect.center()))) {
            continue;
        }
        QString id =wnd->uuid();
        QString title = wnd->title();
        if (title.isEmpty()) {
            title = wnd->name();
        }
        WlcomWindow window(id.toInt(),
                      wnd->uuid(),
                      title,
                      wnd->point().x(),
                      wnd->point().y(),
                      wnd->size().width(),
                      wnd->size().height(),
                      wnd->isMinimized());

//        windowList.append(window);
        result = window;
    }

    return result;
}

MultitaskViewModelByWlcomHandler::MultitaskViewModelByWlcomHandler(QObject *parent) : MultitaskViewAbstractModelHandler{}
{
    createContext();
    m_context->start();
//    connectSignals();
//    getApplicationInfo();
}

MultitaskViewModelByWlcomHandler::~MultitaskViewModelByWlcomHandler()
{
    delete m_context;
}

void MultitaskViewModelByWlcomHandler::createContext()
{
    m_context = new Context(nullptr, Context::Capability::Output | Context::Capability::Toplevel |
                          Context::Capability::Workspace);
    connect(m_context, &Context::workespaceAdded, this, &MultitaskViewModelByWlcomHandler::onWorkSpaceAdded);
    connect(m_context, &Context::toplevelAdded, this, &MultitaskViewModelByWlcomHandler::onWindowAdded);
    connect(m_context, &Context::outputAdded, this, &MultitaskViewModelByWlcomHandler::onOutputAdded);
}

Toplevel *MultitaskViewModelByWlcomHandler::getToplevel(QString windowId)
{
    Toplevel *toplevel = NULL;

    const auto wnds = m_allWindowList;
    for (auto const wnd: wnds) {
        if ((QString)wnd->uuid() == windowId) {
            toplevel = wnd;
        }
    }
    return toplevel;
}

WlcomWindow MultitaskViewModelByWlcomHandler::windowFromToplevel(Toplevel *toplevel)
{
    if (toplevel == nullptr) {
        return WlcomWindow();
    }
    WlcomWindow window(toplevel->uuid().toInt(),
                  toplevel->uuid(),
                  toplevel->title().isEmpty() ? toplevel->name() : toplevel->title(),
                  toplevel->point().x(),
                  toplevel->point().y(),
                  toplevel->size().width(),
                  toplevel->size().height(),
                  toplevel->isMinimized());

    int desktopId = 1;
    for (int i = 0; i < toplevel->workspaces().size(); ++i) {
        if (toplevel->workspaces().contains((QString)m_workspaceList[i]->uuid())) {
            desktopId = i + 1;
        }
    }

    int screenId = 0;
    QRect wndrect(toplevel->point().x(),
                  toplevel->point().y(),
                  toplevel->size().width(),
                  toplevel->size().height());
    for (int i = 0; i < QGuiApplication::screens().size(); i++) {
        if (QGuiApplication::screens().at(i)->geometry().contains(wndrect.center())) {
            screenId = i;
        }
    }

    window.setDesktopId(desktopId);
    window.setScreenId(screenId);
    return window;
}

void MultitaskViewModelByWlcomHandler::removeAppWindow(QString windowId)
{
    Toplevel *toplevel =  getToplevel(windowId);
    if (toplevel == nullptr){
        return;
    }
    toplevel->close();
}

void MultitaskViewModelByWlcomHandler::moveAppWindowToDesktop(QString windowId,
                                                              int srcDesktopInex,
                                                              int destDesktopInex)
{

    if (destDesktopInex -1 >= m_workspaceList.size()){
        return;
    }
    Toplevel *toplevel =  getToplevel(windowId);
    toplevel->moveToWorkspace(m_workspaceList[destDesktopInex-1]->uuid());

}

void MultitaskViewModelByWlcomHandler::activateAppWindow(QString windowId)
{
    getToplevel(windowId)->setActivate();
}

QVariant MultitaskViewModelByWlcomHandler::getWindowIcon(QString windowId)
{
    Toplevel *toplevel =  getToplevel(windowId);
    if (!toplevel){
        return "";
    }

    QIcon themeIcon;
    themeIcon = QIcon::fromTheme(toplevel->icon(), themeIcon);
    if (!themeIcon.isNull()) {
        return themeIcon;
    }

    return toplevel->iconName();
}

int MultitaskViewModelByWlcomHandler::getNumberOfDesktops()
{
    return m_workspaceList.size();
}

void MultitaskViewModelByWlcomHandler::removeDesktop(int desktopIndex)
{
    m_workspaceList[desktopIndex-1]->remove();
}

void MultitaskViewModelByWlcomHandler::moveDesktop(int srcDesktopIndex, int destDesktopIndex)
{
    m_workspaceList[srcDesktopIndex-1]->move(destDesktopIndex - 1);
    m_workspaceList.move(srcDesktopIndex - 1, destDesktopIndex - 1);
    emit desktopMoved(srcDesktopIndex, destDesktopIndex);
}

void MultitaskViewModelByWlcomHandler::appendDesktop()
{
    int desktopCount = m_workspaceList.size();
    m_context->addWorkspace(desktopCount);
    QEventLoop loop;
    connect(this, &MultitaskViewModelByWlcomHandler::workspaceHasCreated, &loop, &QEventLoop::quit);
    loop.exec();
}

int MultitaskViewModelByWlcomHandler::getCurrentDesktop()
{
    for (int i = 0; i < m_workspaceList.size(); i++) {
        if (m_workspaceList[i]->isActivated()) {
            return i+1;
        }
    }
    return 1;
}

bool MultitaskViewModelByWlcomHandler::isCurrentDesktopShowingDesktop()
{
    //wlcom 内部已实现，不需要获取该状态
    return false;
}

void MultitaskViewModelByWlcomHandler::setCurrentDesktop(int desktopIndex)
{
    m_workspaceList[desktopIndex-1]->setActivate();
}

QRect MultitaskViewModelByWlcomHandler::getFullScreenGeometry(int desktopIndex, int screenIndex)
{
    return QGuiApplication::screens().at(screenIndex)->geometry();
}

bool MultitaskViewModelByWlcomHandler::hasModalChildWindow(QString windowId)
{
    Toplevel *toplevel =  getToplevel(windowId);
    if (toplevel){
        return toplevel->hasChildren();
    }
    return false;
}

bool MultitaskViewModelByWlcomHandler::isExtensionMode()
{  
    QRect bounding;
    for (int i = 0; i < QGuiApplication::screens().size(); i++) {
        bounding = bounding.united(QGuiApplication::screens().at(i)->geometry());
    }
    for (int i = 0; i < QGuiApplication::screens().size(); i++) {
        if (bounding == QGuiApplication::screens().at(i)->geometry()) {
            return false;
        };
    }

    return true;
}

int MultitaskViewModelByWlcomHandler::logicalScreenCount()
{
    const auto screenCount = QGuiApplication::screens().size();
    if (screenCount == 1 || !isExtensionMode()) {
        return 1;
    }
    return screenCount;
}


QList<WlcomWindow> MultitaskViewModelByWlcomHandler::createWindowList(int desktopIndex, int screenIndex)
{
    QList<WlcomWindow> windowList;

    const auto wnds = m_allWindowList;

    for (auto const wnd: wnds) {
        if (!shouldAcceptWindow(wnd)) {
            continue;
        }
        bool isOnDesktop = false;
//        qDebug() << "--------------" << wnd->title() << wnd;
        int workspacesize = wnd->workspaces().size();
        for (int i = 0; i < workspacesize; i++) {
//            qDebug() << wnd->workspaces()[i];
//            qDebug() <<m_workspaceList[desktopIndex - 1]->uuid();
            if (wnd->workspaces()[i] == (QString)m_workspaceList[desktopIndex - 1]->uuid()){
                isOnDesktop = true;
                break;
            }
        }
        if ((workspacesize != 0) && !isOnDesktop){
            continue;
        }
        QRect wndrect(wnd->point().x(),
                      wnd->point().y(),
                      wnd->size().width(),
                      wnd->size().height());
        if (!(QGuiApplication::screens().at(screenIndex)->geometry().contains(wndrect.center()))) {
            continue;
        }
        QString id =wnd->uuid();
        QString title = wnd->title();
        if (title.isEmpty()) {
            title = wnd->name();
        }
        WlcomWindow window(id.toInt(),
                      wnd->uuid(),
                      title,
                      wnd->point().x(),
                      wnd->point().y(),
                      wnd->size().width(),
                      wnd->size().height(),
                      wnd->isMinimized());

        windowList.append(window);

    }
    return windowList;
}

bool MultitaskViewModelByWlcomHandler::shouldAcceptWindow(Toplevel *toplevel)
{
    return true;
//    return !(toplevel->capabilities() & Toplevel::Capability::Switcher);
}

QString MultitaskViewModelByWlcomHandler::getDesktopUuid(int desktopIndex)
{
   return m_workspaceList[desktopIndex - 1]->uuid();
}

QString MultitaskViewModelByWlcomHandler::getOutputUuid(int outputIndex)
{
    if (m_outputList.size() == 1) {
        return m_outputList[0]->uuid();
    }

    QString outputName = "";
    if (QGuiApplication::screens().size() > outputIndex) {
        outputName = QGuiApplication::screens().at(outputIndex)->name();
    }

    for (int i= 0; i < m_outputList.size(); i++) {
        if (m_outputList[i]->name() == outputName) {
            return m_outputList[i]->uuid();
        }
    }

    qWarning() << "[Multitask]: getOutputUuid falied, invalid outputIndex" << outputIndex;
    return QString();
}

void MultitaskViewModelByWlcomHandler::setDesktopName(const int desktopIndex, const QString name)
{
    if (m_workspaceList.size() < desktopIndex) {
        return;
    }
    m_workspaceList[desktopIndex - 1]->setName(name);
}

void MultitaskViewModelByWlcomHandler::onWindowAdded(Toplevel *toplevel)
{
//    qDebug() << "onWindowAdded---------------" << toplevel->uuid() << toplevel->isActivated();
//        qDebug() << "onWindowAdded---------------(" << toplevel->point().x() << "," << toplevel->point().y() << ","
//                 << toplevel->size().width() << "," << toplevel->size().height();
    if (!shouldAcceptWindow(toplevel)) {
        //如果非独立子窗口的父窗口显示在任务视图，
        //激活非独立子窗口时，需要调整父窗口显示顺序，因此需要监听窗口状态变化
        if (toplevel->parent() != nullptr && shouldAcceptWindow(toplevel->parent())) {
            connect(toplevel, &Toplevel::stateUpdated, this, &MultitaskViewModelByWlcomHandler::onWindowUpdate);
        }
        return;
    }

    m_allWindowList.push_back(toplevel);
    connect(toplevel, &Toplevel::stateUpdated, this, &MultitaskViewModelByWlcomHandler::onWindowUpdate);
    connect(toplevel, &Toplevel::deleted, this, &MultitaskViewModelByWlcomHandler::onWindowDeleted);
    if (toplevel->icon() == "fallback" || toplevel->title().isEmpty()) {
        // wlcom未找到应用图标，则通过ukui-search接口查找desktop文件，进而获取应用图标
//        QString desktopFile;
//        getApplicationInfo()->desktopFilePathFromName(toplevel->appId(), desktopFile);
//        if (desktopFile.isEmpty()) {
//            desktopFile = desktopFileFromPid(toplevel->pid());
//        }
//        toplevel->setDesktopFile(desktopFile);
    }

//    if (MultitaskViewManagerByWlcom::isOpen) {
//        emit appWindowAdd(windowFromToplevel(toplevel));
//    }
}

void MultitaskViewModelByWlcomHandler::onOutputAdded(Output *output)
{
    qDebug() << "output-----------" << output->description();
    m_outputList.push_back(output);
}

void MultitaskViewModelByWlcomHandler::onWindowDeleted()
{
    Toplevel *toplevel = qobject_cast<Toplevel *>(sender());
    for (int i = 0; i < m_allWindowList.size(); i++){
        if (m_allWindowList[i]->uuid() == toplevel->uuid()){
            m_allWindowList.removeAt(i);
            break;
        }
    }
    QString id = toplevel->uuid();

//    qDebug()<<"toplevel"<<id;
    emit appWindowRemoved(id);
}

void MultitaskViewModelByWlcomHandler::onWindowActivated(Toplevel *toplevel)
{
    if (!shouldAcceptWindow(toplevel)) {
        //激活非独立子窗口时，调整父窗口显示顺序
        if (toplevel->parent() != nullptr && shouldAcceptWindow(toplevel->parent())) {
            int index = m_allWindowList.indexOf(toplevel->parent());
            if (index != -1) {
                m_allWindowList.move(index, m_allWindowList.size() - 1);
            }
        }

        return;
    }
    // KWin在发送windowActivated信号时
    // window对象是有可能为空的。对于多任务
    // 视图来说，只关心真正有窗口被激活的清空。
    // 因此，如果window对象为空，则不转发
    // 该信号
    if (toplevel->appId() == nullptr) {
        return;
    }
    if (toplevel->isActivated()) {
        int index = m_allWindowList.indexOf(toplevel);
        if (index != -1) {
            m_allWindowList.move(index, m_allWindowList.size() - 1);
        }
        emit appWindowActivated();
    }
}

void MultitaskViewModelByWlcomHandler::onWindowUpdate(Toplevel::Masks mask)
{
    Toplevel *toplevel = qobject_cast<Toplevel *>(sender());

    if (mask & Toplevel::Mask::Activated) {
        onWindowActivated(toplevel);
    }

    if (mask & Toplevel::Mask::Workspace) {
        QStringList workspaces = toplevel->workspaces();
        if (!workspaces.isEmpty()) {
            for (int i = 0; i <m_workspaceList.size(); i++) {
                if (m_workspaceList[i]->uuid() == workspaces.at(0)) {
                    onDesktopPresenceChanged(toplevel,0,i+1);
                    break;
                }
            }
        }
    }

}

void MultitaskViewModelByWlcomHandler::onDesktopPresenceChanged(Toplevel *toplevel, int oldDesktopIndex ,int newDesktopIndex)
{
    if (!shouldAcceptWindow(toplevel)) {
        return;
    }
    for (int i = 0; i < m_allWindowList.size(); i++){
        if (m_allWindowList[i]->uuid() == toplevel->uuid()){
            m_allWindowList[i] = toplevel;
            break;
        }
    }
    emit appWindowDesktopChanged(toplevel->uuid(), oldDesktopIndex, newDesktopIndex);
}


void MultitaskViewModelByWlcomHandler::onWorkSpaceAdded(Workspace *workspace)
{
    m_workspaceList.push_back(workspace);
    connect(workspace, &Workspace::stateUpdated, this, &MultitaskViewModelByWlcomHandler::onCurrentDesktopChanged);
    connect(workspace, &Workspace::deleted, this, &MultitaskViewModelByWlcomHandler::onWorkSpaceDeleted);
    emit workspaceHasCreated();
    emit desktopAppended(m_workspaceList.size());
}

void MultitaskViewModelByWlcomHandler::onWorkSpaceDeleted()
{
    Workspace *workspace = qobject_cast<Workspace *>(sender());
    for (int i = 0; i < m_workspaceList.size(); i++) {
        if (workspace->uuid() == (QString)m_workspaceList[i]->uuid()){
            m_workspaceList.removeAt(i);
            emit desktopRemoved(i+1);
            break;
        }
    }
}

void MultitaskViewModelByWlcomHandler::onCurrentDesktopChanged(Workspace::Masks mask)
{
    Workspace *workspace = qobject_cast<Workspace *>(sender());

    if (mask & Workspace::Mask::Position) {
        if (workspace->isActivated()){
            for (int i = 0; i < m_workspaceList.size(); i++) {
                if (workspace->uuid() == m_workspaceList[i]->uuid()){
                    m_workspaceList[i] = workspace;
                    emit currentDesktopChanged(0,i+1);
                    break;
                }
            }
        }
    }

    if (mask & Workspace::Mask::Activated) {
        if (workspace->isActivated()){
            for (int i = 0; i < m_workspaceList.size(); i++) {
                if (workspace->uuid() == m_workspaceList[i]->uuid()){
                    m_workspaceList[i] = workspace;
                    emit currentDesktopChanged(0,i+1);
                    break;
                }
            }
        }
    }

}


void MultitaskViewModelByWlcomHandler::connectSignals()
{
}
