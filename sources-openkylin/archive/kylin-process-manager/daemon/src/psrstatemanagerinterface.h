#ifndef PSRSTATEMANAGERINTERFACE_H
#define PSRSTATEMANAGERINTERFACE_H

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <map>
#include <string>
#include <QDebug>
#include <mutex>

class PSRStateManager {
public:
    PSRStateManager();
    ~PSRStateManager();

    void openPSR();
    void closePSR();
    bool isSupportPSR();

private:
    void initPSRState();
    void getAllDirs(const QString &path, QStringList &dirs);
    std::string m_rawState;
    std::string m_psrFilePath;
    std::mutex m_psrMutex;
    bool m_supportPSR;


    
};


#endif // PSRSTATEMANAGER_H