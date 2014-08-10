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
#include "common/common.h"
#include "os/task.h"

/* Types ---------------------------------------------------------------------*/
typedef struct
{
	task_t		base;
	pthread_t	thread;
	task_start_f	routine;
	void *		arg;
	uint32_t	stack_size;
	uint32_t	priority;
	char *		name;
}	task_internal_t;

/* Prototypes ----------------------------------------------------------------*/
static void *		task_wrapper		(void *arg);
static void		task_delete		(object_t *base);
static char *		task_to_string		(object_t *base);

/* Variables -----------------------------------------------------------------*/
static cexcept_ctx_t *gs_ctx = NULL;
static object_ops_t gs_obj_ops = {
	.delete = task_delete,
	.to_string = task_to_string
};
static volatile uint32_t gs_task_running_count = 0;

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
	pthread_cancel(self->thread);
	free(self);
}

static char *task_to_string(object_t *base)
{
	task_t *this = base_of(base, task_t);
	task_internal_t *self = base_of(this, task_internal_t);
	char * string = malloc(7 + strlen(self->name) + 1);
	if (string != NULL) {
		strcpy(string, "task : ");
		strcpy(string, self->name);
		string[7 + strlen(self->name)] = '\0';
	}
	return string;
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


task_t *task_create(task_start_f routine, void *arg, uint32_t stack_size,
		    uint32_t priority, char *name)
{

	task_internal_t *self = malloc(sizeof(task_internal_t));
	if (self == NULL) {
		return NULL;
	}

	self->base.base.ops = &gs_obj_ops;
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
	bool running = (pthread_create(&self->thread, NULL, task_wrapper, self) == 0);
	if (running) {
		gs_task_running_count++;
	}
	return running;
}

uint32_t task_running_count(void)
{
	return gs_task_running_count;
}
