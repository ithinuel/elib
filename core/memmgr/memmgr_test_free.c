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
#include "tests/common_mock.h"
#include "tests/chunk_mock.h"
#include "tests/chunk_test_tools.h"
#include "memmgr/chunk.h"
#include "os/memmgr.h"
#include "memmgr_conf.h"

/* helpers -------------------------------------------------------------------*/

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr_free);

TEST_GROUP_RUNNER(memmgr_free)
{
	RUN_TEST_CASE(memmgr_free, free_null_does_no_harm);
	RUN_TEST_CASE(memmgr_free, free_cant_merge_prev_if_null);;
	RUN_TEST_CASE(memmgr_free, free_cant_merge_next_if_null);
	RUN_TEST_CASE(memmgr_free, free_cant_merge_next_if_allocated);
	RUN_TEST_CASE(memmgr_free, free_cant_merge_prev_if_allocated);
	RUN_TEST_CASE(memmgr_free, double_free_leads_to_death);
	RUN_TEST_CASE(memmgr_free, free_);
}

TEST_SETUP(memmgr_free)
{
	chunk_test_state_t a_state[] = {{128, false}, {128, false}};
	chunk_test_prepare(a_state, 2);

	mock_chunk_setup();
}

TEST_TEAR_DOWN(memmgr_free)
{
	chunk_test_clear();
	mock_chunk_verify();
}

/* Tests ---------------------------------------------------------------------*/
TEST(memmgr_free, free_null_does_no_harm)
{
	mm_free(NULL);
}

TEST(memmgr_free, free_cant_merge_prev_if_null)
{
	chunk_test_state_t a_expect[] = {{256, false}};
	chunk_test_allocated_set(g_first, true);
	mock_mm_chunk_merge_Expect(g_first);

	mm_free(mm_toptr(g_first));
	chunk_test_verify(a_expect, 1);
}

TEST(memmgr_free, free_cant_merge_next_if_null)
{
	chunk_test_state_t a_expect[] = {{256, false}};
	mm_chunk_t *second = mm_chunk_next_get(g_first);
	chunk_test_allocated_set(second, true);
	mock_mm_chunk_merge_Expect(g_first);

	mm_free(mm_toptr(second));
	chunk_test_verify(a_expect, 1);
}

TEST(memmgr_free, free_cant_merge_next_if_allocated)
{
	chunk_test_state_t a_expect[] = {{128, false}, {128, true}};
	mm_chunk_t *second = mm_chunk_next_get(g_first);

	chunk_test_allocated_set(g_first, true);
	chunk_test_allocated_set(second, true);

	mm_free(mm_toptr(g_first));
	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_free, free_cant_merge_prev_if_allocated)
{
	chunk_test_state_t a_expect[] = {{128, true}, {128, false}};
	mm_chunk_t *second = mm_chunk_next_get(g_first);

	chunk_test_allocated_set(g_first, true);
	chunk_test_allocated_set(second, true);

	mm_free(mm_toptr(second));
	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_free, free_)
{
	chunk_test_allocated_set(g_first, true);
	chunk_test_fill_with_prepare(g_first, 'A');
	mock_mm_chunk_merge_Expect(g_first);
	void *ptr = mm_toptr(g_first);
	mm_free(ptr);
	mm_chunk_validate(g_first);
	TEST_ASSERT_FALSE(g_first->allocated);
	TEST_ASSERT_EQUAL_UINT32(0, g_first->guard_offset);
}

TEST(memmgr_free, double_free_leads_to_death)
{
	chunk_test_allocated_set(g_first, true);
	mock_mm_chunk_merge_Expect(g_first);

	void *ptr = mm_toptr(g_first);
	mm_free(ptr);
	mm_chunk_validate(g_first);
	TEST_ASSERT_FALSE(g_first->allocated);

	EXPECT_ABORT_BEGIN
	mm_free(ptr);
	VERIFY_FAILS_END("MM: double free");
}
