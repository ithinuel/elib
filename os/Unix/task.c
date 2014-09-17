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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "common/common.h"
#include "os/task.h"

/* Types ---------------------------------------------------------------------*/
typedef struct
{
	task_t		base;
	pthread_t	thread;
	task_method_f	routine;
	void *		arg;
	uint32_t	stack_size;
	uint32_t	priority;
	char *		name;
	bool		must_stop;
}	task_internal_t;

/* Prototypes ----------------------------------------------------------------*/
static void *		task_wrapper		(void *arg);
static void		task_delete		(object_t *base);
static char *		task_to_string		(object_t *base);
static void		task_delay_ms_internal	(int32_t ms);

/* Variables -----------------------------------------------------------------*/
static cexcept_ctx_t *gs_ctx = NULL;
static object_ops_t gs_obj_ops = {
	.delete = task_delete,
	.to_string = task_to_string
};
static volatile uint32_t gs_task_running_count = 0;

task_delay_ms_f	task_delay_ms = task_delay_ms_internal;

/* Private functions ---------------------------------------------------------*/
static void *task_wrapper(void *arg)
{
	task_internal_t *t = arg;
	t->routine(t->arg);
	gs_task_running_count--;
	return NULL;
}

static void task_delete(object_t *base)
{
	task_t *this = base_of(base, task_t);
	task_internal_t *self = base_of(this, task_internal_t);
	task_stop(this);
	free(self);
}

static char *task_to_string(object_t *base)
{
	task_t *this = base_of(base, task_t);
	task_internal_t *self = base_of(this, task_internal_t);

	const char *prefix = "task: ";
	uint32_t prefix_len = strlen(prefix);
	uint32_t name_len = strlen(self->name);
	uint32_t total = prefix_len + name_len;

	char * string = malloc(total + 1);
	if (string != NULL) {
		strncpy(string, prefix, prefix_len);
		strncpy(string + prefix_len, self->name, name_len);
		string[total] = '\0';
	}
	return string;
}

static void task_delay_ms_internal(int32_t ms)
{
	usleep(ms * 1000);
}

/* Functions definitions -----------------------------------------------------*/
cexcept_ctx_t *task_cexcept_get_ctx(void)
{
	return gs_ctx;
}

void task_cexcept_set_ctx(cexcept_ctx_t *ctx)
{
	gs_ctx = ctx;
}


task_t *task_create(task_method_f routine, void *arg, uint32_t stack_size,
		    uint32_t priority, char *name)
{

	task_internal_t *self = malloc(sizeof(task_internal_t));
	if (self == NULL) {
		return NULL;
	}

	self->base.base.ops = &gs_obj_ops;
	self->thread = 0;
	self->routine = routine;
	self->arg = arg;
	self->stack_size = stack_size;
	self->priority = priority;
	self->name = name;

	return &self->base;
}

bool task_start(task_t *this)
{
	task_internal_t *self = base_of(this, task_internal_t);
	self->must_stop = false;
	bool running = (pthread_create(&self->thread, NULL, task_wrapper, self) == 0);
	if (running) {
		gs_task_running_count++;
	}
	return running;
}

void task_stop(task_t *this)
{
	task_internal_t *self = base_of(this, task_internal_t);
	if (self->thread != 0) {
		self->must_stop = true;
		pthread_join(self->thread, NULL);
		self->thread = 0;
	}
}

bool task_must_stop(task_t *this)
{
	if (this == NULL) {
		return true;
	}
	task_internal_t *self = base_of(this, task_internal_t);
	return self->must_stop;
}

uint32_t task_running_count(void)
{
	return gs_task_running_count;
}
