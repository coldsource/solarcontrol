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
{"t_config",
"CREATE TABLE `t_config` ( \
  `config_name` varchar(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL, \
  `config_value` varchar(128) NOT NULL, \
  UNIQUE KEY `config_name` (`config_name`) \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci \
"},
{"t_device",
"CREATE TABLE `t_device` ( \
  `device_id` int(10) NOT NULL AUTO_INCREMENT, \
  `device_type` enum('ht','heater','hws','htmini','timerange','cmv','passive') CHARACTER SET ascii COLLATE ascii_bin NOT NULL, \
  `device_name` varchar(64) NOT NULL, \
  `device_config` mediumtext NOT NULL, \
  PRIMARY KEY (`device_id`) \
) ENGINE=InnoDB AUTO_INCREMENT=64 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci \
"},
{"t_device_state",
"CREATE TABLE `t_device_state` ( \
  `device_id` int(10) NOT NULL, \
  `device_state` mediumtext NOT NULL, \
  PRIMARY KEY (`device_id`) \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci \
"},
{"t_log_energy",
"CREATE TABLE `t_log_energy` ( \
  `log_energy_date` date NOT NULL, \
  `device_id` int(11) NOT NULL, \
  `log_energy_type` enum('consumption','excess','production','offload') CHARACTER SET ascii COLLATE ascii_bin NOT NULL, \
  `log_energy` double NOT NULL, \
  `log_energy_peak` double NOT NULL, \
  `log_energy_offpeak` double NOT NULL, \
  UNIQUE KEY `log_energy_date` (`log_energy_date`,`device_id`,`log_energy_type`) USING BTREE \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci \
"},
{"t_log_energy_detail",
"CREATE TABLE `t_log_energy_detail` ( \
  `log_energy_detail_date` datetime NOT NULL, \
  `device_id` int(10) NOT NULL, \
  `log_energy_detail_type` enum('consumption','excess','production','offload') CHARACTER SET ascii COLLATE ascii_bin NOT NULL, \
  `log_energy` double NOT NULL, \
  `log_energy_peak` double NOT NULL, \
  `log_energy_offpeak` double NOT NULL, \
  UNIQUE KEY `log_energy_date` (`log_energy_detail_date`,`device_id`,`log_energy_detail_type`) USING BTREE \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci \
"},
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
{"t_log_state",
"CREATE TABLE `t_log_state` ( \
  `device_id` int(10) unsigned NOT NULL, \
  `log_state_date` datetime NOT NULL DEFAULT current_timestamp(), \
  `log_state_mode` enum('manual','automatic') NOT NULL, \
  `log_state` tinyint(1) DEFAULT NULL, \
  KEY `log_state_date` (`log_state_date`), \
  KEY `device_id` (`device_id`) \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci \
"},
{"t_storage",
"CREATE TABLE `t_storage` ( \
  `storage_name` varchar(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL, \
  `storage_value` mediumtext NOT NULL, \
  PRIMARY KEY (`storage_name`) \
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci \
"}
};

static auto init = DBConfig::GetInstance()->RegisterTables(solarcontrol_tables);

}
