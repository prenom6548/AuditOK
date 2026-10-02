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

#ifndef KMRE_SERVICE_LOOP_H_
#define KMRE_SERVICE_LOOP_H_

#include <boost/asio.hpp>
#include <memory.h>
#include <functional>
#include <thread>

namespace kmre {

class KmreLoop : public std::enable_shared_from_this<KmreLoop> {
public:
    static constexpr const std::uint32_t worker_threads = 1;
    static std::shared_ptr<KmreLoop> initLoop(std::uint32_t pool_size = worker_threads);

    ~KmreLoop() noexcept(true);

    void startLoop();
    void stopLoop();
    boost::asio::io_service& boostService();

private:
    KmreLoop(std::uint32_t poolSize);
    std::uint32_t m_poolSize;
    boost::asio::io_service m_boostService;
    std::vector<std::thread> m_threads;
};

} // namespace kmre

#endif // KMRE_SERVICE_LOOP_H_
