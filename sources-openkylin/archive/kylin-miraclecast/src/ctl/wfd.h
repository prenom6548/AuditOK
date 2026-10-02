/*
 * MiracleCast - Wifi-Display/Miracast Implementation
 *
 * Copyright (c) 2014 Andrey Gusakov <andrey.gusakov@cogentembedded.com>
 * Copyright (c) 2013-2014 David Herrmann <dh.herrmann@gmail.com>
 *
 * MiracleCast is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * MiracleCast is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MiracleCast; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WFD_H
#define WFD_H

#define CEA_1080P_25FPS_CFG (1<<16 | 1<<15 | 1<<12 | 1<<10 | 1<<7 | 1<<5)
#define VESA_19201200_30FPS_CFG 0x15555555
#define HH_960540_30FPS_CFG 0x00000555

void wfd_print_resolutions(char * prefix);
int vfd_get_cea_resolution(uint32_t mask, int *hres, int *vres);
int vfd_get_vesa_resolution(uint32_t mask, int *hres, int *vres);
int vfd_get_hh_resolution(uint32_t mask, int *hres, int *vres);

void pickup_cea(int width, int height, uint32_t *pOut);
void pickup_vesa(int width, int height, uint32_t *pOut);
void pickup_hh(int width, int height, uint32_t *pOut);

#endif /* WFD_H */
