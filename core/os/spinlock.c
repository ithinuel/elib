/*
	Copyright 2014 Chauveau Wilfried

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		 http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/* Includes ------------------------------------------------------------------*/
#include "os/spinlock.h"
#include "os/task.h"

bool spinlock_lock(spinlock_t *this, int32_t max_delay, int32_t spin_delay)
{
	uint32_t max = max_delay / spin_delay;
	// disable interrupt
	while ((this->is_locked) && (max > 0)) {
		// enable interrupt
		task_delay_ms(spin_delay);
		// disable interrupt
		max--;
	}
	this->is_locked = true;
	// disable interrupt
	return (max > 0);
}

void spinlock_unlock(spinlock_t *this)
{
	this->is_locked = false;
}
