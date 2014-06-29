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

/* variables */

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(mm_chunk);
TEST_GROUP_RUNNER(mm_chunk)
{
	RUN_TEST_GROUP(mm_chunk_validate);
	
	RUN_TEST_CASE(mm_chunk, next_get);
	RUN_TEST_CASE(mm_chunk, prev_get);

	RUN_TEST_CASE(mm_chunk, merge_alloc_null);
	RUN_TEST_CASE(mm_chunk, merge_not_alloc_null);
	RUN_TEST_CASE(mm_chunk, merge_both_not_alloc);
	RUN_TEST_CASE(mm_chunk, merge_not_alloc_alloc);
	RUN_TEST_CASE(mm_chunk, merge_alloc_not_alloc);
	RUN_TEST_CASE(mm_chunk, merge_alloc_alloc);
	RUN_TEST_CASE(mm_chunk, merge_bigblocks);
	
	RUN_TEST_CASE(mm_chunk, split);
	RUN_TEST_CASE(mm_chunk, split_too_small);
	
	RUN_TEST_CASE(mm_chunk, find_first_free);
	
	RUN_TEST_CASE(mm_chunk, info);
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

TEST(mm_chunk, merge_not_alloc_null)
{
	chunk_test_state_t a_state[] = {{256, false}};
	chunk_test_prepare(a_state, 1);
	
	mm_chunk_merge(g_first);
	chunk_test_verify(a_state, 1);
}

TEST(mm_chunk, merge_both_not_alloc)
{
	chunk_test_state_t a_state[] = {{128, false}, {128, false}};
	chunk_test_prepare(a_state, 2);
	
	chunk_test_state_t a_expect[] = {{256, false}};
	mm_chunk_merge(g_first);
	chunk_test_verify(a_expect, 1);
}

TEST(mm_chunk, merge_not_alloc_alloc)
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

TEST(mm_chunk, merge_alloc_not_alloc)
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
	
	die_Expect("MM: cant merge");
	VERIFY_DIE_START
	mm_chunk_merge(g_first);
	VERIFY_DIE_END
	die_Verify();
	
	chunk_test_verify(a_state, 3);
	chunk_test_fill_with_verify(mm_toptr(g_first), 'A', payload_a);
	chunk_test_fill_with_verify(mm_toptr(second), 'B', payload_b);
}

TEST(mm_chunk, merge_bigblocks)
{
	mock_memmgr_setup();
	mm_chunk_t *gigablock = (mm_chunk_t *)mm_alloc(2*UINT15_MAX*MM_CFG_ALIGNMENT);

	UT_PTR_SET(g_first, gigablock);

	chunk_test_state_t a_state[] = {{UINT15_MAX/2, false}, {UINT15_MAX/2 +1 , false}, {UINT15_MAX, false}};
	chunk_test_state_t a_expect[] = {{UINT15_MAX, false}, {UINT15_MAX, false}};
	chunk_test_prepare(a_state, 3);

	mm_chunk_merge(g_first);
	chunk_test_verify(a_expect, 2);

	mm_chunk_merge(g_first);
	chunk_test_verify(a_expect, 2);

	mm_free(gigablock);
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
