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


#ifndef MULTISELECTIONFILEDIALOG_H
#define MULTISELECTIONFILEDIALOG_H

#include <QObject>
#include <QFileDialog>
#include <QListView>
#include <QTreeView>
#include <QLineEdit>

class MultiSelectionFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    MultiSelectionFileDialog(QWidget *parent = nullptr,
                             const QString &caption = QString(),
                             const QString directory = QString(),
                             const QString filter = QString());
    ~MultiSelectionFileDialog();
public:
    void setExtendedSelection();
    void setSingleSelection();
    void reset();
private slots:
    void slotsAccept();

private:
    QListView      *m_oListView;
    QTreeView      *m_oTreeView;
    QLineEdit      *m_oFileName;
};

#endif // MULTISELECTIONFILEDIALOG_H
