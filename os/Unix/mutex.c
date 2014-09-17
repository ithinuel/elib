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
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "common/common.h"
#include "os/memmgr.h"
#include "os/mutex.h"
#include "utils/cstring.h"

/* Types ---------------------------------------------------------------------*/
typedef struct
{
	mutex_t		base;
	const char	*name;
	pthread_mutex_t	mtx;
} unix_mutex_t;

/* Prototypes ----------------------------------------------------------------*/
static void		mutex_obj_delete		(object_t *self);
static char *		mutex_obj_to_string		(object_t *self);

/* Variables & constants -----------------------------------------------------*/
static const object_ops_t gs_mutex_object_ops = {
		.delete = mutex_obj_delete,
		.to_string = mutex_obj_to_string
};

/* Functions definitions -----------------------------------------------------*/
static void mutex_obj_delete(object_t *self)
{
	unix_mutex_t *this = base_of(base_of(self, mutex_t), unix_mutex_t);
	mm_free(this);
}

static char *mutex_obj_to_string(object_t *self)
{
	unix_mutex_t *this = base_of(base_of(self, mutex_t), unix_mutex_t);
	return cstring_dup(this->name);
}

mutex_t *mutex_new(bool locked, const char *name)
{
	mutex_t *base = NULL;
	unix_mutex_t *this = mm_zalloc(sizeof(unix_mutex_t));
	if (this != NULL) {
		this->base.base.ops = &gs_mutex_object_ops;
		this->name = name;
		base = &(this->base);

		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&this->mtx, &attr);
	}
	return base;
}

bool mutex_lock(mutex_t *self, int32_t ms)
{
	if (self == NULL) {
		die("null mutex");
	}
	unix_mutex_t *this = base_of(self, unix_mutex_t);

	struct timespec t = {0};
	t.tv_sec = ms/1000;
	t.tv_nsec = (ms - 1000*t.tv_sec) * 1000000;

	return (pthread_mutex_timedlock(&this->mtx, &t) == 0);
}

void mutex_unlock(mutex_t *self)
{
	if (self == NULL) {
		die("null mutex");
	}
	unix_mutex_t *this = base_of(self, unix_mutex_t);
	pthread_mutex_unlock(&this->mtx);
}
