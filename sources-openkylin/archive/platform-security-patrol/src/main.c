/**
 * @file main.c
 * @brief Platform Security Patrol Tool - Main Entry Point
 * 
 * 平台安全能力检测与使能工具主程序
 */

#include "core/cli.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    return cli_run(argc, argv);
}
