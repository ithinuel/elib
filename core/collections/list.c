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
#include "common/common.h"
#include "collections/list.h"
#include "os/memmgr.h"

/* Types ---------------------------------------------------------------------*/
typedef struct
{
	list_t		base;
	list_node_t	*first;
	list_node_t	*last;
} list_internal_t;

/* Prototypes ----------------------------------------------------------------*/
static char *		list_to_string		(object_t *this);
static void		list_delete		(object_t *this);

/* Variables -----------------------------------------------------------------*/
static const object_ops_t stack_obj_ops = {
	.to_string = list_to_string,
	.delete = list_delete
};


/* Functions definitions -----------------------------------------------------*/
static char *list_to_string(object_t *this)
{
	return NULL;
}
static void list_delete(object_t *this)
{
	mm_free(this);
}

/* Functions definitions -----------------------------------------------------*/
list_t *list_create(void)
{
	list_internal_t *si = mm_zalloc(sizeof(list_internal_t));
	if (si != NULL) {
		si->base.base.ops = &stack_obj_ops;
		return &si->base;
	}
	return NULL;
}


bool list_push_back(list_t *this, list_node_t *item)
{
	bool ret = false;
	if ((item != NULL) && (item->owner == NULL) && (this != NULL)) {
		list_internal_t *self = base_of(this, list_internal_t);
		item->owner = this;

		if (self->first == NULL) {
			self->first = item;
		}
		if (self->last != NULL) {
			item->prev = self->last;
			self->last->next = item;
		}
		self->last = item;

		ret = true;
	}
	return ret;
}

list_node_t *list_pop_front(list_t *this)
{
	list_node_t *node = NULL;
	if (this != NULL) {
		list_internal_t *self = base_of(this, list_internal_t);

		node = self->first;
		if (node != NULL) {
			list_node_t *first = node->next;
			if (first != NULL) {
				first->prev = NULL;
			}
			self->first = first;
			node->next = NULL;
			node->owner = NULL;
		}
	}
	return node;
}
