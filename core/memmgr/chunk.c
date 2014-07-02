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
#include <string.h>

#include "common/common.h"
#include "memmgr/chunk.h"
#include "os/mutex.h"

#include "memmgr_conf.h"

/* Macro definitions ---------------------------------------------------------*/
#define MM_GUARD_PAD		(0x3E)

/* Type definitions ----------------------------------------------------------*/
typedef struct
{
	mm_chunk_t	*first;
	mm_chunk_t	*last;
	uint32_t	count;
} mm_boundary_t;

/* Prototypes ----------------------------------------------------------------*/
static uint32_t		mm_to_aligned_csize		(uint32_t size);
static void		mm_chunk_merge_impl		(mm_chunk_t *this);
static mm_chunk_t *	mm_chunk_split_impl		(mm_chunk_t *this,
							 uint16_t csize);
static mm_chunk_t *	mm_find_first_free_impl		(uint16_t wanted_csize);
static bool		mm_validate_csize_impl		(uint16_t min_csize,
							 uint32_t csize);

/* Variables -----------------------------------------------------------------*/
static mm_boundary_t gs_chunk_boundary = {NULL, NULL};

MOCKABLE mm_find_first_free_f mm_find_first_free = mm_find_first_free_impl;
MOCKABLE mm_chunk_merge_f mm_chunk_merge = mm_chunk_merge_impl;
MOCKABLE mm_chunk_split_f mm_chunk_split = mm_chunk_split_impl;
MOCKABLE mm_validate_csize_f mm_validate_csize = mm_validate_csize_impl;

/* Private Functions definitions ---------------------------------------------*/
static uint32_t mm_to_aligned_csize(uint32_t size)
{
	return ((size + (MM_CFG_ALIGNMENT-1))/MM_CFG_ALIGNMENT);
}

static void mm_chunk_merge_impl(mm_chunk_t *this)
{
	mm_chunk_t *next = mm_chunk_next_get(this);
	if (next == NULL) {
		return;
	}
	if (this->allocated && next->allocated) {
		die("MM: cant merge");
	}

	uint32_t guard_offset = this->guard_offset;
	uint32_t size = this->csize + next->csize;
	if (size > CSIZE_MAX) {
		return;
	}
	gs_chunk_boundary.count --;
	this->csize = size;

	if (next->allocated) {
		this->allocated = true;
		guard_offset = next->guard_offset;
		void *src = mm_toptr(next);
		void *dst = mm_toptr(this);
		memmove(dst, src, guard_offset);
	}

	mm_chunk_guard_set(this, guard_offset);
	this->xorsum = mm_chunk_xorsum(this);

	if (gs_chunk_boundary.last == next) {
		gs_chunk_boundary.last = this;
	} else {
		next = mm_chunk_next_get(this);
		next->prev_size = this->csize;
		next->xorsum = mm_chunk_xorsum(next);
	}
}

static mm_chunk_t *mm_chunk_split_impl(mm_chunk_t *this, uint16_t csize)
{
	mm_chunk_t *next = mm_chunk_next_get(this);

	uint32_t new_size = this->csize - csize;
	if (new_size < mm_min_csize()) {
		return NULL;
	}

	this->csize = csize;
	this->xorsum = mm_chunk_xorsum(this);
	mm_chunk_t *new = mm_compute_next(this, csize);

	mm_chunk_init(new, this, new_size);
	gs_chunk_boundary.count ++;

	if (next != NULL) {
		next->prev_size = new_size;
		next->xorsum = mm_chunk_xorsum(next);
	} else {
		gs_chunk_boundary.last = new;
	}

	return new;
}
static mm_chunk_t *mm_find_first_free_impl(uint16_t wanted_csize)
{
	mm_chunk_t *chnk = gs_chunk_boundary.first;
	mm_chunk_validate(chnk);

	while ((chnk != NULL) &&
	       ((chnk->csize < wanted_csize) || chnk->allocated))
	{
		chnk = mm_chunk_next_get(chnk);
	}

	return chnk;
}

static bool mm_validate_csize_impl(uint16_t min_csize, uint32_t csize)
{
	return (min_csize <= csize) && (csize <= CSIZE_MAX);
}

/* Functions' definitions ----------------------------------------------------*/
void mm_chunk_boundary_set(mm_chunk_t *first, mm_chunk_t *last, uint32_t count)
{
	gs_chunk_boundary.first = first;
	gs_chunk_boundary.last = last;
	gs_chunk_boundary.count = count;
}

void mm_chunk_init(mm_chunk_t *this, mm_chunk_t *prev, uint16_t csize)
{
	this->csize = csize;
	this->allocated = false;
	this->allocator = NULL;
	mm_chunk_guard_set(this, 0);
	if (prev != NULL) {
		this->prev_size = prev->csize;
	} else {
		this->prev_size = 0;
	}
	this->xorsum = mm_chunk_xorsum(this);
}

mm_chunk_t *mm_compute_next(mm_chunk_t *this, uint16_t csize)
{
	return (mm_chunk_t *)(((uintptr_t)this) + (csize * MM_CFG_ALIGNMENT));
}

mm_chunk_t *mm_chunk_next_get(mm_chunk_t *this)
{
	if (this == gs_chunk_boundary.last) {
		return NULL;
	}
	mm_chunk_t *next = mm_compute_next(this, this->csize);
	mm_chunk_validate(next);
	return next;
}

mm_chunk_t *mm_chunk_prev_get(mm_chunk_t *this)
{
	if (this->prev_size == 0) {
		return NULL;
	} else {
		return (mm_chunk_t *)((uintptr_t)this - this->prev_size*MM_CFG_ALIGNMENT);
	}
}
uint32_t mm_guard_size(mm_chunk_t *this)
{
	return ((uint32_t)(this->csize - mm_header_csize()) * MM_CFG_ALIGNMENT) - this->guard_offset;
}

bool mm_chunk_guard_get(mm_chunk_t *this)
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

void mm_chunk_guard_set(mm_chunk_t *this, uint32_t offset)
{
	this->guard_offset = offset;
	uint8_t *ptr = mm_toptr(this) + offset;
	uint32_t size = ((this->csize-mm_header_csize()) * MM_CFG_ALIGNMENT)-offset;
	memset(ptr, MM_GUARD_PAD, size);
}

uint16_t mm_chunk_xorsum(mm_chunk_t *this)
{
	return	this->allocated ^
		this->guard_offset ^
		this->prev_size ^
		this->csize ^
		(((uintptr_t)this) & 0xFFFF) ^
		(((uintptr_t)this->allocator >> 16) & 0xFFFF) ^
		((uintptr_t)this->allocator & 0xFFFF);
}

void mm_chunk_validate(mm_chunk_t *this)
{
	// chunk must :
	// is aligned
	if (((uintptr_t)this % MM_CFG_ALIGNMENT) != 0) {
		die("MM: alignment");
	}
	
	if ((this < gs_chunk_boundary.first) ||
	    (gs_chunk_boundary.last < this)) {
		die("MM: out of bound");
	}

	// have a valid header chksum
	if (mm_chunk_xorsum(this) != this->xorsum) {
		die("MM: xorsum");
	}
	
	// have not overflowed
	if (!mm_chunk_guard_get(this)) {
		die("MM: overflowed");
	}
}

bool mm_chunk_is_available(mm_chunk_t *this)
{
	return (this != NULL) && (!this->allocated);
}

uint16_t mm_chunk_available_csize(mm_chunk_t *this)
{
	if (!mm_chunk_is_available(this)) {
		return 0;
	}
	return this->csize;
}

void *mm_toptr(mm_chunk_t *this)
{
	void *ptr = this;
	return ptr + (sizeof(mm_chunk_t));
}

mm_chunk_t *mm_tochunk(void *ptr)
{
	mm_chunk_t *chunk = ptr - (mm_header_csize()*MM_CFG_ALIGNMENT);
	mm_chunk_validate(chunk);
	return chunk;
}

uint32_t mm_chunk_count(void)
{
	return gs_chunk_boundary.count;
}

void mm_chunk_info(mm_cinfo_t *infos, uint32_t size)
{
	uint32_t cnt = 0;

	mm_chunk_t *chnk = gs_chunk_boundary.first;
	mm_chunk_validate(chnk);

	while ((chnk != NULL) && (cnt < size)) {
		infos[cnt].allocated = chnk->allocated;
		infos[cnt].allocator = chnk->allocator;
		infos[cnt].size = chnk->guard_offset;
		infos[cnt].csize = chnk->csize;

		cnt ++;
		chnk = mm_chunk_next_get(chnk);
	}
}

uint32_t mm_to_csize(uint32_t size)
{
	int32_t wanted_csize = mm_to_aligned_csize(size);
	wanted_csize += mm_header_csize() + MM_CFG_GUARD_SIZE;
	return wanted_csize;
}

uint16_t mm_min_csize(void)
{
	return mm_header_csize() + MM_CFG_MIN_PAYLOAD + MM_CFG_GUARD_SIZE;
}

uint16_t mm_header_csize(void)
{
	return mm_to_aligned_csize(sizeof(mm_chunk_t));
}
