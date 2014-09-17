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

#ifndef __COLLECTIONS_LIST_H__
#define __COLLECTIONS_LIST_H__
/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "common/object.h"

/* Types ---------------------------------------------------------------------*/
typedef struct
{
	object_t	base;
}	list_t;

typedef struct _list_node_t	list_node_t;

struct _list_node_t
{
	list_t		*owner;
	list_node_t	*prev;
	list_node_t	*next;
};

list_t	*		list_create		(void);
bool			list_push_back		(list_t *this,
						 list_node_t *item);
list_node_t *		list_pop_front		(list_t *this);

#endif
