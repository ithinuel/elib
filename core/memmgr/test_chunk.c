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
/* functions' prototypes */
static void		prepare_chunk				(uint16_t *csize_array,
								 uint32_t array_len);
static void		eval_chunk_status			(uint16_t *csize_array,
								 uint32_t array_len);

/* variables */
static uint8_t gs_raw[1024];
static mm_chunk_t *gs_chnk = (mm_chunk_t *)gs_raw;

/* functions' definitions */
static void prepare_chunk(uint16_t *csize_array, uint32_t array_len)
{
	mm_chunk_t *prev =  NULL;
	mm_chunk_t *chnk = gs_chnk;
	for (uint32_t i = 0; i < array_len; i++)
	{
		uint16_t csize = csize_array[i];
		mm_chunk_init(chnk, prev, csize);
		prev = chnk;
		chnk = mm_compute_next(chnk, csize);
	}
	mm_chunk_boundary_set(prev);
}

static void eval_chunk_status(uint16_t *csize_array, uint32_t array_len)
{
	//printf("\n");
	mm_chunk_t *chnk = gs_chnk;
	for (uint32_t i = 0; (i < array_len) && (chnk != NULL); i++, chnk = mm_chunk_next_get(chnk))
	{
		//printf("%p %5d %d\n", chnk, chnk->size, chnk->allocated);
		TEST_ASSERT_EQUAL_UINT16(csize_array[i], chnk->size);
	}
	if (chnk != NULL) {
		TEST_FAIL_MESSAGE("more chunk than expected");
	}
}


/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(mm_chunk);
TEST_GROUP_RUNNER(mm_chunk)
{
	RUN_TEST_GROUP(mm_chunk_validate);
	
	RUN_TEST_CASE(mm_chunk, merge_the_first_one);
	RUN_TEST_CASE(mm_chunk, merge_in_the_middle);
	RUN_TEST_CASE(mm_chunk, merge_the_last_one);
	RUN_TEST_CASE(mm_chunk, merge_dont_absorb_next_if_allocated);
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
TEST(mm_chunk, merge_the_first_one)
{
	uint16_t a_csize[4] = {64, 64, 64, 64};
	prepare_chunk(a_csize, 4);
	
	uint16_t a_csize_expected[] = {128, 64, 64};
	mm_chunk_merge(gs_chnk);
	eval_chunk_status(a_csize_expected, 3);
}

TEST(mm_chunk, merge_in_the_middle)
{
	uint16_t a_csize[4] = {64, 64, 64, 64};
	prepare_chunk(a_csize, 4);
	
	mm_chunk_t *second = mm_chunk_next_get(gs_chnk);
	
	uint16_t a_csize_expected[] = {64, 128, 64};
	mm_chunk_merge(second);
	eval_chunk_status(a_csize_expected, 3);
	
	a_csize_expected[1] = 192;
	mm_chunk_merge(second);
	eval_chunk_status(a_csize_expected, 2);
}

TEST(mm_chunk, merge_the_last_one)
{
	uint16_t a_csize[4] = {64, 64, 64, 64};
	prepare_chunk(a_csize, 4);

	mm_chunk_t *second = mm_chunk_next_get(gs_chnk);
	mm_chunk_t *third = mm_chunk_next_get(second);
	mm_chunk_t *forth = mm_chunk_next_get(third);
	
	uint16_t a_csize_expected[] = {64, 64, 64, 64};
	mm_chunk_merge(forth);
	eval_chunk_status(a_csize_expected, 4);
}

TEST(mm_chunk, merge_dont_absorb_next_if_allocated)
{
	uint16_t a_csize_expected[] = {64, 64, 64, 64};
	uint16_t a_csize_expected_2[] = {64, 128, 64};
	uint16_t a_csize[4] = {64, 64, 64, 64};
	prepare_chunk(a_csize, 4);
	mm_chunk_t *second = mm_chunk_next_get(gs_chnk);
	second->allocated = true;
	second->xorsum = mm_chunk_xorsum(second);
	
	mm_chunk_merge(gs_chnk);
	eval_chunk_status(a_csize_expected, 4);
	
	mm_chunk_merge(second);
	eval_chunk_status(a_csize_expected_2, 3);
}

TEST(mm_chunk, split)
{
	uint16_t a_csize[1] = {256};
	prepare_chunk(a_csize, 1);
		
	uint16_t expect_a_csize_1[2] = {128, 128};
	uint16_t expect_a_csize_2[2] = {64, 192};
	
	mm_chunk_split(gs_chnk, 128);
	eval_chunk_status(expect_a_csize_1, 2);
	
	mm_chunk_split(gs_chnk, 64);
	eval_chunk_status(expect_a_csize_2, 2);
}

TEST(mm_chunk, aggregate)
{
	TEST_FAIL();
}

