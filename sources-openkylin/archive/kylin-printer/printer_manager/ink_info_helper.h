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
#ifndef __INK_INFO_HELPER__
#define __INK_INFO_HELPER__

#include "single_thread_worker.h"
#include "ukui_printer.h"

class InkInfoHelper : public  SingleThreadWorker
{
    Q_OBJECT
public:
    InkInfoHelper(UkuiUsbPrinter &printer);
    ~InkInfoHelper();
Q_SIGNALS:
    void succeed();
private:
    void dowork() override;
    void getInkInfo(const QString &uri);
    UkuiUsbPrinter &m_printer;
};

#endif // __INK_INFO_HELPER__