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

#include "view/chatmsg/chatmsg.h"
#include "view/friendlist/friendlist.h"
#include "view/trayicon/trayiconwid.h"
#include "view/localinfo/localinfo.h"
#include "view/friendlist/homepagesearch/searchmsglist.h"
#include "view/friendlist/homepagesearch/search_msg_model.h"
#include "view/kyview.h"
#include "network/tcp_server.h"
#include "network/udp_socket.h"
#include "network/tcp_client.h"
#include "network/tcp_module.h"
