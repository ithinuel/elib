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

#ifndef __COMMON_OBJECT_H__
#define __COMMON_OBJECT_H__

/* Public forward declarations -----------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
/* Public types --------------------------------------------------------------*/
typedef struct object	object_t;
typedef void		(*object_delete_f)			(object_t *);
typedef char *		(*object_to_string_f)			(object_t *);
typedef struct object_ops
{
	object_delete_f		delete;
	object_to_string_f	to_string;
}			object_ops_t;

struct object
{
	const object_ops_t	*ops;
};

/* Public macros -------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public prototypes ---------------------------------------------------------*/
/**
 * Delete this object and release its resources.
 * @param self	this object.
 */
void		object_delete				(object_t *self);

/**
 *
 * @param self
 * @return Dynamically allocated
 */
char *		object_to_string			(object_t *self);

#endif

