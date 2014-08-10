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
#include <unistd.h>
#include <pthread.h>
#include "os/system.h"
#include "os/task.h"

/* function's prototypes -----------------------------------------------------*/
static void system_entry_wrapper(void *v)
{
	system_entry_t *e = v;
	if (e->entry != NULL) {
		e->entry();
	}
}

/* function's definitions ----------------------------------------------------*/
void system_boot(system_entry_t *entry)
{
	task_t *t = task_create(system_entry_wrapper, entry, 512, 1, "entry");
	if (t != NULL) {
		task_start(t);
	}
	while (task_running_count()) {
		sleep(1);
	}
}
