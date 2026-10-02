/***************************************************************************
             isofs.h  -  include this file to use libisofs
                             -------------------
    begin                : Oct 25 2002
    copyright            : (C) 2002 by Szombathelyi Gy鰎gy
    email                : gyurco@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ISOFS_H
#define ISOFS_H

#include <sys/time.h>

#include "iso_fs.h"
#include "el_torito.h"

typedef long sector_t;

typedef struct _rr_entry {
	int		len;	/* length of structure */
	char	*name; /* Name from 'NM' */
	char 	*sl;   /* symbolic link data */
	time_t	t_creat;
	time_t  rr_st_mtime;
	time_t  rr_st_atime;
	time_t  rr_st_ctime;
	time_t  t_backup;
	time_t  t_expire;
	time_t  t_effect;
	int		mode;  /* POSIX file modes */
	int		nlink;
	int		uid;
	int		gid;
	int		serno;
	int		dev_major;
	int		dev_minor;
	int		pl;	/* parent location */
	int		cl; /* child location */
	int		re; /* relocated */
	char	z_algo[2]; /* zizofs algorithm */
	char	z_params[2]; /* zizofs parameters */
	int		z_size; /* zizofs real_size */
} rr_entry;

typedef struct _iso_vol_desc {
	struct _iso_vol_desc	*next;
	struct _iso_vol_desc	*prev;
	struct iso_volume_descriptor	data;	
} iso_vol_desc;

typedef struct _boot_entry {
	struct _boot_entry	*next;
	struct _boot_entry	*prev;
	struct _boot_entry  *parent;
	struct _boot_entry  *child;
	char	data[32];
} boot_entry;

typedef struct _boot_head {
	struct validation_entry	ventry;
	struct _boot_entry	*defentry;
	struct _boot_entry  *sections;
} boot_head;

/**
 * this callback function needs to read 'len' sectors from 'start' into 'buf' 
 */
typedef int readfunc(char *buf,sector_t start, int len,void *);



/**
 * Creates the linked list of the volume descriptors
 * 'sector' is the starting sector number of where the filesystem start
 * (starting sector of a session on a CD-ROM)
 * If the function fails, returns NULL
 * Don't forget to call FreeISO9660 after using the volume descriptor list!
 */
iso_vol_desc *ReadISO9660(readfunc *read,sector_t sector,void *udata);

/**
 * Frees the linked list of volume descriptors.
 */
void FreeISO9660(iso_vol_desc *data);


#endif
