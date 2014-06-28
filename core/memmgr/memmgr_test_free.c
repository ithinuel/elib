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
void *gs_ptr = NULL;

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr_free);

TEST_GROUP_RUNNER(memmgr_free)
{
	RUN_TEST_CASE(memmgr_free, free_null_does_no_harm);
	RUN_TEST_CASE(memmgr_free, free_);
	RUN_TEST_CASE(memmgr_free, double_free_leads_to_death);
	RUN_TEST_CASE(memmgr_free, free_can_merge_next_if_null);
	RUN_TEST_CASE(memmgr_free, free_can_merge_next_if_allocated);
	RUN_TEST_CASE(memmgr_free, free_can_merge_prev_if_null);
	RUN_TEST_CASE(memmgr_free, free_can_merge_prev_if_allocated);
}

TEST_SETUP(memmgr_free)
{
	chunk_test_state_t a_state[] = {{128, false}, {128, false}};
	chunk_test_prepare(a_state, 2);

	gs_ptr = mm_alloc(11);

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

TEST(memmgr_free, free_)
{
	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_free(gs_ptr);
	mm_chunk_validate(chnk);
	TEST_ASSERT_FALSE(chnk->allocated);
}

TEST(memmgr_free, double_free_leads_to_death)
{
	mm_chunk_t *chnk = mm_tochunk(gs_ptr);
	mm_free(gs_ptr);
	mm_chunk_validate(chnk);
	TEST_ASSERT_FALSE(chnk->allocated);

	die_Expect("MM: double free");
	VERIFY_DIE_START
	mm_free(gs_ptr);
	VERIFY_DIE_END
	die_Verify();
}

TEST(memmgr_free, free_can_merge_next_if_null)
{
	TEST_FAIL();
}

TEST(memmgr_free, free_can_merge_next_if_allocated)
{
	TEST_FAIL();
}

TEST(memmgr_free, free_can_merge_prev_if_null)
{
	TEST_FAIL();
}

TEST(memmgr_free, free_can_merge_prev_if_allocated)
{
	TEST_FAIL();
}
