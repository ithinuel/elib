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
	mm_chunk_t	*last;
} mm_boundary_t;

/* Prototypes ----------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/
static mm_boundary_t gs_chunk_boundary = {NULL};



/* Private Functions definitions ---------------------------------------------*/
/* Functions' definitions ----------------------------------------------------*/
void mm_chunk_boundary_set(mm_chunk_t *last)
{
	gs_chunk_boundary.last = last;
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

	// have a valid header chksum
	if (mm_chunk_xorsum(this) != this->xorsum) {
		die("MM: xorsum");
	}
	
	// have not overflowed
	if (!mm_chunk_guard_get(this)) {
		die("MM: overflowed");
	}
}

void mm_chunk_merge(mm_chunk_t *this)
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
	if (size > UINT15_MAX) {
		return;
	}
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

mm_chunk_t *mm_chunk_split(mm_chunk_t *this, uint16_t csize)
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
	
	if (next != NULL) {
		next->prev_size = new_size;
		next->xorsum = mm_chunk_xorsum(next);
	} else {
		gs_chunk_boundary.last = new;
	}

	return new;
}

void *mm_toptr(mm_chunk_t *this)
{
	void *ptr = this;
	return ptr + (sizeof(mm_chunk_t));
}

uint32_t mm_to_csize(uint32_t size)
{
	return ((size + (MM_CFG_ALIGNMENT-1))/MM_CFG_ALIGNMENT);
}

uint16_t mm_min_csize(void)
{
	return mm_header_csize() + MM_CFG_MIN_PAYLOAD + MM_CFG_GUARD_SIZE;
}

uint16_t mm_header_csize(void)
{
	return mm_to_csize(sizeof(mm_chunk_t));
}

mm_chunk_t *mm_tochunk(void *ptr)
{
	if (ptr == NULL) {
		return NULL;
	}
	mm_chunk_t *chunk = ptr - (mm_header_csize()*MM_CFG_ALIGNMENT);
	mm_chunk_validate(chunk);
	return chunk;
}

