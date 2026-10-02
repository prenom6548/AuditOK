//
// Created by sqp on 2022/8/9.
//

#include "softbus_server_frame.h"
#include <unistd.h>

int main(){
    //    LOGD("[MyService] createServiceBinder call");
    InitSoftBusServer();
    while (1) {
        pause();
    }
    return 0;
}