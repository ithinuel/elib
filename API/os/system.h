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

#ifndef __OS_SYSTEM_H__
#define __OS_SYSTEM_H__

#include <stdint.h>

typedef void	(*system_entry_f)	(void);
typedef struct
{
	system_entry_f		entry;
	uint32_t		stack_size;
	uint32_t		priority;
}	system_entry_t;

void		system_boot		(system_entry_t *entry);

#endif
