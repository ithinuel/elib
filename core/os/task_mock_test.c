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
#include "unity_fixture.h"
#include "os/task.h"
#include "tests/task_mock.h"

/* Helpers -------------------------------------------------------------------*/
static void		task_mock_test_then_cbk		(void);

static uint32_t gs_task_mock_test_thencbk_cnt = 0;

static void task_mock_test_then_cbk(void)
{
	gs_task_mock_test_thencbk_cnt++;
}

/* Test group ----------------------------------------------------------------*/
TEST_GROUP(task_mock);

TEST_GROUP_RUNNER(task_mock)
{
	RUN_TEST_CASE(task_mock, fail_on_unexpected_delay);
	RUN_TEST_CASE(task_mock, fail_on_wrong_delay_argument);
	RUN_TEST_CASE(task_mock, expect_1_delay);
	RUN_TEST_CASE(task_mock, expect_n_delay);
	RUN_TEST_CASE(task_mock, thencbk_called_once_at_the_end);
	RUN_TEST_CASE(task_mock, verify_clean_expectation_stack);
}

TEST_SETUP(task_mock)
{
	task_mock_delay_ms_setup();
}

TEST_TEAR_DOWN(task_mock)
{
	task_mock_delay_ms_verify();
}

TEST(task_mock, fail_on_unexpected_delay)
{
	if (TEST_PROTECT()) {
		task_delay_ms(10);
	}

	TEST_ASSERT_EQUAL_MESSAGE(1, Unity.CurrentTestFailed,
				  "This test should have failed");
	if (Unity.CurrentTestFailed == 1) {
		Unity.CurrentTestFailed = 0;
		UnityPrint("===== this is an expected failure =====");
	}
}

TEST(task_mock, fail_on_wrong_delay_argument)
{
	task_delay_ms_ExpectNthenCbk(10, 1, NULL);
	if (TEST_PROTECT()) {
		task_delay_ms(5);
	}

	TEST_ASSERT_EQUAL_MESSAGE(1, Unity.CurrentTestFailed,
				  "This test should have failed");

	if (Unity.CurrentTestFailed == 1) {
		Unity.CurrentTestFailed = 0;
		UnityPrint("===== this is an expected failure =====");
		task_delay_ms(10);
	}
}

TEST(task_mock, expect_1_delay)
{
	task_delay_ms_ExpectNthenCbk(10, 1, NULL);
	task_delay_ms(10);
}

TEST(task_mock, expect_n_delay)
{
	task_delay_ms_ExpectNthenCbk(10, 25, NULL);
	for (uint32_t i = 0; i < 25; i++) {
		task_delay_ms(10);
	}
}

TEST(task_mock, thencbk_called_once_at_the_end)
{
	task_delay_ms_ExpectNthenCbk(10, 25, task_mock_test_then_cbk);
	for (uint32_t i = 0; i < 12; i++) {
		task_delay_ms(10);
	}
	TEST_ASSERT_EQUAL_UINT32(0, gs_task_mock_test_thencbk_cnt);
	for (uint32_t i = 0; i < 13; i++) {
		task_delay_ms(10);
	}
	TEST_ASSERT_EQUAL_UINT32(1, gs_task_mock_test_thencbk_cnt);
}

TEST(task_mock, verify_clean_expectation_stack)
{
	task_delay_ms_ExpectNthenCbk(10, 1, NULL);
	if (TEST_PROTECT()) {
		task_mock_delay_ms_verify();
	}

	TEST_ASSERT_EQUAL_MESSAGE(1, Unity.CurrentTestFailed,
				  "This test should have failed");

	if (Unity.CurrentTestFailed == 1) {
		Unity.CurrentTestFailed = 0;
		UnityPrint("===== this is an expected failure =====");
	}
}
