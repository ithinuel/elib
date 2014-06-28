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
static void *gs_old = NULL;

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr_realloc);

TEST_GROUP_RUNNER(memmgr_realloc)
{
	RUN_TEST_GROUP(memmgr_realloc_new);

	RUN_TEST_CASE(memmgr_realloc, to_zero_should_free);
	RUN_TEST_CASE(memmgr_realloc, too_much_does_nothing);
}

TEST_SETUP(memmgr_realloc)
{
	chunk_test_state_t a_state[] = {{45, false}, {40, true}, {85, false}, {86, false}};
	chunk_test_prepare(a_state, 4);
	gs_old = mm_toptr(mm_chunk_next_get(g_first));

	mock_chunk_setup();
}

TEST_TEAR_DOWN(memmgr_realloc)
{
	chunk_test_clear();
	mock_chunk_verify();
}

TEST_GROUP(memmgr_realloc_new);
TEST_GROUP_RUNNER(memmgr_realloc_new)
{
	RUN_TEST_CASE(memmgr_realloc_new, from_null_zero);
	RUN_TEST_CASE(memmgr_realloc_new, from_null_some);
	RUN_TEST_CASE(memmgr_realloc_new, from_null_too_much);
}

TEST_SETUP(memmgr_realloc_new)
{
	mock_chunk_setup();

	chunk_test_state_t a_state[] = {{128, false}, {128, false}};
	chunk_test_prepare(a_state, 2);
}

TEST_TEAR_DOWN(memmgr_realloc_new)
{
	chunk_test_clear();
	mock_chunk_verify();
}

/* Tests ---------------------------------------------------------------------*/
TEST(memmgr_realloc_new, from_null_zero)
{
	chunk_test_state_t a_expect[] = {{128, false}, {128, false}};
	TEST_ASSERT_NULL(mm_realloc(NULL, 0));

	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_realloc_new, from_null_some)
{
	chunk_test_state_t a_expect[] = {{11, true}, {245, false}};
	mock_mm_find_first_free_ExpectAndReturn(11, g_first);
	mock_mm_chunk_split_ExpectAndReturn(g_first, 11, false);
	mock_mm_chunk_merge_Expect(mm_compute_next(g_first, 11));

	TEST_ASSERT_EQUAL_PTR(mm_toptr(g_first), mm_realloc(NULL, 23));

	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_realloc_new, from_null_too_much)
{
	chunk_test_state_t a_expect[] = {{128, false}, {128, false}};
	TEST_ASSERT_NULL(mm_realloc(NULL, 1024*1024));
	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_realloc, to_zero_should_free)
{
	chunk_test_state_t a_expect[] = {{170, false}, {86, false}};
	mock_mm_chunk_merge_Expect(mm_tochunk(gs_old));
	mock_mm_chunk_merge_Expect(mm_chunk_prev_get(mm_tochunk(gs_old)));
	TEST_ASSERT_NULL(mm_realloc(gs_old, 0));
	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_realloc, too_much_does_nothing)
{
	chunk_test_state_t a_expect[] = {{45, false}, {40, true}, {85, false}, {86, false}};
	TEST_ASSERT_NULL(mm_realloc(gs_old, 1024*1024));
	chunk_test_verify(a_expect, 4);
}
