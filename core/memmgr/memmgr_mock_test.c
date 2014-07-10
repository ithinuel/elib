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
#include "tests/memmgr_mock.h"
#include "unity_fixture.h"

static bool gs_this_test_should_fail = false;
static uint32_t gs_mm_alloc_expect_size = 0;
static bool gs_mm_alloc_called = false;

static void *test_mock_mm_alloc(uint32_t size)
{
	TEST_ASSERT_EQUAL_UINT32(gs_mm_alloc_expect_size, size);
	gs_mm_alloc_called = true;

	return &gs_mm_alloc_expect_size;
}

TEST_GROUP(memmgr_mock);

TEST_GROUP_RUNNER(memmgr_mock)
{
	RUN_TEST_CASE(memmgr_mock, free_expect_alloc_failure);
	RUN_TEST_CASE(memmgr_mock, alloc_expect_alloc_failure);
	RUN_TEST_CASE(memmgr_mock, verify_clean_remaining_expectations);
	RUN_TEST_CASE(memmgr_mock, fail_on_unexpected_call_to_alloc);
	RUN_TEST_CASE(memmgr_mock, fail_on_unexpected_call_to_free);

	RUN_TEST_CASE(memmgr_mock, mock_mm_alloc_should_call_saved_cbk_if_return_not_set);
}

TEST_SETUP(memmgr_mock)
{
	gs_mm_alloc_expect_size = 0;
	gs_mm_alloc_called = false;
	gs_this_test_should_fail = false;
	mock_memmgr_setup();
}

TEST_TEAR_DOWN(memmgr_mock)
{
	mock_memmgr_verify();

	if (gs_this_test_should_fail) {
		TEST_ASSERT_MESSAGE((Unity.CurrentTestFailed == 1),
				"This test should have failed but did not.");
		Unity.CurrentTestFailed = 0;
	}
}

TEST(memmgr_mock, free_expect_alloc_failure)
{
	UnityMalloc_MakeMallocFailAfterCount(0);

	die_Expect("expect init failure");
	VERIFY_DIE_START
	mock_mm_free_Expect(NULL);
	VERIFY_DIE_END
	die_Verify();
}

TEST(memmgr_mock, alloc_expect_alloc_failure)
{
	UnityMalloc_MakeMallocFailAfterCount(0);

	die_Expect("expect init failure");
	VERIFY_DIE_START
	mock_mm_alloc_Expect(0);
	VERIFY_DIE_END
	die_Verify();

	die_Expect("expect init failure");
	VERIFY_DIE_START
	mock_mm_alloc_ExpectAndReturn(0, NULL);
	VERIFY_DIE_END
	die_Verify();
}

TEST(memmgr_mock, verify_clean_remaining_expectations)
{
	mock_mm_alloc_Expect(0);
	if (TEST_PROTECT()) {
		mock_memmgr_verify();
	}
}

TEST(memmgr_mock, fail_on_unexpected_call_to_alloc)
{
	gs_this_test_should_fail = true;
	if (TEST_PROTECT())
	{
		mm_alloc(0);
	}
}

TEST(memmgr_mock, fail_on_unexpected_call_to_free)
{
	gs_this_test_should_fail = true;
	if (TEST_PROTECT())
	{
		mm_free(0);
	}
}

TEST(memmgr_mock, mock_mm_alloc_should_call_saved_cbk_if_return_not_set)
{
	UT_PTR_SET(mm_alloc, test_mock_mm_alloc);
	mock_memmgr_setup();
	gs_mm_alloc_expect_size = 32;
	mock_mm_alloc_Expect(32);

	TEST_ASSERT_EQUAL_PTR(&gs_mm_alloc_expect_size, mm_alloc(32));

	TEST_ASSERT_TRUE_MESSAGE(gs_mm_alloc_called, "saved cbk was not called");
}
