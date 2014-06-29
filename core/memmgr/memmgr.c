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
	uint8_t		*heap;
	mutex_t		*mtx;
} mm_heap_t;

/* Prototypes ----------------------------------------------------------------*/
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
static mm_heap_t	gs_memmgr = {NULL};

MOCKABLE mm_alloc_f	mm_alloc = mm_alloc_impl;
MOCKABLE mm_alloc_f	mm_zalloc = mm_zalloc_impl;
MOCKABLE mm_calloc_f	mm_calloc = mm_calloc_impl;
MOCKABLE mm_realloc_f	mm_realloc = mm_realloc_impl;
MOCKABLE mm_free_f	mm_free = mm_free_impl;

/* Private Functions definitions ---------------------------------------------*/
static void mm_lock(void)
{
	if (gs_memmgr.mtx != NULL) {
		mutex_lock(gs_memmgr.mtx, -1);
	}
}

static void mm_unlock(void)
{
	if (gs_memmgr.mtx != NULL) {
		mutex_unlock(gs_memmgr.mtx);
	}
}

static void *mm_alloc_impl(uint32_t size)
{
	int32_t wanted_csize = 0;
	void *ptr = NULL;
	mm_chunk_t *chnk = NULL;
	if (size == 0) {
		return NULL;
	}

	wanted_csize = mm_to_csize(size);
	if (wanted_csize < 0) {
		return NULL;
	}

	mm_lock();
	chnk = mm_find_first_free(wanted_csize);
	if (chnk != NULL) {
		mm_chunk_t *new = mm_chunk_split(chnk, wanted_csize);
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
	void *ptr = mm_alloc(size);
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
	void *ptr = mm_zalloc(n * size);
	if (ptr != NULL) {
		mm_allocator_update(ptr);
	}
	mm_unlock();
	return ptr;
}

static void *mm_realloc_impl(void *old_ptr, uint32_t size)
{
	int32_t wanted_csize = 0;
	mm_chunk_t *chnk = NULL;
	void *new_ptr = NULL;

	if (size == 0) {
		if (old_ptr != NULL) {
			mm_free_impl(old_ptr);
		}
		return NULL;
	}

	wanted_csize = mm_to_csize(size);
	if (wanted_csize < 0) {
		return NULL;
	}
	mm_lock();
	if (old_ptr == NULL) {
		new_ptr = mm_alloc_impl(size);
		chnk = mm_tochunk(new_ptr);

		chnk->allocator = __builtin_return_address(1);
		chnk->xorsum = mm_chunk_xorsum(chnk);
		mm_unlock();
		return new_ptr;
	}

	chnk = mm_tochunk(old_ptr);

	if (wanted_csize < chnk->csize) {
		mm_chunk_split(chnk, wanted_csize);
	} else if (wanted_csize > chnk->csize) {
		new_ptr = mm_alloc_impl(size);
		if (new_ptr != NULL) {
			memcpy(new_ptr, old_ptr, umin(chnk->guard_offset, size));
			chnk = mm_tochunk(new_ptr);
			mm_free_impl(old_ptr);
		}
	} else {
		new_ptr = old_ptr;
	}

	if (new_ptr != NULL) {
		chnk->allocator = __builtin_return_address(1);
		chnk->xorsum = mm_chunk_xorsum(chnk);
	}

	mm_unlock();
	return new_ptr;
}

static void mm_free_impl(void *ptr)
{
	mm_lock();
	if (ptr != NULL) {
		mm_chunk_t *chnk = mm_tochunk(ptr);
		if (!chnk->allocated) {
			die("MM: double free");
		}

		chnk->allocated = false;
		chnk->allocator = NULL;
		chnk->xorsum = mm_chunk_xorsum(chnk);

		mm_chunk_t *sibbling = mm_chunk_next_get(chnk);
		if ((sibbling != NULL) && !sibbling->allocated) {
			mm_chunk_merge(chnk);
		}
		sibbling = mm_chunk_prev_get(chnk);
		if ((sibbling != NULL) && !sibbling->allocated) {
			mm_chunk_merge(sibbling);
		}
	}
	mm_unlock();
}

/* Functions definitions -----------------------------------------------------*/
void mm_init(uint8_t *heap, uint32_t size)
{
	gs_memmgr.heap = heap;
	mm_chunk_t *chnk = (mm_chunk_t *)heap;

	uint32_t count = 0;
	mm_chunk_t *first = chnk;
	mm_chunk_t *prev = NULL;
	uint32_t heap_size = size/MM_CFG_ALIGNMENT;

	while (heap_size >= mm_min_csize()) {
		uint16_t size = umin(heap_size, UINT15_MAX);
		heap_size -= size;

		mm_chunk_init(chnk, prev, size);
		count++;

		prev = chnk;
		chnk = mm_compute_next(chnk, chnk->csize);
	}
	mm_chunk_boundary_set(first, prev, count);
	gs_memmgr.mtx = mutex_new(false, "memmgr");
}

void mm_check(void)
{
	mm_lock();
	mm_chunk_t *chnk = (mm_chunk_t *)gs_memmgr.heap;
	mm_chunk_validate(chnk);
	while (chnk != NULL) {
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

//mm_info_t *mm_info_get(void)
//{
//	mm_lock();
//	mm_info_t *infos = mm_calloc(mm_chunk_count() + 2, sizeof(mm_info_t));
//	if (infos != NULL) {
//		mm_info_t *it = infos;
//		mm_chunk_t *chnk = (mm_chunk_t *)gs_memmgr.heap;
//		mm_chunk_validate(chnk);
//		while (chnk != NULL) {
//			it->allocated = chnk->allocated;
//			it->allocator = chnk->allocator;
//			it->csize = chnk->csize;
//			it->size = chnk->guard_offset;
//
//			it++;
//			chnk = mm_chunk_next_get(chnk);
//		}
//	}
//	mm_unlock();
//	return infos;
//}

