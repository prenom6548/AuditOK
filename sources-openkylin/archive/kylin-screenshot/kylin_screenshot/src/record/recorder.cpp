/* Copyright (C) 2020, KylinSoft Co., Ltd.
*
* This file is part of Kylin-Screenshot.
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
#include "recorder.h"

#include "ssrtools.h"
#include "mypopup.h"
#include "src/tools/capturetool.h"

Recorder::Recorder(QWidget *parent)
{
    mp =  new mypopup(parent);
    ssr = new ssrtools(parent, mp);
    m_pushbutton_save = new QPushButton(parent);
    m_pushbutton_save->setAccessibleName("kylin-screenshot_QPushButton_pushbutton_save");
    m_pushbutton_cancel = new QPushButton(parent);
    m_pushbutton_cancel->setAccessibleName("kylin-screenshot_QPushButton_pushbutton_cancel");

    connect(m_pushbutton_cancel, SIGNAL(clicked()), this, SLOT(record_cancel_clicked()));
    connect(m_pushbutton_save, SIGNAL(clicked()), this, SLOT(record_save_clicked()));
}

void Recorder::OnRecordCursorClicked(bool isPressed)
{
    ssr->SetVideoRecordRursor(isPressed);
}

void Recorder::OnRecordFollowMouseClicked(bool isPressed)
{
    ssr->SetVideoRecordFollowMouse(isPressed);
}

void Recorder::OnRecordAudioClicked(bool isPressed)
{
    ssr->SetVideoRecordRursor(isPressed);
}

void Recorder::OnRecordOptionClicked()
{

}

void Recorder::OnRecordStartClicked()
{
    ssr->OnRecordStart();
    m_pushbutton_cancel->show();
    m_pushbutton_save->show();
}

void Recorder::record_save_clicked()
{
    m_pushbutton_cancel->hide();
    m_pushbutton_save->hide();
//        syslog(LOG_INFO, "will save, cancel and save button hide");
    ssr->OnRecordSave();
}

void Recorder::record_cancel_clicked()
{
    m_pushbutton_cancel->hide();
    m_pushbutton_save->hide();
//        syslog(LOG_INFO, "will save, cancel and save button hide");
    ssr->OnRecordCancel();
}
