// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _GNU_SOURCE
#include <errno.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "util/macros.h"
#include "util/time.h"

#include "keyboard.h"

struct keymap_entry {
    xkb_keysym_t xkb;
    wchar_t wchr;
};

static int create_shm_file(void)
{
    return memfd_create("wlcctrl-keymap", 0);
}

static int shm_alloc_fd(size_t size)
{
    int fd = create_shm_file();
    if (fd < 0) {
        return -1;
    }

    int ret;
    do {
        ret = ftruncate(fd, size);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static char *keysym_conversion(keyboard *keyboard, char *text)
{
    // keysym remap table
    const struct {
        char *from;
        char *to;
    } remap_table[] = {
        { "PrintScreen", "sys_req" }, { "esc", "escape" }, { "ctrl", "control_l" },
        { "shift", "shift_l" },       { "alt", "alt_l" },  { "winleft", "super_l" },
        { "enter", "return" },
    };

    for (size_t i = 0; i < ARRAY_SIZE(remap_table); i++) {
        if (strcasecmp(text, remap_table[i].from) == 0) {
            return remap_table[i].to;
        }
    }

    return text;
}

int keyboard_init(keyboard *keyboard, const struct xkb_rule_names *rule_names)
{
    keyboard->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!keyboard->context) {
        return -1;
    }

    keyboard->keymap =
        xkb_keymap_new_from_names(keyboard->context, rule_names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keyboard->keymap) {
        printf("xkb_keymap_new_from_names failed\n");
        goto keymap_failure;
    }

    if (xkb_keymap_num_layouts(keyboard->keymap) > 1) {
        printf("Multiple keyboard layouts have been specified, but only one is supported.\n");
    }

    keyboard->state = xkb_state_new(keyboard->keymap);
    if (!keyboard->state) {
        printf("xkb_state_new failed\n");
        goto state_failure;
    }

    char *keymap_string = xkb_keymap_get_as_string(keyboard->keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
    if (!keymap_string) {
        printf("xkb_keymap_get_as_string failed\n");
        goto keymap_string_failure;
    }

    size_t keymap_size = strlen(keymap_string) + 1;

    int keymap_fd = shm_alloc_fd(keymap_size);
    if (keymap_fd < 0) {
        printf("shm_alloc_fd failed\n");
        goto fd_failure;
    }

    size_t written = 0;
    while (written < keymap_size) {
        ssize_t ret = write(keymap_fd, keymap_string + written, keymap_size - written);
        if (ret == -1 && errno == EINTR)
            continue;
        if (ret == -1)
            goto write_failure;
        written += ret;
    }

    free(keymap_string);

    zwp_virtual_keyboard_v1_keymap(keyboard->virtual_keyboard, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
                                   keymap_fd, keymap_size);
    wl_display_roundtrip(keyboard->display);
    close(keymap_fd);

    return 0;

write_failure:
    close(keymap_fd);
fd_failure:
    free(keymap_string);
keymap_string_failure:
    xkb_state_unref(keyboard->state);
    keyboard->state = NULL;
state_failure:
    xkb_keymap_unref(keyboard->keymap);
    keyboard->keymap = NULL;
keymap_failure:
    xkb_context_unref(keyboard->context);
    keyboard->context = NULL;
    return -1;
}

static char *get_symbol_name(xkb_keysym_t sym, char *dst, size_t size)
{
    if (xkb_keysym_get_name(sym, dst, size) >= 0) {
        return dst;
    }

    snprintf(dst, size, "UNKNOWN (%x)", sym);
    return dst;
}

static void send_key(keyboard *keyboard, xkb_keycode_t code)
{
    zwp_virtual_keyboard_v1_key(keyboard->virtual_keyboard, current_time_msec(), code,
                                WL_KEYBOARD_KEY_STATE_PRESSED);
    zwp_virtual_keyboard_v1_key(keyboard->virtual_keyboard, current_time_msec(), code,
                                WL_KEYBOARD_KEY_STATE_RELEASED);
    wl_display_roundtrip(keyboard->display);
}

static xkb_keycode_t get_key_code(keyboard *keyboard, xkb_keysym_t keysym)
{
    xkb_keycode_t min_keycode, max_keycode;
    uint32_t code;

    min_keycode = xkb_keymap_min_keycode(keyboard->keymap);
    max_keycode = xkb_keymap_max_keycode(keyboard->keymap);

    for (code = min_keycode; code < max_keycode; code++) {
        size_t n_levels = xkb_keymap_num_levels_for_key(keyboard->keymap, code, 0);

        for (size_t level = 0; level < n_levels; level++) {
            const xkb_keysym_t *symbols;
            size_t n_syms =
                xkb_keymap_key_get_syms_by_level(keyboard->keymap, code, 0, level, &symbols);
            for (size_t sym_idx = 0; sym_idx < n_syms; sym_idx++) {
                if (symbols[sym_idx] == keysym) {
                    return code;
                }
            }
        }
    }

    char name[256];
    printf("Failed to look up keyboard symbol: %s\n", get_symbol_name(keysym, name, sizeof(name)));
    return 0;
}

static void run_keyboard(keyboard *keyboard, char *text)
{
    keyboard_press(keyboard, text, true);
    keyboard_press(keyboard, text, false);
}

static void apply_modifier(keyboard *keyboard, xkb_keycode_t code, bool press)
{
    enum xkb_state_component comp, compmask;
    comp = xkb_state_update_key(keyboard->state, code,
                                press == WL_KEYBOARD_KEY_STATE_PRESSED ? XKB_KEY_DOWN : XKB_KEY_UP);

    compmask = XKB_STATE_MODS_DEPRESSED | XKB_STATE_MODS_LATCHED | XKB_STATE_MODS_LOCKED |
               XKB_STATE_MODS_EFFECTIVE;

    if (!(comp & compmask)) {
        return;
    }

    xkb_mod_mask_t depressed, latched, locked, group;

    depressed = xkb_state_serialize_mods(keyboard->state, XKB_STATE_MODS_DEPRESSED);
    latched = xkb_state_serialize_mods(keyboard->state, XKB_STATE_MODS_LATCHED);
    locked = xkb_state_serialize_mods(keyboard->state, XKB_STATE_MODS_LOCKED);
    group = xkb_state_serialize_mods(keyboard->state, XKB_STATE_MODS_EFFECTIVE);
    zwp_virtual_keyboard_v1_modifiers(keyboard->virtual_keyboard, depressed, latched, locked,
                                      group);
    wl_display_roundtrip(keyboard->display);
    xkb_state_update_mask(keyboard->state, depressed, latched, locked, 0, 0, group);
    if (depressed & 0x01)
        keyboard->shift = true;
    else
        keyboard->shift = false;
}

void keyboard_press(keyboard *keyboard, char *text, bool press)
{
    if (!keyboard) {
        return;
    }

    char *name = keysym_conversion(keyboard, text);
    xkb_keysym_t keysym = xkb_keysym_from_name(name, XKB_KEYSYM_CASE_INSENSITIVE);
    if (keysym == XKB_KEY_NoSymbol) {
        fprintf(stderr, "Failed to convert character to keysym: %s\n", text);
        return;
    }

    if (!keyboard->shift) {
        if (strlen(name) == 1 && name[0] >= 'A' && name[0] <= 'Z') {
            zwp_virtual_keyboard_v1_modifiers(keyboard->virtual_keyboard, 1, 0, 0, 0);
            wl_display_roundtrip(keyboard->display);
        }
    }

    xkb_keycode_t code = get_key_code(keyboard, keysym);
    if (code != 0) {
        if (XKB_KEY_Super_L != keysym && XKB_KEY_Super_R != keysym)
            apply_modifier(keyboard, code, press);
        zwp_virtual_keyboard_v1_key(keyboard->virtual_keyboard, current_time_msec(), code - 8,
                                    press ? WL_KEYBOARD_KEY_STATE_PRESSED
                                          : WL_KEYBOARD_KEY_STATE_RELEASED);
        wl_display_roundtrip(keyboard->display);
        if (XKB_KEY_Super_L == keysym || XKB_KEY_Super_R == keysym)
            apply_modifier(keyboard, code, press);
    }

    if (keyboard->shift)
        return;
    if (strlen(name) == 1 && name[0] >= 'A' && name[0] <= 'Z') {
        zwp_virtual_keyboard_v1_modifiers(keyboard->virtual_keyboard, 0, 0, 0, 0);
        wl_display_roundtrip(keyboard->display);
    }
}

static void keyboard_send_key(keyboard *keyboard, char *text)
{
    char *name = keysym_conversion(keyboard, text);
    xkb_keysym_t keysym = xkb_keysym_from_name(name, XKB_KEYSYM_CASE_INSENSITIVE);
    if (keysym == XKB_KEY_NoSymbol) {
        int len = strlen(text);
        char str[2];
        for (int i = 0; i < len; i++) {
            str[0] = name[i];
            str[1] = '\0';
            run_keyboard(keyboard, str);
        }
        return;
    }
    run_keyboard(keyboard, name);
}

static void keyboard_send_key_width_modifier(keyboard *keyboard, char *text, bool press)
{
    char *name = keysym_conversion(keyboard, text);
    xkb_keysym_t keysym = xkb_keysym_from_name(name, XKB_KEYSYM_CASE_INSENSITIVE);
    if (keysym == XKB_KEY_NoSymbol) {
        if (press) {
            int len = strlen(name);
            char str[2];
            for (int i = 0; i < len; i++) {
                str[0] = name[i];
                str[1] = '\0';
                keyboard_press(keyboard, str, press);
            }
        } else {
            int len = strlen(name) - 1;
            char str[2];
            for (int i = len; i >= 0; i--) {
                str[0] = name[i];
                str[1] = '\0';
                keyboard_press(keyboard, str, press);
            }
        }
        return;
    }
    keyboard_press(keyboard, name, press);
}

void keyboard_set(keyboard *keyboard, char *text)
{
    const char delimiter[] = "+";
    char *token;
    char *tokens[5];
    int token_count = 0;

    token = strtok(text, delimiter);

    while (token != NULL && token_count < 5) {
        tokens[token_count] = strdup(token);
        if (tokens[token_count] == NULL) {
            perror("strdup");
            return;
        }
        token_count++;
        token = strtok(NULL, delimiter);
    }

    if (token_count > 1) {
        for (int i = 0; i < token_count; i++) {
            keyboard_send_key_width_modifier(keyboard, tokens[i], true);
        }
        for (int i = token_count - 1; i >= 0; i--) {
            keyboard_send_key_width_modifier(keyboard, tokens[i], false);
        }
    } else {
        keyboard_send_key(keyboard, text);
    }
    for (int i = 0; i < token_count; i++) {
        free(tokens[i]);
    }
}

static void print_keysym_name(xkb_keysym_t keysym, FILE *f)
{
    char sym_name[256];

    int ret = xkb_keysym_get_name(keysym, sym_name, sizeof(sym_name));
    if (ret <= 0) {
        printf("Unable to get XKB symbol name for keysym %04x\n", keysym);
        return;
    }

    fprintf(f, "%s", sym_name);
}

static void upload_keymap(keyboard *keyboard)
{
    char filename[] = "/tmp/wlcctrl-XXXXXX";
    int fd = mkstemp(filename);
    if (fd < 0) {
        printf("Failed to create the temporary keymap file");
    }
    unlink(filename);
    FILE *f = fdopen(fd, "w");

    fprintf(f, "xkb_keymap {\n");

    fprintf(f,
            "xkb_keycodes \"(unnamed)\" {\n"
            "minimum = 8;\n"
            "maximum = %ld;\n",
            keyboard->keymap_len + 9 + 1);
    for (size_t i = 0; i < keyboard->keymap_len; i++) {
        fprintf(f, "<K%ld> = %ld;\n", i + 1, i + 9 + 1);
    }
    fprintf(f, "};\n");

    // TODO: Is including "complete" here really a good idea?
    fprintf(f, "xkb_types \"(unnamed)\" { include \"complete\" };\n");
    fprintf(f, "xkb_compatibility \"(unnamed)\" { include \"complete\" };\n");

    fprintf(f, "xkb_symbols \"(unnamed)\" {\n");
    for (size_t i = 0; i < keyboard->keymap_len; i++) {
        fprintf(f, "key <K%ld> {[", i + 1);
        print_keysym_name(keyboard->keymap_entry[i].xkb, f);
        fprintf(f, "]};\n");
    }
    fprintf(f, "};\n");

    fprintf(f, "};\n");
    fputc('\0', f);
    fflush(f);
    size_t keymap_size = ftell(f);

    zwp_virtual_keyboard_v1_keymap(keyboard->virtual_keyboard, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
                                   fileno(f), keymap_size);

    wl_display_roundtrip(keyboard->display);

    fclose(f);
}

static unsigned int append_keymap_entry(keyboard *keyboard, wchar_t ch, xkb_keysym_t xkb)
{
    keyboard->keymap_entry =
        realloc(keyboard->keymap_entry, ++keyboard->keymap_len * sizeof(keyboard->keymap_entry[0]));
    keyboard->keymap_entry[keyboard->keymap_len - 1].wchr = ch;
    keyboard->keymap_entry[keyboard->keymap_len - 1].xkb = xkb;
    return keyboard->keymap_len;
}

static int get_key_code_by_wchar(keyboard *keyboard, wchar_t ch)
{
    const struct {
        wchar_t from;
        xkb_keysym_t to;
    } remap_table[] = {
        { L'\n', XKB_KEY_Return },
        { L'\t', XKB_KEY_Tab },
        { L'\e', XKB_KEY_Escape },
    };

    for (unsigned int i = 0; i < keyboard->keymap_len; i++) {
        if (keyboard->keymap_entry[i].wchr == ch) {
            return i + 1;
        }
    }

    xkb_keysym_t xkb = xkb_utf32_to_keysym(ch);
    if (xkb == XKB_KEY_NoSymbol) {
        fprintf(stderr, "Failed to convert character to keysym: %lc\n", ch);
        return -1;
    }

    for (size_t i = 0; i < ARRAY_SIZE(remap_table); i++) {
        if (remap_table[i].from == ch) {
            // This overwrites whatever xkb gave us before.
            xkb = remap_table[i].to;
            break;
        }
    }

    return append_keymap_entry(keyboard, ch, xkb);
}

int key_string_action(keyboard *keyboard, char *str)
{
    size_t raw_len = strlen(str) + 2; // NULL byte and the potential space
    wchar_t text[raw_len];            // Upper bound on size
    memset(text, 0, sizeof(text));
    setlocale(LC_CTYPE, "");
    ssize_t ret = mbstowcs(text, str, ARRAY_SIZE(text));
    if (ret < 0) {
        printf("Failed to deencode input.\n");
        return -1;
    }

    keyboard->key_codes = calloc(ret, sizeof(keyboard->key_codes[0]));
    if (!keyboard->key_codes) {
        return -1;
    }

    keyboard->key_codes_len = ret;

    for (ssize_t k = 0; k < ret; k++) {
        int key_code = get_key_code_by_wchar(keyboard, text[k]);
        if (key_code < 0)
            break;
        keyboard->key_codes[k] = key_code;
    }

    upload_keymap(keyboard);

    for (size_t i = 0; i < keyboard->key_codes_len; i++) {
        send_key(keyboard, keyboard->key_codes[i] + 1);
    }

    return 0;
}

void keyboard_destroy(keyboard *keyboard)
{
    free(keyboard->key_codes);
    free(keyboard->keymap_entry);
    if (keyboard->state) {
        xkb_state_unref(keyboard->state);
    }
    if (keyboard->keymap) {
        xkb_keymap_unref(keyboard->keymap);
    }
    if (keyboard->context) {
        xkb_context_unref(keyboard->context);
    }
    if (keyboard->virtual_keyboard) {
        zwp_virtual_keyboard_v1_destroy(keyboard->virtual_keyboard);
    }
    free(keyboard);
}
