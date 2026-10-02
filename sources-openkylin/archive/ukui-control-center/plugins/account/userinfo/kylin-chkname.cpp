/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <libintl.h>
#include <locale.h>

#include "kylin-chkname.h"

#define USERNAME_MAXLEN 32
#define RESERVED_NAMES "/usr/share/kylin-chkname/reserved-names"

#define _(STRING) gettext(STRING)

static int reserve_check(const char *name)
{
    FILE *fp;
    char buf[USERNAME_MAXLEN+2];
    int find = 0;

    fp = fopen(RESERVED_NAMES, "r");
    if (NULL == fp) {
        syslog(LOG_INFO, "Open reserved-names file failed: %s", strerror(errno));
        find = 2;
        return find;
    }

    while (fgets(buf, USERNAME_MAXLEN+2, fp) != NULL) {
        if (!strnlen(buf, sizeof(buf)) || !strncmp(buf, "#", 1))
           continue;
        buf[strnlen(buf, sizeof(buf)) - 1] = '\0';
        if (!strcmp(name, buf)) {
            find = 1;
            break;
        }
    }

    fclose(fp);
    return find;
}

int  kylin_username_check(const char *name, int reserve)
{
   /*
    * User/group names must match gnu e-regex:
    *    [a-zA-Z0-9_.][a-zA-Z0-9_.-]{0,30}[a-zA-Z0-9_.$-]?
    *
    * as a non-POSIX, extension, allow "$" as the last char for
    * sake of Samba 3.x "add machine script"
    *
    * Also do not allow fully numeric, hexadecimal, octal number names
    * or just "." or "..".
    */
    if (name == NULL)
        return NAME_ERROR;

    if (strnlen(name, sizeof(name)) == 0 || strnlen(name, sizeof(name)) > USERNAME_MAXLEN)
        return LENGTH_ERROR;

    if (reserve && reserve_check(name)) {
        if (reserve_check(name) == 1)
            return RESERVED_ERROR;
        else
            return OPEN_RESERVED_NAMES_ERROR;
    }

    if ('.' == *name && (('.' == name[1] && '\0' == name[2]) ||
                          '\0' == name[1]))
        return REGEX_ERROR;

    if (!((*name >= 'a' && *name <= 'z') ||
          (*name >= 'A' && *name <= 'Z') ||
          (*name >= '0' && *name <= '9') ||
          *name == '_' || *name == '.'))
        return FIRST_CHAR_ERROR;

    int numberic = 1;
    int hex = 1;
    int octal = 1;
    int chars_checked = 1;

    if (*name != '0' || *(name + 1) != 'x')
        hex = 0;
    if (*name != '0' || *(name + 1) != 'o')
        octal = 0;
    if (*name < '0' || *name > '9')
        numberic = 0;

    while ('\0' != *++name) {
        if (!((*name >= 'a' && *name <= 'z') ||
              (*name >= 'A' && *name <= 'Z') ||
              (*name >= '0' && *name <= '9') ||
              *name == '_' ||
              *name == '.' ||
              *name == '-' ||
              (*name == '$' && name[1] == '\0')))
                return REGEX_ERROR;

        if (hex && chars_checked >= 2) {
            if ((*name < '0' || *name > '9') &&
                (*name < 'A' || *name > 'F') &&
                (*name < 'a' || *name > 'f'))
                 hex = 0;
        }

        if (octal && chars_checked >= 2) {
            if (*name < '0' || *name > '7')
               octal = 0;
        }

        if (numberic) {
            if (*name < '0' || *name > '9')
               numberic = 0;
        }
        chars_checked++;
    }
    if (hex)
        return HEX_ERROR;

    if (octal)
        return OCTAL_ERROR;

    if (numberic)
        return NUMBERIC_ERROR;

    return CHECK_SUCCESS;
}

char *kylin_username_strerror(int err_num)
{
    setlocale(LC_ALL, "");
    bindtextdomain("kylin-chkname", "/usr/share/locale");
    textdomain("kylin-chkname");

    switch(err_num) {
        case CHECK_SUCCESS:
            return _("Success");
        case LENGTH_ERROR:
            return _("Username's length must be between 1 and 32 characters");
        case REGEX_ERROR:
            return _("Invalid username regex");
        case RESERVED_ERROR:
            return _("Reserved username");
        case NAME_ERROR:
            return _("Parameter name is null");
        case OPEN_RESERVED_NAMES_ERROR:
            return _("Open reserved-names file failed");
        case FIRST_CHAR_ERROR:
            return _("Username must start with a letter, number, dot, or underscore");
        case HEX_ERROR:
            return _("Username cannot be a hexadecimal number");
        case OCTAL_ERROR:
            return _("Username cannot be octal number");
        case NUMBERIC_ERROR:
            return _("Username cannot be fully numeric");
        default:
            return _("Unknown error");
    }
}
