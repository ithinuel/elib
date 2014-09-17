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
#include "tests/memmgr_unity.h"
#include "os/memmgr.h"
#include "os/mutex.h"
#include "os/task.h"

/*----------------------------------------------------------------------------*/
static mutex_t *gs_mtx = NULL;
static bool gs_test_mtx = false;

void test_mutex(void *arg)
{
	if (mutex_lock(gs_mtx, 50)) {
		gs_test_mtx = true;
		task_delay_ms(250);
		mutex_unlock(gs_mtx);
	}
}

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(mutex);

TEST_GROUP_RUNNER(mutex)
{
	RUN_TEST_CASE(mutex, to_string);
	RUN_TEST_CASE(mutex, lock_null_should_die);
	RUN_TEST_CASE(mutex, unlock_null_should_die);

	RUN_TEST_CASE(mutex, lock_once);
	RUN_TEST_CASE(mutex, lock_recursive);
	RUN_TEST_CASE(mutex, unlock_once);
	RUN_TEST_CASE(mutex, unlock_recursive);

	RUN_TEST_CASE(mutex, lock_fail_on_multithread);
}

TEST_SETUP(mutex)
{
	unity_mock_setup();
	gs_mtx = mutex_new(false, "unit_tests");
	gs_test_mtx = false;
}

TEST_TEAR_DOWN(mutex)
{
	object_delete(&gs_mtx->base);
	gs_mtx = NULL;
}

/* Tests ---------------------------------------------------------------------*/
TEST(mutex, to_string)
{
	char *string = object_to_string(&gs_mtx->base);
	TEST_ASSERT_EQUAL_STRING("unit_tests", string);
	mm_free(string);
}

TEST(mutex, lock_null_should_die)
{
	EXPECT_ABORT_BEGIN
	mutex_lock(NULL, -1);
	VERIFY_FAILS_END("null mutex");
}

TEST(mutex, unlock_null_should_die)
{
	EXPECT_ABORT_BEGIN
	mutex_unlock(NULL);
	VERIFY_FAILS_END("null mutex");
}

TEST(mutex, lock_once)
{
	TEST_ASSERT_TRUE(mutex_lock(gs_mtx, -1));
}

TEST(mutex, lock_recursive)
{
	TEST_ASSERT_TRUE(mutex_lock(gs_mtx, -1));
	TEST_ASSERT_TRUE(mutex_lock(gs_mtx, -1));
}

TEST(mutex, unlock_once)
{
	mutex_unlock(gs_mtx);
}

TEST(mutex, unlock_recursive)
{
	mutex_unlock(gs_mtx);
	mutex_unlock(gs_mtx);
}

TEST(mutex, lock_fail_on_multithread)
{
	task_t *tsk = task_create(test_mutex, NULL, 0, 0, "test_mutex");

	TEST_ASSERT_TRUE(mutex_lock(gs_mtx, -1));
	task_start(tsk);
	task_delay_ms(250);
	TEST_ASSERT_FALSE(gs_test_mtx);
	task_stop(tsk);
	mutex_unlock(gs_mtx);

	task_start(tsk);
	task_delay_ms(10);
	TEST_ASSERT_FALSE(mutex_lock(gs_mtx, 10));
	task_delay_ms(250);
	TEST_ASSERT_TRUE(gs_test_mtx);
	TEST_ASSERT_TRUE(mutex_lock(gs_mtx, -1));

	object_delete(&tsk->base);
}


