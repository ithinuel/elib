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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "unity_fixture.h"
#include "common/common.h"
#include "memmgr/chunk.h"
#include "tests/tests.h"

#include "memmgr_conf.h"

/* helpers -------------------------------------------------------------------*/
typedef struct
{
	uint16_t size;
	bool	 allocated;
} test_chunk_state_t;

/* functions' prototypes */
static void		prepare_chunk				(test_chunk_state_t *array,
								 uint32_t array_len);
static char *		bool_to_string				(bool val);
static void		eval_chunk_status			(test_chunk_state_t *array,
								 uint32_t array_len);
static void		chunk_allocated_set			(mm_chunk_t *this,
								 bool val);
static uint32_t		chunk_fill_with				(mm_chunk_t *this,
								 char val);
static void		eval_fill_with				(mm_chunk_t *this,
								 char val,
								 uint32_t payload_size);

/* variables */
static uint8_t gs_raw[1024];
static mm_chunk_t *gs_chnk = (mm_chunk_t *)gs_raw;

/* functions' definitions */
static void prepare_chunk(test_chunk_state_t *array, uint32_t array_len)
{
	mm_chunk_t *prev =  NULL;
	mm_chunk_t *chnk = gs_chnk;
	for (uint32_t i = 0; i < array_len; i++)
	{
		uint16_t csize = array[i].size;
		mm_chunk_init(chnk, prev, csize);
		chunk_allocated_set(chnk, array[i].allocated);
		
		prev = chnk;
		chnk = mm_compute_next(chnk, csize);
	}
	mm_chunk_boundary_set(prev);
}

static char *bool_to_string(bool val)
{
	return val?"true":"false";
}

static void eval_chunk_status(test_chunk_state_t *array, uint32_t array_len)
{
	//printf("\n");
	mm_chunk_t *chnk = gs_chnk;
	uint32_t i = 0;
	for (i = 0; (i < array_len) && (chnk != NULL); i++, chnk = mm_chunk_next_get(chnk))
	{
		//printf("%p %5d %5s %5d\n", chnk, chnk->size, bool_to_string(chnk->allocated), chnk->guard_offset);
		TEST_ASSERT_EQUAL_UINT16(array[i].size, chnk->size);
		TEST_ASSERT_EQUAL_STRING(bool_to_string(array[i].allocated), bool_to_string(chnk->allocated));
	}
	TEST_ASSERT_NULL_MESSAGE(chnk, "more chunk than expected");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(array_len, i, "less chunk than expected");
}

static void chunk_allocated_set(mm_chunk_t *this, bool val)
{
	this->allocated = val;
	this->xorsum = mm_chunk_xorsum(this);
}

static uint32_t chunk_fill_with(mm_chunk_t *this, char val)
{
	uint32_t payload_size = mm_guard_size(this) - (MM_CFG_GUARD_SIZE*MM_CFG_ALIGNMENT); 
	
	mm_chunk_guard_set(this, payload_size);
	this->xorsum = mm_chunk_xorsum(this);
	
	char *ptr = mm_toptr(this);
	memset(ptr, val, payload_size);
	
	return payload_size;
}

static void eval_fill_with(mm_chunk_t *this, char val, uint32_t payload_size)
{
	uint8_t *ptr = mm_toptr(this);
	for (uint32_t i = 0; i < payload_size; i++) {
		TEST_ASSERT_EQUAL_UINT8_MESSAGE(val, ptr[i], "Data has beed lost");
	}
}

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(mm_chunk);
TEST_GROUP_RUNNER(mm_chunk)
{
	RUN_TEST_GROUP(mm_chunk_validate);
	
	/*
	 * n : not allocated
	 * . : null
	 * a : allocated
	 *
	 * this + next =>
	 * a + . = a
	 * n + . = n
	 * n + n = n
	 * n + a = a (data moved from a)
	 * a + n = a
	 * a + a = !!die!!
	 */
	RUN_TEST_CASE(mm_chunk, merge_alloc_null);
	RUN_TEST_CASE(mm_chunk, merge_not_alloc_null);
	RUN_TEST_CASE(mm_chunk, merge_both_not_alloc);
	RUN_TEST_CASE(mm_chunk, merge_not_alloc_alloc);
	RUN_TEST_CASE(mm_chunk, merge_alloc_not_alloc);
	RUN_TEST_CASE(mm_chunk, merge_alloc_alloc);
	RUN_TEST_CASE(mm_chunk, split);
	RUN_TEST_CASE(mm_chunk, aggregate);
}
TEST_SETUP(mm_chunk)
{
}
TEST_TEAR_DOWN(mm_chunk)
{
	memset(gs_raw, 0, 1024);
}

/* Tests ---------------------------------------------------------------------*/
TEST(mm_chunk, merge_alloc_null)
{
	test_chunk_state_t a_state[] = {{256, true}};
	prepare_chunk(a_state, 1);
	
	mm_chunk_merge(gs_chnk);
	eval_chunk_status(a_state, 1);
}

TEST(mm_chunk, merge_not_alloc_null)
{
	test_chunk_state_t a_state[] = {{256, false}};
	prepare_chunk(a_state, 1);
	
	mm_chunk_merge(gs_chnk);
	eval_chunk_status(a_state, 1);
}

TEST(mm_chunk, merge_both_not_alloc)
{
	test_chunk_state_t a_state[] = {{128, false}, {128, false}};
	prepare_chunk(a_state, 2);
	
	test_chunk_state_t a_expect[] = {{256, false}};
	mm_chunk_merge(gs_chnk);
	eval_chunk_status(a_expect, 1);
}

TEST(mm_chunk, merge_not_alloc_alloc)
{
	test_chunk_state_t a_state[] = {{32, false}, {128, true}, {96, false}};
	test_chunk_state_t a_expect[] = {{160, true}, {96, false}};
	prepare_chunk(a_state, 3);
	
	mm_chunk_t *second = mm_chunk_next_get(gs_chnk);
	uint32_t payload_size = chunk_fill_with(second, 'A');
	
	mm_chunk_merge(gs_chnk);
	eval_chunk_status(a_expect, 2);
	eval_fill_with(gs_chnk, 'A', payload_size);
}

TEST(mm_chunk, merge_alloc_not_alloc)
{
	test_chunk_state_t a_state[] = {{32, true}, {128, false}, {96, false}};
	test_chunk_state_t a_expect[] = {{160, true}, {96, false}};
	prepare_chunk(a_state, 3);
	
	uint32_t payload_size = chunk_fill_with(gs_chnk, 'A');
	
	mm_chunk_merge(gs_chnk);
	eval_chunk_status(a_expect, 2);
	eval_fill_with(gs_chnk, 'A', payload_size);
}

TEST(mm_chunk, merge_alloc_alloc)
{
	test_chunk_state_t a_state[] = {{32, true}, {128, true}, {96, false}};
	prepare_chunk(a_state, 3);
	
	uint32_t payload_a = chunk_fill_with(gs_chnk, 'A');
	mm_chunk_t *second = mm_chunk_next_get(gs_chnk);
	uint32_t payload_b = chunk_fill_with(second, 'B');
	
	die_Expect("MM: cant merge");
	VERIFY_DIE_START
	mm_chunk_merge(gs_chnk);
	VERIFY_DIE_END
	die_Verify();
	
	eval_chunk_status(a_state, 3);
	eval_fill_with(gs_chnk, 'A', payload_a);
	eval_fill_with(second, 'B', payload_b);
}

TEST(mm_chunk, split)
{
	TEST_FAIL();
}

TEST(mm_chunk, aggregate)
{
	TEST_FAIL();
}

