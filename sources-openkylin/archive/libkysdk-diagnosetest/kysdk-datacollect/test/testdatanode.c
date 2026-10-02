/*
* libkysdk-diagnosetest's Library
*
* Copyright (C) 2023, KylinSoft Co., Ltd.
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
* Authors: Zhen Sun <sunzhen1@kylinos.cn>
*
*/

#include "libkydatacollect.h"
#include <stdio.h>
#include <unistd.h>

void test_default_upload()
{
    //只需要填入必填项，需要手动释放data_node
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
    kdk_dia_upload_default(node,"openFile","mainWindow");
    kdk_dia_data_free(node);
}

void test_process_start_and_exit()
{
    //在程序启动时调用，不需要传入参数
    kdk_dia_upload_program_startup();

    //模拟程序运行时间
    sleep(5);

    //在程序退出前调用，不需要传入参数
    kdk_dia_upload_program_exit();
}


void test_upload_search_content()
{
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_SEARCH);
    kdk_dia_upload_search_content(node,"searchApp","normalPage","音乐");
    kdk_dia_data_free(node);
}

void test_upload_page_info()
{
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
    kdk_dia_append_page_info(node,"pageUrl","pagePath");
    kdk_dia_append_page_flag(node,true,false);
    kdk_dia_upload_default(node,"test_upload_page_info","mainPage");
    kdk_dia_data_free(node);
}

void test_upload_custom_info()
{
    KCustomProperty property[2];
    property[0].key = "key1";
    property[0].value = "value1";
    property[1].key = "key2";
    property[1].value = "value2";
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_WEB,5);
    kdk_dia_append_custom_property(node,property,2);
    kdk_dia_upload_default(node,"test_upload_custom_info","mainPage");
    kdk_dia_data_free(node);
}

int main()
{
    // test_default_upload();
    test_process_start_and_exit();
    // test_upload_search_content();
    // test_upload_page_info();
    // test_upload_custom_info();
    return 0;
}