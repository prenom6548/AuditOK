/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMMACROS_H__
#define __KMMACROS_H__

#include <gio/gio.h>

#define _CONCAT(a, b) a##b
#define _CONCAT_INDIRECT(a, b) _CONCAT(a, b)
#define _MAKE_ANONYMOUS(a) _CONCAT_INDIRECT(a, __COUNTER__)

#define _HASH_TABLE_FOREACH_IMPL_KV(guard, ht, it, kt, k, vt, v)                                  \
    gboolean guard = TRUE;                                                                        \
    G_STATIC_ASSERT(sizeof(kt) == sizeof(void *));                                                \
    G_STATIC_ASSERT(sizeof(vt) == sizeof(void *));                                                \
    for (GHashTableIter it; guard && ({ g_hash_table_iter_init(&it, ht), TRUE; }); guard = FALSE) \
        for (kt k; guard; guard = FALSE)                                                          \
            for (vt v; g_hash_table_iter_next(&it, (void **)&k, (void **)&v);)

/* Variant of HASH_TABLE_FOREACH without having to specify an iterator. An
 * anonymous iterator will be created. */
#define HASH_TABLE_FOREACH_KV(ht, kt, k, vt, v) _HASH_TABLE_FOREACH_IMPL_KV(_MAKE_ANONYMOUS(_ht_iter_guard_), ht, _MAKE_ANONYMOUS(_ht_iter_it_), kt, k, vt, v)

/* Variant of GLNX_HASH_TABLE_FOREACH_KV which omits unpacking vals. */
#define HASH_TABLE_FOREACH(ht, kt, k) _HASH_TABLE_FOREACH_IMPL_KV(_MAKE_ANONYMOUS(_ht_iter_guard_), ht, _MAKE_ANONYMOUS(_ht_iter_it_), kt, k, gpointer, _MAKE_ANONYMOUS(_glnx_ht_iter_v_))

#endif