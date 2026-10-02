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
#ifndef KYLIN_PRINTER_CONNECT_H
#define KYLIN_PRINTER_CONNECT_H

int try_connect(const char *addrname = nullptr, int port = 0, const char *data = nullptr);
bool pingCheck(const QString &destination);
bool checkLocalhostPort(int port);
#endif // KYLIN_PRINTER_CONNECT_H