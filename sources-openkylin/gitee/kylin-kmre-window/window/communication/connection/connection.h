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

#ifndef KMRE_UTILS_H_
#define KMRE_UTILS_H_

#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <mutex>

namespace kmre {

template <typename stream_protocol2>
class BaseConnectionWorker {
public:
    virtual void add_connection_and_read_data(std::shared_ptr<boost::asio::basic_stream_socket<stream_protocol2>> const&socket) = 0;
};



template <class Connection>
class ConnectionBox {
public:
    ConnectionBox() {}
    ~ConnectionBox()
    {
        clear();
    }

    void add(std::shared_ptr<Connection> const& connection)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_connections.insert({connection->id(), connection});
    }

    void remove(uint32_t id)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_connections.erase(id);
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_connections.clear();
    }

private:
    ConnectionBox(ConnectionBox const&) = delete;
    ConnectionBox& operator=(ConnectionBox const&) = delete;

    std::mutex m_mutex;
    std::map<int, std::shared_ptr<Connection>> m_connections;
};

}  // namespace kmre

#endif // KMRE_UTILS_H_
