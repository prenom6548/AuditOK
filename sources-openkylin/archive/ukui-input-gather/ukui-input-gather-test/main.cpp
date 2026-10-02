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
 *
 */

#include <QCoreApplication>

#include <QDebug>

#include <functional>
#include <QTime>
#include <QtCore/qcommandlineoption.h>
#include <QtCore/qcommandlineparser.h>
#include <string.h>

#include "inputgatherclient.h"
#include "socketclient.h"
#include "ukuiinputgatherclient.h"
#include "testcommand.h"

void getEvent2(Event *event);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if(!argv[1]){
        qDebug()<<"please input parse..."<<endl<<
                  "may be you can input parse  "<<"--help";
        return 0;
    }

    UKUIInputGatherClient inputGatherClient;
    testCommand testTool;
    QCommandLineParser parser;
    QCommandLineOption showHelpInfo("help","help info");
    QCommandLineOption showConnectServer("connectServer","show info for connect");
    QCommandLineOption showDebugEvents("debug-events","show info for events");
    QCommandLineOption callMethodRequest("call", "call method request");
    parser.addOption(showHelpInfo);
    parser.addOption(showConnectServer);
    parser.addOption(showDebugEvents);
    parser.addOption(callMethodRequest);
    parser.process(a);

    if(parser.isSet("help")){
        testTool.testCommandHelpStr();
        return 0;
    }
    if(parser.isSet("connectServer")){
        inputGatherClient.setEventCallBack(getEvent2);
        inputGatherClient.startToReceiveEvent();
        return 0;
    }
    if(parser.isSet("debug-events")){
        inputGatherClient.setEventCallBack(getEvent2);
        inputGatherClient.startToReceiveEvent();
    }
    if (parser.isSet("call")) {
        QString func = parser.positionalArguments().at(0);

        if (func == "libinputTabletSwitchState") {
            qDebug() << "libinputTabletSwitchState return:" << inputGatherClient.libinputTabletSwitchState();
            return 0;
        }
    }
    a.exec();
    return 0;
}


void getEvent2(Event *event)
{
    testCommand testTool;
    testTool.getEvent2(event);
}
