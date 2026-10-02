#include <stdio.h>
#include <unistd.h>

#include "libfidoukey/fidoukey_realization.h"
//#include "extra/demo.h"
#include "id_tables.h"

int main()
{
    int ret = 0;
    int uid = 0;

    uid = getuid();
    if (uid != 0)
        return -1;

    /*
     *  厂商可根据需要进行设备的初始化
    */

    //ret = demo_device_discover();
    ret = 0;
    
    if (ret < 0)
        return -2;

    if (ret == 0)
        return 0;

    /*
     * 厂商可根据需要进行设备的释放
    */

    printf("[%d] %s\n", ret, BIO_NAME);


    return 0;
}
