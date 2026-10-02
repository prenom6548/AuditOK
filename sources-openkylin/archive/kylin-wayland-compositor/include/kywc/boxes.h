// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _KYWC_BOXES_H_
#define _KYWC_BOXES_H_

#include <stdbool.h>

#include <wayland-server-protocol.h>

#define KYWC_API static __attribute__((unused)) inline

/* rect with top-left and size */
struct kywc_box {
    int x, y, width, height;
};

struct kywc_fbox {
    double x, y, width, height;
};

KYWC_API void kywc_box_adjust(struct kywc_box *box, int dx, int dy, int dw, int dh)
{
    box->x += dx;
    box->y += dy;
    box->width += dw;
    box->height += dh;
}

KYWC_API void kywc_box_adjust_ex(struct kywc_box *box, int dx1, int dy1, int dx2, int dy2)
{
    box->x += dx1;
    box->y += dy1;
    box->width += dx1 + dx2;
    box->height += dy1 + dy2;
}

KYWC_API struct kywc_box kywc_box_adjusted(const struct kywc_box *box, int dx, int dy, int dw,
                                           int dh)
{
    return (struct kywc_box){ box->x + dx, box->y + dy, box->width + dw, box->height + dh };
}

KYWC_API struct kywc_box kywc_box_adjusted_ex(const struct kywc_box *box, int dx1, int dy1, int dx2,
                                              int dy2)
{
    return (struct kywc_box){ box->x + dx1, box->y + dy1, box->width + dx1 + dx2,
                              box->height + dy1 + dy2 };
}

KYWC_API bool kywc_box_contains_point(const struct kywc_box *box, int x, int y)
{
    return box && x >= box->x && x < box->x + box->width && y >= box->y && y < box->y + box->height;
}

KYWC_API bool kywc_box_equal(const struct kywc_box *a, const struct kywc_box *b)
{
    return (a == b) || (a && b && a->x == b->x && a->y == b->y && a->width == b->width &&
                        a->height == b->height);
}

KYWC_API bool kywc_box_empty(const struct kywc_box *box)
{
    return !box || box->width <= 0 || box->height <= 0;
}

KYWC_API bool kywc_box_not_empty(const struct kywc_box *box)
{
    return box && box->width > 0 && box->height > 0;
}

// box b is contained in box a
KYWC_API bool kywc_box_contains_box(const struct kywc_box *a, const struct kywc_box *b)
{
    return kywc_box_not_empty(a) && b && b->x >= a->x && b->y >= a->y && b->width <= a->width &&
           b->height <= a->height;
}

KYWC_API bool kywc_fbox_empty(const struct kywc_fbox *box)
{
    return !box || box->width <= 0 || box->height <= 0;
}

KYWC_API bool kywc_fbox_not_empty(const struct kywc_fbox *box)
{
    return box && box->width > 0 && box->height > 0;
}

/**
 * Returns true if the two boxes are equal, false otherwise.
 */
bool kywc_fbox_equal(const struct kywc_fbox *a, const struct kywc_fbox *b);

/**
 * Checks whether two boxes intersect.
 *
 * Returns false if either box is empty.
 */
bool kywc_box_intersects(const struct kywc_box *a, const struct kywc_box *b);

bool kywc_box_intersection(struct kywc_box *dest, const struct kywc_box *box_a,
                           const struct kywc_box *box_b);

void kywc_box_transform(struct kywc_box *dest, const struct kywc_box *box,
                        enum wl_output_transform transform, int width, int height);

void kywc_fbox_transform(struct kywc_fbox *dest, const struct kywc_fbox *box,
                         enum wl_output_transform transform, double width, double height);

#endif /* _KYWC_BOXES_H_ */
