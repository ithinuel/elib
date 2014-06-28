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
#include <stddef.h>
#include <string.h>

#include "common/common.h"
#include "unity_fixture.h"
#include "tests/chunk_test_tools.h"
#include "memmgr_conf.h"

/* Variables -----------------------------------------------------------------*/
static uint8_t gs_raw[CHUNK_TEST_HEAP_SIZE] __attribute__((aligned(MM_CFG_ALIGNMENT)));

mm_chunk_t *g_first = (mm_chunk_t *)gs_raw;

/* functions -----------------------------------------------------------------*/
void chunk_test_prepare(chunk_test_state_t *array, uint32_t array_len)
{
	mm_chunk_t *prev =  NULL;
	mm_chunk_t *chnk = g_first;
	for (uint32_t i = 0; i < array_len; i++)
	{
		uint16_t csize = array[i].size;

		mm_chunk_init(chnk, prev, csize);
		chunk_test_allocated_set(chnk, array[i].allocated);

		prev = chnk;
		chnk = mm_compute_next(chnk, csize);
	}
	mm_chunk_boundary_set(g_first, prev, array_len);
}

void chunk_test_verify(chunk_test_state_t *array, uint32_t array_len)
{
	mm_chunk_t *chnk = g_first;
	uint32_t i = 0;

	TEST_ASSERT_EQUAL_UINT32(array_len, mm_chunk_count());

	for (i = 0; (i < array_len) && (chnk != NULL); i++, chnk = mm_chunk_next_get(chnk))
	{
		TEST_ASSERT_EQUAL_UINT16(array[i].size, chnk->csize);
		TEST_ASSERT_EQUAL_STRING(bool_to_string(array[i].allocated), bool_to_string(chnk->allocated));
	}
	TEST_ASSERT_NULL_MESSAGE(chnk, "more chunk than expected");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(array_len, i, "less chunk than expected");
}

void chunk_test_allocated_set(mm_chunk_t *this, bool val)
{
	this->allocated = val;
	this->xorsum = mm_chunk_xorsum(this);
}

uint32_t chunk_test_fill_with_prepare(mm_chunk_t *this, char val)
{
	uint32_t payload_size = mm_guard_size(this) - (MM_CFG_GUARD_SIZE*MM_CFG_ALIGNMENT);

	mm_chunk_guard_set(this, payload_size);
	this->xorsum = mm_chunk_xorsum(this);

	char *ptr = mm_toptr(this);
	memset(ptr, val, payload_size);

	return payload_size;
}

void chunk_test_fill_with_verify(mm_chunk_t *this, char val, uint32_t payload_size)
{
	uint8_t *ptr = mm_toptr(this);
	for (uint32_t i = 0; i < payload_size; i++) {
		TEST_ASSERT_EQUAL_UINT8_MESSAGE(val, ptr[i], "Data has beed lost");
	}
}

void chunk_test_clear(void)
{
	memset(gs_raw, 0, sizeof(gs_raw));
}
