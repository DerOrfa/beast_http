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

#include "orthancrest.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;

OrthancRest::OrthancRest(std::string host, int port, bool use_ssl, std::string user, const char* passw):Session(host,port,use_ssl,user,passw)
{
}

boost::property_tree::ptree OrthancRest::get_json(std::string target)
{
	boost::property_tree::ptree ret;
	std::istringstream s(get_string(target));
	boost::property_tree::read_json(s,ret);
	return ret;
}
bool OrthancRest::put_json(std::string target, const boost::property_tree::ptree& payload)
{
	std::ostringstream o;
	boost::property_tree::write_json(o,payload);
	return put_string(target,o.str());
}
std::list<std::string> OrthancRest::get_studies(){
	const ptree studies=get_json("/studies");
	std::list<std::string> ret;
	for(auto s:studies){
		ret.push_back(s.second.data());
	}
	return ret;
}
