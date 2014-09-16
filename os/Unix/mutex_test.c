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

/*----------------------------------------------------------------------------*/
static mutex_t *gs_mtx = NULL;

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(mutex);

TEST_GROUP_RUNNER(mutex)
{
	RUN_TEST_CASE(mutex, to_string);
	RUN_TEST_CASE(mutex, lock_null_should_die);
	RUN_TEST_CASE(mutex, unlock_null_should_die);
}

TEST_SETUP(mutex)
{
	unity_mock_setup();
	gs_mtx = mutex_new(false, "unit_tests");

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
