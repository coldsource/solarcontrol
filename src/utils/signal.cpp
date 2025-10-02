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

#include <utils/signal.hpp>
#include <thread/SensorsManager.hpp>
#include <thread/DevicesManager.hpp>
#include <thread/Stats.hpp>
#include <device/Devices.hpp>

#include <signal.h>

extern bool startup_finished;

namespace utils {

void set_sighandler(void (*sigh) (int), const std::vector<int> &sigs)
{
	struct sigaction sa;
	sigset_t block_mask;

	sigemptyset(&block_mask);

	for(size_t i=0;i<sigs.size();i++)
		sigaddset(&block_mask, sigs[i]);

	sa.sa_handler = sigh;
	sa.sa_mask = block_mask;
	sa.sa_flags = 0;

	for(size_t i=0;i<sigs.size();i++)
		sigaction(sigs[i],&sa,0);

}

void signal_callback_handler(int signum)
{
	if(!startup_finished)
		return; // Ignore signals while booting

	if(signum==SIGINT || signum==SIGTERM)
	{
		::thread::SensorsManager::GetInstance()->Shutdown();
		::thread::DevicesManager::GetInstance()->Shutdown();
		::thread::Stats::GetInstance()->Shutdown();
	}
	else if(signum==SIGHUP)
	{
		device::Devices().Reload();
	}
}

}
