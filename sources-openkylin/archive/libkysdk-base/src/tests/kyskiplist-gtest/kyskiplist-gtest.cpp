/*
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */
#include <stdio.h>
#include <string.h>

#include <gtest/gtest.h>
#include "../cpp_stub/src/stub.h"
extern "C"
{
#include "../../utils/data-structure/linklist/skip_linklist/skip_linklist.h"
}

#define ARRAY_SIZE 20

int array[ARRAY_SIZE];
int del_flag[ARRAY_SIZE] = {0};
kysdk_skiplist *list;

TEST(TestSuite1, kysdk_skiplist_insert)
{
    list = kysdk_create_skiplist();

    kysdk_skiplist_setmaxlevels(list, 5);

    srand(time(NULL));
    for (int i = 0; i < 20; i++)
    {
        int num = random() % 50 + 1;
        kysdk_listdata data;
        data.num = i;
        kysdk_skiplist_insert(list, num, data);
        printf("[%d] %d has been insert.\n", i, num);
        sleep(1);
    }
}

int kysdk_skiplist_search_stub(kysdk_skiplist *list, int key)
{
    return 1;
}

TEST(TestSuite2, kysdk_skiplist_search)
{
    for (int i = 0; i < 5; i++)
    {
        int target = random() % 50 + 1;
        kysdk_listdata data = kysdk_skiplist_search(list, target);
        printf("Searching for %d, data is %d\n", target, data.num);

        kysdk_skiplist_node *curnode = list->children[0];
        short finded = 0;
        while (curnode)
        {
            if (curnode->key == target)
            {
                finded = 1;
                break;
            }
            curnode = curnode->next[0];
        }

        if (finded && data.num == -1)
        {
            printf("Skiplist search test failed.\n");
            return;
        }
    }

    printf("Skiplist search test pass.\n");
}

TEST(TestSuite3, kysdk_skiplist_delete)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (random() % 2)
        {
            del_flag[i] = 1;
            kysdk_skiplist_delete(list, array[i]);
            printf("Deleted %d\n", array[i]);
        }
    }

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (del_flag[i] && kysdk_skiplist_search(list, array[i]).num != -1)
        {
            printf("Skiplist delete test failed.\n");
            return;
        }
    }

    printf("Skiplist order test pass.\n");

    kysdk_destroy_skiplist(list);
}