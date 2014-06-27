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
#include <stdbool.h>
#include <string.h>

#include "common/common.h"
#include "os/memmgr.h"
#include "os/mutex.h"
#include "memmgr/chunk.h"
#include "memmgr_conf.h"

/* Macro definitions ---------------------------------------------------------*/

/* Type definitions ----------------------------------------------------------*/
typedef struct
{
	mm_chunk_t	*first;
	mutex_t		*mtx;
	uint32_t	counter;
} mm_heap_t;

/* Prototypes ----------------------------------------------------------------*/
static mm_chunk_t *		mm_find_first_free	(uint16_t wanted_size);

static int32_t			mm_wanted_size		(uint32_t size);
static void			mm_lock			(void);
static void			mm_unlock		(void);

static void *			mm_alloc_impl		(uint32_t size);
static void *			mm_zalloc_impl		(uint32_t size);
static void *			mm_calloc_impl		(uint32_t n,
							 uint32_t size);
static void *			mm_realloc_impl		(void *old_ptr,
							 uint32_t size);
static void 			mm_free_impl		(void *ptr);

/* Variables -----------------------------------------------------------------*/
static mm_heap_t	gs_heap;
static uint8_t		gs_raw[MM_CFG_HEAP_SIZE] __attribute__((aligned(MM_CFG_ALIGNMENT)));

MOCKABLE mm_alloc_f	mm_alloc = mm_alloc_impl;
MOCKABLE mm_alloc_f	mm_zalloc = mm_zalloc_impl;
MOCKABLE mm_calloc_f	mm_calloc = mm_calloc_impl;
MOCKABLE mm_realloc_f	mm_realloc = mm_realloc_impl;
MOCKABLE mm_free_f	mm_free = mm_free_impl;

/* Private Functions definitions ---------------------------------------------*/
static mm_chunk_t *mm_find_first_free(uint16_t wanted_size)
{
	mm_chunk_t *chnk = gs_heap.first;

	while ((chnk != NULL) &&
	       ((chnk->csize < wanted_size) || chnk->allocated))
	{
		chnk = mm_chunk_next_get(chnk);
	}

	return chnk;
}

static int32_t mm_wanted_size(uint32_t size)
{
	int32_t wanted_size = mm_to_csize(size);
	wanted_size += mm_header_csize() + MM_CFG_GUARD_SIZE;

	if (wanted_size > UINT15_MAX) {
		return -1;
	}
	return wanted_size;
}

static void mm_lock(void)
{
	if (gs_heap.mtx != NULL) {
		mutex_lock(gs_heap.mtx, -1);
	}
}

static void mm_unlock(void)
{
	if (gs_heap.mtx != NULL) {
		mutex_unlock(gs_heap.mtx);
	}
}

static void *mm_alloc_impl(uint32_t size)
{
	int32_t wanted_size = 0;
	void *ptr = NULL;
	mm_chunk_t *chnk = NULL;
	if (size == 0) {
		return NULL;
	}

	wanted_size = mm_wanted_size(size);
	if (wanted_size < 0) {
		return NULL;
	}

	mm_lock();
	chnk = mm_find_first_free(wanted_size);
	if (chnk != NULL) {
		mm_chunk_t *new = mm_chunk_split(chnk, wanted_size);
		if (new != NULL) {
			mm_chunk_t *next = mm_chunk_next_get(new);
			if ((next != NULL) && !next->allocated){
				mm_chunk_merge(new);
			}
		}

		chnk->allocated = true;
		mm_chunk_guard_set(chnk, size);
		chnk->allocator = __builtin_return_address(0);
		chnk->xorsum = mm_chunk_xorsum(chnk);
		ptr = mm_toptr(chnk);
	}
	mm_unlock();
	return ptr;
}

static void *mm_zalloc_impl(uint32_t size)
{
	mm_lock();
	void *ptr = mm_alloc_impl(size);
	if (ptr != NULL) {
		memset(ptr, 0, size);
		mm_allocator_update(ptr);
	}
	mm_unlock();
	return ptr;
}

static void *mm_calloc_impl(uint32_t n, uint32_t size)
{
	mm_lock();
	void *ptr = mm_zalloc_impl(n * size);
	if (ptr != NULL) {
		mm_allocator_update(ptr);
	}
	mm_unlock();
	return ptr;
}

static void *mm_realloc_impl(void *old_ptr, uint32_t size)
{
	int32_t wanted_size = 0;
	uint32_t available_on_current = 0;
	mm_chunk_t *chnk = NULL;
	void *new_ptr = NULL;

	if (size == 0) {
		mm_free_impl(old_ptr);
		return NULL;
	}

	wanted_size = mm_wanted_size(size);
	if (wanted_size < 0) {
		return NULL;
	}

	if (old_ptr == NULL) {
		return mm_alloc_impl(size);
	}

	mm_lock();
	chnk = mm_tochunk(old_ptr);
	available_on_current = 0;//mm_chunk_aggregate(chnk, true);

	if (available_on_current >= wanted_size) {
		/* we're ok to merge & shrink this chunk*/
		//mm_chunk_aggregate(chnk, false);
		mm_chunk_guard_set(chnk, size);
		chnk->xorsum = mm_chunk_xorsum(chnk);

		mm_chunk_split(chnk, wanted_size);
		new_ptr = mm_toptr(chnk);
	} else {
		new_ptr = mm_alloc_impl(size);
		if (new_ptr != NULL) {
			memcpy(new_ptr, old_ptr, umin(chnk->guard_offset, size));
		}
		chnk = mm_tochunk(new_ptr);
		mm_free(old_ptr);
	}

	chnk->allocator = __builtin_return_address(1);
	chnk->xorsum = mm_chunk_xorsum(chnk);
	mm_unlock();
	return new_ptr;
}

static void mm_free_impl(void *ptr)
{
	mm_lock();
	mm_chunk_t *chnk = mm_tochunk(ptr);
	if (chnk != NULL) {
		if (!chnk->allocated) {
			die("MM: double free");
		}
	//	mm_chunk_delete(chnk);
	}
	mm_unlock();
}

/* Functions definitions -----------------------------------------------------*/
void mm_init(void)
{
	mm_chunk_t *chnk = (mm_chunk_t *)gs_raw;
	gs_heap.counter = 0;

	mm_chunk_t *prev = NULL;
	uint32_t heap_size = MM_CFG_HEAP_SIZE/MM_CFG_ALIGNMENT;

	gs_heap.first = chnk;
	while (heap_size >= mm_min_csize()) {
		uint16_t size = umin(heap_size, UINT15_MAX);
		heap_size -= size;

		mm_chunk_init(chnk, prev, size);
		gs_heap.counter++;

		prev = chnk;
		chnk = mm_compute_next(chnk, chnk->csize);
	}
	mm_chunk_boundary_set(prev);
	gs_heap.mtx = mutex_new(false, "memmgr");
}

void mm_check(void)
{
	mm_lock();
	mm_chunk_t *chnk = gs_heap.first;
	mm_chunk_validate(chnk);
	while (chnk != NULL) {
		chnk = mm_chunk_next_get(chnk);
	}
	mm_unlock();
}

uint32_t mm_nb_chunk(void)
{
	return gs_heap.counter;
}

void mm_chunk_info(mm_stats_t *stats, uint32_t size)
{
	uint32_t cnt = 0;

	if (stats == NULL) {
		return;
	}

	mm_lock();
	mm_chunk_t *chnk = gs_heap.first;
	mm_chunk_validate(chnk);

	while ((chnk != NULL) && (cnt < size)) {
		stats[cnt].allocated = chnk->allocated;
		stats[cnt].allocator = chnk->allocator;
		stats[cnt].size = chnk->guard_offset;
		stats[cnt].total_csize = chnk->csize;

		cnt ++;
		chnk = mm_chunk_next_get(chnk);
	}
	mm_unlock();
}


void mm_allocator_set(void *ptr, void *lr)
{
	mm_lock();
	if (ptr != NULL) {
		mm_chunk_t *chnk = mm_tochunk(ptr);
		chnk->allocator = lr;
		chnk->xorsum = mm_chunk_xorsum(chnk);
	}
	mm_unlock();
}
