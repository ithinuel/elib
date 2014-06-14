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
#include <stdio.h>
#include <stdlib.h>

#include "common/common.h"
#include "os/memmgr.h"

#include "memmgr_conf.h"

/* Macro definitions ---------------------------------------------------------*/
#define MM_TO_CSIZE(size)	((size + (MM_CFG_ALIGNMENT-1))/MM_CFG_ALIGNMENT)
#define UINT15_MAX		(32767)
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
} mm_chunk_t;

typedef struct
{
	mm_chunk_t	*first_free;
	mm_chunk_t	*last;
	uint8_t		raw[MM_CFG_HEAP_SIZE] __attribute__((aligned(MM_CFG_ALIGNMENT)));
} mm_heap_t;

/* Prototypes ----------------------------------------------------------------*/
static mm_chunk_t *		mm_chunk_prev_get	(mm_chunk_t *this);
static mm_chunk_t *		mm_chunk_next_get	(mm_chunk_t *this);

static bool			mm_guard_get		(mm_chunk_t *this);
static void			mm_chunk_guard_set		(mm_chunk_t *this,
							 uint32_t offset);
static void *			mm_toptr		(mm_chunk_t *this);
static mm_chunk_t *		mm_tochunk		(void *ptr);

static uint16_t			mm_header_csize		(void);
static uint16_t			mm_min_csize	(void);

static mm_chunk_t *		mm_chunk_split		(mm_chunk_t *this,
							 uint16_t csize);
static uint16_t			mm_chunk_xorsum		(mm_chunk_t *this);
static void			mm_chunk_delete		(mm_chunk_t *this);
static bool			mm_chunk_merge		(mm_chunk_t *this);

static mm_chunk_t *		mm_compute_next		(mm_chunk_t *this, uint16_t csize);

static mm_chunk_t *		mm_find_first_free	(uint16_t wanted_size);
static void			mm_chunk_validate	(mm_chunk_t *this);

static void *			mm_zalloc_impl		(uint32_t size);
static void 			mm_free_impl		(void *ptr);

/* Variables -----------------------------------------------------------------*/
static mm_heap_t gs_heap;

MOCKABLE mm_zalloc_f mm_zalloc = mm_zalloc_impl;
MOCKABLE mm_free_f mm_free = mm_free_impl;

/* Private Functions definitions ---------------------------------------------*/
static mm_chunk_t *mm_chunk_next_get(mm_chunk_t *this)
{
	if (this == NULL) {
		return NULL;
	}

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

static bool mm_guard_get(mm_chunk_t *this)
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
	mm_chunk_t *chnk = gs_heap.first_free;

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
	if ((this < (mm_chunk_t *)gs_heap.raw) ||
	    (gs_heap.last < this)) {
		die("MM: out of bound");
	}

	// have not overflowed
	if (!mm_guard_get(this)) {
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

	uint16_t new_size = this->size - csize;
	this->size = csize;
	this->xorsum = mm_chunk_xorsum(this);
	mm_chunk_t *new = mm_compute_next(this, csize);

	new->prev_size = this->size;
	new->size = new_size;
	new->allocated = false;
	mm_chunk_guard_set(new, 0);
	new->xorsum = mm_chunk_xorsum(new);

	if (next != NULL) {
		next->prev_size = new_size;
		next->xorsum = mm_chunk_xorsum(next);
	}

	if (gs_heap.last == this) {
		gs_heap.last = new;
	}

	mm_chunk_merge(new);
	return new;
}

static uint16_t mm_chunk_xorsum(mm_chunk_t *this)
{
	return this->allocated ^ this->guard_offset ^ this->prev_size ^ this->size ^ (((uintptr_t)this) & 0xFFFF);
}

static void mm_chunk_delete(mm_chunk_t *this)
{
	this->allocated = false;
	mm_chunk_guard_set(this, 0);
	this->xorsum = mm_chunk_xorsum(this);

	mm_chunk_merge(this);
	mm_chunk_t *prev = mm_chunk_prev_get(this);
	if (prev != NULL) {
		mm_chunk_merge(prev);
	}
}

static bool mm_chunk_merge(mm_chunk_t *this)
{
	mm_chunk_t *next = mm_chunk_next_get(this);
	if (next == NULL) {
		return false;
	}
	if (this->allocated || next->allocated) {
		return false;
	}

	uint32_t new_size = (uint32_t)this->size + (uint32_t) next->size;
	if (new_size > UINT15_MAX) {
		return false;
	}

	mm_chunk_t *new_next = mm_chunk_next_get(next);
	this->size = this->size + next->size;
	mm_chunk_guard_set(this, 0);
	this->xorsum = mm_chunk_xorsum(this);

	if (next == gs_heap.last) {
		gs_heap.last = this;
	}

	if (new_next != NULL) {
		new_next->prev_size = this->size;
		new_next->xorsum = mm_chunk_xorsum(new_next);
	}
	return true;
}

/* Functions definitions -----------------------------------------------------*/
void mm_init(void)
{
	mm_chunk_t *chnk = (mm_chunk_t *)gs_heap.raw;
	gs_heap.first_free = chnk;
	gs_heap.last = (void *)-1;

	mm_chunk_t *prev = NULL;
	uint32_t heap_size = MM_CFG_HEAP_SIZE/MM_CFG_ALIGNMENT;

	while (heap_size >= mm_min_csize()) {
		uint16_t size = umin(heap_size, UINT15_MAX);
		heap_size -= size;

		chnk->allocated = false;
		chnk->size = size;
		if (prev != NULL) {
			chnk->prev_size = prev->size;
		} else {
			chnk->prev_size = 0;
		}
		mm_chunk_guard_set(chnk, 0);
		chnk->xorsum = mm_chunk_xorsum(chnk);

		prev = chnk;
		chnk = mm_compute_next(chnk, chnk->size);
	}
	gs_heap.last = prev;
}

void mm_print(void)
{
	uint32_t sum = 0;
	printf("\n--------|-----|------|-----|------|------\n");
	mm_chunk_t *this = (mm_chunk_t *)gs_heap.raw;
	mm_chunk_validate(this);
	while (this != NULL) {
		sum += this->size;
		printf("%8p|%5d|%6d|%5s|%#.4x|%d\n",
				(void *)this,
				this->size,
				this->guard_offset,
				this->allocated?"true":"false",
				this->xorsum,
				sum);
		this = mm_chunk_next_get(this);
	}
}

void mm_check(void)
{
	mm_chunk_t *chunk = (mm_chunk_t *)gs_heap.raw;
	mm_chunk_validate(chunk);
	while (chunk != NULL) {
		chunk = mm_chunk_next_get(chunk);
	}
}

/*
 * algorithme approximatif
fonction zalloc(size: entier): pointeur
	taille_voulue: entier
	chnk: chunk
	ptr: pointeur
début
	taille_voulue <- size + header_size + guard_size

	// trouver un chunk adéquat
	chnk <- chercher_le_premier_libre_de_taille_suffisante(taille_voulue);
	si chnk <> NULL alors
		si mm_size(chnk) > (taille_voulue + taille_mini_chunk) alors
			split(chnk)
		sinon
			chercher_le_prochain_libre()
		finsi

		initialiser_guard(chnk, size)
		ptr <- mm_toptr(chnk)
		memset(ptr, 0, size)
		retourner ptr
	sinon
		retourner NULL
	finsi
fin
 */
static void *mm_zalloc_impl(uint32_t size)
{
	uint32_t wanted_size = 0;
	void *ptr = NULL;
	mm_chunk_t *chnk = NULL;
	if (size == 0) {
		return NULL;
	}

	wanted_size = (size + (MM_CFG_ALIGNMENT-1)) / MM_CFG_ALIGNMENT;
	wanted_size += mm_header_csize() + MM_CFG_GUARD_SIZE;

	chnk = mm_find_first_free(wanted_size);
	if (chnk != NULL) {
		if (chnk->size > (wanted_size + mm_min_csize())) {
			mm_chunk_split(chnk, wanted_size);
		}

		chnk->allocated = true;
		mm_chunk_guard_set(chnk, size);
		chnk->xorsum = mm_chunk_xorsum(chnk);
		ptr = mm_toptr(chnk);
		memset(ptr, 0, size);
	}
	mm_print();
	return ptr;
}

static void mm_free_impl(void *ptr)
{
	mm_chunk_t *chnk = mm_tochunk(ptr);
	if (!chnk->allocated) {
		die("MM: double free");
	}
	mm_chunk_delete(chnk);
	mm_print();
}
