/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef DISKCHECK_H
#define DISKCHECK_H

#include <QWidget>
#include <QFileDialog>

namespace Ui {
class DiskCheck;
}

class DiskCheck : public QWidget
{
    friend class DiskCheckTest;
    Q_OBJECT
Q_SIGNALS:
    void change(bool);
public:
    explicit DiskCheck(QWidget *parent = nullptr);
    ~DiskCheck();
public slots:
    void accept();
     void slotChangeUIFontSize(int fontSize);
     void slotChangeDiscDisplayInfo(void *cdrom, QString discInfo);

public:
    bool check();
private slots:
    void slotsAddMedium(void *);
    void slotsRemoveMedium(void *);
    void slotsChangeMedium(int);
    void slotsBrowseFile();
private:
    QStringList    m_oCheckFiles;
    QList<void *>  m_oCDROMS;
    QFileDialog*   m_fileDialog;
private:
    Ui::DiskCheck *ui;
};

#endif // DISKCHECK_H
