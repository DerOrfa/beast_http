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

#ifndef ORTHANCREST_H
#define ORTHANCREST_H

#include "basichttps.hpp"
#include <boost/property_tree/ptree_fwd.hpp>
#include <list>

class OrthancRest : public Session
{
public:
    OrthancRest(std::string host,int port,bool use_ssl,std::string user,const char *passw=nullptr);
	boost::property_tree::ptree get_json(std::string target);
	bool put_json(std::string target, const boost::property_tree::ptree &payload);
	std::list<std::string> get_studies();
};

#endif // ORTHANCREST_H
