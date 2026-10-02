#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <syslog.h>

#define PRINT_DEBUG 1

#define PRINT_DBG(...)                      \
    {                                       \
        if (PRINT_DEBUG) {                  \
            syslog(LOG_DEBUG, __VA_ARGS__); \
        }                                   \
    }

#define PRINT_NOTICES 1
#define PRINT_NOTICE(...)                    \
    {                                        \
        if (PRINT_NOTICES) {                 \
            syslog(LOG_NOTICE, __VA_ARGS__); \
        }                                    \
    }

#endif // !LOGHELPER_H