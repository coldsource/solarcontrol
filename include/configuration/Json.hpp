/*
 * This file is part of SolarControl
 *
 * SolarControl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SolarControl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SolarControl. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Thibault Kummer <bob@coldsource.net>
 */

#ifndef __CONFIGURATION_JSON_H__
#define __CONFIGURATION_JSON_H__

#include <nlohmann/json.hpp>

#include <string>

namespace configuration
{

class Json
{
	protected:
		nlohmann::json json;

		void check_entry(const std::string &name, const std::string &type) const;

	public:
		Json() {}
		Json(const nlohmann::json &json): json(json) {}
		Json(const std::string &json_str);

		void Check(const std::string &name, const std::string &type, bool required = true) const;
		bool Has(const std::string &name) const;

		std::string GetString(const std::string &name) const;
		std::string GetString(const std::string &name, const std::string &default_value) const;
		int GetInt(const std::string &name) const;
		int GetInt(const std::string &name, int default_value) const;
		unsigned int GetUInt(const std::string &name) const;
		unsigned int GetUInt(const std::string &name, unsigned int default_value) const;
		double GetFloat(const std::string &name) const;
		double GetFloat(const std::string &name, double default_value) const;
		bool GetBool(const std::string &name) const;
		bool GetBool(const std::string &name, bool default_value) const;
		const Json GetArray(const std::string &name) const;
		const Json GetArray(const std::string &name, const nlohmann::json &default_value) const;
		const Json GetObject(const std::string &name) const;

		void Set(const std::string &name, const std::string &value) { json[name] = value; }
		void Set(const std::string &name, bool value) { json[name] = value; }
		void Set(const std::string &name, int value) { json[name] = value; }
		void Set(const std::string &name, double value) { json[name] = value; }

		const std::string ToString() const { return json.dump(); }
		operator nlohmann::json() const { return json; }
		operator int() const { return (int)json; }

		// Iterator
		struct Iterator
		{
			private:
				const Json *conf_ptr;
				size_t pos;

			public:
				Iterator(const Json *ptr, size_t pos): conf_ptr(ptr), pos(pos) { }

				Iterator& operator++();
				bool operator==(const Iterator& r) const;
				Json operator*() const;
		};

		Iterator begin() const;
		Iterator end() const;
		size_t size() const;
};

}

#endif
