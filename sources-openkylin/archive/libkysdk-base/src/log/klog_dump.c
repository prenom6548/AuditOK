/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */


#include "klog_dump.h"
#include "libkyconf.h"
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef K_MAX_PATH
#define K_MAX_PATH 4096
#endif //K_MAX_PATH

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

static int verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/etc", 4);
}

int klog_rotate_init(int cid, const char *name, const char *rootpath)
{
	const char *ruler = kdk_conf_get_value(cid, "DUMP", "rules");
	if (strcasecmp(ruler, "none") == 0)
		return 0;
	const char *oversize = kdk_conf_get_value(cid, "DUMP", "size");
	int compress = atoi(kdk_conf_get_value(cid, "DUMP", "compress"));

	char tmp[1025];
	snprintf(tmp, 1024, "/etc/kysdk/kysdk-base/logrotate.d/%s", name);
	char canonical_filename[K_MAX_PATH] = "\0";
	memset(canonical_filename, 0, sizeof(canonical_filename));
	realpath(tmp, canonical_filename);
	if(!verify_file(canonical_filename))
	{
		return -1;
	}
	FILE *fp = fopen(canonical_filename, "wt+");
	if (!fp)
		return -1;
	
	snprintf(tmp, 1024, "%s/%s* {\n", rootpath, name);
	fputs(tmp, fp);
	if (strcasecmp(ruler, "daily") == 0)
		fputs("\tdaily\n", fp);
	else if (strcasecmp(ruler, "weekly") == 0)
		fputs("\tweekly\n", fp);
	else
		fputs("\tmonthly\n", fp);
	
	fputs("\trotate 7\n", fp);
	fputs("\tnotifempty\n", fp);
	fputs("\tnocopytruncate\n", fp);
	if (compress)
		fputs("\tcompress\n", fp);
	else
		fputs("\tnocompress\n", fp);
	
	if (strcasecmp(ruler, "size") == 0)
	{
		int size = atoi(oversize);
		snprintf(tmp, 1024, "\tsize %d", size);
		char *p = oversize;
		while (*p && isdigit(p))
			p ++;
		if (strncasecmp(p, "M", 1) == 0)
			strncat(tmp, "M", 1024);
		else if (strncasecmp(p, "K", 1) == 0)
			strncat(tmp, "k", 1024);
		else if (strncasecmp(p, "G", 1) == 0)
			strncat(tmp, "G", 1024);
		strncat(tmp, "\n", 1024);
		fputs(tmp, fp);
	}
	fputs("}", fp);
	fclose(fp);
	return 0;
}
