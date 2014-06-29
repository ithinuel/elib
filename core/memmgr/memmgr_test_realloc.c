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
static uint8_t *gs_ptr = NULL;
static uint32_t gs_size = 0;

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr_realloc);

TEST_GROUP_RUNNER(memmgr_realloc)
{
	RUN_TEST_GROUP(memmgr_realloc_new);

	RUN_TEST_CASE(memmgr_realloc, to_zero_should_free);
	RUN_TEST_CASE(memmgr_realloc, too_much_does_nothing);

	/*
	 * same csize => return old_ptr & update allocator & guard
	 *
	 * shrink: diff := old_csize - new_csize
	 * diff  < min_csize			=> return old_ptr & update allocator
	 * diff >= min_csize && next == null	=> split
	 * diff >= min_csize && next allocated	=> split
	 * diff >= min_csize && next free	=> split & merge
	 *
	 * grow:
	 * this
	 * this + next
	 * prev + this
	 * prev + this + next
	 *
	 * wanted_csize == this->csize
	 * wanted_csize  < this->csize
	 * wanted_csize  > this->csize
	 */

	RUN_TEST_CASE(memmgr_realloc, shrink_same_csize_update_guard);
	RUN_TEST_CASE(memmgr_realloc, grow_dont_fit_in_sibling_alloc_new);
	RUN_TEST_CASE(memmgr_realloc, grow_dont_fit_in_sibling_alloc_new_but_none_found);
}

TEST_SETUP(memmgr_realloc)
{
	chunk_test_state_t a_state[] = {{16, false}, {10, true}, {30, false}, {200, false}};
	chunk_test_prepare(a_state, 4);
	mm_chunk_t *second = mm_chunk_next_get(g_first);
	gs_size = (10-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT;
	mm_chunk_guard_set(second, gs_size);
	second->xorsum = mm_chunk_xorsum(second);
	gs_ptr = mm_toptr(second);
	memset(gs_ptr, 'A', gs_size);

	mock_chunk_setup();
}

TEST_TEAR_DOWN(memmgr_realloc)
{
	if (gs_ptr != NULL) {
		chunk_test_fill_with_verify(gs_ptr, 'A', gs_size);
	}
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

	uint8_t *ptr = mm_realloc(NULL, 22);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(g_first), ptr);
	memset(ptr, 'A', 22);

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
	chunk_test_state_t a_expect[] = {{56, false}, {200, false}};
	mock_mm_chunk_merge_Expect(mm_tochunk(gs_ptr));
	mock_mm_chunk_merge_Expect(mm_chunk_prev_get(mm_tochunk(gs_ptr)));
	TEST_ASSERT_NULL(mm_realloc(gs_ptr, 0));
	gs_ptr = NULL;
	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_realloc, too_much_does_nothing)
{
	chunk_test_state_t a_expect[] = {{16, false}, {10, true}, {30, false}, {200, false}};
	TEST_ASSERT_NULL(mm_realloc(gs_ptr, 1024*1024));
	chunk_test_verify(a_expect, 4);
}

TEST(memmgr_realloc, shrink_same_csize_update_guard)
{
	uint8_t *ptr = mm_realloc(gs_ptr, 20);
	TEST_ASSERT_EQUAL_PTR(ptr, gs_ptr);
	chunk_test_fill_with_verify(ptr, 'A', 20);
}

TEST(memmgr_realloc, grow_dont_fit_in_sibling_alloc_new)
{
	chunk_test_state_t a_expect[] = {{56, false}, {57, true}, {143, false}};

	mm_chunk_t *this = mm_tochunk(gs_ptr);
	mm_chunk_t *last = mm_chunk_next_get(this);
	last = mm_chunk_next_get(last);

	mock_mm_find_first_free_ExpectAndReturn(57, last);
	mock_mm_chunk_split_ExpectAndReturn(last, 57, false);
	mock_mm_chunk_merge_Expect(this);
	mock_mm_chunk_merge_Expect(mm_chunk_prev_get(this));

	uint8_t *new = mm_realloc(gs_ptr, (57-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(last), new);
	gs_ptr = new;

	chunk_test_verify(a_expect, 3);
}

TEST(memmgr_realloc, grow_dont_fit_in_sibling_alloc_new_but_none_found)
{
	chunk_test_state_t a_expect[] = {{16, false}, {10, true}, {30, false}, {200, false}};

	mm_chunk_t *this = mm_tochunk(gs_ptr);
	mm_chunk_t *last = mm_chunk_next_get(this);
	last = mm_chunk_next_get(last);

	mock_mm_find_first_free_ExpectAndReturn(57, NULL);

	uint8_t *new = mm_realloc(gs_ptr, (57-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT);
	TEST_ASSERT_NULL(new);

	chunk_test_verify(a_expect, 4);
}
