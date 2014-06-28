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
#define			DEFAULT_SIZE		(11)

static void		prepare_alloc		(void);

static void prepare_alloc(void)
{
	uint32_t size = DEFAULT_SIZE;
	uint16_t expect_csize = mm_to_csize(size);
	mock_mm_find_first_free_ExpectAndReturn(expect_csize, g_first);
	mock_mm_chunk_split_ExpectAndReturn(g_first, expect_csize, false);
	mm_chunk_t *new = mm_compute_next(g_first, expect_csize);
	mock_mm_chunk_merge_Expect(new);
}

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr_alloc);

TEST_GROUP_RUNNER(memmgr_alloc)
{
	RUN_TEST_CASE(memmgr_alloc, alloc_zero);
	RUN_TEST_CASE(memmgr_alloc, alloc_too_big);
	RUN_TEST_CASE(memmgr_alloc, alloc_none_available);
	RUN_TEST_CASE(memmgr_alloc, alloc_no_split);
	RUN_TEST_CASE(memmgr_alloc, alloc_no_merge_if_null);
	RUN_TEST_CASE(memmgr_alloc, alloc_no_merge_if_allocated);
	RUN_TEST_CASE(memmgr_alloc, _alloc);
}

TEST_SETUP(memmgr_alloc)
{
	mock_chunk_setup();

	chunk_test_state_t a_state[] = {{128, false}, {128, false}};
	chunk_test_prepare(a_state, 2);
}

TEST_TEAR_DOWN(memmgr_alloc)
{
	chunk_test_clear();
	mock_chunk_verify();
}

/* Tests ---------------------------------------------------------------------*/
TEST(memmgr_alloc, alloc_zero)
{
	TEST_ASSERT_NULL(mm_alloc(0));
}

TEST(memmgr_alloc, alloc_too_big)
{
	TEST_ASSERT_NULL(mm_alloc(1024*1024));
}

TEST(memmgr_alloc, alloc_none_available)
{
	uint32_t size = 11;
	uint16_t expect_csize = mm_to_csize(size);
	mock_mm_find_first_free_ExpectAndReturn(expect_csize, NULL);

	TEST_ASSERT_NULL(mm_alloc(size));
}

TEST(memmgr_alloc, alloc_no_split)
{
	uint32_t size = 11;
	uint16_t expect_csize = mm_to_csize(size);
	chunk_test_state_t a_expect_state[] = {{128, true}, {128, false}};

	mock_mm_find_first_free_ExpectAndReturn(expect_csize, g_first);
	mock_mm_chunk_split_ExpectAndReturn(g_first, expect_csize, true);

	void *ptr = mm_alloc(size);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(g_first), ptr);

	memset(ptr, 'A', size);
	mm_chunk_validate(g_first);
	TEST_ASSERT_TRUE(g_first->allocated);

	chunk_test_verify(a_expect_state, 2);
}

TEST(memmgr_alloc, alloc_no_merge_if_null)
{
	uint32_t size = 11;
	uint16_t expect_csize = mm_to_csize(size);
	chunk_test_state_t a_expect_state[] = {{128, false}, {expect_csize, true}, {128-expect_csize, false}};

	mm_chunk_t *second = mm_chunk_next_get(g_first);

	mock_mm_find_first_free_ExpectAndReturn(expect_csize, second);
	mock_mm_chunk_split_ExpectAndReturn(second, expect_csize, false);

	void *ptr = mm_alloc(size);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(second), ptr);

	memset(ptr, 'A', size);
	mm_chunk_validate(second);
	TEST_ASSERT_TRUE(second->allocated);

	chunk_test_verify(a_expect_state, 3);
}

TEST(memmgr_alloc, alloc_no_merge_if_allocated)
{
	uint32_t size = 11;
	uint16_t expect_csize = mm_to_csize(size);
	chunk_test_state_t a_expect_state[] = {{expect_csize, true}, {128-expect_csize, false}, {128, true}};

	mm_chunk_t *second = mm_chunk_next_get(g_first);
	chunk_test_allocated_set(second, true);

	mock_mm_find_first_free_ExpectAndReturn(expect_csize, g_first);
	mock_mm_chunk_split_ExpectAndReturn(g_first, expect_csize, false);

	void *ptr = mm_alloc(size);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(g_first), ptr);

	memset(ptr, 'A', size);
	mm_chunk_validate(g_first);
	TEST_ASSERT_TRUE(g_first->allocated);

	chunk_test_verify(a_expect_state, 3);
}

TEST(memmgr_alloc, _alloc)
{
	uint16_t expect_csize = mm_to_csize(DEFAULT_SIZE);
	chunk_test_state_t a_expect_state[] = {{expect_csize, true}, {256-expect_csize, false}};
	prepare_alloc();

	void *ptr = mm_alloc(DEFAULT_SIZE);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(g_first), ptr);

	memset(ptr, 'A', DEFAULT_SIZE);
	mm_chunk_validate(g_first);
	TEST_ASSERT_TRUE(g_first->allocated);

	chunk_test_verify(a_expect_state, 2);
}
