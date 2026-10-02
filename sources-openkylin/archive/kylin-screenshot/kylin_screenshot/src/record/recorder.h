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
#ifndef RECORD_H
#define RECORD_H

#include <QObject>

#include "src/widgets/capture/capturebutton.h"

class ssrtools;
class mypopup;

class Recorder : public QObject
{
    Q_OBJECT
    friend class CaptureWidget;
public:
    explicit Recorder(QWidget *parent = nullptr);

    void OnRecordCursorClicked(bool isPressed);
    void OnRecordFollowMouseClicked(bool isPressed);
    void OnRecordAudioClicked(bool isPressed);
    void OnRecordOptionClicked();
    void OnRecordStartClicked();

private:
    ssrtools* ssr;
    mypopup *mp;
    QPushButton *m_pushbutton_save, *m_pushbutton_cancel;

private slots:
    void record_save_clicked();
    void record_cancel_clicked();
};

#endif // RECORD_H
