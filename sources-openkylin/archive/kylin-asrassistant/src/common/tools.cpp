#include "tools.h"
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include "libkydatacollect.h"

using namespace std;

namespace Tools {
bool checkNet()
{
    try {
        vector<string> v;
        std::array<char, 128> buffer;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("ping www.baidu.com -c 2 -w 2 2> /dev/null", "r"), pclose);

        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            v.push_back(buffer.data());
        }

        //读取倒数第二行 2 packets transmitted, 2 received, 0% packet loss, time 1001ms
        if (v.size() > 1) {
            string data = v[v.size() - 2];
            int iPos = data.find("received,");
            if (iPos != -1) {
                data = data.substr(iPos - 3, 3);  //截取字符串返回packet loss
                int n = atoi(data.c_str());
                if (n == 0) {
                    return false;
                } else {
                    return true;
                }
            } else {
                int iPos2 = data.find("已接收");
                if (iPos2 != -1) {
                    data = data.substr(iPos + 10, 3);  //截取字符串返回packet loss
                    int n = atoi(data.c_str());
                    if (n == 0) {
                        return false;
                    } else {
                        return true;
                    }
                }
            }
        } else {
            return false;
        }
    } catch (std::exception &e) {
        return true;
    }
    return true;
}

void processStr(QString &str)
{
    str.remove("▁");
    str.remove("_");
    str = str.replace("传输", "传书");
    str = str.replace("客录", "刻录");
    str = str.replace("变迁", "便签");
    str = str.replace("归党", "归档");
}

void addDiaNode(QString appName)
{
    /* 
    typedef struct 
    {
        char* key;
        char* value;
    }KCustomProperty;
    */
    KCustomProperty property;
    strcpy(property.key, "triggerSoureApp");
    strcpy(property.value, appName.toStdString().c_str());
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node,&property,1);
    kdk_dia_upload_default(node,"asr_assistant_wake_up_event","asr_assistant_home_page");
    kdk_dia_data_free(node);
}

}  // namespace Tools