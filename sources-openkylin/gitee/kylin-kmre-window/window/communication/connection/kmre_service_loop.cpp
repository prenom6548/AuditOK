/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <boost/version.hpp>
#include <sys/syslog.h>

#include "kmre_service_loop.h"

namespace {

void boost_service_run_loop(boost::asio::io_service &service)
{
    //异步IO，io_service::run()监听io事件响应，并执行响应回调，即在调用io_service::run()后，io_service返回一个操作结>果，并将其翻译为error_code，传递到事件回调函数中
    while (true) {
        try {
            service.run();//run()和poll()都循环执行I/O对象的事件，区别在于如果事件没有被触发（ready），run()会等待，但是poll()会立即返回。也就是说poll()只会执行已经触发的I/O事件
            break;
        }
        catch (const std::exception &e) {
            syslog(LOG_ERR, "boost_service_run_loop: exception caught (%s).", e.what());
        }
        catch (...) {
            syslog(LOG_ERR, "boost_service_run_loop: Unknown exception caught while executing boost::asio::io_service.");
        }
    }
}

}

namespace kmre {

std::shared_ptr<KmreLoop> KmreLoop::initLoop(std::uint32_t poolSize)
{
    return std::shared_ptr<KmreLoop>(new KmreLoop(poolSize));
}

KmreLoop::KmreLoop(std::uint32_t poolSize)
    : m_poolSize{poolSize}
      
    #if BOOST_VERSION >= 106600
    , m_boostService{static_cast<int>(m_poolSize)}
    #else
    , m_boostService{m_poolSize}
    #endif
{

}

//noexcept说明：　https://blog.csdn.net/qianqin_2014/article/details/51321631
KmreLoop::~KmreLoop() noexcept(true)
{
    try {
        stopLoop();
    }
    catch (...) {
        // Dropping all exceptions to satisfy the nothrow guarantee.
    }
}

void KmreLoop::startLoop()
{
    for (unsigned int i = 0; i < m_poolSize; i++) {
        m_threads.push_back(std::thread{boost_service_run_loop, std::ref(m_boostService)});
    }
}

void KmreLoop::stopLoop()
{
    m_boostService.stop();

    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

boost::asio::io_service& KmreLoop::boostService()
{
    return m_boostService;
}

}  // namespace kmre
