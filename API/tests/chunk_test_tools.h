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

#ifndef __TESTS_CHUNK_TEST_TOOLS_H__
#define __TESTS_CHUNK_TEST_TOOLS_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include "memmgr/chunk.h"

/* Macros --------------------------------------------------------------------*/
#define CHUNK_TEST_HEAP_SIZE	(1024)

/* Types ---------------------------------------------------------------------*/
typedef struct
{
	uint16_t size;
	bool	 allocated;
} chunk_test_state_t;

/* Variables -----------------------------------------------------------------*/
extern mm_chunk_t *g_first;

/* Prototypes ----------------------------------------------------------------*/
void		chunk_test_prepare			(chunk_test_state_t *array,
							 uint32_t array_len);
void		chunk_test_verify			(chunk_test_state_t *array,
							 uint32_t array_len);
void		chunk_test_allocated_set		(mm_chunk_t *this,
							 bool val);
uint32_t	chunk_test_fill_with_prepare		(mm_chunk_t *this,
							 char val);
void		chunk_test_fill_with_verify		(uint8_t *ptr,
							 char val,
							 uint32_t payload_size);
void		chunk_test_clear			(void);

#endif
