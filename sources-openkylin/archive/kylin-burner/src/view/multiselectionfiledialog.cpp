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

#include "multiselectionfiledialog.h"

#include <QDialogButtonBox>

MultiSelectionFileDialog::MultiSelectionFileDialog(QWidget *parent,
                                                   const QString &caption,
                                                   const QString directory,
                                                   const QString filter)
    : QFileDialog(parent, caption, directory, filter)
{
    this->setObjectName("kylin-burner_MultiSelectionFileDialog");
    m_oListView = findChild<QListView *>("listView");
    m_oTreeView = findChild<QTreeView *>();
    QDialogButtonBox *btn = findChild<QDialogButtonBox *>("buttonBox");
    if (btn)
    {
        disconnect(btn, SIGNAL(accepted()), this, SLOT(accept()));
        connect(btn, SIGNAL(accepted()), this, SLOT(slotsAccept()));
    }
    m_oFileName = findChild<QLineEdit *>("fileNameEdit");
}

void MultiSelectionFileDialog::setExtendedSelection()
{
    if (m_oListView)
        m_oListView->setSelectionMode(QListView::ExtendedSelection);
    if (m_oTreeView)
        m_oTreeView->setSelectionMode(QTreeView::ExtendedSelection);
}

void MultiSelectionFileDialog::setSingleSelection()
{
    if (m_oListView)
        m_oListView->setSelectionMode(QListView::SingleSelection);
    if (m_oTreeView)
        m_oTreeView->setSelectionMode(QTreeView::SingleSelection);
}

void MultiSelectionFileDialog::reset()
{
    if (m_oListView) m_oListView->clearSelection();
    if (m_oTreeView) m_oTreeView->clearSelection();
    if (m_oFileName) m_oFileName->setText("");
}

void MultiSelectionFileDialog::slotsAccept()
{
    QDialog::accept();
}

MultiSelectionFileDialog::~MultiSelectionFileDialog()
{}
