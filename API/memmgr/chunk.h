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

#ifndef __MEMMGR_CHUNK_H__
#define __MEMMGR_CHUNK_H__

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>

/* Macro definitions ---------------------------------------------------------*/
#define UINT15_MAX		(32767)
#define MM_GUARD_PAD		(48)

/* Types ---------------------------------------------------------------------*/
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


/* Functions prototypes ------------------------------------------------------*/
mm_chunk_t *		mm_compute_next		(mm_chunk_t *this,
						 uint16_t csize);
mm_chunk_t *		mm_chunk_prev_get	(mm_chunk_t *this);
mm_chunk_t *		mm_chunk_next_get	(mm_chunk_t *this);

bool			mm_chunk_guard_get	(mm_chunk_t *this);
void			mm_chunk_guard_set	(mm_chunk_t *this,
						 uint32_t offset);
void			mm_chunk_split		(mm_chunk_t *this,
						 uint16_t csize);
void			mm_chunk_merge		(mm_chunk_t *this);
uint16_t		mm_chunk_xorsum		(mm_chunk_t *this);
void			mm_chunk_delete		(mm_chunk_t *this);

void *			mm_toptr		(mm_chunk_t *this);
mm_chunk_t *		mm_tochunk		(void *ptr);

uint16_t		mm_header_csize		(void);
uint16_t		mm_min_csize		(void);

uint32_t		mm_chunk_aggregate	(mm_chunk_t *this,
						 bool dry_run);
void			mm_chunk_validate	(mm_chunk_t *this);

uint16_t 		mm_to_csize		(uint32_t size);

#endif
