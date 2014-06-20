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

#include "memmgr_conf.h"

/* Macro definitions ---------------------------------------------------------*/
#define MM_TO_CSIZE(size)	((size + (MM_CFG_ALIGNMENT-1))/MM_CFG_ALIGNMENT)
#define MM_GUARD_PAD		(48)

/* Type definitions ----------------------------------------------------------*/
typedef struct
{
	/* previous chunk size. used to find previous chunk in chain */
	uint16_t	prev_size:15;
	bool		allocated:1;
	uint16_t	xorsum;

	uint16_t	size:15;
	uint32_t	guard_offset:17;

	void *		allocator;
} mm_chunk_t;

typedef struct
{
	mm_chunk_t	*first;
	mm_chunk_t	*last;
	uint32_t	counter;
} mm_heap_t;

/* Prototypes ----------------------------------------------------------------*/
static mm_chunk_t *		mm_compute_next		(mm_chunk_t *this,
							 uint16_t csize);
static mm_chunk_t *		mm_chunk_prev_get	(mm_chunk_t *this);
static mm_chunk_t *		mm_chunk_next_get	(mm_chunk_t *this);

static bool			mm_chunk_guard_get	(mm_chunk_t *this);
static void			mm_chunk_guard_set	(mm_chunk_t *this,
							 uint32_t offset);
static mm_chunk_t *		mm_chunk_split		(mm_chunk_t *this,
							 uint16_t csize);
static void			mm_chunk_merge		(mm_chunk_t *this);
static uint16_t			mm_chunk_xorsum		(mm_chunk_t *this);
static void			mm_chunk_delete		(mm_chunk_t *this);

static void *			mm_toptr		(mm_chunk_t *this);
static mm_chunk_t *		mm_tochunk		(void *ptr);

static uint16_t			mm_header_csize		(void);
static uint16_t			mm_min_csize		(void);

static uint32_t			mm_chunk_aggregate		(mm_chunk_t *this,
							 bool dry_run);

static mm_chunk_t *		mm_find_first_free	(uint16_t wanted_size);
static void			mm_chunk_validate	(mm_chunk_t *this);

static int32_t			mm_wanted_size		(uint32_t size);

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
static mm_chunk_t *mm_chunk_next_get(mm_chunk_t *this)
{
	if (this == gs_heap.last) {
		return NULL;
	}
	mm_chunk_t *next = mm_compute_next(this, this->size);
	mm_chunk_validate(next);
	return next;
}

static mm_chunk_t *mm_chunk_prev_get(mm_chunk_t *this)
{
	if (this->prev_size == 0) {
		return NULL;
	} else {
		return (mm_chunk_t *)((uintptr_t)this - this->prev_size*MM_CFG_ALIGNMENT);
	}
}

static uint32_t mm_guard_size(mm_chunk_t *this)
{
	return ((this->size - mm_header_csize()) * MM_CFG_ALIGNMENT) - this->guard_offset;
}

static bool mm_chunk_guard_get(mm_chunk_t *this)
{
	uint8_t *ptr = mm_toptr(this) + this->guard_offset;
	uint32_t size = mm_guard_size(this);
	for (uint32_t i = 0; i < size; i++) {
		if (ptr[i] != MM_GUARD_PAD) {
			return false;
		}
	}
	return true;
}

static void mm_chunk_guard_set(mm_chunk_t *this, uint32_t offset)
{
	this->guard_offset = offset;
	uint8_t *ptr = mm_toptr(this) + offset;
	uint32_t size = ((this->size-mm_header_csize()) * MM_CFG_ALIGNMENT)-offset;
	memset(ptr, MM_GUARD_PAD, size);
}

static void *mm_toptr(mm_chunk_t *this)
{
	void *ptr = this;
	return ptr + (sizeof(mm_chunk_t));
}

static mm_chunk_t *mm_tochunk(void *ptr)
{
	if (ptr == NULL) {
		return NULL;
	}
	mm_chunk_t *chunk = ptr - (mm_header_csize()*MM_CFG_ALIGNMENT);
	mm_chunk_validate(chunk);
	return chunk;
}

static mm_chunk_t *mm_compute_next(mm_chunk_t *this, uint16_t csize)
{
	return (mm_chunk_t *)(((uintptr_t)this) + (csize * MM_CFG_ALIGNMENT));
}

static mm_chunk_t *mm_find_first_free(uint16_t wanted_size)
{
	mm_chunk_t *chnk = gs_heap.first;

	while ((chnk != NULL) &&
	       ((chnk->size < wanted_size) || chnk->allocated))
	{
		chnk = mm_chunk_next_get(chnk);
	}

	return chnk;
}

static void mm_chunk_validate(mm_chunk_t *this)
{
	// chunk must :
	// is aligned
	if (((uintptr_t)this % MM_CFG_ALIGNMENT) != 0) {
		die("MM: not aligned chunk");
	}
	// be in bounds of gs_heap.raw
	if ((this < gs_heap.first) ||
	    (gs_heap.last < this)) {
		die("MM: out of bound");
	}

	// have not overflowed
	if (!mm_chunk_guard_get(this)) {
		die("MM: Overflowed");
	}

	// have a valid header chksum
	if (mm_chunk_xorsum(this) != this->xorsum) {
		die("MM: xorsum");
	}
}

static uint16_t mm_header_csize(void)
{
	return MM_TO_CSIZE(sizeof(mm_chunk_t));
}

static uint16_t mm_min_csize(void)
{
	return mm_header_csize() + MM_CFG_MIN_PAYLOAD + MM_CFG_GUARD_SIZE;
}

static mm_chunk_t *mm_chunk_split(mm_chunk_t *this, uint16_t csize)
{
	mm_chunk_t *next = mm_chunk_next_get(this);

	uint32_t new_size = this->size - csize;
	this->size = csize;
	this->xorsum = mm_chunk_xorsum(this);
	mm_chunk_t *new = mm_compute_next(this, csize);

	new->prev_size = this->size;
	new->size = new_size;
	new->allocated = false;
	new->allocator = 0;
	mm_chunk_guard_set(new, 0);
	new->xorsum = mm_chunk_xorsum(new);

	gs_heap.counter++;

	if (next != NULL) {
		next->prev_size = new_size;
		next->xorsum = mm_chunk_xorsum(next);
		if (!next->allocated &&
		    ((new_size + (uint32_t)next->size) <= UINT15_MAX)) {
			mm_chunk_merge(new);
		}
	}

	if (gs_heap.last == this) {
		gs_heap.last = new;
	}

	return new;
}

static uint16_t mm_chunk_xorsum(mm_chunk_t *this)
{
	return	this->allocated ^
		this->guard_offset ^
		this->prev_size ^
		this->size ^
		(((uintptr_t)this) & 0xFFFF) ^
		(((uintptr_t)this->allocator >> 16) & 0xFFFF) ^
		((uintptr_t)this->allocator & 0xFFFF);
}

static void mm_chunk_delete(mm_chunk_t *this)
{
	this->allocated = false;
	this->allocator = 0;
	mm_chunk_guard_set(this, 0);
	this->xorsum = mm_chunk_xorsum(this);

	mm_chunk_aggregate(this, false);
}

static void mm_chunk_merge(mm_chunk_t *this)
{
	mm_chunk_t *next = mm_chunk_next_get(this);
	mm_chunk_t *next_next = mm_chunk_next_get(next);
	this->size = this->size + next->size;
	mm_chunk_guard_set(this, this->guard_offset);
	this->xorsum = mm_chunk_xorsum(this);

	gs_heap.counter--;

	if (next == gs_heap.last) {
		gs_heap.last = this;
	}

	if (next_next != NULL) {
		next_next->prev_size = this->size;
		next_next->xorsum = mm_chunk_xorsum(next_next);
	}
}

static uint32_t mm_chunk_aggregate(mm_chunk_t *this, bool dry_run)
{
	mm_chunk_t *prev = mm_chunk_prev_get(this);
	mm_chunk_t *next = mm_chunk_next_get(this);
	uint32_t available_on_current = this->size;
	uint32_t tmp = 0;

	if ((next != NULL) && !(next->allocated)) {
		tmp = available_on_current + next->size;
		if (tmp <= UINT15_MAX) {
			available_on_current = tmp;
			if (!dry_run) {
				mm_chunk_merge(this);
			}
		}
	}
	if ((prev != NULL) && !(prev->allocated)) {
		tmp = available_on_current + prev->size;
		if (tmp <= UINT15_MAX) {
			available_on_current = tmp;
			if (!dry_run) {
				mm_chunk_merge(prev);
			}
		}
	}

	return available_on_current;
}

static int32_t mm_wanted_size(uint32_t size)
{
	int32_t wanted_size = (size + (MM_CFG_ALIGNMENT-1)) / MM_CFG_ALIGNMENT;
	wanted_size += mm_header_csize() + MM_CFG_GUARD_SIZE;

	if (wanted_size > UINT15_MAX) {
		return -1;
	}
	return wanted_size;
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

	chnk = mm_find_first_free(wanted_size);
	if (chnk != NULL) {
		if (chnk->size > (wanted_size + mm_min_csize())) {
			mm_chunk_split(chnk, wanted_size);
		}

		chnk->allocated = true;
		mm_chunk_guard_set(chnk, size);
		chnk->allocator = __builtin_return_address(1);
		chnk->xorsum = mm_chunk_xorsum(chnk);
		ptr = mm_toptr(chnk);
	}
	return ptr;
}

static void *mm_zalloc_impl(uint32_t size)
{
	void *ptr = mm_alloc_impl(size);
	if (ptr != NULL) {
		memset(ptr, 0, size);
		mm_allocator_set(ptr);
	}
	return ptr;
}

static void *mm_calloc_impl(uint32_t n, uint32_t size)
{
	void *ptr = mm_zalloc_impl(n * size);
	if (ptr != NULL) {
		mm_allocator_set(ptr);
	}
	return ptr;
}

static void *mm_realloc_impl(void *old_ptr, uint32_t size)
{
	int32_t wanted_size = 0;
	uint32_t available_on_current = 0;
	mm_chunk_t *chnk = mm_tochunk(old_ptr);
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

	available_on_current = mm_chunk_aggregate(chnk, true);

	if (available_on_current >= wanted_size) {
		/* we're ok to merge & shrink this chunk*/
		mm_chunk_aggregate(chnk, false);
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
	return new_ptr;
}

static void mm_free_impl(void *ptr)
{
	mm_chunk_t *chnk = mm_tochunk(ptr);
	if (chnk != NULL) {
		if (!chnk->allocated) {
			die("MM: double free");
		}
		mm_chunk_delete(chnk);
	}
}

/* Functions definitions -----------------------------------------------------*/
void mm_init(void)
{
	mm_chunk_t *chnk = (mm_chunk_t *)gs_raw;
	gs_heap.last = (void *)-1;
	gs_heap.counter = 0;

	mm_chunk_t *prev = NULL;
	uint32_t heap_size = MM_CFG_HEAP_SIZE/MM_CFG_ALIGNMENT;

	gs_heap.first = chnk;
	while (heap_size >= mm_min_csize()) {
		uint16_t size = umin(heap_size, UINT15_MAX);
		heap_size -= size;

		chnk->allocated = false;
		chnk->allocator = 0;
		chnk->size = size;
		if (prev != NULL) {
			chnk->prev_size = prev->size;
		} else {
			chnk->prev_size = 0;
		}
		mm_chunk_guard_set(chnk, 0);
		chnk->xorsum = mm_chunk_xorsum(chnk);

		gs_heap.counter++;

		prev = chnk;
		chnk = mm_compute_next(chnk, chnk->size);
	}
	gs_heap.last = prev;
}

void mm_check(void)
{
	mm_chunk_t *chnk = gs_heap.first;
	mm_chunk_validate(chnk);
	while (chnk != NULL) {
		chnk = mm_chunk_next_get(chnk);
	}
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

	mm_chunk_t *chnk = gs_heap.first;
	mm_chunk_validate(chnk);

	while ((chnk != NULL) && (cnt < size)) {
		stats[cnt].allocated = chnk->allocated;
		stats[cnt].allocator = chnk->allocator;
		stats[cnt].size = chnk->guard_offset;
		stats[cnt].total_csize = chnk->size;

		cnt ++;
		chnk = mm_chunk_next_get(chnk);
	}
}


void mm_allocator_set(void *ptr)
{
	if (ptr != NULL) {
		mm_chunk_t *chnk = mm_tochunk(ptr);
		chnk->allocator = __builtin_return_address(1);
		chnk->xorsum = mm_chunk_xorsum(chnk);
	}
}
