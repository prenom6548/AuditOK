/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QWidget>
#include <QIcon>
#include <QProcess>
#include <QGSettings>
#include <QAction>
#include <QMenu>
#include <QToolButton>
#include <QDialog>

#include "controller/Interaction.h"
#include "global/corevar.h"
#include "global/viewvar.h"
#include "controller/core/viewcontrol.h"

// Static members initialization
ViewVar* ViewVar::m_viewVar = nullptr;
viewControl* viewControl::m_viewControl = nullptr;

// Interaction Mock
Interaction::~Interaction() {}
static Interaction* g_interaction = nullptr;
Interaction* Interaction::getInstance() {
    return g_interaction;
}
void Interaction::allowLocationYes() {}
void Interaction::allowLocationNo() {}
void Interaction::closePrograme() {}

// CoreVar Mock
QVariant CoreVar::getSettings(const QString &key) {
    if (key == "allow-locate") return QString("1");
    return QVariant();
}
void CoreVar::setSettings(const QString &key, const QVariant &value) {
    qDebug() << "Setting" << key << "to" << value;
}

// ViewVar Mock
ViewVar* ViewVar::getInstance() {
    if (!m_viewVar) m_viewVar = new ViewVar();
    return m_viewVar;
}
ViewVar::ViewVar() {}

// viewControl Mock
viewControl* viewControl::getInstance() {
    if (!m_viewControl) m_viewControl = new viewControl();
    return m_viewControl;
}
viewControl::viewControl() : QObject(nullptr) {}
void viewControl::autoStart() {}
void viewControl::updateIsStart(bool) {}
void viewControl::toShowView() {}
void viewControl::toShowViewAndRefresh() {}
QWidget* viewControl::activeWindow() { 
    static QWidget dummy;
    return &dummy; 
}
