#include "statemanager.h"

#include "loghelper.h"

#include <QDebug>
#include <tuple>

#define TIMTERINTERVAL 1000

namespace{
constexpr StateTarget mutex_statt_target[] = {
    StateTarget::Self,
    StateTarget::PowerMode,
    StateTarget::PowerType,
    StateTarget::DeviceMode,
    StateTarget::ResourceLimit,
    StateTarget::SceneManager,
    StateTarget::ResourceScheduling,
    StateTarget::AppWhiteList,
    StateTarget::TopAppList,
    StateTarget::StatusMode,
    StateTarget::ScreenMode,
    StateTarget::DPMSMode,
    StateTarget::ProcessInfo,
    StateTarget::AppFocusSpell,
    StateTarget::AppWinChangedSpell,
    StateTarget::SoftFreezeModeEnabled,
    StateTarget::ResourceThreshold,
    StateTarget::BenchmarkStateChanged,
    StateTarget::InputMode};
constexpr size_t mtx_num = sizeof(mutex_statt_target) / sizeof(mutex_statt_target[0]);

std::array<FairMutex, mtx_num> fair_mutex_array;

MultiLockMutex<FairMutex, mtx_num> multi_lock_mutex(fair_mutex_array);
}


StateManager::StateManager()
    : m_stateTarget(StateTarget::None)
{
    initMutex();
    initTimer();
}

StateManager::~StateManager()
{
    PRINT_NOTICE("~StateManager\n");
    m_timer.stop();
    for (auto &conn : m_conns) {
        QObject::disconnect(conn);
        conn = {};
    }
    m_conns.clear();
}

void StateManager::enableTarget(const StateTarget &target, bool isEmit)
{
    std::lock_guard<FairMutex> lock(fairMutex(StateTarget::Self));
    m_stateTarget |= target;
    m_stateTarget |= StateTarget::Self;
    if (isEmit) {
        Q_EMIT notifyTimerStart();
    }
}
void StateManager::disableTarget(const StateTarget &target)
{
    std::lock_guard<FairMutex> lock(fairMutex(StateTarget::Self));
    m_stateTarget &= (~target);
}

bool StateManager::isHaveStateTarget(const StateTarget &target)
{
    return static_cast<bool>(m_stateTarget & target);
}

const StateTarget &StateManager::stateTarget()
{
    return m_stateTarget;
}

FairMutex &StateManager::fairMutex(const StateTarget &target)
{
    return fair_mutex_array[m_mtxMap[target]];
}

void StateManager::multiLockMutexUnlock()
{
    m_stateTarget = StateTarget::None;
    multi_lock_mutex.unlock();
}

void StateManager::handleTimerTimeOut()
{
    multi_lock_mutex.lock();
    Q_EMIT stateTargetChanged();
}

void StateManager::initTimer()
{
    m_timer.setInterval(TIMTERINTERVAL);
    m_timer.setSingleShot(true);
    m_conns.emplace_back(connect(&m_timer, &QTimer::timeout, this, &StateManager::handleTimerTimeOut));
    m_conns.emplace_back(connect(this, &StateManager::notifyTimerStart, &m_timer, QOverload<>::of(&QTimer::start)));
}

void StateManager::initMutex()
{
    for (size_t i = 0; i < mtx_num; ++i){
        m_mtxMap[mutex_statt_target[i]] = i;
    }
}