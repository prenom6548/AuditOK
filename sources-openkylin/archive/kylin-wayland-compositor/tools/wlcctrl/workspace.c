// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdio.h>

#include "commands.h"

static bool print_workspace(kywc_workspace *workspace, void *data)
{
    if (!workspace) {
        return false;
    }

    printf("workspace \"%s\"\n", workspace->uuid ? workspace->uuid : "unknown");
    printf("  name: %s\n", workspace->name ? workspace->name : "unknown");
    printf("  position: %u\n", workspace->position);
    printf("  activated: %s\n", workspace->activated ? "true" : "false");
    printf("\n");
    return false;
}

int cmd_list_workspaces(struct context *ctx, const char *param)
{
    kywc_context_for_each_workspace(ctx->kywc, print_workspace, NULL);
    return 0;
}

static bool count_workspace(kywc_workspace *workspace, void *data)
{
    (*(uint32_t *)data)++;
    return false;
}

int cmd_workspaceadd(struct context *ctx, const char *param)
{
    uint32_t count = 0;
    kywc_context_for_each_workspace(ctx->kywc, count_workspace, &count);

    uint32_t num = 1;
    if (param && !parse_u32(param, &num)) {
        fprintf(stderr, "The --workspaceadd option expects an integer.\n");
        return -1;
    }
    if (num == 0) {
        fprintf(stderr, "The --workspaceadd option expects a positive integer.\n");
        return -1;
    }
    if (count + num > MAX_WORKSPACES) {
        fprintf(stderr, "The total number of workspaces cannot exceed %d.\n", MAX_WORKSPACES);
        return -1;
    }

    for (uint32_t i = 0; i < num; i++) {
        kywc_workspace_create(ctx->kywc, NULL, count + i);
    }

    if (num == 1) {
        printf("Successfully added a new workspace.\n");
    } else {
        printf("Successfully added %u new workspaces.\n", num);
    }
    return 0;
}

static kywc_workspace *find_workspace(struct context *ctx, const char *uuid)
{
    kywc_workspace *workspace = kywc_context_find_workspace(ctx->kywc, uuid);
    if (!workspace) {
        fprintf(stderr, "Please enter the correct uuid for the specified workspace.\n");
    }
    return workspace;
}

int cmd_workspaceremove(struct context *ctx, const char *param)
{
    kywc_workspace *workspace = find_workspace(ctx, param);
    if (!workspace) {
        return -1;
    }

    kywc_workspace_remove(workspace);
    return 0;
}

int cmd_workspaceactive(struct context *ctx, const char *param)
{
    kywc_workspace *workspace = find_workspace(ctx, param);
    if (!workspace) {
        return -1;
    }

    kywc_workspace_activate(workspace);
    return 0;
}
