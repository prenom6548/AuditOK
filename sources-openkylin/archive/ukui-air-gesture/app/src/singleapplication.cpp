/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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

#include "singleapplication.h"

#include "dbusadaptor.h"

#include <QDebug>

SingleApplication::SingleApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    const DBusManager::RegisterResult dbusResult = m_dbusManager.registerAll();

    switch (dbusResult) {
    case DBusManager::RegisterResult::Registered:
        m_initResult = InitResult::Success;
        break;
    case DBusManager::RegisterResult::AlreadyRunning:
        m_initResult = InitResult::AlreadyRunning;
        return;
    case DBusManager::RegisterResult::Failed:
        m_initResult = InitResult::DBusFailed;
        return;
    }

    setupGestureMonitoring();
    syncPythonProcess();
}

SingleApplication::InitResult SingleApplication::getInitResult() const
{
    return m_initResult;
}

void SingleApplication::setupGestureMonitoring()
{
    if (QGSettings::isSchemaInstalled(AirGestureSettings::kGestureSchema)) {
        m_pGestureSettings = new QGSettings(AirGestureSettings::kGestureSchema, "", this);

        connect(m_pGestureSettings, &QGSettings::changed, this, [this](const QString &) {
            syncPythonProcess();
        });
    }

    if (DBusAdaptor *adaptor = m_dbusManager.adaptor()) {
        connect(adaptor, &DBusAdaptor::trialStateChanged, this, &SingleApplication::syncPythonProcess);
    }
}

bool SingleApplication::isRecognitionNeeded() const
{
    if (DBusAdaptor *adaptor = m_dbusManager.adaptor()) {
        if (adaptor->isTrialActive()) {
            return true;
        }
    }

    return AirGestureSettings::anyGestureEnabled(m_pGestureSettings);
}

void SingleApplication::syncPythonProcess()
{
    const bool needed = isRecognitionNeeded();

    if (!needed) {
        if (m_pythonProcessManager.isRunning()) {
            qInfo() << "Stopping Python subprocess: no gesture recognition needed.";
            m_pythonProcessManager.stop();
        }
        return;
    }

    if (m_pythonProcessManager.isRunning()) {
        return;
    }

    qInfo() << "Starting Python subprocess: gesture recognition needed.";
    if (!m_pythonProcessManager.start()) {
        qWarning() << "Failed to start Python subprocess for gesture recognition.";
        if (m_initResult == InitResult::Success) {
            m_initResult = InitResult::PythonFailed;
        }
    }
}
