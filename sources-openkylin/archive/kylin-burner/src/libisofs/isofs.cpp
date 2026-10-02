/***************************************************************************
                 	isofs.c  -  libisofs implementation
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "isofs.h"
#include "rock.h"


/**
 * Creates the linked list of the volume descriptors
 */
iso_vol_desc *ReadISO9660(readfunc *read,sector_t sector,void *udata) {

	int i;
	struct iso_volume_descriptor buf;
	iso_vol_desc *first = nullptr;
	iso_vol_desc *current = nullptr;
	iso_vol_desc *prev = nullptr;

	for (i=0;i<100;i++) {
		if (read( (char*) &buf, sector+i+16, 1, udata) != 1 ) {
			FreeISO9660(first);
			return nullptr;
		}
		if (!memcmp(ISO_STANDARD_ID,&buf.id,5)) {
			switch ( isonum_711(&buf.type[0]) ) {

				case ISO_VD_BOOT:
				case ISO_VD_PRIMARY:
				case ISO_VD_SUPPLEMENTARY:
					current=(iso_vol_desc*) malloc(sizeof(iso_vol_desc));
					if (!current) {
						FreeISO9660(first);
						return nullptr;
					}
					current->prev = prev;
					current->next = nullptr;
					if (prev) prev->next = current;
					memcpy(&(current->data),&buf,2048);
					if (!first) first = current;
					prev = current;
					break;

				case ISO_VD_END:
					return first;
			}
		} else if (!memcmp(HS_STANDARD_ID,(struct hs_volume_descriptor*) &buf,5)) {
			/* High Sierra format not supported (yet) */
		}
	}

	return first;
}

/**
 * Frees the linked list of volume descriptors
 */
void FreeISO9660(iso_vol_desc *data) {

	iso_vol_desc *current;


	while (data) {
		current=data;
		data=current->next;
		free(current);
	}
}



