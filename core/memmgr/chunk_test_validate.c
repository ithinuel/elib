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
#include "tests/common_mock.h"

#include "memmgr_conf.h"

/* helpers -------------------------------------------------------------------*/
/* functions' prototypes */
static void		eval_validate_xorsum			(void);

/* variables */
static uint8_t gs_raw[1024] __attribute__((aligned(MM_CFG_ALIGNMENT)));
static mm_chunk_t *gs_chnk = (mm_chunk_t *)gs_raw;

/* functions's definitions */
static void eval_validate_xorsum(void)
{
	die_Expect("MM: xorsum");
	VERIFY_DIE_START
	
	mm_chunk_validate(gs_chnk);
	
	VERIFY_DIE_END
	die_Verify();
}

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(mm_chunk_validate);
TEST_GROUP_RUNNER(mm_chunk_validate)
{
	RUN_TEST_CASE(mm_chunk_validate, validate);
	RUN_TEST_CASE(mm_chunk_validate, validate_alignment);
	RUN_TEST_CASE(mm_chunk_validate, validate_out_of_bound);
	RUN_TEST_CASE(mm_chunk_validate, validate_overflow);
	RUN_TEST_CASE(mm_chunk_validate, validate_corruption_prev_size);
	RUN_TEST_CASE(mm_chunk_validate, validate_corruption_allocated);
	RUN_TEST_CASE(mm_chunk_validate, validate_corruption_xorsum);
	RUN_TEST_CASE(mm_chunk_validate, validate_corruption_size);
	RUN_TEST_CASE(mm_chunk_validate, validate_corruption_guard_offset);
	RUN_TEST_CASE(mm_chunk_validate, validate_corruption_allocator);
}
TEST_SETUP(mm_chunk_validate)
{
	mm_chunk_init(gs_chnk, NULL, 256);
	mm_chunk_boundary_set(gs_chnk, gs_chnk, 1);
}
TEST_TEAR_DOWN(mm_chunk_validate)
{
	memset(gs_raw, 0, 1024);
}

/* Tests ---------------------------------------------------------------------*/
TEST(mm_chunk_validate, validate)
{
	mm_chunk_validate(gs_chnk);
}

TEST(mm_chunk_validate, validate_alignment)
{
	die_Expect("MM: alignment");
	VERIFY_DIE_START
	mm_chunk_validate((mm_chunk_t *)((uintptr_t)gs_chnk + 1));
	VERIFY_DIE_END
	die_Verify();
}

TEST(mm_chunk_validate, validate_out_of_bound)
{
	die_Expect("MM: out of bound");
	VERIFY_DIE_START
	
	mm_chunk_validate((mm_chunk_t *)MM_CFG_ALIGNMENT);
	
	VERIFY_DIE_END
	die_Verify();
	
	die_Expect("MM: out of bound");
	VERIFY_DIE_START
	
	mm_chunk_validate((mm_chunk_t *)(gs_raw+MM_CFG_ALIGNMENT));
	
	VERIFY_DIE_END
	die_Verify();
}

TEST(mm_chunk_validate, validate_overflow)
{
	die_Expect("MM: overflowed");
	VERIFY_DIE_START

	memset(gs_raw + 512, 0, 512);
	mm_chunk_validate(gs_chnk);

	VERIFY_DIE_END
	die_Verify();
}

TEST(mm_chunk_validate, validate_corruption_prev_size)
{
	gs_chnk->prev_size = 32;
	eval_validate_xorsum();
}

TEST(mm_chunk_validate, validate_corruption_allocated)
{
	gs_chnk->allocated = true;
	eval_validate_xorsum();
}

TEST(mm_chunk_validate, validate_corruption_xorsum)
{
	gs_chnk->xorsum = 0;
	eval_validate_xorsum();
}

TEST(mm_chunk_validate, validate_corruption_size)
{
	gs_chnk->csize = 0;
	eval_validate_xorsum();
}

TEST(mm_chunk_validate, validate_corruption_guard_offset)
{
	gs_chnk->guard_offset = 42;
	eval_validate_xorsum();
}

TEST(mm_chunk_validate, validate_corruption_allocator)
{
	gs_chnk->allocator = __builtin_return_address(0);
	eval_validate_xorsum();
}

