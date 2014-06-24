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
#define MM_GUARD_PAD		(48)

/* Type definitions ----------------------------------------------------------*/
/* Prototypes ----------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/

/* Private Functions definitions ---------------------------------------------*/
mm_chunk_t *mm_chunk_next_get(mm_chunk_t *this)
{
	mm_chunk_t *next = mm_compute_next(this, this->size);
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
	return ((uint32_t)(this->size - mm_header_csize()) * MM_CFG_ALIGNMENT) - this->guard_offset;
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
	uint32_t size = ((this->size-mm_header_csize()) * MM_CFG_ALIGNMENT)-offset;
	memset(ptr, MM_GUARD_PAD, size);
}

void *mm_toptr(mm_chunk_t *this)
{
	void *ptr = this;
	return ptr + (sizeof(mm_chunk_t));
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

mm_chunk_t *mm_compute_next(mm_chunk_t *this, uint16_t csize)
{
	return (mm_chunk_t *)(((uintptr_t)this) + (csize * MM_CFG_ALIGNMENT));
}

void mm_chunk_validate(mm_chunk_t *this)
{
	// chunk must :
	// is aligned
	if (((uintptr_t)this % MM_CFG_ALIGNMENT) != 0) {
		die("MM: not aligned chunk");
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

uint16_t mm_header_csize(void)
{
	return mm_to_csize(sizeof(mm_chunk_t));
}

uint16_t mm_min_csize(void)
{
	return mm_header_csize() + MM_CFG_MIN_PAYLOAD + MM_CFG_GUARD_SIZE;
}


uint16_t mm_chunk_xorsum(mm_chunk_t *this)
{
	return	this->allocated ^
		this->guard_offset ^
		this->prev_size ^
		this->size ^
		(((uintptr_t)this) & 0xFFFF) ^
		(((uintptr_t)this->allocator >> 16) & 0xFFFF) ^
		((uintptr_t)this->allocator & 0xFFFF);
}

void mm_chunk_delete(mm_chunk_t *this)
{
	this->allocated = false;
	this->allocator = 0;
	mm_chunk_guard_set(this, 0);
	this->xorsum = mm_chunk_xorsum(this);

	mm_chunk_aggregate(this, false);
}

void mm_chunk_split(mm_chunk_t *this, uint16_t csize)
{
	mm_chunk_t *next = mm_chunk_next_get(this);

	uint32_t new_size = this->size - csize;
	if (new_size < mm_min_csize()) {
		return;
	}

	this->size = csize;
	this->xorsum = mm_chunk_xorsum(this);
	mm_chunk_t *new = mm_compute_next(this, csize);

	new->prev_size = this->size;
	new->size = new_size;
	new->allocated = false;
	new->allocator = 0;
	mm_chunk_guard_set(new, 0);
	new->xorsum = mm_chunk_xorsum(new);

	if (next != NULL) {
		next->prev_size = new_size;
		next->xorsum = mm_chunk_xorsum(next);
		if (!next->allocated &&
		    ((new_size + (uint32_t)next->size) <= UINT15_MAX)) {
			mm_chunk_merge(new);
		}
	}
}

void mm_chunk_merge(mm_chunk_t *this)
{
	mm_chunk_t *next = mm_chunk_next_get(this);
	mm_chunk_t *next_next = mm_chunk_next_get(next);
	this->size = this->size + next->size;
	mm_chunk_guard_set(this, this->guard_offset);
	this->xorsum = mm_chunk_xorsum(this);

	if (next_next != NULL) {
		next_next->prev_size = this->size;
		next_next->xorsum = mm_chunk_xorsum(next_next);
	}
}

uint32_t mm_chunk_aggregate(mm_chunk_t *this, bool dry_run)
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

uint16_t mm_to_csize(uint32_t size)
{
	return ((size + (MM_CFG_ALIGNMENT-1))/MM_CFG_ALIGNMENT);
}

