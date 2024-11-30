/*
 * This file is part of evQueue
 * 
 * evQueue is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * evQueue is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with evQueue. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Author: Thibault Kummer <bob@coldsource.net>
 */

#include <database/DBConfig.hpp>

#include <string>
#include <map>

using namespace std;

namespace database {

map<string,string> solarcontrol_tables = {
{"t_log_ht",
"CREATE TABLE `t_log_ht` ( \
  `log_ht_date` datetime NOT NULL, \
  `device_id` int(10) unsigned NOT NULL, \
  `log_ht_min_h` double NOT NULL, \
  `log_ht_max_h` double NOT NULL, \
  `log_ht_min_t` double NOT NULL, \
  `log_ht_max_t` double NOT NULL, \
  UNIQUE KEY `log_ht_date` (`log_ht_date`,`device_id`) USING BTREE \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci \
"},
{"t_device_state",
"CREATE TABLE `t_device_state` ( \
  `device_id` int(10) unsigned NOT NULL, \
  `device_state` mediumtext NOT NULL, \
  PRIMARY KEY (`device_id`) \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci \
"}
};

static auto init = DBConfig::GetInstance()->RegisterTables(solarcontrol_tables);

}
