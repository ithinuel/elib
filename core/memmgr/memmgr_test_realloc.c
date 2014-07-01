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
	 * wanted_csize  < this->csize => shrink
	 * wanted_csize == this->csize => nothing
	 * wanted_csize  > this->csize => grow
	 *
	 * same csize => return old_ptr & update allocator & guard
	 *
	 * is available: not null & not allocated
	 *
	 * shrink:
	 * # valid only if next is available
	 * after_merge := this.csize + next.csize
	 *
	 * next is available && after_merge > max && (this.csize-new_csize)  < min_csize	=> .
	 * next is available && after_merge > max && (this.csize-new_csize) >= min_csize	=> split(this)
	 * next is available && after_merge <= max						=> merge(this) o split(this)
	 *
	 *
	 * grow:
	 * # valid only if next is available
	 * after_merge_next := this.csize + next.csize
	 * # valid only if next & prev are available
	 * after_merge_both := prev.csize + this.csize + next.csize
	 * # valid only if prev is available
	 * after_merge_prev := prev.csize + this.csize
	 *
	 * new_csize <= max
	 * 
	 * next is available && after_merge_next <= max
	 *		     && after_merge_next >= new_csize	=> merge(this)
	 * prev is available && ((after_merge_next > max) || (after_merge_next < new_csize))
	 *		     && after_merge_prev <= max
	 *		     && after_merge_prev >= new_csizez	=> merge(prev) o this = prev
	 * prev is available && next is available
	 *		     && after_merge_next < new_csize
	 *		     && after_merge_prev < new_csize
	 *		     && after_merge_both <= max
	 *		     && after_merge_both >= new_csize	=> merge(prev) o this = prev o merge(this)
	 *
	 * N: next is available
	 * N_m: after_merge_next <= max
	 * N_e: after_merge_next >= new_csize
	 * B_m: after_merge_both <= max
	 *
	 * P: prev is available
	 * P_m: after_merge_prev <= max
	 * P_e: after_merge_prev >= new_csize
	 * B_e: after_merge_both >= new_csize
	 *
	 * merge(this) = (N & N_m & N_e) | (P & N & !N_e & !P_e & B_m & B_e)
	 * merge(prev) o this = prev = (P & (!N_m | !N_e) & P_m & P_e) | (P & N & !N_e & !P_e & B_m & B_e)
	 * 
	 * new <- split(this, new_csize)
	 * new  => merge(new)
	 */
	 
	RUN_TEST_CASE(memmgr_realloc, must_not_merge_nor_split);
	RUN_TEST_CASE(memmgr_realloc, must_not_merge_then_split);
	RUN_TEST_CASE(memmgr_realloc, must_not_merge_then_split_and_merge);

	RUN_TEST_CASE(memmgr_realloc, must_merge_with_next_then_cant_split);
	RUN_TEST_CASE(memmgr_realloc, must_merge_with_next_then_can_split_and_merge);
	RUN_TEST_CASE(memmgr_realloc, must_merge_with_prev_then_cant_split);
	RUN_TEST_CASE(memmgr_realloc, must_merge_with_prev_then_can_split_and_merge);
	RUN_TEST_CASE(memmgr_realloc, must_merge_with_both_then_cant_split);
	RUN_TEST_CASE(memmgr_realloc, must_merge_with_both_then_can_split_and_merge);

	RUN_TEST_CASE(memmgr_realloc, merging_next_go_over_max_csize);
	RUN_TEST_CASE(memmgr_realloc, merging_prev_go_over_max_csize);
	RUN_TEST_CASE(memmgr_realloc, merging_both_go_over_max_csize);
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

TEST(memmgr_realloc, must_not_merge_nor_split)
{
	chunk_test_state_t a_expect[] = {{30, false}, {20, true}, {20, false}, {186, false}};
	uint32_t new_payload = 60;

	mock_mm_chunk_split_ExpectAndReturn(mm_tochunk(gs_ptr), (new_payload/MM_CFG_ALIGNMENT)+gs_chunk_envelop, true);

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);

	chunk_test_fill_with_verify(ptr, 'A', umin(gs_size, new_payload));
	memset(ptr, 'B', new_payload);

	gs_fill_val = 'B';
	gs_size = new_payload;

	chunk_test_verify(a_expect, 4);
}

TEST(memmgr_realloc, must_not_merge_then_split)
{
	chunk_test_state_t a_expect[] = {{30, false}, {20, true}, {20, false}, {186, false}};
	uint32_t new_payload = 40;

	mock_chunk_setup();
	mock_mm_chunk_split_ExpectAndReturn(mm_tochunk(gs_ptr), (new_payload/MM_CFG_ALIGNMENT)+gs_chunk_envelop, true);

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);

	chunk_test_fill_with_verify(ptr, 'A', umin(gs_size, new_payload));
	memset(ptr, 'B', new_payload);

	gs_fill_val = 'B';
	gs_size = new_payload;

	chunk_test_verify(a_expect, 4);
}

TEST(memmgr_realloc, must_not_merge_then_split_and_merge)
{
	uint32_t csize = 14;
	chunk_test_state_t a_expect[] = {{30, false}, {csize, true}, {40-csize, false}, {186, false}};
	uint32_t new_payload = (csize-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT - 1;

	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *new = mm_compute_next(chnk, csize);
	mock_mm_chunk_split_ExpectAndReturn(chnk, csize, false);
	mock_mm_chunk_merge_Expect(new);

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);

	chunk_test_fill_with_verify(ptr, 'A', umin(gs_size, new_payload));
	memset(ptr, 'B', new_payload);

	gs_fill_val = 'B';
	gs_size = new_payload;

	chunk_test_verify(a_expect, 4);
}

TEST(memmgr_realloc, must_merge_with_next_then_cant_split)
{
	uint32_t csize = 36;
	chunk_test_state_t a_expect[] = {{30, false}, {40, true}, {186, false}};
	uint32_t new_payload = (csize-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT - 1;

	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mock_mm_chunk_merge_Expect(chnk);
	mock_mm_chunk_split_ExpectAndReturn(chnk, csize, true);

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);

	chunk_test_fill_with_verify(ptr, 'A', umin(gs_size, new_payload));
	memset(ptr, 'B', new_payload);

	gs_fill_val = 'B';
	gs_size = new_payload;

	chunk_test_verify(a_expect, 3);
}

TEST(memmgr_realloc, must_merge_with_next_then_can_split_and_merge)
{
	uint32_t csize = 30;
	chunk_test_state_t a_expect[] = {{30, false}, {30, true}, {196, false}};
	uint32_t new_payload = (csize-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT - 1;

	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *new = mm_compute_next(chnk, csize);
	mock_mm_chunk_merge_Expect(chnk);
	mock_mm_chunk_split_ExpectAndReturn(chnk, csize, false);
	mock_mm_chunk_merge_Expect(new);

	uint8_t *ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(gs_ptr, ptr);

	chunk_test_fill_with_verify(ptr, 'A', umin(gs_size, new_payload));
	memset(ptr, 'B', new_payload);

	gs_fill_val = 'B';
	gs_size = new_payload;

	chunk_test_verify(a_expect, 3);
}

TEST(memmgr_realloc, must_merge_with_prev_then_cant_split)
{
	uint32_t csize = 46;
	chunk_test_state_t a_expect[] = {{50, true}, {20, false}, {186, false}};
	uint32_t new_payload = (csize-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT - 1;

	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *prev = mm_chunk_prev_get(chnk);
	mock_mm_chunk_merge_Expect(prev);
	mock_mm_chunk_split_ExpectAndReturn(prev, csize, true);

	gs_ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(prev), gs_ptr);

	chunk_test_fill_with_verify(gs_ptr, 'A', umin(gs_size, new_payload));
	memset(gs_ptr, 'B', new_payload);

	gs_fill_val = 'B';
	gs_size = new_payload;

	chunk_test_verify(a_expect, 3);
}

TEST(memmgr_realloc, must_merge_with_prev_then_can_split_and_merge)
{
	uint32_t csize = 42;
	chunk_test_state_t a_expect[] = {{42, true}, {28, false}, {186, false}};
	uint32_t new_payload = (csize-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT - 1;

	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *prev = mm_chunk_prev_get(chnk);
	mm_chunk_t *new = mm_compute_next(prev, csize);
	mock_mm_chunk_merge_Expect(prev);
	mock_mm_chunk_split_ExpectAndReturn(prev, csize, false);
	mock_mm_chunk_merge_Expect(new);

	gs_ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(prev), gs_ptr);

	chunk_test_fill_with_verify(gs_ptr, 'A', umin(gs_size, new_payload));
	memset(gs_ptr, 'B', new_payload);

	gs_fill_val = 'B';
	gs_size = new_payload;

	chunk_test_verify(a_expect, 3);
}

TEST(memmgr_realloc, must_merge_with_both_then_cant_split)
{
	uint32_t csize = 66;
	chunk_test_state_t a_expect[] = {{70, true}, {186, false}};
	uint32_t new_payload = (csize-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT - 1;

	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *prev = mm_chunk_prev_get(chnk);
	mock_mm_chunk_merge_Expect(chnk);
	mock_mm_chunk_merge_Expect(prev);
	mock_mm_chunk_split_ExpectAndReturn(prev, csize, true);

	gs_ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(prev), gs_ptr);

	chunk_test_fill_with_verify(gs_ptr, 'A', umin(gs_size, new_payload));
	memset(gs_ptr, 'B', new_payload);

	gs_fill_val = 'B';
	gs_size = new_payload;

	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_realloc, must_merge_with_both_then_can_split_and_merge)
{
	uint32_t csize = 60;
	chunk_test_state_t a_expect[] = {{60, true}, {196, false}};
	uint32_t new_payload = (csize-(mm_header_csize()+MM_CFG_GUARD_SIZE))*MM_CFG_ALIGNMENT - 1;

	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_chunk_t *prev = mm_chunk_prev_get(chnk);
	mm_chunk_t *new = mm_compute_next(prev, csize);
	mock_mm_chunk_merge_Expect(chnk);
	mock_mm_chunk_merge_Expect(prev);
	mock_mm_chunk_split_ExpectAndReturn(prev, csize, false);
	mock_mm_chunk_merge_Expect(new);

	gs_ptr = mm_realloc(gs_ptr, new_payload);
	TEST_ASSERT_EQUAL_PTR(mm_toptr(prev), gs_ptr);

	chunk_test_fill_with_verify(gs_ptr, 'A', umin(gs_size, new_payload));
	memset(gs_ptr, 'B', new_payload);

	gs_fill_val = 'B';
	gs_size = new_payload;

	chunk_test_verify(a_expect, 2);
}

TEST(memmgr_realloc, merging_next_go_over_max_csize)
{
	TEST_FAIL();
}
TEST(memmgr_realloc, merging_prev_go_over_max_csize)
{
	TEST_FAIL();
}
TEST(memmgr_realloc, merging_both_go_over_max_csize)
{
	TEST_FAIL();
}
