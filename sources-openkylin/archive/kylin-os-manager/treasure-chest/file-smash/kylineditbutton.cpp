/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "kylineditbutton.h"
#include <QHBoxLayout>
#include <QLineEdit>

KylinEditButton::KylinEditButton(QLineEdit *edit) : QPushButton(edit)
{
    QSize size = QSize(40, edit->sizeHint().height());
    //    setMinimumSize(size);
    //    setMaximumSize(size);
    this->setFocusPolicy(Qt::NoFocus);
    this->setFlat(true);
    this->setIcon(QIcon("://res/folder.png"));
    this->setIconSize(QSize(16, 16));
    this->setCursor(QCursor(Qt::PointingHandCursor));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(this);
    edit->setLayout(layout);
    edit->setTextMargins(0, 1, size.width(), 1);
}
