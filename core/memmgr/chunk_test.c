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
#include "os/memmgr.h"
#include "tests/common_mock.h"
#include "tests/memmgr_mock.h"
#include "tests/chunk_test_tools.h"

#include "memmgr_conf.h"

/* helpers -------------------------------------------------------------------*/

/* functions' prototypes */
void *			unity_malloc				(size_t size);
void			unity_free				(void *ptr);

/* variables */

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(mm_chunk);
TEST_GROUP_RUNNER(mm_chunk)
{
	RUN_TEST_GROUP(mm_chunk_validate);
	
	RUN_TEST_CASE(mm_chunk, next_get);
	RUN_TEST_CASE(mm_chunk, prev_get);

	RUN_TEST_CASE(mm_chunk, merge_alloc_null);
	RUN_TEST_CASE(mm_chunk, merge_nalloc_null);
	RUN_TEST_CASE(mm_chunk, merge_both_nalloc);
	RUN_TEST_CASE(mm_chunk, merge_nalloc_alloc);
	RUN_TEST_CASE(mm_chunk, merge_alloc_nalloc);
	RUN_TEST_CASE(mm_chunk, merge_alloc_alloc);
	RUN_TEST_CASE(mm_chunk, merge_bigblocks);
	
	RUN_TEST_CASE(mm_chunk, split);
	RUN_TEST_CASE(mm_chunk, split_too_small);
	
	RUN_TEST_CASE(mm_chunk, find_first_free);
	
	RUN_TEST_CASE(mm_chunk, info);

	RUN_TEST_CASE(mm_chunk, valid_between_included_wanted_csize_and_csize_max);
	RUN_TEST_CASE(mm_chunk, when_not_available_then_it_should_return_0);
}
TEST_SETUP(mm_chunk)
{
}
TEST_TEAR_DOWN(mm_chunk)
{
	chunk_test_clear();
}

/* Tests ---------------------------------------------------------------------*/
TEST(mm_chunk, next_get)
{
	chunk_test_state_t a_state[] = {{128, true}, {128, true}};
	chunk_test_prepare(a_state, 2);

	mm_chunk_t *second = mm_compute_next(g_first, g_first->csize);

	TEST_ASSERT_EQUAL_PTR(second, mm_chunk_next_get(g_first));
	TEST_ASSERT_NULL(mm_chunk_next_get(second));
}

TEST(mm_chunk, prev_get)
{
	chunk_test_state_t a_state[] = {{128, true}, {128, true}};
	chunk_test_prepare(a_state, 2);

	mm_chunk_t *second = mm_compute_next(g_first, g_first->csize);

	TEST_ASSERT_EQUAL_PTR(g_first, mm_chunk_prev_get(second));
	TEST_ASSERT_NULL(mm_chunk_prev_get(g_first));
}

TEST(mm_chunk, merge_alloc_null)
{
	chunk_test_state_t a_state[] = {{256, true}};
	chunk_test_prepare(a_state, 1);
	
	mm_chunk_merge(g_first);
	chunk_test_verify(a_state, 1);
}

TEST(mm_chunk, merge_nalloc_null)
{
	chunk_test_state_t a_state[] = {{256, false}};
	chunk_test_prepare(a_state, 1);
	
	mm_chunk_merge(g_first);
	chunk_test_verify(a_state, 1);
}

TEST(mm_chunk, merge_both_nalloc)
{
	chunk_test_state_t a_state[] = {{128, false}, {128, false}};
	chunk_test_prepare(a_state, 2);
	
	chunk_test_state_t a_expect[] = {{256, false}};
	mm_chunk_merge(g_first);
	chunk_test_verify(a_expect, 1);
}

TEST(mm_chunk, merge_nalloc_alloc)
{
	chunk_test_state_t a_state[] = {{32, false}, {128, true}, {96, false}};
	chunk_test_state_t a_expect[] = {{160, true}, {96, false}};
	chunk_test_prepare(a_state, 3);
	
	mm_chunk_t *second = mm_chunk_next_get(g_first);
	uint32_t payload_size = chunk_test_fill_with_prepare(second, 'A');
	
	mm_chunk_merge(g_first);
	chunk_test_verify(a_expect, 2);
	chunk_test_fill_with_verify(mm_toptr(g_first), 'A', payload_size);
}

TEST(mm_chunk, merge_alloc_nalloc)
{
	chunk_test_state_t a_state[] = {{32, true}, {128, false}, {96, false}};
	chunk_test_state_t a_expect[] = {{160, true}, {96, false}};
	chunk_test_prepare(a_state, 3);
	
	uint32_t payload_size = chunk_test_fill_with_prepare(g_first, 'A');
	
	mm_chunk_merge(g_first);
	chunk_test_verify(a_expect, 2);
	chunk_test_fill_with_verify(mm_toptr(g_first), 'A', payload_size);
}

TEST(mm_chunk, merge_alloc_alloc)
{
	chunk_test_state_t a_state[] = {{32, true}, {128, true}, {96, false}};
	chunk_test_prepare(a_state, 3);
	
	uint32_t payload_a = chunk_test_fill_with_prepare(g_first, 'A');
	mm_chunk_t *second = mm_chunk_next_get(g_first);
	uint32_t payload_b = chunk_test_fill_with_prepare(second, 'B');
	
	EXPECT_ABORT_BEGIN
	mm_chunk_merge(g_first);
	VERIFY_FAILS_END("MM: cant merge");
	
	chunk_test_verify(a_state, 3);
	chunk_test_fill_with_verify(mm_toptr(g_first), 'A', payload_a);
	chunk_test_fill_with_verify(mm_toptr(second), 'B', payload_b);
}

TEST(mm_chunk, merge_bigblocks)
{
	mm_chunk_t *gigablock = (mm_chunk_t *)unity_malloc(2*CSIZE_MAX*MM_CFG_ALIGNMENT);

	UT_PTR_SET(g_first, gigablock);

	chunk_test_state_t a_state[] = {{CSIZE_MAX/2, false}, {CSIZE_MAX/2 +1 , false}, {mm_min_csize(), false}};
	chunk_test_state_t a_expect[] = {{CSIZE_MAX, false}, {mm_min_csize(), false}};
	chunk_test_prepare(a_state, 3);

	mm_chunk_merge(g_first);
	chunk_test_verify(a_expect, 2);

	mm_chunk_merge(g_first);
	chunk_test_verify(a_expect, 2);

	unity_free(gigablock);
}

TEST(mm_chunk, split)
{
	chunk_test_state_t a_state[] = {{256, false}};
	chunk_test_state_t a_expect[] = {{128, false}, {128, false}};
	chunk_test_state_t a_expect_2[] = {{64, false}, {64, false}, {128, false}};
	chunk_test_prepare(a_state, 1);

	mm_chunk_t *expect_ptr = mm_compute_next(g_first, 128);
	mm_chunk_t *expect_ptr_2 = mm_compute_next(g_first, 64);

	TEST_ASSERT_EQUAL_PTR(expect_ptr, mm_chunk_split(g_first, 128));
	chunk_test_verify(a_expect, 2);


	TEST_ASSERT_EQUAL_PTR(expect_ptr_2, mm_chunk_split(g_first, 64));
	chunk_test_verify(a_expect_2, 3);
}

TEST(mm_chunk, split_too_small)
{
	chunk_test_state_t a_state[] = {{50 + (mm_min_csize()/2), false}};
	chunk_test_prepare(a_state, 1);

	TEST_ASSERT_NULL(mm_chunk_split(g_first, 50));
	chunk_test_verify(a_state, 1);
}

TEST(mm_chunk, find_first_free)
{
	chunk_test_state_t a_state[] = {{64, true}, {64, false}, {128, true}};
	chunk_test_prepare(a_state, 3);
	
	mm_chunk_t *expect_ptr = mm_compute_next(g_first, 64);
	
	TEST_ASSERT_EQUAL_PTR(expect_ptr, mm_find_first_free(0));
	TEST_ASSERT_EQUAL_PTR(expect_ptr, mm_find_first_free(12));
	TEST_ASSERT_EQUAL_PTR(expect_ptr, mm_find_first_free(64));
	TEST_ASSERT_NULL(mm_find_first_free(65));
	TEST_ASSERT_NULL(mm_find_first_free(128));
	
	chunk_test_verify(a_state, 3);
}

TEST(mm_chunk, info)
{
	chunk_test_state_t a_state[] = {{64, true}, {64, false}, {128, true}};
	chunk_test_prepare(a_state, 3);
	mm_cinfo_t a_expect[4] = {{0, 64, true, NULL},{0, 64, false, NULL}, {0, 128, true, NULL}, {0, 0, false, NULL}};
	mm_cinfo_t a_expect_2[4] = {{0, 64, true, NULL},{0, 64, false, NULL}, {0, 0, false, NULL}, {0, 0, false, NULL}};

	mm_cinfo_t a_out[4];
	memset(a_out, 0, sizeof(a_out));
	
	mm_chunk_info(a_out, 4);
	TEST_ASSERT_EQUAL_MEMORY(a_expect, a_out, sizeof(a_expect));
	
	memset(a_out, 0, sizeof(a_out));
	mm_chunk_info(a_out, 2);
	TEST_ASSERT_EQUAL_MEMORY(a_expect_2, a_out, sizeof(a_expect_2));
}

TEST(mm_chunk, valid_between_included_wanted_csize_and_csize_max)
{
	TEST_ASSERT_TRUE(mm_validate_csize(0, 0));
	TEST_ASSERT_FALSE(mm_validate_csize(10, 0));

	TEST_ASSERT_TRUE(mm_validate_csize(10, 10));
	TEST_ASSERT_TRUE(mm_validate_csize(10, 1234));
	TEST_ASSERT_TRUE(mm_validate_csize(10, CSIZE_MAX));

	TEST_ASSERT_FALSE(mm_validate_csize(10, CSIZE_MAX + 1));
	TEST_ASSERT_FALSE(mm_validate_csize(10, CSIZE_MAX * CSIZE_MAX));

	TEST_ASSERT_TRUE(mm_validate_csize(CSIZE_MAX, CSIZE_MAX));
	TEST_ASSERT_FALSE(mm_validate_csize(CSIZE_MAX, 10));
	TEST_ASSERT_FALSE(mm_validate_csize(CSIZE_MAX, CSIZE_MAX * CSIZE_MAX));
}

TEST(mm_chunk, when_not_available_then_it_should_return_0)
{
	chunk_test_state_t a_state[] = {{64, true}, {64, false}, {128, true}};
	chunk_test_prepare(a_state, 3);

	mm_chunk_t *first = g_first;
	mm_chunk_t *second = mm_chunk_next_get(g_first);
	mm_chunk_t *third = mm_chunk_next_get(second);

	TEST_ASSERT_EQUAL_UINT16(0, mm_chunk_available_csize(NULL));
	TEST_ASSERT_EQUAL_UINT16(0, mm_chunk_available_csize(first));
	TEST_ASSERT_EQUAL_UINT16(64, mm_chunk_available_csize(second));
	TEST_ASSERT_EQUAL_UINT16(0, mm_chunk_available_csize(third));
}
