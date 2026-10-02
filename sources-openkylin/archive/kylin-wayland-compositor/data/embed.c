// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdio.h>

#define COLS 16

int main(int argc, char *argv[])
{
    const char *varname = (argc > 1) ? argv[1] : "data";
    static const char hex[] = "0123456789abcdef";

    printf("static const char %s[] = {\n", varname);

    int count = 0, ch;

    while ((ch = getchar()) != EOF) {
        if (count % COLS == 0) {
            if (count > 0) {
                putchar('\n');
            }
            fputs("    ", stdout);
        } else {
            putchar(' ');
        }

        putchar('0');
        putchar('x');
        putchar(hex[(unsigned char)ch >> 4]);
        putchar(hex[(unsigned char)ch & 0x0F]);
        putchar(',');

        ++count;
    }

    if (count % COLS == 0 && count > 0) {
        fputs("\n    0x00,\n", stdout);
    } else {
        fputs(" 0x00,\n", stdout);
    }

    fputs("};", stdout);

    return 0;
}
