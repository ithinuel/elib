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

/* Features :
 * memory corruption detection.
 * low footprint.
 * allocation traceability.
 */
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "common/common.h"
#include "os/memmgr.h"

#include "memmgr_conf.h"

/* Macro definitions ---------------------------------------------------------*/
/* Type definitions ----------------------------------------------------------*/
typedef struct
{
	uint16_t	prev_size:15;
	bool		allocated:1;
	uint16_t	size:15;
#if MM_CFG_INTEGRITY > 0
	uint16_t	xorsum;
#endif
} mm_chunk_t;

typedef struct
{
	mm_chunk_t	*first_free;
	uint8_t		raw[MM_CFG_HEAP_SIZE] __attribute__((aligned(MM_CFG_ALIGNMENT)));
} mm_heap_t;

/* Prototypes ----------------------------------------------------------------*/
static mm_chunk_t *		mm_next_get		(mm_chunk_t *chunk);
#if MM_CFG_INTEGRITY > 0
static void			mm_check_integrity	(mm_chunk_t *chunk);
#endif
static void *			mm_zalloc_impl		(uint32_t size);
static void 			mm_free_impl		(void *ptr);


/* Variables -----------------------------------------------------------------*/
static mm_heap_t gs_heap;
#if MM_CFG_INTEGRITY > 0
static const uint8_t const gscc_footer[] = "EN";
#endif

MOCKABLE mm_zalloc_f mm_zalloc = mm_zalloc_impl;
MOCKABLE mm_free_f mm_free = mm_free_impl;

/* Private Functions definitions ---------------------------------------------*/
static mm_chunk_t *mm_next_get(mm_chunk_t *chunk)
{
	if (chunk == NULL) {
		return NULL;
	}
	uint8_t *raw_ptr = (uint8_t *)chunk;
	raw_ptr += chunk->size*MM_CFG_ALIGNMENT;
#if MM_CFG_INTEGRITY > 0
	div_t d = div(sizeof(gscc_footer), MM_CFG_ALIGNMENT);
	raw_ptr += d.quot + (d.rem != 0);
	mm_check_integrity((mm_chunk_t *)raw_ptr);
#endif

	return (mm_chunk_t *)raw_ptr;
}

#if MM_CFG_INTEGRITY > 0
static void mm_check_integrity(mm_chunk_t *chunk)
{
	// chunk must :
	// be in bounds of gs_heap.raw
	if ((gs_heap.raw < (uint8_t*)chunk) ||
	    ((uint8_t*)chunk < (gs_heap.raw+MM_CFG_HEAP_SIZE))) {
		die("MM: out of bound");
	}

	// have a valid header chksum
}
#endif

/* Functions definitions -----------------------------------------------------*/
void mm_init(void)
{
	mm_chunk_t *chunk = (mm_chunk_t *)gs_heap.raw;
	gs_heap.first_free = chunk;

	uint32_t prev_size = 0;
	uint32_t heap_size = MM_CFG_HEAP_SIZE/MM_CFG_ALIGNMENT;

	while (heap_size > (sizeof(mm_chunk_t)/MM_CFG_ALIGNMENT)) {
		uint16_t size = umin(heap_size, UINT16_MAX/2);
		heap_size -= size;

		chunk->allocated = false;
		chunk->prev_size = prev_size;
		chunk->size = size;

		prev_size = size;
		chunk = mm_next_get(chunk);
	}
}

void mm_check(void)
{
#if MM_CFG_INTEGRITY > 0
	mm_chunk_t *chunk = (mm_chunk_t *)gs_heap.raw;
	mm_check_integrity(chunk);
	while (chunk != NULL) {
		chunk = mm_next_get(chunk);
	}
#endif
}

static void *mm_zalloc_impl(uint32_t size)
{
	return NULL;
}

void mm_free_impl(void *ptr)
{

}
