/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 * Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */
#include "ukuimodule.h"

#include <QFileInfo>
#include <QDebug>
#include "modulemanager.h"
#include <QLatin1String>

UkuiModule::UkuiModule(const XdgDesktopFile& file, QObject* parent) : QProcess(parent)
                                                                    , file(file)
                                                                    , fileName(QFileInfo(file.fileName()).fileName())
                                                                    , mIsTerminating(false)
{
    restartNum = 0;
    QProcess::setProcessChannelMode(QProcess::ForwardedChannels);
    connect(this, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(updateState(QProcess::ProcessState)));
}

void UkuiModule::startUKUIModule(const QStringList& arguments/* = QStringList()*/, bool keepArgs/* = false*/)
{
    mIsTerminating = false;
    QStringList args = file.expandExecString(arguments);
    if (keepArgs) {
        file.setValue(QLatin1String("Exec"), args.join(" "));
    }
    QString command = args.takeFirst();

    qDebug() << "Start ukui module: " << command << "args: " << args;
    QProcess::start(command, args);

    ModuleManager::insertStartupList(std::move(command));
}

void UkuiModule::terminate()
{
    mIsTerminating = true;
    QProcess::terminate();
}

bool UkuiModule::isTerminating()
{
    return mIsTerminating;
}

void UkuiModule::updateState(QProcess::ProcessState newState)
{
    if (newState != QProcess::Starting)
        emit moduleStateChanged(fileName, (newState == QProcess::Running));
}

QString UkuiModule::getProcName()
{
    return file.name();
}
