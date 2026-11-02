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

#include <mqtt/MessageQueue.hpp>

using namespace std;

namespace mqtt {

void MessageQueue::Push(const st_message &message)
{
	{
		lock_guard<mutex> lock(m_mutex);
		messages.push(message);
	}

	m_condition.notify_one();
}

st_message MessageQueue::Pop()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	m_condition.wait(lock, [this]()
	{
		return !messages.empty();
	});

	st_message message = std::move(messages.front());
	messages.pop();

	return message;
}

}

