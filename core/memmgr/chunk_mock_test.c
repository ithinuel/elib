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
#include <stddef.h>
#include "os/memmgr.h"
#include "tests/common_mock.h"
#include "tests/chunk_mock.h"
#include "unity_fixture.h"

static uint32_t gs_mm_alloc_expect_size = 0;
static bool gs_mm_alloc_called = false;

TEST_GROUP(chunk_mock);

TEST_GROUP_RUNNER(chunk_mock)
{
	RUN_TEST_CASE(chunk_mock, alloc_failure_on_expect_find_first_free);
	RUN_TEST_CASE(chunk_mock, alloc_failure_on_expect_validate_csize);
	RUN_TEST_CASE(chunk_mock, alloc_failure_on_expect_split);
	RUN_TEST_CASE(chunk_mock, alloc_failure_on_expect_merge);
	RUN_TEST_CASE(chunk_mock, verify_clean_remaining_expectations);
	RUN_TEST_CASE(chunk_mock, fail_on_unexpected_call_to_split);
	RUN_TEST_CASE(chunk_mock, fail_on_unexpected_call_to_merge);
	RUN_TEST_CASE(chunk_mock, fail_on_unexpected_call_to_validate_csize);
	RUN_TEST_CASE(chunk_mock, fail_on_unexpected_call_to_find_first_free);
}

TEST_SETUP(chunk_mock)
{
	gs_mm_alloc_expect_size = 0;
	gs_mm_alloc_called = false;
	mock_chunk_setup();
}

TEST_TEAR_DOWN(chunk_mock)
{
	mock_chunk_verify();
}

TEST(chunk_mock, alloc_failure_on_expect_find_first_free)
{
	UnityMalloc_MakeMallocFailAfterCount(0);
	EXPECT_ABORT_BEGIN
	mock_mm_find_first_free_ExpectAndReturn(0, NULL);
	VERIFY_FAILS_END("expect init failure");
}

TEST(chunk_mock, alloc_failure_on_expect_validate_csize)
{
	UnityMalloc_MakeMallocFailAfterCount(0);
	EXPECT_ABORT_BEGIN
	mock_mm_validate_csize_ExpectAndReturn(0, 0, false);
	VERIFY_FAILS_END("expect init failure");
}

TEST(chunk_mock, alloc_failure_on_expect_split)
{
	UnityMalloc_MakeMallocFailAfterCount(0);

	EXPECT_ABORT_BEGIN
	mock_mm_chunk_split_ExpectAndReturn(NULL, 0, true);
	VERIFY_FAILS_END("expect init failure");
}

TEST(chunk_mock, alloc_failure_on_expect_merge)
{
	UnityMalloc_MakeMallocFailAfterCount(0);

	EXPECT_ABORT_BEGIN
	mock_mm_chunk_merge_Expect(NULL);
	VERIFY_FAILS_END("expect init failure");
}

TEST(chunk_mock, verify_clean_remaining_expectations)
{
	mock_mm_chunk_merge_Expect(NULL);
	EXPECT_ABORT_BEGIN
	mock_chunk_verify();
	VERIFY_FAILS_END("Calls were still expected");
}

TEST(chunk_mock, fail_on_unexpected_call_to_split)
{
	EXPECT_ABORT_BEGIN
	mm_chunk_split(NULL, 0);
	VERIFY_FAILS_END("Unexpected call to mm_chunk_split.");
}

TEST(chunk_mock, fail_on_unexpected_call_to_merge)
{
	EXPECT_ABORT_BEGIN
	mm_chunk_merge(NULL);
	VERIFY_FAILS_END("Unexpected call to mm_chunk_merge.");
}

TEST(chunk_mock, fail_on_unexpected_call_to_validate_csize)
{
	EXPECT_ABORT_BEGIN
	mm_validate_csize(0, 0);
	VERIFY_FAILS_END("Unexpected call to mm_validate_csize.");
}

TEST(chunk_mock, fail_on_unexpected_call_to_find_first_free)
{
	EXPECT_ABORT_BEGIN
	mm_find_first_free(0);
	VERIFY_FAILS_END("Unexpected call to mm_find_first_free.");
}
