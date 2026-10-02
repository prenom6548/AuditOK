/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <cups/cups.h>
#include <jni.h>
// "printer-state": "3" if the destination is idle, "4" if the destination is printing a job, and "5" if the destination is stopped.
// https://www.cups.org/doc/cupspm.html
int get_printer_status(const char *printer_name) {
    cups_dest_t *dests;

    int num_dests = cupsGetDests(&dests);
    int printer_state = 0;
    // 遍历打印机列表
    for (int i = 0; i < num_dests; i++) {
        if (strcmp(dests[i].name, printer_name) == 0) {
            // 打印打印机状态
            for (int k = 0; k < dests[i].num_options; k++) {
                if (strcmp(dests[i].options[k].name, "printer-state") == 0) {
                    printer_state = *(dests[i].options[k].value) - '0';
                    break ;
                }
            }
            break;
        }
    }

    // 释放资源
    cupsFreeDests(num_dests, dests);
    return printer_state;
}
#include <jni.h>

JNIEXPORT jstring JNICALL Java_PrinterStatus_getPrinterStatus(JNIEnv *env, jobject obj) {

    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    int printer_state = 0;
    // 遍历打印机列表
    for (int i = 0; i < num_dests; i++) {
        if (strcmp(dests[i].name, "test") == 0) {
            // 打印打印机状态
            for (int k = 0; k < dests[i].num_options; k++) {
                if (strcmp(dests[i].options[k].name, "printer-state") == 0) {
                    printer_state = *(dests[i].options[k].value) - '0';
                    break ;
                }
            }
            break;
        }
    }

    // 释放资源
    cupsFreeDests(num_dests, dests);
    return (*env)->NewStringUTF(env, "Printer is idle");
}
