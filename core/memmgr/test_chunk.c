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
static void		 eval_validate_xorsum			(void);

/* variables */
static uint8_t gs_raw[1024];
static mm_chunk_t *gs_chnk = (mm_chunk_t *)gs_raw;

/* functions' definitions */
static void eval_validate_xorsum(void)
{
	die_Expect("MM: xorsum");
	VERIFY_DIE_START
	
	mm_chunk_validate(gs_chnk);
	
	VERIFY_DIE_END
	die_Verify();
}


/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(subgroup_chunk_validate);
TEST_GROUP_RUNNER(subgroup_chunk_validate)
{
	RUN_TEST_CASE(subgroup_chunk_validate, validate);
	RUN_TEST_CASE(subgroup_chunk_validate, validate_alignment);
	RUN_TEST_CASE(subgroup_chunk_validate, validate_overflow);
	RUN_TEST_CASE(subgroup_chunk_validate, validate_corruption_prev_size);
	RUN_TEST_CASE(subgroup_chunk_validate, validate_corruption_allocated);
	RUN_TEST_CASE(subgroup_chunk_validate, validate_corruption_xorsum);
	RUN_TEST_CASE(subgroup_chunk_validate, validate_corruption_size);
	RUN_TEST_CASE(subgroup_chunk_validate, validate_corruption_guard_offset);
	RUN_TEST_CASE(subgroup_chunk_validate, validate_corruption_allocator);
}
TEST_SETUP(subgroup_chunk_validate)
{
	mm_chunk_init(gs_chnk, NULL, 1024/MM_CFG_ALIGNMENT);
}
TEST_TEAR_DOWN(subgroup_chunk_validate)
{
	memset(gs_raw, 0, 1024);
}

TEST_GROUP(subgroup_chunk_merge);
TEST_GROUP_RUNNER(subgroup_chunk_merge)
{
	RUN_TEST_CASE(subgroup_chunk_merge, merge);
}
TEST_SETUP(subgroup_chunk_merge)
{
	mm_chunk_t *prev = NULL;
	mm_chunk_t *chnk = gs_chnk;
	uint32_t total = 1024/MM_CFG_ALIGNMENT;
		
	while (total > mm_min_csize()) {
		mm_chunk_init(chnk, prev, umin(total, 64));
		
		total -= chnk->size;
		prev = chnk;
		chnk = mm_compute_next(chnk, chnk->size);
	}
}
TEST_TEAR_DOWN(subgroup_chunk_merge)
{
	memset(gs_raw, 0, 1024);
}

TEST_GROUP(mm_chunk);
TEST_GROUP_RUNNER(mm_chunk)
{
	RUN_TEST_GROUP(subgroup_chunk_validate);
	RUN_TEST_GROUP(subgroup_chunk_merge);
	RUN_TEST_CASE(mm_chunk, split);
	RUN_TEST_CASE(mm_chunk, aggregate);
}
TEST_SETUP(mm_chunk)
{
}
TEST_TEAR_DOWN(mm_chunk)
{
}

/* Tests ---------------------------------------------------------------------*/
TEST(subgroup_chunk_validate, validate)
{
	mm_chunk_validate(gs_chnk);
}

TEST(subgroup_chunk_validate, validate_alignment)
{
	die_Expect("MM: alignment");
	VERIFY_DIE_START
	mm_chunk_validate((mm_chunk_t *)((uintptr_t)gs_chnk + 1));
	VERIFY_DIE_END
	die_Verify();
}

TEST(subgroup_chunk_validate, validate_overflow)
{
	die_Expect("MM: overflowed");
	VERIFY_DIE_START

	memset(gs_raw + 512, 0, 512);
	mm_chunk_validate(gs_chnk);

	VERIFY_DIE_END
	die_Verify();
}

TEST(subgroup_chunk_validate, validate_corruption_prev_size)
{
	gs_chnk->prev_size = 32;
	eval_validate_xorsum();
}

TEST(subgroup_chunk_validate, validate_corruption_allocated)
{
	gs_chnk->allocated = true;
	eval_validate_xorsum();
}

TEST(subgroup_chunk_validate, validate_corruption_xorsum)
{
	gs_chnk->xorsum = 0;
	eval_validate_xorsum();
}

TEST(subgroup_chunk_validate, validate_corruption_size)
{
	gs_chnk->size = 0;
	eval_validate_xorsum();
}

TEST(subgroup_chunk_validate, validate_corruption_guard_offset)
{
	gs_chnk->guard_offset = 42;
	eval_validate_xorsum();
}

TEST(subgroup_chunk_validate, validate_corruption_allocator)
{
	gs_chnk->allocator = __builtin_return_address(0);
	eval_validate_xorsum();
}

/* -------------------------------------------------------------------------- */
TEST(subgroup_chunk_merge, merge)
{
	mm_chunk_merge(gs_chnk);
}

/* -------------------------------------------------------------------------- */
TEST(mm_chunk, split)
{
	TEST_FAIL();
}

TEST(mm_chunk, aggregate)
{
	TEST_FAIL();
}

