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

#include "basichttps.hpp"
#include <boost/beast/version.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/verb.hpp>
#include <regex>
#include <iostream>

#include "root_certificates.hpp"

SSLConnection::SSLConnection(std::string host, int port):stream{ios, get_ssl_context()}{
	// Look up the domain name
	tcp::resolver resolver{ios};
	auto const lookup = resolver.resolve({host, std::to_string(port)});

	// Make the connection on the IP address we get from a lookup
	boost::asio::connect(stream.next_layer(), lookup);

	// Perform the SSL handshake
	stream.handshake(ssl::stream_base::client);
}
SSLConnection::~SSLConnection(){
	// Gracefully close the stream
	boost::system::error_code ec;
	stream.shutdown(ec);
	if(ec){
		if ((ec.category() == boost::asio::error::get_ssl_category()) && (ERR_GET_REASON(ec.value()) == 219)){
// 			std::cout << "Remote peer failed to send a close_notify message." << std::endl;
		} else {
			std::cerr << ec.message() << std::endl;
		}
	}
}
http::response<http::dynamic_body> SSLConnection::send_request(http::request<http::string_body> req)
{
	// Send the HTTP request to the remote host
	http::write(stream, req);

	// Declare a container to hold the response
	http::response<http::dynamic_body> res;

	// Receive the HTTP response
	http::read(stream, buffer, res);
	return res;
}
ssl::context &SSLConnection::get_ssl_context()
{
	static ssl::context ctx{ssl::context::sslv23_client};
	static bool loaded=false;
	// The SSL context is required, and holds certificates
	if(!loaded){
		load_root_certificates(ctx);
		loaded=true;
	}
	return ctx;
}

Connection::Connection(std::string host, int port):socket{ios}
{
	tcp::resolver resolver{ios};
	auto const lookup = resolver.resolve({host, std::to_string(port)});

	// Make the connection on the IP address we get from a lookup
	boost::asio::connect(socket, lookup);
}
Connection::~Connection()
{
	// Gracefully close the socket
	boost::system::error_code ec;
	socket.shutdown(tcp::socket::shutdown_both, ec);

	// not_connected happens sometimes
	// so don't bother reporting it.
	//
	if(ec && ec != boost::system::errc::not_connected)
		throw boost::system::system_error{ec};
}
http::response<http::dynamic_body> Connection::send_request(http::request<http::string_body> req)
{
	// Send the HTTP request to the remote host
	http::write(socket, req);

	// Declare a container to hold the response
	http::response<http::dynamic_body> res;

	// Receive the HTTP response
	http::read(socket, buffer, res);
	return res;
}



Session::Session(std::string host, int port, bool use_ssl, std::string user,const char *passw):m_host(host),m_use_ssl(use_ssl),m_port(port){
	if(!user.empty()){
		if(!passw)
			passw=getpass("Password:");
	
		m_auth = std::string("Basic ")+boost::beast::detail::base64_encode(user+":"+passw);
	}
}

ConnectionBase & Session::get_connection()
{
	if(!m_connection){
		m_connection=m_use_ssl?
			std::unique_ptr<ConnectionBase>(new SSLConnection(m_host,m_port)):
			std::unique_ptr<ConnectionBase>(new Connection(m_host,m_port));
	}
	
	assert(m_connection);
	return *m_connection;
}

http::request<http::string_body> Session::make_request(std::string target,http::verb method)
{
	http::request<http::string_body> req{method, uri_encode(target), 11};
	req.set(http::field::host, m_host);
	req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
	if(!m_auth.empty())
		req.set(http::field::authorization,m_auth);
	if(!m_token.empty())
		req.set(http::field::cookie,m_token);
	return req;
}

http::response<http::dynamic_body> Session::send_request(http::request<http::string_body> req)
{
	auto res=get_connection().send_request(req);

	if(res[http::field::connection].to_string()=="close")//the server asked us to close the connection
		m_connection.reset();
	return res;
}

http::response<http::dynamic_body> Session::request(std::string target,http::verb method)
{
	return send_request(make_request(target,method));
}

http::response<http::dynamic_body> Session::request(std::string target, const std::string &payload, std::string content_type, const http::verb &method)
{
	http::request<http::string_body> req=make_request(uri_encode(target),method);
	req.set(http::field::content_type, content_type);
	req.content_length(payload.size());
	req.body()=payload;
	
	return send_request(req);
}


std::string Session::get_token(){
	//just ask for anything, answer will include the token
	auto res= request("/");

	if(res.find(http::field::set_cookie)!=res.end()){
		std::string cookie= res[http::field::set_cookie].to_string();
		std::smatch result;
		if(std::regex_match(cookie,result,std::regex(".*(JSESSIONID=[0-9a-fA-F]+).*"))){
			std::clog << "Got token " << result[1] << std::endl;
			return result[1];
		}
		// Write the message to standard out
		std::cerr << "Failed to get token. response was:" << std::endl << res << std::endl;
	} 
	return "";
}


std::string Session::get_string(std::string target,http::verb method)
{
	const auto res=request(target,method);
	if(res.result_int()<300){
		return boost::beast::buffers_to_string(res.body().data());
	} else if(res.result()==boost::beast::http::status::not_found){
		std::cerr << target << " was not found" << std::endl;
		std::cerr << res << std::endl;
	} else {
		std::cerr 
			<< "result for request on " << target << " was not \"OK\", but:" << std::endl 
			<< res << std::endl;
	}
	return "";
}
bool Session::put_string(std::string target, std::string value, std::string content_type)
{
	const auto res=request(target,value,content_type,http::verb::put);
	switch(res.result())
	{
		case boost::beast::http::status::ok:
		case boost::beast::http::status::no_content:
			return true;
			break;
		case boost::beast::http::status::not_found:
			std::cerr << target << " was not found";
			break;
		default:
			std::cerr 
				<< "result for request on " << target << " was not ok, but:" << std::endl 
				<< res << std::endl;
			break;
	}
	return false;
}

std::string Session::uri_encode(std::string target){
	std::string new_str = "";
	char c;
	int ic;
	const char* chars = target.c_str();
	char bufHex[10];
	int len = strlen(chars);

	for(int i=0;i<len;i++){
		c = chars[i];
		ic = c;
		/*if (c==' ') new_str += '+';   
		else */if (isalnum(c) || 
			c == '/' || c == '-' || c == '_' || c == '.' || 
			c == '~' || c == '?' || c == '&' || c == '=') new_str += c;
		else {
			sprintf(bufHex,"%X",c);
			if(ic < 16) 
				new_str += "%0"; 
			else
				new_str += "%";
			new_str += bufHex;
		}
	}
	return new_str;
}

boost::asio::io_service ConnectionBase::ios;
