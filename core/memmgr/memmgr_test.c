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
#include "tests/tests.h"
#include "memmgr/chunk.h"
#include "os/memmgr.h"
#include "memmgr_conf.h"

/* helper functions ----------------------------------------------------------*/
typedef struct
{
	uint16_t size;
	bool	 allocated;
} test_mem_state_t;


/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr);

TEST_GROUP_RUNNER(memmgr)
{
	RUN_TEST_CASE(memmgr, _alloc);
	RUN_TEST_CASE(memmgr, _zalloc);
	RUN_TEST_CASE(memmgr, _calloc);
	RUN_TEST_CASE(memmgr, _free);
	
	RUN_TEST_CASE(memmgr, alloc_zero);
	RUN_TEST_CASE(memmgr, alloc_no_available_chunk);
	RUN_TEST_CASE(memmgr, alloc_far_too_big);
	RUN_TEST_CASE(memmgr, alloc_cant_split);
	RUN_TEST_CASE(memmgr, alloc_cant_merge_allocated);
	RUN_TEST_CASE(memmgr, alloc_cant_merge_last);
	
	RUN_TEST_CASE(memmgr, free_cant_refree);
}

TEST_SETUP(memmgr)
{
	mm_init(gs_raw_small, 1024);
}

TEST_TEAR_DOWN(memmgr)
{
	mm_check();
}

/* Tests ---------------------------------------------------------------------*/
TEST(memmgr, _alloc)
{
	TEST_ASSERT_NOT_NULL(mm_alloc(12));
}

TEST(memmgr, _zalloc)
{
	
	uint8_t a_zero[12];
	memset(a_zero, 0, 12);
	
	uint8_t *ptr = mm_zalloc(12);
	TEST_ASSERT_EQUAL_MEMORY(a_zero, ptr, 12);
}

TEST(memmgr, _calloc)
{
	uint32_t a_zero[12];
	memset(a_zero, 0, 12*sizeof(uint32_t));
	
	uint8_t *ptr = mm_calloc(12, sizeof(uint32_t));
	TEST_ASSERT_EQUAL_MEMORY(a_zero, ptr, 12*sizeof(uint32_t));
}

TEST(memmgr, _free)
{
	uint8_t *ptr = mm_alloc(12);
	memset(ptr, 'A', 12);
	mm_free(ptr);
}
	
TEST(memmgr, alloc_zero)
{
	TEST_ASSERT_NULL(mm_alloc(0));
}

TEST(memmgr, alloc_no_available_chunk)
{
	mm_alloc(512);
	TEST_ASSERT_NULL(mm_alloc(784));
}

TEST(memmgr, alloc_far_too_big)
{
	TEST_ASSERT_NULL(mm_alloc(2*1024));
}

TEST(memmgr, alloc_cant_split)
{
	TEST_FAIL();
}

TEST(memmgr, alloc_cant_merge_allocated)
{
	TEST_FAIL();
}

TEST(memmgr, alloc_cant_merge_last)
{
	TEST_FAIL();
}

TEST(memmgr, free_cant_refree)
{
	TEST_FAIL();
}

