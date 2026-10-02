/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "CustomBoxUtil.h"

QMessageBox *CustomBoxUtil::createCritialBox(QString windowTitle,QString text, QString info,QWidget *parent)
{
    QMessageBox *msgBox = new QMessageBox(parent);
    QPushButton *btn = new QPushButton(parent);
    btn->setText(QObject::tr("OK"));
    msgBox->setIcon(QMessageBox::Critical);
    msgBox->addButton(btn, QMessageBox::AcceptRole);
    msgBox->setContextMenuPolicy(Qt::NoContextMenu);
    if (!windowTitle.isEmpty())
    {
        msgBox->setWindowTitle(windowTitle);

    }
    if (!text.isEmpty())
    {
        msgBox->setText(text);
    }
    if (!info.isEmpty())
    {
        msgBox->setInformativeText(info);
    }
    return msgBox;
}
