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
#include "os/spinlock.h"
#include "tests/task_mock.h"
#include "unity_fixture.h"

spinlock_declare(lock);

void helper_end_loop(void)
{
	spinlock_unlock(&lock);
}

TEST_GROUP(spinlock);

TEST_GROUP_RUNNER(spinlock)
{
	RUN_TEST_CASE(spinlock, unlock);
	RUN_TEST_CASE(spinlock, lock);
}

TEST_SETUP(spinlock)
{
	TEST_ASSERT_FALSE(lock.is_locked);
	lock.is_locked = true;
	TEST_ASSERT_TRUE(lock.is_locked);
}

TEST_TEAR_DOWN(spinlock)
{

}

TEST(spinlock, unlock)
{
	spinlock_unlock(&lock);
	TEST_ASSERT_FALSE(lock.is_locked);
}

TEST(spinlock, lock)
{
	task_delay_ms_setup();

	TEST_ASSERT_FALSE(spinlock_lock(&lock, 0, 1));

	task_delay_ms_ExpectNthenCbk(10, 3, helper_end_loop);
	TEST_ASSERT_TRUE(spinlock_lock(&lock, 50, 10));

	task_delay_ms_ExpectNthenCbk(10, 5, NULL);
	TEST_ASSERT_FALSE(spinlock_lock(&lock, 50, 10));

	task_delay_ms_verify();
}
