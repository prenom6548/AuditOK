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
#include "cups_debug_logging_checkbox.h"
#include "ukui_printer.h"

#include <QVBoxLayout>
CupsDebugLoggingCheckbox::CupsDebugLoggingCheckbox(QWidget *parent) :
    CustomCheckBox(parent)
{
    setText(tr("Retain debug info for troubleshooting"));
    setChecked(UkuiPrinterManager::getInstance().getCupsDebugLogging());
}

CupsDebugLoggingCheckbox::~CupsDebugLoggingCheckbox()
{

}

void CupsDebugLoggingCheckbox::saveDebugLogging()
{
    UkuiPrinterManager::getInstance().setCupsDebugLogging(isChecked());
	return ;
}
