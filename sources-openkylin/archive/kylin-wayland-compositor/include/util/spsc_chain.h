// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_SPSC_CHAIN_H_
#define _UTIL_SPSC_CHAIN_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * Lock-free SPSC queue with chain-based dynamic expansion.
 *
 * Each node is a fixed-capacity lock-free SPSC queue that never resizes.
 * Overflow is handled by linking a new  node at the head —
 * no element copy, no lock.
 *
 * Producer  — owns head; writes, creates new heads on overflow.
 * Consumer  — owns tail; reads, frees drained tails as it advances.
 */

struct spsc_chain;

struct spsc_chain *spsc_chain_create(uint32_t element_size, uint32_t capacity);

void spsc_chain_destroy(struct spsc_chain *sq);

bool spsc_chain_push(struct spsc_chain *sq, const void *element);

bool spsc_chain_pop(struct spsc_chain *sq, void *element);

#endif /* _UTIL_SPSC_CHAIN_H_ */
