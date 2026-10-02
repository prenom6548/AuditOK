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
#include "custom_dialog.h"
#include "common_ui.h"
#include "global_variable.h"

CustomDialog::CustomDialog(QWidget *parent, bool isSubWindow) :
    kdk::KDialog(parent),
    m_isSubWindow(isSubWindow),
    m_parentWindow(parent)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    if (m_parentWindow != nullptr && m_isSubWindow == false) {
        m_isSubWindow = true;
    }

    if (m_isSubWindow == true) {
        setWindowModality(Qt::ApplicationModal);
    }
    setWindowIcon(QIcon::fromTheme(APP_ICON_NAME));
}

CustomDialog::~CustomDialog()
{
}

void CustomDialog::show()
{
    if (m_isSubWindow == true && m_parentWindow != nullptr) {
        centerTheWindow(this, m_parentWindow);
    }
    QWidget::show();
}

int CustomDialog::exec()
{
    QWidget::show();
    if (m_isSubWindow == true && m_parentWindow != nullptr) {
        centerTheWindow(this, m_parentWindow);
    }
    return QDialog::exec();
}

void CustomDialog::setWindowTitle(const QString& widgetName)
{
    kdk::KDialog::setWindowTitle(widgetName);
    QWidget::setWindowTitle(widgetName);
}
