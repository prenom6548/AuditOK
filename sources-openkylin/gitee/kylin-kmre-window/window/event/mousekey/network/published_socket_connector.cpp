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

#include "published_socket_connector.h"
#include "socket_helper.h"

#include <QDebug>
#include <QThread>
#include <QFile>
#include <sys/stat.h>
#include <syslog.h>

namespace kmre {
namespace network {
PublishedSocketConnector::PublishedSocketConnector(
    const std::string& socket_file, const std::shared_ptr<Runtime>& rt,
    const std::shared_ptr<ConnectionCreator<boost::asio::local::stream_protocol>>& connection_creator)
    : socket_file_(remove_socket_if_stale(socket_file)),
      runtime_(rt),
      connection_creator_(connection_creator),
      acceptor_(rt->service(), socket_file_)//全局只有一个io_service和一个acceptor_(rt->service() is boost::asio::io_service)
{
    if (QFile::exists(QString::fromStdString(socket_file_))) {
        ::chmod(socket_file_.c_str(), 0666);
    }

    setSocketCloexec(acceptor_.native_handle());

    start_accept();//kobe:以 boost套接字的方式循环监控读取数据
}

PublishedSocketConnector::~PublishedSocketConnector() noexcept
{
    
}

void PublishedSocketConnector::start_accept() {
    auto socket = std::make_shared<boost::asio::local::stream_protocol::socket>(runtime_->service());
    acceptor_.async_accept(*socket,
                         [this, socket](boost::system::error_code const& err) {
                           //qDebug() << "PublishedSocketConnector::start_accept async_accept";
                           on_new_connection(socket, err);
                         });

    //async_read  异步读客户端发来的消息             //async_开头的函数都是boost的异步方式
    //server:   async_accept              client:  async_connect异步连接
    //kobe test
    //sock_ptr sock(new socket_type(m_io));
    //m_accacceptor_eptor.async_accept(*sock, boost::bind(&this_type::accept_handler, this, boost::asio::placeholders::error, sock));
}

void PublishedSocketConnector::on_new_connection(std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket, boost::system::error_code const& err) {
    if (!err) {
        connection_creator_->create_connection_for(socket);//读取客户端发送过来的数据
        setSocketCloexec(socket->native_handle());
    }
    if (err.value() == boost::asio::error::operation_aborted) {
        return;
    }

    //if (err.value() == boost::system::errc::success) {
        //qDebug() << "new boost connection success!!!";
    //}

    start_accept();//kobe:发送完毕后继续监听，否则io_service将认为没有事件处理而结束运行(每连接上一个socket都会调用  )
}

}  // namespace network
}  // namespace kmre
