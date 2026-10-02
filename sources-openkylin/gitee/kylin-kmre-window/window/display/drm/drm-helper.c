/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include <errno.h>

#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xf86drm.h>
#include <drm_fourcc.h>

static int dev_fd = -1;

extern int authenticate_magic(drm_magic_t magic);

static char *drm_construct_id_path_tag(drmDevicePtr device)
{
   char *tag = NULL;

   if (device->bustype == DRM_BUS_PCI) {
      if (asprintf(&tag, "pci-%04x:%02x:%02x.%1u",
                   device->businfo.pci->domain,
                   device->businfo.pci->bus,
                   device->businfo.pci->dev,
                   device->businfo.pci->func) < 0) {
         return NULL;
      }
   } else if (device->bustype == DRM_BUS_PLATFORM ||
              device->bustype == DRM_BUS_HOST1X) {
      char *fullname, *name, *address;

      if (device->bustype == DRM_BUS_PLATFORM)
         fullname = device->businfo.platform->fullname;
      else
         fullname = device->businfo.host1x->fullname;

      name = strrchr(fullname, '/');
      if (!name)
         name = strdup(fullname);
      else
         name = strdup(name + 1);

      address = strchr(name, '@');
      if (address) {
         *address++ = '\0';

         if (asprintf(&tag, "platform-%s_%s", address, name) < 0)
            tag = NULL;
      } else {
         if (asprintf(&tag, "platform-%s", name) < 0)
            tag = NULL;
      }

      free(name);
   }
   return tag;
}

static char *drm_get_id_path_tag_for_fd(int fd)
{
   drmDevicePtr device;
   char *tag;

   if (drmGetDevice2(fd, 0, &device) != 0)
       return NULL;

   tag = drm_construct_id_path_tag(device);
   drmFreeDevice(&device);
   return tag;
}

static int open_device(const char *device_name)
{
   int fd;
   fd = open(device_name, O_RDWR | O_CLOEXEC);
   if (fd == -1 && errno == EINVAL)
   {
      fd = open(device_name, O_RDWR | O_CLOEXEC);
      if (fd != -1)
         fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
   }
   return fd;
}

static bool drm_device_matches_tag(drmDevicePtr device, const char *prime_tag)
{
   char *tag = drm_construct_id_path_tag(device);
   int ret;

   if (tag == NULL)
      return false;

   ret = strcmp(tag, prime_tag);

   free(tag);
   return ret == 0;
}

static int get_user_preferred_fd(int default_fd)
{
/* Arbitrary "maximum" value of drm devices. */
#define MAX_DRM_DEVICES 32
    const char *dri_prime = getenv("DRI_PRIME");
    char *default_tag, *prime = NULL;
    drmDevicePtr devices[MAX_DRM_DEVICES];
    int i, num_devices, fd;
    bool found = false;

    if (dri_prime)
        prime = strdup(dri_prime);

    if (prime == NULL) {
        return default_fd;
    }

    default_tag = drm_get_id_path_tag_for_fd(default_fd);
    if (default_tag == NULL)
        goto err;

    num_devices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
    if (num_devices < 0)
        goto err;

    /* two format are supported:
     * "1": choose any other card than the card used by default.
     * id_path_tag: (for example "pci-0000:02:00.0") choose the card
     * with this id_path_tag.
     */
    if (!strcmp(prime,"1")) {
        /* Hmm... detection for 2-7 seems to be broken. Oh well ...
         * Pick the first render device that is not our own.
         */
        for (i = 0; i < num_devices; i++) {
            if (devices[i]->available_nodes & 1 << DRM_NODE_RENDER &&
                    !drm_device_matches_tag(devices[i], default_tag)) {
                found = true;
                break;
            }
        }
    } else {
        for (i = 0; i < num_devices; i++) {
            if (devices[i]->available_nodes & 1 << DRM_NODE_RENDER &&
                    drm_device_matches_tag(devices[i], prime)) {
                found = true;
                break;
            }
        }
    }

    if (!found) {
        drmFreeDevices(devices, num_devices);
        goto err;
    }

    fd = open_device(devices[i]->nodes[DRM_NODE_RENDER]);
    drmFreeDevices(devices, num_devices);
    if (fd < 0)
        goto err;

    close(default_fd);


    free(default_tag);
    free(prime);
    return fd;

err:

    free(default_tag);
    free(prime);
    return default_fd;
}

static int dri3_open(xcb_connection_t *conn,
                     xcb_window_t root,
                     uint32_t provider)
{
   xcb_dri3_open_cookie_t       cookie;
   xcb_dri3_open_reply_t        *reply;
   int                          fd;

   cookie = xcb_dri3_open(conn,
                          root,
                          provider);

   reply = xcb_dri3_open_reply(conn, cookie, NULL);
   if (!reply)
      return -1;

   if (reply->nfd != 1) {
      free(reply);
      return -1;
   }

   fd = xcb_dri3_open_reply_fds(conn, reply)[0];
   free(reply);
   fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);

   return fd;
}

static int xcb_open(xcb_connection_t **connection, xcb_screen_t **screen)
{
    int screen_number;
    const xcb_setup_t *setup;
    xcb_connection_t *c;
    xcb_screen_t *scrn;
    xcb_screen_iterator_t screen_iter;

    /* getting the connection */
    c = xcb_connect(NULL, &screen_number);
    if (xcb_connection_has_error(c)) {
        return -1;
    }

    /* getting the current screen */
    setup = xcb_get_setup(c);

    scrn = NULL;
    screen_iter = xcb_setup_roots_iterator(setup);
    for (; screen_iter.rem != 0; --screen_number, xcb_screen_next(&screen_iter))
    {
        if (screen_number == 0) {
            scrn = screen_iter.data;
            break;
        }
    }
    if (!scrn) {
        xcb_disconnect(c);
        return -1;
    }

    *connection = c;
    *screen = scrn;
    return 0;
}

static int get_device_node_fd_from_dri3()
{
    int ret = 0;
    int fd = -1;
    xcb_connection_t* connection = NULL;
    xcb_screen_t* screen = NULL;

    ret = xcb_open(&connection, &screen);
    if (ret < 0)
        return -1;

    fd = dri3_open(connection, screen->root, 0);
    if (fd < 0) {
        return -1;
    }

    fd = get_user_preferred_fd(fd);

    if (connection) {
        xcb_disconnect(connection);
    }

    return fd;
}

static int get_device_node_fd()
{
    int fd = -1;
    char* id_path_tag = NULL;
    drmDevicePtr devices[MAX_DRM_DEVICES];
    int num_devices = -1;
    bool found = false;
    int i;

    if (dev_fd >= 0) {
        return dev_fd;
    }

    fd = get_device_node_fd_from_dri3();
    if (fd < 0) {
        dev_fd = open_device("/dev/dri/card0");
        return dev_fd;
    }

    id_path_tag = drm_get_id_path_tag_for_fd(fd);
    if (!id_path_tag) {
        dev_fd = open_device("/dev/dri/card0");
        goto out;
    }

    num_devices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
    if (num_devices < 0) {
        goto out;
    }
    for (i = 0; i < num_devices; i++) {
        if (devices[i]->available_nodes & 1 << DRM_NODE_PRIMARY &&
                drm_device_matches_tag(devices[i], id_path_tag)) {
            found = true;
            break;
        }
    }

    if (found) {
        dev_fd = open_device(devices[i]->nodes[DRM_NODE_PRIMARY]);
    } else {
        dev_fd = open_device("/dev/dri/card0");
    }
    drmFreeDevices(devices, num_devices);

out:
    if (id_path_tag) {
        free(id_path_tag);
    }

    if (fd >= 0 && fd != dev_fd) {
        close(fd);
    }

    return dev_fd;
}

int drm_helper_get_dmabuf_from_name(uint32_t name)
{
    int fd = -1;
    int buffer_fd = -1;
    drm_magic_t magic = 0;
    static int last_fd = -1;

    struct drm_gem_open open_arg;

    fd = get_device_node_fd();
    if (fd < 0) {
        return -1;
    }

    if (fd >= 0 && fd != last_fd) {
        if (drmGetMagic(fd, &magic) == 0) {
            if (magic != 0) {
                last_fd = fd;
                authenticate_magic(magic);
            }
        }
    }

    /* gem name to drm handle */
    memset(&open_arg, 0, sizeof(open_arg));
    open_arg.name = name;

    if (ioctl(fd, DRM_IOCTL_GEM_OPEN, &open_arg) < 0) {
        //syslog(LOG_ERR, "DRM_IOCTL_GEM_OPEN failed");
        goto out;
    }

    if (drmPrimeHandleToFD(fd, open_arg.handle, DRM_CLOEXEC, &buffer_fd)) {
        //syslog(LOG_ERR, "drmPrimeHandleToFD failed");
        goto out;
    }

    if (buffer_fd < 0) {
        //syslog(LOG_ERR, "buffer_fd < 0");
    }

out:
    if (fd != dev_fd) {
        close(fd);
    }

    return buffer_fd;
}
