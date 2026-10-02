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
#ifndef __SINGLE_H__
#define __SINGLE_H__

static int get_fd(char *p_file_path_in , int i_flag_in);
extern int judge_single(int i_flag_in);
extern int unlock_uninstall_file();
static int write_pid(char *p_file_path_in);
extern int read_pid(int i_flag_in);
extern int catch_signal();
extern void sig_callback_35(int);
extern int kill_signal(int);
extern void init_export_args(char *);
extern int lock_extra_update_lock(void);
extern int unlock_extra_update_lock(void);

#endif
