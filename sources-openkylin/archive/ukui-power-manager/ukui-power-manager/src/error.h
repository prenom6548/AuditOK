#ifndef ERROR_H
#define ERROR_H

//Qt
#include <QDebug>

#define SUCCESS                                     0

#define INIT_CONFIG_ERROR                           100
#define CONFIG_QSET_INIT_ERROR                      101
#define CONFIG_PRODUCT_NAME_ERROR                   102
#define CONFIG_VENDOR_GET_ERROR                     103
#define CONFIG_GET_CONF_DATA_ERROR                  104

#define INIT_BRIGHTNESS_ERROR                       200

#define INIT_LID_ERROR                              300
#define LID_GET_INHIBIT_ERROR                       301
#define LID_CREATE_LOGIN1_CONNECTION_ERROR          302
#define LID_CREATE_UPOWER_CONNECTION_ERROR          303

#define INIT_POWER_MANAGER_CONTROL_ERROR            400

#define INIT_IDLE_ERROR                             500

#define INIT_LOW_BATTERY_ERROR                      600

#define INIT_POWER_POLICY_ERROR                     700



#endif // ERROR_H
