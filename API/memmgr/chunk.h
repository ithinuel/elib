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

/* Types ---------------------------------------------------------------------*/
typedef struct
{
	/* previous chunk size. used to find previous chunk in chain */
	uint16_t	prev_size:15;
	bool		allocated:1;
	uint16_t	xorsum;

	uint16_t	csize:15;
	uint32_t	guard_offset:17;

	void *		allocator;
} mm_chunk_t;


/* Functions prototypes ------------------------------------------------------*/
void			mm_chunk_boundary_set	(mm_chunk_t *last);
void			mm_chunk_init		(mm_chunk_t *this,
						 mm_chunk_t *prev,
						 uint16_t csize);
mm_chunk_t *		mm_compute_next		(mm_chunk_t *this,
						 uint16_t csize);
mm_chunk_t *		mm_chunk_prev_get	(mm_chunk_t *this);
mm_chunk_t *		mm_chunk_next_get	(mm_chunk_t *this);

uint32_t		mm_guard_size		(mm_chunk_t *this);
bool			mm_chunk_guard_get	(mm_chunk_t *this);
void			mm_chunk_guard_set	(mm_chunk_t *this,
						 uint32_t offset);
uint16_t		mm_chunk_xorsum		(mm_chunk_t *this);
void			mm_chunk_validate	(mm_chunk_t *this);
void			mm_chunk_merge		(mm_chunk_t *this);
mm_chunk_t *		mm_chunk_split		(mm_chunk_t *this,
						 uint16_t csize);

void *			mm_toptr		(mm_chunk_t *this);
mm_chunk_t *		mm_tochunk		(void *ptr);

uint32_t 		mm_to_csize		(uint32_t size);
uint16_t		mm_min_csize		(void);
uint16_t		mm_header_csize		(void);

#endif
