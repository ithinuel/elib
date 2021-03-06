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

#ifndef __COMMON_COMMON_H__
#define __COMMON_COMMON_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Macros --------------------------------------------------------------------*/
#define		container_of(ptr, type, member) \
	(type *)((uintptr_t)ptr - __builtin_offsetof(type, member))
#define		base_of(ptr, type) \
	container_of(ptr, type, base)

/* Public functions ----------------------------------------------------------*/
/**
 * Returns the lower from a or b.
 */
uint32_t		umin			(uint32_t a,
						 uint32_t b);
char *			bool_to_string		(bool val);
void			die			(const char *reason);

#endif
