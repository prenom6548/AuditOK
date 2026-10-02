// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later
#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdlib.h>
#include <strings.h>

#include <xkbcommon/xkbcommon.h>

#include <kywc/binding.h>
#include <kywc/log.h>
#include <util/string.h>

#include "config.h"
#include "input_p.h"
#include "server.h"
#include "util/dbus.h"

static const char *service_path = "/com/kylin/Wlcom/GlobalShortcuts";
static const char *service_manager_iface = "com.kylin.Wlcom.GlobalShortcuts.Manager";
static const char *service_session_iface = "com.kylin.Wlcom.GlobalShortcuts.Session";
static const char *session_prefix = "/com/kylin/Wlcom/GlobalShortcuts/Session";

struct block_shortcut {
    const char *unique;
    bool override_set; // has override
    bool blocked;      // override value

    struct wl_list link;
};

struct session_block_scope {
    const char *target_session_handle;
    bool all_set;     // has ALL rule
    bool all_blocked; // ALL rule value

    struct wl_list shortcuts; // block_shortcut

    struct wl_list link;
};

struct global_shortcut_session {
    const char *handle;
    const char *path;
    const char *owner;
    struct sd_bus_slot *watcher;

    struct dbus_object *dbus;

    struct wl_list shortcuts;
    struct wl_list blocks; // session_block_scope
    struct wl_list link;
};

struct global_shortcut {
    struct global_shortcut_session *session;
    struct key_binding *binding;

    const char *unique;
    const char *default_key;
    const char *current_key;

    bool enabled;
    uint32_t options;

    enum function_id func_id;
    const char *action;
    const char *desc_l10n;

    struct wl_list link;
};

static struct global_shortcuts_manager {
    struct config *config;

    struct wl_listener server_start;
    struct wl_listener server_destroy;

    struct wl_list sessions; // all session
} *manager = NULL;

#define IDENTIFIER_ALL "ALL"

static const char *session_builtin = "kylin_wlcom";

static bool system_shortcut_write_config(struct global_shortcut *shortcut);
static void session_block_scopes_destroy(struct global_shortcut_session *session);
static int session_name_owner_changed_handle(sd_bus_message *m, void *userdata,
                                             sd_bus_error *ret_error);

static bool str_equal(const char *a, const char *b)
{
    return (a == b) || (a && b && strcasecmp(a, b) == 0);
}

static bool shortcut_session_is_builtin(const struct global_shortcut_session *session)
{
    return session && strcmp(session->handle, session_builtin) == 0;
}

static const char *shortcut_binding_desc(const struct global_shortcut_session *session,
                                         const struct global_shortcut *shortcut)
{
    if (!session || !shortcut || strcmp(session->handle, session_builtin) != 0) {
        return NULL;
    }

    return shortcut->unique;
}

static uint32_t shortcut_key_binding_options(struct global_shortcut *shortcut)
{
    uint32_t options = shortcut->options;
    if (options || shortcut_session_is_builtin(shortcut->session)) {
        return options;
    }

    return KEY_BINDING_TRIGGER_IN_ACTIVE | KEY_BINDING_TRIGGER_IN_INACTIVE |
           KEY_BINDING_TRIGGER_IN_REPEAT;
}

static void global_shortcut_destroy(struct global_shortcut *shortcut, bool clean)
{
    if (shortcut->binding && clean) {
        kywc_key_binding_destroy(shortcut->binding);
    }

    wl_list_remove(&shortcut->link);
    free((void *)shortcut->unique);
    free((void *)shortcut->current_key);
    free((void *)shortcut->default_key);
    free((void *)shortcut->action);
    free((void *)shortcut->desc_l10n);
    free(shortcut);
}

static void shortcut_session_destroy(struct global_shortcut_session *session, bool clean)
{
    session_block_scopes_destroy(session);

    struct global_shortcut *shortcut, *tmp;
    wl_list_for_each_safe(shortcut, tmp, &session->shortcuts, link) {
        global_shortcut_destroy(shortcut, clean);
    }

    if (session->watcher) {
        sd_bus_slot_unref(session->watcher);
    }
    dbus_unregister_object(session->dbus);
    wl_list_remove(&session->link);
    free((void *)session->handle);
    free((void *)session->path);
    free((void *)session->owner);
    free(session);
}

static struct global_shortcut *shortcut_by_unique(struct global_shortcut_session *session,
                                                  const char *unique)
{
    struct global_shortcut *shortcut;
    wl_list_for_each(shortcut, &session->shortcuts, link) {
        if (strcmp(unique, shortcut->unique) == 0) {
            return shortcut;
        }
    }

    return NULL;
}

static bool shortcut_key_name_parse(const char *key_name, uint32_t *modifiers, uint32_t *keysym)
{
    if (!key_name || *key_name == '\0') {
        return false;
    }

    *modifiers = 0;
    *keysym = 0;
    size_t len = 0;
    char **split_str = string_split(key_name, "+", &len);
    for (size_t i = 0; i < len; i++) {
        uint32_t mod = keyboard_get_modifier_mask_by_name(split_str[i]);
        if (mod) {
            *modifiers |= mod;
            continue;
        }

        /* whatever keep the lower keysym */
        xkb_keysym_t sym = xkb_keysym_from_name(split_str[i], XKB_KEYSYM_CASE_INSENSITIVE);
        *keysym = xkb_keysym_to_lower(sym);
    }

    string_free_split(split_str);

    return true;
}

static struct global_shortcut *
shortcut_get_or_create_by_unique(struct global_shortcut_session *session, const char *unique)
{
    struct global_shortcut *shortcut = shortcut_by_unique(session, unique);
    if (shortcut) {
        return shortcut;
    }

    shortcut = calloc(1, sizeof(*shortcut));
    if (!shortcut) {
        return NULL;
    }

    shortcut->unique = strdup(unique);
    shortcut->enabled = true;
    shortcut->func_id = FUNC_ID_CUSTOM;
    shortcut->session = session;
    wl_list_insert(&session->shortcuts, &shortcut->link);

    return shortcut;
}

static bool shortcut_create_binding_and_register(struct global_shortcut_session *session,
                                                 struct global_shortcut *shortcut)
{
    bool builtin_session = shortcut_session_is_builtin(session);
    if (!shortcut->enabled && !builtin_session) {
        if (shortcut->binding) {
            kywc_key_binding_destroy(shortcut->binding);
            shortcut->binding = NULL;
        }
        return true;
    }

    uint32_t modifiers;
    uint32_t keysym;
    const char *key_name = shortcut->current_key ? shortcut->current_key : shortcut->default_key;
    if (!shortcut_key_name_parse(key_name, &modifiers, &keysym)) {
        kywc_log(KYWC_WARN, "shortcut key parse failed session=%s unique=%s key=%s",
                 session ? session->handle : "NULL", shortcut->unique, key_name ? key_name : "");
        return false;
    }

    if (shortcut->binding) {
        uint32_t options = shortcut_key_binding_options(shortcut);
        if (!kywc_key_binding_update(shortcut->binding, keysym, modifiers)) {
            return false;
        }

        if (!kywc_key_binding_update_state(shortcut->binding, shortcut->func_id, options)) {
            return false;
        }
        return true;
    }

    uint32_t options = shortcut_key_binding_options(shortcut);
    struct key_binding *binding = kywc_key_binding_create_by_symbol(
        keysym, modifiers, shortcut_binding_desc(session, shortcut), shortcut->func_id, options);
    if (!binding) {
        return false;
    }

    if (!kywc_key_binding_register(binding, function_entry_get_type(shortcut->func_id),
                                   function_manager_handle_key_binding, shortcut)) {
        kywc_key_binding_destroy(binding);
        return false;
    }

    shortcut->binding = binding;

    if (builtin_session && !shortcut->enabled) {
        kywc_key_binding_block(shortcut->binding, true);
    }

    return true;
}

static bool session_shortcut_update(struct global_shortcut_session *session,
                                    struct global_shortcut *shortcut,
                                    struct global_shortcut *pending)
{
    bool builtin_session = shortcut_session_is_builtin(session);
    if (!pending->enabled && !builtin_session) {
        if (shortcut->binding) {
            kywc_key_binding_destroy(shortcut->binding);
            shortcut->binding = NULL;
        }
        shortcut->enabled = false;
        return true;
    }

    bool created_binding = false;
    // only built-in session can change the function ID
    if (shortcut->func_id != pending->func_id && strcmp(session->handle, session_builtin) != 0) {
        return false;
    }

    uint32_t old_mod = 0, old_key = 0;
    uint32_t new_mod = 0, new_key = 0;
    const char *old_key_name =
        shortcut->current_key ? shortcut->current_key : shortcut->default_key;
    const char *new_key_name = pending->current_key ? pending->current_key : shortcut->default_key;

    if (!shortcut_key_name_parse(old_key_name, &old_mod, &old_key)) {
        return false;
    }

    if (!shortcut_key_name_parse(new_key_name, &new_mod, &new_key)) {
        return false; /* invalid key */
    }

    if (!shortcut->binding) {
        uint32_t options = shortcut_key_binding_options(pending);
        struct key_binding *binding = kywc_key_binding_create_by_symbol(
            new_key, new_mod, shortcut_binding_desc(session, shortcut), pending->func_id, options);
        if (!binding) {
            return false;
        }

        if (!kywc_key_binding_register(binding, function_entry_get_type(pending->func_id),
                                       function_manager_handle_key_binding, shortcut)) {
            kywc_key_binding_destroy(binding);
            return false;
        }
        shortcut->binding = binding;
        created_binding = true;
    }

    if (old_mod != new_mod || old_key != new_key) {
        if (!kywc_key_binding_update(shortcut->binding, new_key, new_mod)) {
            return false;
        }

        free((void *)shortcut->current_key);
        shortcut->current_key = NULL;

        if (pending->current_key) {
            if (!shortcut->default_key ||
                strcmp(shortcut->default_key, pending->current_key) != 0) {
                shortcut->current_key = strdup(pending->current_key);
            }
        }
    }

    uint32_t options = shortcut_key_binding_options(pending);
    if (!kywc_key_binding_update_state(shortcut->binding, pending->func_id, options)) {
        return false;
    }

    if (builtin_session && shortcut->binding) {
        if (shortcut->enabled != pending->enabled) {
            kywc_key_binding_block(shortcut->binding, !pending->enabled);
        } else if (!pending->enabled && created_binding) {
            kywc_key_binding_block(shortcut->binding, true);
        }
    }

    shortcut->enabled = pending->enabled;
    shortcut->func_id = pending->func_id;
    shortcut->options = pending->options;

    if (!str_equal(shortcut->action, pending->action)) {
        free((void *)shortcut->action);
        shortcut->action = pending->action ? strdup(pending->action) : NULL;
    }

    if (!str_equal(shortcut->desc_l10n, pending->desc_l10n)) {
        free((void *)shortcut->desc_l10n);
        shortcut->desc_l10n = pending->desc_l10n ? strdup(pending->desc_l10n) : NULL;
    }

    return true;
}

static void shortcuts_reset_default_key(struct global_shortcut_session *session)
{
    struct global_shortcut *shortcut;
    wl_list_for_each(shortcut, &session->shortcuts, link) {
        if (!shortcut->binding) {
            continue;
        }
        kywc_key_binding_destroy(shortcut->binding);
        shortcut->binding = NULL;

        if (str_equal(shortcut->current_key, shortcut->default_key)) {
            free((void *)shortcut->current_key);
            shortcut->current_key = NULL;
        }

        if (strcmp(session->handle, session_builtin) == 0) {
            system_shortcut_write_config(shortcut);
        }
    }

    wl_list_for_each(shortcut, &session->shortcuts, link) {
        if (!shortcut->default_key) {
            continue;
        }

        shortcut_create_binding_and_register(session, shortcut);
    }

    wl_list_for_each(shortcut, &session->shortcuts, link) {
        if (!shortcut->current_key) {
            continue;
        }
        shortcut_create_binding_and_register(session, shortcut);
    }
}

static int list_shortcuts(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    sd_bus_message *reply = NULL;
    struct global_shortcut_session *session = userdata;

    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "{sv}"));

    struct global_shortcut *shortcut;
    wl_list_for_each(shortcut, &session->shortcuts, link) {
        struct key_binding *binding = shortcut->binding;
        if (!binding) {
            continue;
        }

        CK(sd_bus_message_open_container(reply, 'e', "sv"));
        CK(sd_bus_message_append_basic(reply, 's', shortcut->unique));
        CK(sd_bus_message_open_container(reply, 'v', "a{sv}"));
        CK(sd_bus_message_open_container(reply, 'a', "{sv}"));

        CK(sd_bus_message_append(reply, "{sv}", "default_key", "s", shortcut->default_key));
        CK(sd_bus_message_append(reply, "{sv}", "current_key", "s",
                                 shortcut->current_key ? shortcut->current_key
                                                       : shortcut->default_key));
        enum function_id func_id = shortcut->func_id;
        if (!shortcut->action) {
            CK(sd_bus_message_append(reply, "{sv}", "action_id", "u", func_id));
            CK(sd_bus_message_append(reply, "{sv}", "desc", "s", function_entry_get_desc(func_id)));
        } else {
            CK(sd_bus_message_append(reply, "{sv}", "action", "s", shortcut->action));
        }

        if (shortcut->desc_l10n) {
            CK(sd_bus_message_append(reply, "{sv}", "desc_l10n", "s", shortcut->desc_l10n));
        } else if (func_id != FUNC_ID_CUSTOM) {
            CK(sd_bus_message_append(reply, "{sv}", "desc_l10n", "s",
                                     function_entry_get_l10n_name(func_id)));
        }
        CK(sd_bus_message_append(reply, "{sv}", "options", "u",
                                 shortcut_key_binding_options(shortcut)));

        CK(sd_bus_message_close_container(reply));
        CK(sd_bus_message_close_container(reply));
        CK(sd_bus_message_close_container(reply));
    }

    CK(sd_bus_message_close_container(reply));
    sd_bus_send(NULL, reply, NULL);
    sd_bus_message_unref(reply);

    return 1;
}

static int configure_shortcuts(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    uint32_t ret_count = 0;
    struct global_shortcut_session *session = userdata;

    CK(sd_bus_message_enter_container(m, 'a', "{sv}"));

    while (!sd_bus_message_at_end(m, 0)) {
        const char *unique;

        CK(sd_bus_message_enter_container(m, 'e', "sv"));
        CK(sd_bus_message_read(m, "s", &unique));

        struct global_shortcut *shortcut = shortcut_by_unique(session, unique);
        if (!shortcut) {
            CK(sd_bus_message_skip(m, "v"));
            CK(sd_bus_message_exit_container(m));
            continue;
        }

        CK(sd_bus_message_enter_container(m, 'v', "a{sv}"));
        CK(sd_bus_message_enter_container(m, 'a', "{sv}"));

        struct global_shortcut shortcut_state = {
            .session = shortcut->session,
            .enabled = shortcut->enabled,
            .binding = shortcut->binding,
            .default_key = shortcut->default_key,
            .current_key = shortcut->current_key,
            .func_id = shortcut->func_id,
            .action = shortcut->action,
            .desc_l10n = shortcut->desc_l10n,
            .options = shortcut->options,
        };

        while (!sd_bus_message_at_end(m, 0)) {
            const char *key;
            CK(sd_bus_message_enter_container(m, 'e', "sv"));
            CK(sd_bus_message_read(m, "s", &key));

            CK(sd_bus_message_enter_container(m, 'v', NULL));
            if (strcmp(key, "current_key") == 0) {
                CK(sd_bus_message_read(m, "s", &shortcut_state.current_key));
            } else if (strcmp(key, "action_id") == 0) {
                CK(sd_bus_message_read(m, "u", &shortcut_state.func_id));
            } else if (strcmp(key, "action") == 0) {
                CK(sd_bus_message_read(m, "s", &shortcut_state.action));
            } else if (strcmp(key, "desc_l10n") == 0) {
                CK(sd_bus_message_read(m, "s", &shortcut_state.desc_l10n));
            } else if (strcmp(key, "enabled") == 0) {
                int enabled;
                CK(sd_bus_message_read(m, "b", &enabled));
                shortcut_state.enabled = enabled;
            } else if (strcmp(key, "options") == 0) {
                CK(sd_bus_message_read(m, "u", &shortcut_state.options));
            } else {
                CK(sd_bus_message_skip(m, NULL));
            }

            CK(sd_bus_message_exit_container(m));
            CK(sd_bus_message_exit_container(m));
        }

        if (session_shortcut_update(session, shortcut, &shortcut_state)) {
            if (strcmp(session->handle, session_builtin) == 0) {
                system_shortcut_write_config(shortcut);
            }

            ret_count++;
        }

        CK(sd_bus_message_exit_container(m));
        CK(sd_bus_message_exit_container(m));
        CK(sd_bus_message_exit_container(m));
    }

    CK(sd_bus_message_exit_container(m));

    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_append(reply, "u", ret_count));

    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);

    return 1;
}

static int bind_shortcuts(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    uint32_t ret_count = 0;
    sd_bus_message *reply = NULL;
    struct global_shortcut_session *session = userdata;
    if (strcmp(session->handle, session_builtin) == 0) {
        kywc_log(KYWC_WARN, "bind denied on builtin session=%s", session->handle);
        goto done;
    }

    CK(sd_bus_message_enter_container(m, 'a', "{sv}"));

    while (!sd_bus_message_at_end(m, 0)) {
        const char *unique;

        CK(sd_bus_message_enter_container(m, 'e', "sv"));
        CK(sd_bus_message_read(m, "s", &unique));

        struct global_shortcut *shortcut = shortcut_by_unique(session, unique);
        if (shortcut) {
            CK(sd_bus_message_skip(m, "v"));
            CK(sd_bus_message_exit_container(m));
            continue;
        }

        shortcut = shortcut_get_or_create_by_unique(session, unique);
        if (!shortcut) {
            CK(sd_bus_message_skip(m, "v"));
            CK(sd_bus_message_exit_container(m));
            continue;
        }

        CK(sd_bus_message_enter_container(m, 'v', "a{sv}"));
        CK(sd_bus_message_enter_container(m, 'a', "{sv}"));

        while (!sd_bus_message_at_end(m, 0)) {
            const char *key;
            CK(sd_bus_message_enter_container(m, 'e', "sv"));
            CK(sd_bus_message_read(m, "s", &key));

            CK(sd_bus_message_enter_container(m, 'v', NULL));
            if (strcmp(key, "default_key") == 0) {
                const char *default_key;
                CK(sd_bus_message_read(m, "s", &default_key));
                shortcut->default_key = strdup(default_key);
            } else if (strcmp(key, "current_key") == 0) {
                const char *current_key;
                CK(sd_bus_message_read(m, "s", &current_key));
                shortcut->current_key = strdup(current_key);
            } else if (strcmp(key, "action") == 0) {
                const char *action;
                CK(sd_bus_message_read(m, "s", &action));
                shortcut->action = strdup(action);
            } else if (strcmp(key, "desc_l10n") == 0) {
                const char *desc_l10n;
                CK(sd_bus_message_read(m, "s", &desc_l10n));
                shortcut->desc_l10n = strdup(desc_l10n);
            } else if (strcmp(key, "enabled") == 0) {
                int enabled;
                CK(sd_bus_message_read(m, "b", &enabled));
                shortcut->enabled = enabled;
            } else if (strcmp(key, "options") == 0) {
                CK(sd_bus_message_read(m, "u", &shortcut->options));
            } else {
                kywc_log(KYWC_DEBUG, "bind ignore unknown field session=%s unique=%s field=%s",
                         session->handle, unique, key);
                CK(sd_bus_message_skip(m, NULL));
            }

            CK(sd_bus_message_exit_container(m));
            CK(sd_bus_message_exit_container(m));
        }

        if (shortcut_create_binding_and_register(session, shortcut)) {
            ret_count++;
        } else {
            global_shortcut_destroy(shortcut, true);
        }

        CK(sd_bus_message_exit_container(m));
        CK(sd_bus_message_exit_container(m));
        CK(sd_bus_message_exit_container(m));
    }

    CK(sd_bus_message_exit_container(m));

done:
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_append(reply, "u", ret_count));
    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);

    return 1;
}

static int reset_shortcuts(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct global_shortcut_session *session = userdata;
    shortcuts_reset_default_key(session);

    uint32_t ret_count = 0;
    struct global_shortcut *shortcut;
    wl_list_for_each(shortcut, &session->shortcuts, link) {
        if (!shortcut->binding) {
            continue;
        }
        ret_count++;
    }

    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_append(reply, "u", ret_count));
    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);

    return 1;
}

static struct global_shortcut_session *shortcut_session_get(const char *session_handle)
{
    struct global_shortcut_session *session;
    wl_list_for_each(session, &manager->sessions, link) {
        if (!strcmp(session_handle, session->handle)) {
            return session;
        }
    }

    return NULL;
}

static bool shortcut_session_block(struct global_shortcut_session *session, const char *unique,
                                   bool block)
{
    if (!session) {
        return !block;
    }

    bool is_block_all = strcmp(unique, IDENTIFIER_ALL) == 0;
    if (!is_block_all) {
        struct global_shortcut *shortcut = shortcut_by_unique(session, unique);
        if (!shortcut || !shortcut->binding) {
            return !block;
        }
        kywc_key_binding_block(shortcut->binding, block);
        return true;
    }

    struct global_shortcut *shortcut;
    wl_list_for_each(shortcut, &session->shortcuts, link) {
        if (!shortcut->binding) {
            continue;
        }
        kywc_key_binding_block(shortcut->binding, block);
    }

    return true;
}

static bool apply_target_shortcut_block_state(const char *target_handle,
                                              const char *shortcut_unique, bool block)
{
    bool is_all_target = strcmp(target_handle, IDENTIFIER_ALL) == 0;
    bool is_all_shortcut = strcmp(shortcut_unique, IDENTIFIER_ALL) == 0;
    if (is_all_target) {
        if (!is_all_shortcut) {
            return false;
        }
        kywc_key_binding_block_all(block);
        return true;
    }

    struct global_shortcut_session *target = shortcut_session_get(target_handle);
    return shortcut_session_block(target, shortcut_unique, block);
}

static struct session_block_scope *
session_block_scope_get_or_create(struct global_shortcut_session *session,
                                  const char *session_unique)
{
    struct session_block_scope *session_block_scope;
    wl_list_for_each(session_block_scope, &session->blocks, link) {
        if (strcmp(session_block_scope->target_session_handle, session_unique)) {
            continue;
        }

        return session_block_scope;
    }

    if (!shortcut_session_get(session_unique) && strcmp(session_unique, IDENTIFIER_ALL)) {
        kywc_log(KYWC_WARN, "Session: %s invalid", session_unique);
        return NULL;
    }

    session_block_scope = calloc(1, sizeof(*session_block_scope));
    if (!session_block_scope) {
        return NULL;
    }

    wl_list_init(&session_block_scope->shortcuts);
    session_block_scope->target_session_handle = strdup(session_unique);
    if (!session_block_scope->target_session_handle) {
        free(session_block_scope);
        return NULL;
    }

    wl_list_insert(&session->blocks, &session_block_scope->link);

    return session_block_scope;
}

static struct block_shortcut *
block_shortcut_state_get(struct session_block_scope *session_block_scope, const char *unique)
{
    struct block_shortcut *block_shortcut;
    wl_list_for_each(block_shortcut, &session_block_scope->shortcuts, link) {
        if (strcmp(block_shortcut->unique, unique)) {
            continue;
        }

        return block_shortcut;
    }

    return NULL;
}

static struct block_shortcut *
block_shortcut_state_get_or_create(struct session_block_scope *session_block_scope,
                                   const char *unique)
{
    struct block_shortcut *block_shortcut = block_shortcut_state_get(session_block_scope, unique);
    if (block_shortcut) {
        return block_shortcut;
    }

    bool is_block_all = strcmp(session_block_scope->target_session_handle, IDENTIFIER_ALL) == 0;
    if (is_block_all && strcmp(unique, IDENTIFIER_ALL) != 0) {
        return NULL;
    }

    block_shortcut = calloc(1, sizeof(*block_shortcut));
    if (!block_shortcut) {
        return NULL;
    }

    block_shortcut->unique = strdup(unique);
    if (!block_shortcut->unique) {
        free(block_shortcut);
        return NULL;
    }
    block_shortcut->override_set = false;
    block_shortcut->blocked = false;

    wl_list_insert(&session_block_scope->shortcuts, &block_shortcut->link);

    return block_shortcut;
}

static bool block_shortcut_effective_blocked(struct session_block_scope *session_block_scope,
                                             struct block_shortcut *block_shortcut)
{
    if (block_shortcut && block_shortcut->override_set) {
        return block_shortcut->blocked;
    }

    return session_block_scope->all_set && session_block_scope->all_blocked;
}

static bool apply_session_all_block_state(struct session_block_scope *session_block_scope,
                                          bool block)
{
    bool old_all = session_block_scope->all_set && session_block_scope->all_blocked;
    if (session_block_scope->all_set && old_all == block) {
        kywc_log(KYWC_DEBUG, "block no-op target=%s unique=%s block=%d",
                 session_block_scope->target_session_handle, IDENTIFIER_ALL, block);
        return false;
    }

    bool is_all_target = strcmp(session_block_scope->target_session_handle, IDENTIFIER_ALL) == 0;
    if (is_all_target) {
        if (!apply_target_shortcut_block_state(session_block_scope->target_session_handle,
                                               IDENTIFIER_ALL, block)) {
            return false;
        }
    } else {
        struct global_shortcut_session *target =
            shortcut_session_get(session_block_scope->target_session_handle);
        if (!target) {
            if (block) {
                return false;
            }
        } else {
            struct global_shortcut *shortcut;
            wl_list_for_each(shortcut, &target->shortcuts, link) {
                if (!shortcut->binding) {
                    continue;
                }

                struct block_shortcut *block_shortcut =
                    block_shortcut_state_get(session_block_scope, shortcut->unique);
                if (block_shortcut && block_shortcut->override_set) {
                    continue;
                }

                bool old_effective = old_all;
                bool new_effective = block;

                if (old_effective == new_effective) {
                    continue;
                }

                if (!apply_target_shortcut_block_state(session_block_scope->target_session_handle,
                                                       shortcut->unique, new_effective)) {
                    return false;
                }
            }
        }
    }

    session_block_scope->all_set = true;
    session_block_scope->all_blocked = block;
    kywc_log(KYWC_DEBUG, "block apply target=%s unique=%s block=%d",
             session_block_scope->target_session_handle, IDENTIFIER_ALL, block);
    return true;
}

static void session_block_scope_shortcuts_destroy(struct session_block_scope *session_block_scope)
{
    if (session_block_scope->all_set && session_block_scope->all_blocked) {
        apply_session_all_block_state(session_block_scope, false);
    }
    session_block_scope->all_set = false;
    session_block_scope->all_blocked = false;

    struct block_shortcut *block_shortcut, *tmp;
    wl_list_for_each_safe(block_shortcut, tmp, &session_block_scope->shortcuts, link) {
        if (block_shortcut->override_set &&
            block_shortcut_effective_blocked(session_block_scope, block_shortcut)) {
            apply_target_shortcut_block_state(session_block_scope->target_session_handle,
                                              block_shortcut->unique, false);
        }

        free((void *)block_shortcut->unique);
        wl_list_remove(&block_shortcut->link);
        free(block_shortcut);
    }
}

static void session_block_scopes_destroy(struct global_shortcut_session *session)
{
    struct session_block_scope *session_block_scope, *tmp;
    wl_list_for_each_safe(session_block_scope, tmp, &session->blocks, link) {
        session_block_scope_shortcuts_destroy(session_block_scope);
        wl_list_remove(&session_block_scope->link);
        free((void *)session_block_scope->target_session_handle);
        free(session_block_scope);
    }
}

static bool apply_session_shortcut_block_state(struct session_block_scope *session_block_scope,
                                               const char *shortcut_unique, bool block)
{
    if (!session_block_scope || !shortcut_unique) {
        return false;
    }

    if (strcmp(shortcut_unique, IDENTIFIER_ALL) == 0) {
        return apply_session_all_block_state(session_block_scope, block);
    }

    struct block_shortcut *block_shortcut =
        block_shortcut_state_get_or_create(session_block_scope, shortcut_unique);
    if (!block_shortcut) {
        return false;
    }

    bool old_effective = block_shortcut_effective_blocked(session_block_scope, block_shortcut);
    if (block_shortcut->override_set && block_shortcut->blocked == block) {
        kywc_log(KYWC_DEBUG, "block no-op target=%s unique=%s block=%d",
                 session_block_scope->target_session_handle, shortcut_unique, block);
        return false;
    }

    bool new_effective = block;
    if (old_effective != new_effective &&
        !apply_target_shortcut_block_state(session_block_scope->target_session_handle,
                                           shortcut_unique, new_effective)) {
        return false;
    }

    block_shortcut->override_set = true;
    block_shortcut->blocked = block;
    kywc_log(KYWC_DEBUG, "block apply target=%s unique=%s block=%d",
             session_block_scope->target_session_handle, shortcut_unique, block);
    return true;
}

static int block_shortcuts(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    uint32_t ret_count = 0;
    struct global_shortcut_session *session = userdata;

    CK(sd_bus_message_enter_container(m, 'a', "{sv}"));
    while (!sd_bus_message_at_end(m, 0)) {
        const char *session_unique;

        CK(sd_bus_message_enter_container(m, 'e', "sv"));
        CK(sd_bus_message_read(m, "s", &session_unique));

        struct session_block_scope *session_block_scope =
            session_block_scope_get_or_create(session, session_unique);
        if (!session_block_scope) {
            CK(sd_bus_message_skip(m, "v"));
            CK(sd_bus_message_exit_container(m));
            continue;
        }

        CK(sd_bus_message_enter_container(m, 'v', "a{sv}"));
        CK(sd_bus_message_enter_container(m, 'a', "{sv}"));

        while (!sd_bus_message_at_end(m, 0)) {
            const char *key = NULL;
            int block = false;
            CK(sd_bus_message_enter_container(m, 'e', "sv"));
            CK(sd_bus_message_read(m, "s", &key));

            CK(sd_bus_message_enter_container(m, 'v', NULL));
            CK(sd_bus_message_read(m, "b", &block));
            kywc_log(KYWC_DEBUG, "block request source=%s target=%s unique=%s block=%d",
                     session->handle, session_block_scope->target_session_handle, key, block);
            if (apply_session_shortcut_block_state(session_block_scope, key, block)) {
                ret_count++;
            }

            CK(sd_bus_message_exit_container(m));
            CK(sd_bus_message_exit_container(m));
        }

        CK(sd_bus_message_exit_container(m));
        CK(sd_bus_message_exit_container(m));
        CK(sd_bus_message_exit_container(m));
    }

    CK(sd_bus_message_exit_container(m));

    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_append(reply, "u", ret_count));

    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);
    return 1;
}

static bool shortcut_key_name_check_available(const char *key_name)
{
    uint32_t modifiers;
    uint32_t keysym;
    if (!shortcut_key_name_parse(key_name, &modifiers, &keysym)) {
        return false;
    }

    struct key_binding *binding =
        kywc_key_binding_create_by_symbol(keysym, modifiers, NULL, FUNC_ID_CUSTOM, 0);
    if (!binding) {
        return false;
    }

    if (!kywc_key_binding_register(binding, KEY_BINDING_TYPE_CUSTOM_DEF, NULL, NULL)) {
        kywc_key_binding_destroy(binding);
        return false;
    }

    kywc_key_binding_destroy(binding);
    return true;
}

static int is_shortcut_available(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    uint32_t ret_result = true;

    CK(sd_bus_message_enter_container(m, 'a', "s"));
    const char *key_name;
    while (sd_bus_message_read(m, "s", &key_name)) {
        if (!ret_result) {
            continue;
        }
        if (!shortcut_key_name_check_available(key_name)) {
            ret_result = false;
        }
    }

    CK(sd_bus_reply_method_return(m, "b", ret_result));

    return 1;
}

static const char *global_shortcut_signal_name(enum key_binding_stage stage)
{
    switch (stage) {
    case KEY_BINDING_STAGE_ACTIVE:
        return "Activated";
    case KEY_BINDING_STAGE_REPEAT:
        return "Repeated";
    case KEY_BINDING_STAGE_INACTIVE:
        return "Deactivated";
    }

    return NULL;
}

void global_shortcuts_manager_action(void *data, enum key_binding_stage stage)
{
    if (!data) {
        return;
    }

    struct global_shortcut *shortcut = data;
    if (!shortcut) {
        return;
    }

    struct global_shortcut_session *session = shortcut->session;
    if (!session) {
        return;
    }

    const char *signal = global_shortcut_signal_name(stage);
    if (!signal) {
        return;
    }

    dbus_emit_signal(session->path, service_session_iface, signal, "sta{sv}", shortcut->unique,
                     (int64_t)time(NULL), 1, "action", "s",
                     shortcut->action ? shortcut->action : "");
}

static const sd_bus_vtable shortcut_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_SIGNAL("Activated", "sta{sv}", 0),
    SD_BUS_SIGNAL("Repeated", "sta{sv}", 0),
    SD_BUS_SIGNAL("Deactivated", "sta{sv}", 0),
    SD_BUS_METHOD("ListShortcuts", "", "a{sv}", list_shortcuts, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("BindShortcuts", "a{sv}", "u", bind_shortcuts, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("ConfigureShortcuts", "a{sv}", "u", configure_shortcuts,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("ResetShortcuts", "", "u", reset_shortcuts, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("BlockShortcuts", "a{sv}", "u", block_shortcuts, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("IsShortcutsAvailable", "as", "b", is_shortcut_available,
                  SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

static int session_name_owner_changed_handle(sd_bus_message *m, void *userdata,
                                             sd_bus_error *ret_error)
{
    const char *new_owner = NULL;
    CK(sd_bus_message_skip(m, "ss"));
    CK(sd_bus_message_read(m, "s", &new_owner));
    if (!new_owner || *new_owner) {
        return 0;
    }

    struct global_shortcut_session *session = userdata;
    shortcut_session_destroy(session, true);

    return 0;
}

static const char *get_session_dbus_path(const char *name)
{
    int len = snprintf(NULL, 0, "%s/%s", session_prefix, name) + 1;
    char *path = malloc(len);
    if (!path) {
        return NULL;
    }

    snprintf(path, len, "%s/%s", session_prefix, name);

    /* DBus path can only contain ASCII characters */
    char *p = path + strlen(session_prefix) + 1;
    for (; *p; ++p) {
        unsigned char c = (unsigned char)*p;
        if (!isalnum(c) || (c & ~0x7f) != 0) {
            *p = '_';
        }
    }

    return path;
}

static struct global_shortcut_session *shortcut_session_get_or_create(const char *session_handle,
                                                                      const char *sender)
{
    struct global_shortcut_session *session = shortcut_session_get(session_handle);
    if (session) {
        return session;
    }

    session = calloc(1, sizeof(*session));
    if (!session) {
        return NULL;
    }

    wl_list_init(&session->shortcuts);
    wl_list_init(&session->blocks);
    wl_list_init(&session->link);

    session->handle = strdup(session_handle);
    if (!session->handle) {
        goto error;
    }
    session->path = get_session_dbus_path(session_handle);
    if (!session->path) {
        goto error;
    }

    session->owner = sender ? strdup(sender) : NULL;
    if (sender && !session->owner) {
        goto error;
    }

    if (session->owner) {
        char *match_rule = string_create("sender='org.freedesktop.DBus',"
                                         "type='signal',"
                                         "interface='org.freedesktop.DBus',"
                                         "member='NameOwnerChanged',"
                                         "path='/org/freedesktop/DBus',"
                                         "arg0='%s'",
                                         session->owner);
        if (!match_rule) {
            goto error;
        }

        if (!dbus_add_match_slot(&session->watcher, match_rule, session_name_owner_changed_handle,
                                 session)) {
            free(match_rule);
            goto error;
        }
        free(match_rule);
    }

    session->dbus =
        dbus_register_object(NULL, session->path, service_session_iface, shortcut_vtable, session);
    if (!session->dbus) {
        goto error;
    }

    wl_list_insert(&manager->sessions, &session->link);

    return session;

error:
    shortcut_session_destroy(session, true);
    return NULL;
}

static int list_session(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    struct global_shortcuts_manager *manager = userdata;

    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "{sv}"));

    struct global_shortcut_session *session;
    wl_list_for_each(session, &manager->sessions, link) {
        CK(sd_bus_message_open_container(reply, 'e', "sv"));
        CK(sd_bus_message_append_basic(reply, 's', session->handle));

        CK(sd_bus_message_open_container(reply, 'v', "o"));
        CK(sd_bus_message_append_basic(reply, 'o', session->path));

        CK(sd_bus_message_close_container(reply));
        CK(sd_bus_message_close_container(reply));
    }

    CK(sd_bus_message_close_container(reply));
    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);

    return 1;
}

static int create_session(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    sd_bus_message *reply = NULL;
    struct global_shortcut_session *session = NULL;

    const char *session_token;
    CK(sd_bus_message_read(m, "s", &session_token));

    if (!session_token || *session_token == '\0') {
        goto done;
    }

    if (strcmp(session_token, session_builtin) == 0 || strcmp(session_token, IDENTIFIER_ALL) == 0) {
        goto done;
    }

    const char *sender = sd_bus_message_get_sender(m);
    session = shortcut_session_get_or_create(session_token, sender);
    if (!session) {
        goto done;
    }

done:
    CK(sd_bus_message_new_method_return(m, &reply));
    if (!session) {
        CK(sd_bus_message_append(reply, "b", false));
        CK(sd_bus_message_append(reply, "o", "/"));
    } else {
        CK(sd_bus_message_append(reply, "b", true));
        CK(sd_bus_message_append(reply, "o", session->path));
    }

    CK(sd_bus_send(NULL, reply, NULL));
    sd_bus_message_unref(reply);

    return 1;
}

static const sd_bus_vtable session_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("ListSessions", "", "a{sv}", list_session, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("CreateSession", "s", "bo", create_session, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END,
};

static bool setup_system_shortcuts_from_config(struct global_shortcuts_manager *manager)
{
    if (!manager->config || !manager->config->json) {
        return false;
    }

    struct global_shortcut_session *session = shortcut_session_get_or_create(session_builtin, NULL);
    if (!session) {
        kywc_log(KYWC_WARN, "Session: %s get or create failed", session_builtin);
        return false;
    }

    json_object_object_foreach(manager->config->json, shortcut_unique, shortcut_data) {
        struct global_shortcut *shortcut =
            shortcut_get_or_create_by_unique(session, shortcut_unique);
        if (!shortcut) {
            continue;
        }

        json_object *data;
        if (json_object_object_get_ex(shortcut_data, "enabled", &data)) {
            shortcut->enabled = json_object_get_boolean(data);
        }

        if (json_object_object_get_ex(shortcut_data, "current", &data)) {
            shortcut->current_key = strdup(json_object_get_string(data));
        }

        if (json_object_object_get_ex(shortcut_data, "default", &data)) {
            shortcut->default_key = strdup(json_object_get_string(data));
        }

        if (json_object_object_get_ex(shortcut_data, "func_id", &data)) {
            shortcut->func_id = json_object_get_uint64(data);
        }

        if (json_object_object_get_ex(shortcut_data, "options", &data)) {
            shortcut->options = json_object_get_uint64(data);
        } else {
            bool no_repeat = json_object_object_get_ex(shortcut_data, "no_repeat", &data) &&
                             json_object_get_boolean(data);

            uint32_t options = function_entry_get_key_binding_options(shortcut->func_id);
            if (!options) {
                options =
                    KEY_BINDING_TRIGGER_IN_ACTIVE | (no_repeat ? 0 : KEY_BINDING_TRIGGER_IN_REPEAT);
            }

            shortcut->options = options;
        }

        if (!shortcut_create_binding_and_register(session, shortcut)) {
            global_shortcut_destroy(shortcut, false);
        }
    }

    return true;
}

static bool system_shortcut_write_config(struct global_shortcut *shortcut)
{
    if (!manager || !manager->config || !manager->config->json || !shortcut) {
        return false;
    }

    json_object *config;
    config = json_object_object_get(manager->config->json, shortcut->unique);
    if (!config) {
        config = json_object_new_object();
        json_object_object_add(manager->config->json, shortcut->unique, config);
    }

    if (shortcut->default_key) {
        json_object_object_add(config, "default", json_object_new_string(shortcut->default_key));
    } else {
        json_object_object_del(config, "default");
    }

    if (shortcut->current_key) {
        json_object_object_add(config, "current", json_object_new_string(shortcut->current_key));
    } else {
        json_object_object_del(config, "current");
    }

    if (shortcut->func_id != FUNC_ID_CUSTOM) {
        json_object_object_add(config, "func_id", json_object_new_uint64(shortcut->func_id));
    } else {
        json_object_object_del(config, "func_id");
    }

    if (!shortcut->enabled) {
        json_object_object_add(config, "enabled", json_object_new_boolean(shortcut->enabled));
    } else {
        json_object_object_del(config, "enabled");
    }

    json_object_object_add(config, "options",
                           json_object_new_uint64(shortcut_key_binding_options(shortcut)));

    return true;
}

static bool system_builtin_shortcuts(struct key_binding *binding, char *key_name, char *unique,
                                     int32_t modifiers, int32_t key)
{
    enum function_id id = bindings_get_key_binding_function_id(binding);
    if (id == FUNC_ID_CUSTOM || !unique) {
        return false;
    }

    struct global_shortcut_session *session = shortcut_session_get_or_create(session_builtin, NULL);
    if (!session) {
        return false;
    }

    struct global_shortcut *shortcut = shortcut_get_or_create_by_unique(session, unique);
    if (!shortcut) {
        return false;
    }

    shortcut->binding = binding;
    if (!shortcut->default_key) {
        shortcut->default_key = key_name ? strdup(key_name) : NULL;
    }
    shortcut->func_id = id;
    if (!shortcut->options) {
        uint32_t options = function_entry_get_key_binding_options(id);
        if (!options) {
            options = KEY_BINDING_TRIGGER_IN_ACTIVE;
        }

        if (!bindings_get_key_binding_no_repeat(binding)) {
            options |= KEY_BINDING_TRIGGER_IN_REPEAT;
        }
        if (bindings_get_key_binding_bypass_grab(binding)) {
            options |= KEY_BINDING_BYPASS_GRAB;
        }

        shortcut->options = options;
    }

    /* save shortcut config */
    system_shortcut_write_config(shortcut);

    return false;
}

static void handle_server_start(struct wl_listener *listener, void *data)
{
    kywc_key_binding_for_each(system_builtin_shortcuts);
}

static void handle_server_destroy(struct wl_listener *listener, void *data)
{
    wl_list_remove(&manager->server_destroy.link);
    wl_list_remove(&manager->server_start.link);

    /* free all session shortcuts */
    struct global_shortcut_session *session, *tmp;
    wl_list_for_each_safe(session, tmp, &manager->sessions, link) {
        shortcut_session_destroy(session, false);
    }
}

static bool global_shortcuts_manager_config_init(struct global_shortcuts_manager *manager)
{
    manager->config = config_manager_add_config("Shortcuts");
    if (!manager->config) {
        return false;
    }

    return dbus_register_object(NULL, service_path, service_manager_iface, session_vtable, manager);
}

bool global_shortcuts_manager_create(struct server *server)
{
    manager = calloc(1, sizeof(*manager));
    if (!manager) {
        return false;
    }

    wl_list_init(&manager->sessions);
    global_shortcuts_manager_config_init(manager);
    setup_system_shortcuts_from_config(manager);

    manager->server_start.notify = handle_server_start;
    wl_signal_add(&server->events.start, &manager->server_start);

    manager->server_destroy.notify = handle_server_destroy;
    server_add_destroy_listener(server, &manager->server_destroy);
    return true;
}
