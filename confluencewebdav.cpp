/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2018  Enrico Reimer <email>
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
 * 
 */

#include "confluencewebdav.hpp"
#include "basichttps.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/beast.hpp>
#include <sstream>

ConfluenceWEBDav::ConfluenceWEBDav(std::string host, std::string user, const char* passw, std::string root):Session(host,true,user,passw),m_root(root)
{
}

std::string ConfluenceWEBDav::get_string(std::string target)
{
	const auto res=request(m_root+"/"+target);
	switch(res.result())
	{
		case boost::beast::http::status::ok:
			return boost::beast::buffers_to_string(res.body().data());
			break;
		case boost::beast::http::status::not_found:
			std::cerr << m_root+"/"+target << " was not found";
			break;
		default:
			std::cerr 
				<< "result for request on " << target << " was not ok, but:" << std::endl 
				<< res << std::endl;
			break;
	}
	return "";
}
bool ConfluenceWEBDav::put_string(std::string target, std::string value)
{
	const auto res=put_request(m_root+"/"+target,value);
	switch(res.result())
	{
		case boost::beast::http::status::ok:
		case boost::beast::http::status::no_content:
			return true;
			break;
		case boost::beast::http::status::not_found:
			std::cerr << m_root+"/"+target << " was not found";
			break;
		default:
			std::cerr 
				<< "result for request on " << target << " was not ok, but:" << std::endl 
				<< res << std::endl;
			break;
	}
	return false;
}


boost::property_tree::ptree ConfluenceWEBDav::get_xml(std::string target){
	
	std::istringstream src(get_string(target));
	boost::property_tree::ptree ret;
	boost::property_tree::read_xml(src,ret);
	return ret;
}

bool ConfluenceWEBDav::put_xml(std::string name, const boost::property_tree::ptree &tree)
{
	std::ostringstream o;
	write_xml(o,tree);

	return put_string(name,o.str());
}
