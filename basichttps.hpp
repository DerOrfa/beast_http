/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2018  Enrico Reimer <reimer@cbs.mpg.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BASICHTTPS_H
#define BASICHTTPS_H

#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <iostream>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

class ConnectionBase {
protected:
	// This buffer is used for reading and must be persisted
	boost::beast::flat_buffer buffer;
	// The io_service is required for all I/O
	static boost::asio::io_service ios;
public:
	virtual http::response<http::dynamic_body> send_request(http::request<http::string_body> req)=0;
	virtual ~ConnectionBase(){}
};

class SSLConnection:public ConnectionBase {
	ssl::stream<tcp::socket> stream;
	static ssl::context &get_ssl_context();
public:
	SSLConnection(std::string host, int port);
	~SSLConnection();
	virtual http::response<http::dynamic_body> send_request(http::request<http::string_body> req)override;
};

class Connection:public ConnectionBase {
	tcp::socket socket;
public:
	Connection(std::string host, int port);
	~Connection();
	virtual http::response<http::dynamic_body> send_request(http::request<http::string_body> req)override;
};

class Session{
	std::string m_auth, m_host;
	std::unique_ptr<ConnectionBase> m_connection;
	bool m_use_ssl;
	int m_port;
	
protected:
	ConnectionBase &get_connection();
	std::string get_token();
	std::string m_token;
public:
	Session(std::string host,int port,bool use_ssl,std::string user,const char *passw=nullptr);
	http::request<http::string_body> make_request(std::string target);
	http::response<http::dynamic_body> request(std::string target);
	http::response<http::dynamic_body> put_request(std::string target, const std::string &payload);
	
	std::string get_string(std::string name);
	bool put_string(std::string name,std::string value);
};

#endif // BASICHTTPS_H
