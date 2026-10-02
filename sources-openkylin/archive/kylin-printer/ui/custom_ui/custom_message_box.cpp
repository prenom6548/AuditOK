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
#include <QWindow>
#include <QTimer>
#include "custom_message_box.h"
#include "common_ui.h"

CustomMessageBox::CustomMessageBox(QWidget *parent) :
      kdk::KMessageBox(parent)
{

}

CustomMessageBox::CustomMessageBox(kdk::KMessageBox::Icon icon,
                                   const QString &title,
                                   const QString &text,
                                   kdk::KMessageBox::StandardButtons buttons,
                                   QWidget *parent) :
      kdk::KMessageBox(parent)
{
    setIcon(icon);
    setText(text);
    setStandardButtons(buttons);
}

CustomMessageBox::CustomMessageBox(QIcon icon,
                                   const QString &title,
                                   const QString &text,
                                   kdk::KMessageBox::StandardButtons buttons,
                                   QWidget *parent) :
      kdk::KMessageBox(parent)
{
    setCustomIcon(icon);
    setText(text);
    setStandardButtons(buttons);
}

QPushButton *CustomMessageBox::addButton(const QString &text, ButtonRole role)
{
    QPushButton *button = kdk::KMessageBox::addButton(text, role);
    switch (role)
    {
    case kdk::KMessageBox::YesRole:
        m_yesButton = button;
        break;
    case kdk::KMessageBox::NoRole:
        m_noButton = button;
        break;
    default:
        break;
    }
    return button;
}

QPushButton *CustomMessageBox::clickedButton()
{
    return dynamic_cast<QPushButton *>(kdk::KMessageBox::clickedButton());
}

int CustomMessageBox::exec()
{
#ifdef UNIT_TEST
    auto autoCloseTimer = new QTimer(this);
    connect(autoCloseTimer, &QTimer::timeout, this, &CustomMessageBox::close);
    autoCloseTimer->start(500);
#endif
    m_ret = QDialog::exec();
    return m_ret;
}

void CustomMessageBox::show()
{
    kdk::KMessageBox::show();
}
