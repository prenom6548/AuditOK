#ifndef VRRSTATEMANAGERINTERFACE_H
#define VRRSTATEMANAGERINTERFACE_H
#include <string>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <fstream>
#include "loghelper.h"
#include <unistd.h>
#include <mutex>

class VRRStateManager {
public:
    VRRStateManager();
    ~VRRStateManager();
    void setVRRRefreshRate(std::string mode);
    void reverseVRRRefreshRate();
    bool isSupportVRR();
private:
    void initVRRState();
    bool initVRRData(const std::string& vrrFullPath);
    std::string getFreshRateByMode(std::string mode);
    std::string m_minRefreshRate;
    std::string m_maxRefreshRate;
    std::string m_rawVRRState;
    std::string m_vrrStateFullPath;
    std::mutex m_vrrMutex;
    bool m_supportVRR;
};

#endif