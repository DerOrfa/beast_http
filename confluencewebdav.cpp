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
#include <boost/property_tree/xml_parser.hpp>

ConfluenceWEBDav::ConfluenceWEBDav(std::string host, std::string user, const char* passw, std::string root):Session(host,443,true,user,passw),m_root(root)
{
	m_token = get_token();
}

boost::property_tree::ptree ConfluenceWEBDav::get_xml(std::string target,http::verb method){
	
	std::istringstream src(get_string(m_root+"/"+target,method));
	boost::property_tree::ptree ret;
	boost::property_tree::read_xml(src,ret);
	return ret;
}

bool ConfluenceWEBDav::put_xml(std::string target, const boost::property_tree::ptree &tree)
{
	std::ostringstream o;
	write_xml(o,tree);

	return put_string(m_root+"/"+target,o.str(),"application/octet-stream");
}

std::map<std::string,std::string> ConfluenceWEBDav::ls(std::string name)
{
	auto prop=get_xml(name);
	std::map<std::string,std::string> ret;
	
	for(auto p:prop.get_child("html.body.ul")){
		ret[p.second.get<std::string>("a")]=p.second.get<std::string>("a.<xmlattr>.href");
	}
	return ret;
}
