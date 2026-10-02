#ifndef _HILOG_H
#define _HILOG_H

#include <stdio.h>
//#include <android/log.h>

#define HILOG_DEBUG(type, fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#define HILOG_INFO(type, fmt, ...)  printf(fmt"\n", ##__VA_ARGS__)
#define HILOG_WARN(type, fmt, ...)  printf(fmt"\n", ##__VA_ARGS__)
#define HILOG_ERROR(type, fmt, ...) printf(fmt"\n", ##__VA_ARGS__)

//#define HILOG_DEBUG(type, fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##__VA_ARGS__)
//#define HILOG_INFO(type, fmt, ...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt, ##__VA_ARGS__)
//#define HILOG_WARN(type, fmt, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
//#define HILOG_ERROR(type, fmt, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##__VA_ARGS__)

#endif
