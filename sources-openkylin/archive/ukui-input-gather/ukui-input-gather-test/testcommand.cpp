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

#include "testcommand.h"

testCommand::testCommand()
{

}
testCommand::~testCommand()
{

}
void testCommand::testCommandHelpStr()
{
    qDebug()<<"-----Welcome to the help interface-----      "<<endl
            <<"     ukui-input-test --help                  "<<endl
            <<"     ukui-input-test --connectServer         "<<endl
            <<"     ukui-input-test --debug-events          ";

}
void testCommand::testCommandConnectServer()
{
    qDebug()<<"begin connect server..."<<endl;


}
void testCommand::testCommanDebugEvents()
{
//    qDebug()<<"debug-events...."<<endl;
//    UKUIInputGatherClient inputGatherClient;
//    inputGatherClient.setEventCallBack(testCommand::getEvent2());
//    inputGatherClient.startToReceiveEvent();

}

//void testCommand::getEvent2(Event *event)

