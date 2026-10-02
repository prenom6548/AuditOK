/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "stdio.h"
#include <unistd.h>
#include "../libkypnp.h"

void on_signal(char *action, char *sys_path)
{
    printf("%s: %s\n", action, sys_path);
}

int main(int argc, char *argv[])
{
    kdk_pnp_info *info = kdk_pnp_get_info();
    for(kdk_pnp_info *tmp = info; tmp; tmp = tmp->next)
    {
        printf("sysname: %s\n", tmp->sysname);
        printf("\tsysnum: %s\n", tmp->sysnum);
        printf("\tid: %s\n", tmp->id);
        printf("\tmem: %s\n", tmp->mem);
        printf("\thid: %s\n", tmp->hid);
        printf("\tcontrol: %s\n", tmp->control);
    }

    kdk_pnp_register_switch_connect_handle(on_signal, NULL);
    kdk_pnp_register_switch_disconnect_handle(on_signal, NULL);

    sleep(10);

    kdk_pnp_unregister_switch_connect_handle();
    kdk_pnp_unregister_switch_disconnect_handle();
    return 0;
}
