#include "vrrstatemanagerinterface.h"


namespace {
    const std::string vrr_path = "/sys/kernel/debug/dri/";
    const std::string signedRateRangeLineStr = "panel supports VRR, Adjustment range ";
    const std::string signedRawRateLineStr = "VRR state is";
    const std::string signedRawRateStr = "rate is ";
}

VRRStateManager::VRRStateManager()
{
    initVRRState();
}

VRRStateManager::~VRRStateManager()
{
}

void VRRStateManager::setVRRRefreshRate(std::string mode)
{
    PRINT_DBG("VRRStateManager::setVRRRefreshRate\n");
    std::lock_guard<std::mutex> lock(m_vrrMutex);
    std::fstream psrFile(m_vrrStateFullPath);
    if(!psrFile) {
        PRINT_NOTICE("setVRRRefreshRate: can not open file\n");
        return;
    }
    std::string rate = getFreshRateByMode(mode);
    psrFile << "manual" << std::endl;
    psrFile << "1 " << rate << std::endl;
    PRINT_DBG("setVRRRefreshRate: set psr to %s\n", rate.c_str()); 
    psrFile.close();
}

void VRRStateManager::reverseVRRRefreshRate()
{
    PRINT_DBG("VRRStateManager::reverseVRRRefreshRate\n");
    std::lock_guard<std::mutex> lock(m_vrrMutex);
    std::fstream psrFile(m_vrrStateFullPath);
    if(!psrFile) {
        PRINT_NOTICE("setVRRRefreshRate: can not open file\n");
        return;
    }
    psrFile << "0 " << m_rawVRRState << std::endl;
    psrFile << "auto" << std::endl;
    PRINT_DBG("reverseVRRRefreshRate: set psr to %s\n", m_rawVRRState.c_str()); 
    psrFile.close();
}

bool VRRStateManager::isSupportVRR()
{
    return m_supportVRR;
}

void VRRStateManager::initVRRState()
{

    // 打开目录
    DIR* dir = opendir(vrr_path.c_str());
    if (!dir) {
        PRINT_NOTICE("VRRStateManager::initVRRState: opendir failed %s\n", vrr_path.c_str());
        return;
    }

    dirent* entry;
    // 遍历目录项
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过当前目录(.)和父目录(..)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构建完整路径
        std::string fullPath = vrr_path;
        if (!fullPath.empty() && fullPath.back() != '/') {
            fullPath += "/";
        }
        fullPath += entry->d_name;

        // 获取文件信息
        struct stat fileInfo;
        if (stat(fullPath.c_str(), &fileInfo) == 0) {
            // 检查是否为目录
            if (S_ISDIR(fileInfo.st_mode)) {
                if (!fullPath.empty() && fullPath.back() != '/') {
                    fullPath += "/";
                }
                fullPath += "eDP-1/vrr_state";
                PRINT_DBG("VRRStateManager::initVRRState: fullPath %s\n", fullPath.c_str());
                if(access(fullPath.c_str(), F_OK) == 0) {
                    PRINT_DBG("VRRStateManager::initVRRState access: fullPath %s\n", fullPath.c_str());
                    initVRRData(fullPath);
                    break;
                }
                
            }
        }
    }

    // 关闭目录
    closedir(dir);
}

bool VRRStateManager::initVRRData(const std::string& vrrFullPath)
{
    std::fstream psrFile(vrrFullPath);
    if(!psrFile) {
        PRINT_NOTICE("initPSRState: can not open file\n");
        return false;
    }       

    std::string line;
    while(getline(psrFile, line)) {
        if(line.find(signedRateRangeLineStr) != std::string::npos) {
            size_t pos = line.find(signedRateRangeLineStr);
            std::string subStr = line.erase(pos, signedRateRangeLineStr.length());
            size_t posTail = subStr.find(' ');
            m_minRefreshRate = subStr.substr(0, posTail);
            PRINT_DBG("initVRRData: m_minRefreshRate: %s, posTail: %ld\n", m_minRefreshRate.c_str(), posTail);
            size_t posMaxRate = line.find("- ");
            m_maxRefreshRate = line.substr(posMaxRate+2, std::string::npos);
            PRINT_DBG("initVRRData: m_maxRefreshRate: %s, posTail: %ld\n", m_maxRefreshRate.c_str(), posTail);
            m_supportVRR = true;
            m_vrrStateFullPath = vrrFullPath;
            PRINT_DBG("initVRRData: m_vrrStateFullPath: %s\n", m_vrrStateFullPath.c_str());
        }
        
        if(line.find(signedRawRateLineStr) != std::string::npos) {
            size_t posRawRate = line.find(signedRawRateStr);
            m_rawVRRState = line.substr(posRawRate + signedRawRateStr.length(), std::string::npos);
            PRINT_DBG("initVRRData: m_rawVRRState: %s\n", m_rawVRRState.c_str());
        }
    }
    if(m_supportVRR) {
        PRINT_DBG("initVRRData: support VRR\n");
    } else {
        PRINT_DBG("initVRRData: not support VRR\n");
    }
    return false;
}

std::string VRRStateManager::getFreshRateByMode(std::string mode)
{
    std::string freshRate = "0";
    if(mode == "VRR_LOW") {
        freshRate = m_minRefreshRate;
    } else if(mode == "VRR_BALANCE") {
        int balanceRate = (std::stoi(m_minRefreshRate) + std::stoi(m_maxRefreshRate)) / 2;
        freshRate = balanceRate > 60 ? "60" : std::to_string(balanceRate);
    } else if(mode == "VRR_AUTO") {
        freshRate = m_rawVRRState;
    }

    return freshRate;

}
