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

#ifndef CONFLUENCEWEBDAV_H
#define CONFLUENCEWEBDAV_H


#include "basichttps.hpp"
#include <boost/property_tree/ptree_fwd.hpp>
#include <map>

class ConfluenceWEBDav:public Session
{
	std::string m_root;
public:
	ConfluenceWEBDav(std::string host,std::string user,const char *passw=nullptr, std::string root="/plugins/servlet/confluence/default");
	boost::property_tree::ptree get_xml(std::string name,http::verb method=http::verb::get);
	bool put_xml(std::string name, const boost::property_tree::ptree &tree);
	std::map<std::string,std::string> ls(std::string name);
};

#endif // CONFLUENCEWEBDAV_H
