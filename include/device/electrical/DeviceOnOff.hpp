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

#ifndef __DEVICE_DEVICEONOFF_HPP__
#define __DEVICE_DEVICEONOFF_HPP__

#include <device/electrical/DeviceElectrical.hpp>
#include <datetime/TimespanHistory.hpp>
#include <datetime/Timestamp.hpp>

#include <string>

namespace device {

enum en_wanted_state
{
	ON,
	OFF,
	OFFLOAD,
	UNCHANGED
};

class DeviceOnOff: public DeviceElectrical
{
	protected:
		bool need_update = true; // Force state update on reload
		bool manual_state_changed = false; // Notify we have a change from API

		datetime::Timestamp last_on;
		datetime::Timestamp last_off;

		int prio = 0;
		double expected_consumption;

		datetime::TimespanHistory on_history;

	public:
		DeviceOnOff(unsigned int id, const std::string &name, const configuration::Json &config);
		virtual ~DeviceOnOff();

		static void CheckConfig(const configuration::Json &conf);

		en_category GetCategory() const { return ONOFF; }

		int GetPrio() const { return prio; }

		virtual en_wanted_state GetWantedState() const = 0;
		virtual void SetState(bool new_state);
		virtual void SetManualState(bool new_state);

		bool WasChanged() const { return manual_state_changed; }
		void AckChanged() { manual_state_changed = false; }

		bool NeedStateUpdate() const { return need_update; }
		void UpdateState();

		double GetExpectedConsumption() const;
};

}

#endif

