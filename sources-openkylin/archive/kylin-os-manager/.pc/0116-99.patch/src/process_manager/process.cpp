#include <unistd.h>
#include <iostream>
#include <QProcess>
#include <QString>
#include <QStringList>

#include "process.h"

namespace process_manager {

Process::Process(process_manager::ProcessConf conf)
    : m_qtProcess(new QProcess(this))
    , m_conf(conf)
    , m_isManualStop(false)
{
    connect(m_qtProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &Process::onProcessFinished);
}

Process::~Process() {
    // nothing to do
}

std::pair<bool, std::string> Process::start() {
    std::string errorMessage;
    if (m_qtProcess->state() != QProcess::ProcessState::NotRunning) {
        errorMessage.clear();
        errorMessage.append(m_conf.name());
        errorMessage.append(" ");
        errorMessage.append("already running.");
        std::cout << errorMessage << std::endl;
        return std::make_pair(false, errorMessage);
    }

    auto commands = m_conf.commands();
    if (commands.empty()) {
        errorMessage.clear();
        errorMessage.append(m_conf.name());
        errorMessage.append(" ");
        errorMessage.append("not command specified to run, please specify it in the configuration file.");
        std::cout << errorMessage << std::endl;
        return std::make_pair(false, errorMessage);
    }
    QString program = QString::fromStdString(commands.front());
    QStringList args;
    auto iter = commands.begin();
    for (iter++; iter != commands.end(); iter++) {
        args << QString::fromStdString(*iter);
    }

    std::cout << m_conf.name() << " starting..." << std::endl;
    m_qtProcess->setProgram(program);
    m_qtProcess->setArguments(args);
    m_qtProcess->start();
    if (!m_qtProcess->waitForStarted(10000)) {
        errorMessage.clear();
        errorMessage.append(m_conf.name());
        errorMessage.append(" ");
        errorMessage.append("start timeout or error occurred.");
        std::cout << errorMessage << "QProcess: " << m_qtProcess->error() << std::endl;
        m_qtProcess->kill();
        return std::make_pair(false,errorMessage);
    }

    m_isManualStop = false;
    time(&m_startTime);
    std::cout << m_conf.name() << " successfully started." << std::endl;
    return std::make_pair(true, "");
}

std::pair<bool, std::string> Process::stop() {
    std::string errorMessage;
    if (m_qtProcess->state() == QProcess::ProcessState::NotRunning) {
        errorMessage.clear();
        errorMessage.append(m_conf.name());
        errorMessage.append(" ");
        errorMessage.append("not running.");
        std::cout << errorMessage << std::endl;
        return std::make_pair(false, errorMessage);
    }

    m_isManualStop = true;
    m_qtProcess->terminate();
    if (!m_qtProcess->waitForFinished(10000))
        m_qtProcess->kill();
    return std::make_pair(true, "");
}

bool Process::isAutostart() {
    return m_conf.autostart();
}

bool Process::isRunning() {
    return m_qtProcess->state() != QProcess::ProcessState::NotRunning;
}

time_t Process::startTime() {
    if (m_qtProcess->state() != QProcess::ProcessState::Running)
        return 0;
    return m_startTime;
}

pid_t Process::pid() {
    return m_qtProcess->processId();
}

void Process::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    std::cout << m_conf.name() << " exit.\n"
              << "\tExitCode: " << exitCode << "\n"
              << "\tExitStatus: " << (exitStatus == QProcess::ExitStatus::NormalExit? "normal exit.": "crash exit.")
              << std::endl;

    if (!m_isManualStop)
        if (m_conf.autorestart())
            start();
}

}
