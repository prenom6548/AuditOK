// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _EFFECT_OUTPUT_TRANSFORM_H_
#define _EFFECT_OUTPUT_TRANSFORM_H_

#include "kywc/output.h"

enum output_transform_effect_type {
    OUTPUT_TRANSFORM_EFFECT_SCALE = 0,
    OUTPUT_TRANSFORM_EFFECT_ROTATE,
};

bool output_add_transform_effect(struct kywc_output *kywc_output,
                                 enum output_transform_effect_type type,
                                 struct kywc_output_state *old_state,
                                 struct kywc_output_state *new_state);

#endif /* _EFFECT_OUTPUT_TRANSFORM_H_ */
