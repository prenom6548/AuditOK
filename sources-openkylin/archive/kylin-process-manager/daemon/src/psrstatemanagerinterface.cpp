#include "psrstatemanagerinterface.h"
#include "loghelper.h"
#include <fstream>
#include <unistd.h>
namespace {
    const QString psr_path = "/sys/kernel/debug/dri/";
    const std::string signStr = "Currently in psr ";
    const std::string dontSupportFlag = "eDP panel does not support PSR";
}

PSRStateManager::PSRStateManager()
{
    m_supportPSR = false;
    initPSRState();

}

PSRStateManager::~PSRStateManager()
{
}

void PSRStateManager::openPSR()
{
    std::lock_guard<std::mutex> lock(m_psrMutex);
    std::fstream psrFile(m_psrFilePath);
    if(!psrFile) {
        PRINT_NOTICE("openPSR: can not open file\n");
        return;
    }
    psrFile << "auto\n";
    PRINT_DBG("openPSR: set psr to auto\n");
    psrFile.close();
}

void PSRStateManager::closePSR()
{
    std::lock_guard<std::mutex> lock(m_psrMutex);
    std::fstream psrFile(m_psrFilePath);
    if(!psrFile) {
        PRINT_NOTICE("closePSR: can not open file\n");
        return;
    }
    if(m_rawState != "auto") {
        psrFile << "enter" << "\n";
        PRINT_DBG("closePSR: set psr to %s\n", m_rawState.c_str());
    } else {
        psrFile << "enter\n";
        PRINT_DBG("closePSR: set psr to enter\n");
    }  
    psrFile.close();
}

bool PSRStateManager::isSupportPSR()
{
    return m_supportPSR;
}

void PSRStateManager::initPSRState()
{
    QStringList dirs = {};
    getAllDirs(psr_path, dirs);
    for(auto psr_dir : dirs) {
        std::string whole_psr_path = psr_dir.toStdString() + "/eDP-1/psr_state";
        PRINT_DBG("initPSRState: whole_psr_path: %s\n", whole_psr_path.c_str());
        if(access(whole_psr_path.c_str(), F_OK) != 0) {
            PRINT_DBG("initPSRState: file is not exists\n");
            continue;
        }
        m_psrFilePath = whole_psr_path;
        std::fstream psrFile(whole_psr_path);
        if(!psrFile) {
            PRINT_DBG("initPSRState: can not open file\n");
            continue;
        }       

        std::string line;
        while(getline(psrFile, line)) {
            if(line.find(dontSupportFlag) != std::string::npos) {
                m_supportPSR = false;
                break;
            }
            
            if(line.find(signStr) != std::string::npos) {
                size_t pos = line.find(signStr);
                std::string subStr = line.erase(pos, signStr.length());
                size_t posTail = subStr.find(' ');
                m_rawState = subStr.substr(0, posTail);
                m_supportPSR = true;
                PRINT_DBG("initPSRState: raw state: %s, posTail: %ld\n", m_rawState.c_str(), posTail);
                break;
            }
        }

        if(m_supportPSR &&  m_rawState == "auto") {
            PRINT_DBG("initPSRState: init psr state to manual\n");
            psrFile << "enter\n";
        }
        psrFile.close();
        break;
    }
}

void PSRStateManager::getAllDirs(const QString &path, QStringList &dirs) {
    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        if (fileInfo.isDir()) {
            dirs << fileInfo.filePath();
        }
    }
}
