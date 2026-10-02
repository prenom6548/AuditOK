// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <assert.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "util/spsc_chain.h"

struct spsc_node {
    uint8_t *data;
    uint32_t element_size;
    uint32_t capacity;          /* never changes */
    _Atomic uint32_t write_idx; /* producer */
    _Atomic uint32_t read_idx;  /* consumer */
    _Atomic void *next;         /* → newer node (toward head) */
};

/* Chain of nodes linked oldest-first. */
struct spsc_chain {
    uint32_t element_size;
    uint32_t base_capacity;
    struct spsc_node *head; /* newest — push here */
    struct spsc_node *tail; /* oldest — pop from here */
};

static struct spsc_node *node_create(uint32_t element_size, uint32_t capacity)
{
    struct spsc_node *queue = calloc(1, sizeof(*queue));
    if (!queue) {
        return NULL;
    }

    queue->element_size = element_size;
    queue->capacity = capacity;
    queue->data = calloc(capacity, element_size);
    if (!queue->data) {
        free(queue);
        return NULL;
    }

    atomic_init(&queue->write_idx, 0);
    atomic_init(&queue->read_idx, 0);
    atomic_init(&queue->next, NULL);

    return queue;
}

static void node_destroy(struct spsc_node *q)
{
    if (q) {
        free(q->data);
        free(q);
    }
}

struct spsc_chain *spsc_chain_create(uint32_t element_size, uint32_t capacity)
{
    assert(element_size > 0);
    assert(capacity > 1);

    struct spsc_chain *sc = calloc(1, sizeof(*sc));
    if (!sc) {
        return NULL;
    }

    struct spsc_node *queue = node_create(element_size, capacity);
    if (!queue) {
        free(sc);
        return NULL;
    }

    sc->element_size = element_size;
    sc->base_capacity = capacity;
    sc->head = queue;
    sc->tail = queue;

    return sc;
}

void spsc_chain_destroy(struct spsc_chain *sc)
{
    if (!sc) {
        return;
    }

    struct spsc_node *q = sc->tail;
    while (q) {
        struct spsc_node *next =
            (struct spsc_node *)atomic_load_explicit(&q->next, memory_order_relaxed);
        node_destroy(q);
        q = next;
    }

    free(sc);
}

bool spsc_chain_push(struct spsc_chain *sc, const void *element)
{
    struct spsc_node *head = sc->head;
    uint32_t cap = head->capacity;
    uint32_t w = atomic_load_explicit(&head->write_idx, memory_order_relaxed);
    uint32_t r = atomic_load_explicit(&head->read_idx, memory_order_acquire);
    uint32_t next = (w + 1) % cap;

    if (next == r) {
        /* Head full — link a new node at head.  SPSC: producer
         * exclusively owns head, no lock needed. */
        struct spsc_node *q = node_create(sc->element_size, cap * 2);
        if (!q) {
            return false;
        }

        atomic_store_explicit(&head->next, (void *)q, memory_order_release);
        sc->head = q;

        /* Write into the fresh node. */
        head = q;
        cap = head->capacity;
        w = 0;
        next = 1;
    }

    uint8_t *data = head->data;
    memcpy(data + (size_t)w * sc->element_size, element, sc->element_size);
    atomic_store_explicit(&head->write_idx, next, memory_order_release);

    return true;
}

bool spsc_chain_pop(struct spsc_chain *sc, void *element)
{
    struct spsc_node *tail = sc->tail;

    while (tail) {
        uint32_t cap = tail->capacity;
        uint32_t r = atomic_load_explicit(&tail->read_idx, memory_order_relaxed);
        uint32_t w = atomic_load_explicit(&tail->write_idx, memory_order_acquire);

        if (r != w) {
            uint8_t *data = tail->data;
            memcpy(element, data + (size_t)r * sc->element_size, sc->element_size);
            atomic_store_explicit(&tail->read_idx, (r + 1) % cap, memory_order_release);
            return true;
        }

        /* Tail drained — advance toward head. */
        struct spsc_node *next =
            (struct spsc_node *)atomic_load_explicit(&tail->next, memory_order_acquire);
        if (!next) {
            return false;
        }

        sc->tail = next;
        node_destroy(tail);
        tail = next;
    }

    return false;
}
