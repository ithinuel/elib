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
#include "tests/common_mock.h"
#include "tests/chunk_mock.h"
#include "tests/chunk_test_tools.h"
#include "memmgr/chunk.h"
#include "os/memmgr.h"
#include "memmgr_conf.h"

/* helpers -------------------------------------------------------------------*/
static uint8_t *gs_ptr = NULL;
static uint32_t gs_size = 0;
static char	gs_fill_val = 0;
static uint32_t gs_chunk_envelop = 0;

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr_realloc);

TEST_GROUP_RUNNER(memmgr_realloc)
{
	RUN_TEST_GROUP(memmgr_realloc_new);

	RUN_TEST_CASE(memmgr_realloc, to_zero_should_free);
	RUN_TEST_CASE(memmgr_realloc, too_much_does_nothing);


	/*
	 * merge next validated
	 * merge prev validated
	 * merge both validated
	 * else alloc
	 */
	RUN_TEST_CASE(memmgr_realloc, same_csize);
	RUN_TEST_CASE(memmgr_realloc, shrink_a_bit);
	RUN_TEST_CASE(memmgr_realloc, shrink_a_lot);
	RUN_TEST_CASE(memmgr_realloc, grow_a_bit);
	RUN_TEST_CASE(memmgr_realloc, grow_a_lot_eat_next);
	RUN_TEST_CASE(memmgr_realloc, grow_a_lot_eat_prev);
	RUN_TEST_CASE(memmgr_realloc, grow_a_lot_eat_both);
	RUN_TEST_CASE(memmgr_realloc, grow_a_lot_cant_eat);
}

TEST_SETUP(memmgr_realloc)
{
	chunk_test_state_t a_state[] = {{30, false}, {20, true}, {20, false}, {186, false}};
	chunk_test_prepare(a_state, 4);
	
	mm_chunk_t *second = mm_chunk_next_get(g_first);
	gs_chunk_envelop = mm_header_csize()+MM_CFG_GUARD_SIZE;
	
	gs_size = 51;
	mm_chunk_guard_set(second, gs_size);
	second->xorsum = mm_chunk_xorsum(second);
	
	gs_fill_val = 'A';
	gs_ptr = mm_toptr(second);
	memset(gs_ptr, gs_fill_val, gs_size);

	mock_chunk_setup();
}

TEST_TEAR_DOWN(memmgr_realloc)
{
	if (gs_ptr != NULL) {
		chunk_test_fill_with_verify(gs_ptr, gs_fill_val, gs_size);
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
	chunk_test_state_t a_expect[] = {{70, false}, {186, false}};
	mock_mm_chunk_merge_Expect(mm_tochunk(gs_ptr));
	mock_mm_chunk_merge_Expect(mm_chunk_prev_get(mm_tochunk(gs_ptr)));
	TEST_ASSERT_NULL(mm_realloc(gs_ptr, 0));
	gs_ptr = NULL;
	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_realloc, too_much_does_nothing)
{
	chunk_test_state_t a_expect[] = {{30, false}, {20, true}, {20, false}, {186, false}};
	TEST_ASSERT_NULL(mm_realloc(gs_ptr, 1024*1024));
	chunk_test_verify(a_expect, 4);
}

TEST(memmgr_realloc, same_csize)
{
	mock_mm_chunk_split_ExpectAndReturn(mm_tochunk(gs_ptr), 18, false);
	uint8_t *ptr = mm_realloc(gs_ptr, gs_size);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);
}

TEST(memmgr_realloc, shrink_a_bit)
{
	uint32_t csize = 17;
	mock_mm_chunk_split_ExpectAndReturn(mm_tochunk(gs_ptr), csize, false);
	uint32_t new_payload = (csize - (mm_header_csize() + MM_CFG_GUARD_SIZE)) * MM_CFG_ALIGNMENT;

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);
	gs_size = new_payload;
}

TEST(memmgr_realloc, shrink_a_lot)
{
	uint32_t csize = 14;
	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *new = mm_compute_next(chnk, csize);
	mock_mm_chunk_split_ExpectAndReturn(chnk, csize, false);
	mock_mm_chunk_merge_Expect(new);
	uint32_t new_payload = (csize - (mm_header_csize() + MM_CFG_GUARD_SIZE)) * MM_CFG_ALIGNMENT;

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);
	gs_size = new_payload;
}

TEST(memmgr_realloc, grow_a_bit)
{
	uint32_t csize = 18;
	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mock_mm_chunk_split_ExpectAndReturn(chnk, csize, false);
	uint32_t new_payload = (csize - (mm_header_csize() + MM_CFG_GUARD_SIZE)) * MM_CFG_ALIGNMENT;

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);
	memset(ptr, 'A', new_payload);
	gs_size = new_payload;
}

TEST(memmgr_realloc, grow_a_lot_eat_next)
{
	uint32_t csize = 30;
	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *new = mm_compute_next(chnk, csize);

	mock_mm_validate_csize_ExpectAndReturn(csize, 40, true);
	mock_mm_chunk_merge_Expect(chnk);
	mock_mm_chunk_split_ExpectAndReturn(chnk, csize, false);
	mock_mm_chunk_merge_Expect(new);
	uint32_t new_payload = (csize - (mm_header_csize() + MM_CFG_GUARD_SIZE)) * MM_CFG_ALIGNMENT;

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);
	memset(ptr, 'A', new_payload);
	gs_size = new_payload;
}

TEST(memmgr_realloc, grow_a_lot_eat_prev)
{
	uint32_t csize = 30;
	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *prev = mm_chunk_prev_get(chnk);
	mm_chunk_t *new = mm_compute_next(prev, csize);

	mock_mm_validate_csize_ExpectAndReturn(csize, 40, false);
	mock_mm_validate_csize_ExpectAndReturn(csize, 50, true);
	mock_mm_chunk_merge_Expect(prev);
	mock_mm_chunk_split_ExpectAndReturn(prev, csize, false);
	mock_mm_chunk_merge_Expect(new);
	uint32_t new_payload = (csize - (mm_header_csize() + MM_CFG_GUARD_SIZE)) * MM_CFG_ALIGNMENT;

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(prev), ptr);
	memset(ptr, 'A', new_payload);
	gs_ptr = ptr;
	gs_size = new_payload;
}

TEST(memmgr_realloc, grow_a_lot_eat_both)
{
	uint32_t csize = 30;
	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *prev = mm_chunk_prev_get(chnk);
	mm_chunk_t *new = mm_compute_next(prev, csize);

	mock_mm_validate_csize_ExpectAndReturn(csize, 40, false);
	mock_mm_validate_csize_ExpectAndReturn(csize, 50, false);
	mock_mm_validate_csize_ExpectAndReturn(csize, 70, true);
	mock_mm_chunk_merge_Expect(chnk);
	mock_mm_chunk_merge_Expect(prev);
	mock_mm_chunk_split_ExpectAndReturn(prev, csize, false);
	mock_mm_chunk_merge_Expect(new);
	uint32_t new_payload = (csize - (mm_header_csize() + MM_CFG_GUARD_SIZE)) * MM_CFG_ALIGNMENT;

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(prev), ptr);
	memset(ptr, 'A', new_payload);
	gs_ptr = ptr;
	gs_size = new_payload;
}

TEST(memmgr_realloc, grow_a_lot_cant_eat)
{
	uint32_t csize = 30;
	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *prev = mm_chunk_prev_get(chnk);
	mm_chunk_t *new = mm_compute_next(prev, csize);

	mock_mm_validate_csize_ExpectAndReturn(csize, 40, false);
	mock_mm_validate_csize_ExpectAndReturn(csize, 50, false);
	mock_mm_validate_csize_ExpectAndReturn(csize, 70, false);

	mock_mm_chunk_split_ExpectAndReturn(prev, csize, false);
	mock_mm_chunk_merge_Expect(new);
	uint32_t new_payload = (csize - (mm_header_csize() + MM_CFG_GUARD_SIZE)) * MM_CFG_ALIGNMENT;

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(prev), ptr);
	memset(ptr, 'A', new_payload);
	gs_ptr = ptr;
	gs_size = new_payload;
}
