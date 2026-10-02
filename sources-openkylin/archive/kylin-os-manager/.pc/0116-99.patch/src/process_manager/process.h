#ifndef KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_PROCESS_H
#define KYLIN_OS_MANAGER_SRC_PROCESS_MANAGER_PROCESS_H

#include <utility>
#include <memory>
#include <QObject>
#include <QProcess>
#include "conf.h"

namespace process_manager {

class Process : public QObject {
    Q_OBJECT

public:
    Process(ProcessConf conf);
    ~Process();

    std::pair<bool, std::string> start();
    std::pair<bool, std::string> stop();

    pid_t pid();
    time_t startTime();
    bool isAutostart();
    bool isRunning();

private:
    QProcess *m_qtProcess;
    ProcessConf m_conf;
    bool m_isManualStop;    // 手动停止时不应该再根据配置文件自动拉起
    time_t m_startTime;

private Q_SLOTS:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

}

#endif
