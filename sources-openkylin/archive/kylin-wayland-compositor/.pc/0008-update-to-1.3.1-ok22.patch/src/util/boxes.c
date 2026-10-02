// SPDX-FileCopyrightText: 2022 The wlroots contributors
// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <math.h>
#include <stddef.h>

#include <kywc/boxes.h>

bool kywc_fbox_equal(const struct kywc_fbox *a, const struct kywc_fbox *b)
{
    if (kywc_fbox_empty(a)) {
        a = NULL;
    }
    if (kywc_fbox_empty(b)) {
        b = NULL;
    }

    if (a == NULL || b == NULL) {
        return a == b;
    }

    return a->x == b->x && a->y == b->y && a->width == b->width && a->height == b->height;
}

bool kywc_box_intersection(struct kywc_box *dest, const struct kywc_box *box_a,
                           const struct kywc_box *box_b)
{
    bool a_empty = kywc_box_empty(box_a);
    bool b_empty = kywc_box_empty(box_b);

    if (a_empty || b_empty) {
        *dest = (struct kywc_box){ 0 };
        return false;
    }

    int x1 = fmax(box_a->x, box_b->x);
    int y1 = fmax(box_a->y, box_b->y);
    int x2 = fmin(box_a->x + box_a->width, box_b->x + box_b->width);
    int y2 = fmin(box_a->y + box_a->height, box_b->y + box_b->height);

    dest->x = x1;
    dest->y = y1;
    dest->width = x2 - x1;
    dest->height = y2 - y1;

    return !kywc_box_empty(dest);
}

void kywc_box_transform(struct kywc_box *dest, const struct kywc_box *box,
                        enum wl_output_transform transform, int width, int height)
{
    struct kywc_box src = { 0 };
    if (box != NULL) {
        src = *box;
    }

    if (transform % 2 == 0) {
        dest->width = src.width;
        dest->height = src.height;
    } else {
        dest->width = src.height;
        dest->height = src.width;
    }

    switch (transform) {
    case WL_OUTPUT_TRANSFORM_NORMAL:
        dest->x = src.x;
        dest->y = src.y;
        break;
    case WL_OUTPUT_TRANSFORM_90:
        dest->x = height - src.y - src.height;
        dest->y = src.x;
        break;
    case WL_OUTPUT_TRANSFORM_180:
        dest->x = width - src.x - src.width;
        dest->y = height - src.y - src.height;
        break;
    case WL_OUTPUT_TRANSFORM_270:
        dest->x = src.y;
        dest->y = width - src.x - src.width;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED:
        dest->x = width - src.x - src.width;
        dest->y = src.y;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        dest->x = src.y;
        dest->y = src.x;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED_180:
        dest->x = src.x;
        dest->y = height - src.y - src.height;
        break;
    case WL_OUTPUT_TRANSFORM_FLIPPED_270:
        dest->x = height - src.y - src.height;
        dest->y = width - src.x - src.width;
        break;
    }
}
