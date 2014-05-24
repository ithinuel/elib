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
#include <stdint.h>
#include <stddef.h>

#include "os/memmgr.h"

#include "memmgr_conf.h"

/* Type definitions ----------------------------------------------------------*/
typedef struct
{
	void	*first_free;
	uint8_t	raw[MM_CFG_HEAP_SIZE] __attribute__((aligned(MM_CFG_ALIGNMENT)));;
} mm_heap_t;

/* Prototypes ----------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/
static mm_heap_t gs_heap = {
};

/* Functions definitions -----------------------------------------------------*/
void mm_init(void)
{
}

void *mm_alloc(uint32_t size)
{
	return NULL;
}

