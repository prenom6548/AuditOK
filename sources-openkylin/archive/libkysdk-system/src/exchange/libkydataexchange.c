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
 * Authors: JingWen Wang <wangjingwen@kylinos.cn>
 *
 */

// libkydataexchange.c - 修复版本
#include "libkydataexchange.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    Display* display;
    Window window;
    Atom clipboard_atom;
    Atom targets_atom;
    Atom text_atom;
    Atom utf8_string_atom;
    Atom image_atom;
    
    // 修改数据存储结构
    struct {
        void* data;
        size_t size;
        ClipboardDataType type;
        bool owns_clipboard;
    } current;
    
    ClipboardChangeCallback callback;
    void* callback_user_data;
    
    enum {
        PROTOCOL_X11,
        PROTOCOL_WAYLAND
    } protocol;
} ClipboardContext;

static ClipboardContext* g_ctx = NULL;

// 新增：深度复制图片数据
static void* deep_copy_image(const ClipboardImage* src) {
    if (!src || !src->data) return NULL;
    
    ClipboardImage* dest = malloc(sizeof(ClipboardImage));
    if (!dest) return NULL;
    
    size_t data_size = src->width * src->height * src->bytes_per_pixel;
    dest->data = malloc(data_size);
    if (!dest->data) {
        free(dest);
        return NULL;
    }
    
    dest->width = src->width;
    dest->height = src->height;
    dest->bytes_per_pixel = src->bytes_per_pixel;
    memcpy(dest->data, src->data, data_size);
    
    return dest;
}

// 新增：释放数据
static void free_clipboard_data(void) {
    if (!g_ctx || !g_ctx->current.data) return;
    
    if (g_ctx->current.type == CLIPBOARD_TYPE_IMAGE) {
        ClipboardImage* img = (ClipboardImage*)g_ctx->current.data;
        free(img->data);
        free(img);
    } else {
        free(g_ctx->current.data);
    }
    g_ctx->current.data = NULL;
    g_ctx->current.size = 0;
}

static bool x11_init(void) {
    g_ctx->display = XOpenDisplay(NULL);
    if (!g_ctx->display) return false;
    
    g_ctx->window = XCreateSimpleWindow(g_ctx->display,
                                      DefaultRootWindow(g_ctx->display),
                                      0, 0, 1, 1, 0, 0, 0);
    
    g_ctx->clipboard_atom = XInternAtom(g_ctx->display, "CLIPBOARD", False);
    g_ctx->targets_atom = XInternAtom(g_ctx->display, "TARGETS", False);
    g_ctx->text_atom = XInternAtom(g_ctx->display, "TEXT", False);
    g_ctx->utf8_string_atom = XInternAtom(g_ctx->display, "UTF8_STRING", False);
    g_ctx->image_atom = XInternAtom(g_ctx->display, "IMAGE", False);
    
    return true;
}

static void x11_cleanup(void) {
    if (!g_ctx->display) return;
    
    if (g_ctx->current.owns_clipboard) {
        XSetSelectionOwner(g_ctx->display, g_ctx->clipboard_atom, None, CurrentTime);
    }
    
    if (g_ctx->window) {
        XDestroyWindow(g_ctx->display, g_ctx->window);
    }
    XCloseDisplay(g_ctx->display);
}

static ClipboardOperationResult x11_set_data(ClipboardDataType type, const void* data, size_t size) {
    if (!data || size == 0) return CLIPBOARD_OPERATION_FAILURE;
    
    // 清理现有数据
    free_clipboard_data();
    
    // 根据类型处理数据
    if (type == CLIPBOARD_TYPE_TEXT) {
        g_ctx->current.data = malloc(size);
        if (!g_ctx->current.data) return CLIPBOARD_OPERATION_FAILURE;
        memcpy(g_ctx->current.data, data, size);
        g_ctx->current.size = size;
    } else if (type == CLIPBOARD_TYPE_IMAGE) {
        g_ctx->current.data = deep_copy_image((const ClipboardImage*)data);
        if (!g_ctx->current.data) return CLIPBOARD_OPERATION_FAILURE;
        g_ctx->current.size = size;
    } else {
        return CLIPBOARD_OPERATION_UNSUPPORTED_TYPE;
    }
    
    g_ctx->current.type = type;
    
    // 设置剪贴板所有权
    XSetSelectionOwner(g_ctx->display, g_ctx->clipboard_atom, g_ctx->window, CurrentTime);
    
    if (XGetSelectionOwner(g_ctx->display, g_ctx->clipboard_atom) != g_ctx->window) {
        free_clipboard_data();
        return CLIPBOARD_OPERATION_FAILURE;
    }
    
    g_ctx->current.owns_clipboard = true;
    XFlush(g_ctx->display);
    
    // 触发回调
    if (g_ctx->callback) {
        g_ctx->callback(g_ctx->callback_user_data);
    }
    
    return CLIPBOARD_OPERATION_SUCCESS;
}

static ClipboardOperationResult x11_get_data(ClipboardDataType* type, void** data) {
    if (!type || !data) return CLIPBOARD_OPERATION_FAILURE;
    
    Window owner = XGetSelectionOwner(g_ctx->display, g_ctx->clipboard_atom);
    if (owner == None) {
        return CLIPBOARD_OPERATION_FAILURE;
    }
    
    if (owner == g_ctx->window && g_ctx->current.data) {
        // 我们拥有剪贴板，返回当前数据的副本
        *type = g_ctx->current.type;
        if (g_ctx->current.type == CLIPBOARD_TYPE_IMAGE) {
            *data = deep_copy_image((ClipboardImage*)g_ctx->current.data);
        } else {
            *data = malloc(g_ctx->current.size);
            if (*data) {
                memcpy(*data, g_ctx->current.data, g_ctx->current.size);
            }
        }
        return *data ? CLIPBOARD_OPERATION_SUCCESS : CLIPBOARD_OPERATION_FAILURE;
    }
    
    // TODO: 实现从其他窗口获取数据
    return CLIPBOARD_OPERATION_FAILURE;
}

bool kdk_exchange_init(void) {
    if (g_ctx) return false;  // 已经初始化
    
    g_ctx = calloc(1, sizeof(ClipboardContext));
    if (!g_ctx) return false;
    
    if (x11_init()) {
        g_ctx->protocol = PROTOCOL_X11;
        return true;
    }
    
    free(g_ctx);
    g_ctx = NULL;
    return false;
}

void kdk_exchange_cleanup(void) {
    if (!g_ctx) return;
    
    if (g_ctx->protocol == PROTOCOL_X11) {
        x11_cleanup();
    }
    
    free_clipboard_data();
    free(g_ctx);
    g_ctx = NULL;
}

ClipboardOperationResult kdk_exchange_set_data(ClipboardDataType type, const void* data, size_t size) {
    if (!g_ctx) return CLIPBOARD_OPERATION_FAILURE;
    return x11_set_data(type, data, size);
}

ClipboardOperationResult kdk_exchange_get_data(ClipboardDataType* type, void** data) {
    if (!g_ctx) return CLIPBOARD_OPERATION_FAILURE;
    return x11_get_data(type, data);
}

ClipboardOperationResult kdk_exchange_clear(void) {
    if (!g_ctx) return CLIPBOARD_OPERATION_FAILURE;
    
    if (g_ctx->protocol == PROTOCOL_X11) {
        // 首先放弃剪贴板所有权
        if (g_ctx->current.owns_clipboard) {
            XSetSelectionOwner(g_ctx->display, g_ctx->clipboard_atom, None, CurrentTime);
            XFlush(g_ctx->display);
            
            // 验证是否成功放弃所有权
            if (XGetSelectionOwner(g_ctx->display, g_ctx->clipboard_atom) == g_ctx->window) {
                // printf("y=test");
                return CLIPBOARD_OPERATION_FAILURE;
            }
            g_ctx->current.owns_clipboard = false;
        }
    }
    
    // 清理内部数据
    free_clipboard_data();
    
    // 触发回调通知变化
    if (g_ctx->callback) {
        g_ctx->callback(g_ctx->callback_user_data);
    }
    
    return CLIPBOARD_OPERATION_SUCCESS;
}

bool kdk_exchange_is_empty(void) {
    if (!g_ctx) return true;
    
    if (g_ctx->protocol == PROTOCOL_X11) {
        return XGetSelectionOwner(g_ctx->display, g_ctx->clipboard_atom) == None;
    }
    
    return true;
}

ClipboardOperationResult kdk_exchange_register_callback(ClipboardChangeCallback callback, void* user_data) {
    if (!g_ctx || !callback) return CLIPBOARD_OPERATION_FAILURE;
    
    g_ctx->callback = callback;
    g_ctx->callback_user_data = user_data;
    return CLIPBOARD_OPERATION_SUCCESS;
}

ClipboardOperationResult kdk_exchange_unregister_callback(ClipboardChangeCallback callback) {
    if (!g_ctx || !callback || g_ctx->callback != callback) {
        return CLIPBOARD_OPERATION_FAILURE;
    }
    
    g_ctx->callback = NULL;
    g_ctx->callback_user_data = NULL;
    return CLIPBOARD_OPERATION_SUCCESS;
}
