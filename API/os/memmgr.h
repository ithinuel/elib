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

#ifndef __OS_MEMMGR_H__
#define __OS_MEMMGR_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include "common/mockable.h"

/* macros --------------------------------------------------------------------*/
#define mm_allocator_update(this)	mm_allocator_set(this, __builtin_return_address(0))

/* Public types --------------------------------------------------------------*/
typedef void *		(* mm_alloc_f)			(uint32_t total_csize);
typedef void *		(* mm_calloc_f)			(uint32_t n,
							 uint32_t total_csize);
typedef void *		(* mm_realloc_f)		(void *old_ptr,
							 uint32_t total_csize);
typedef void		(* mm_free_f)			(void *ptr);


/* Public functions ----------------------------------------------------------*/
/**
 * Initialise the memory manager.
 * @param	heap	Heap buffer.
 *			Must be aligned according to MM_CFG_ALIGNMENT;
 * @param	size	Heap buffer size in byte.
 */
void			mm_init				(uint8_t *heap,
							 uint32_t size);
/**
 * Check heap integrity.
 */
void			mm_check			(void);
/**
 * Gives chunk number.
 * @return Integer.
 */
void			mm_allocator_set		(void *ptr,
							 void *lr);

MOCKABLE mm_alloc_f	mm_alloc;
MOCKABLE mm_alloc_f	mm_zalloc;
MOCKABLE mm_calloc_f	mm_calloc;
MOCKABLE mm_realloc_f	mm_realloc;
MOCKABLE mm_free_f	mm_free;

#endif
