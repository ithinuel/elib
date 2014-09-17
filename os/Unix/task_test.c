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
#include <stdlib.h>
#include "unity_fixture.h"
#include "os/task.h"

static task_t *gs_tsk = NULL;
static volatile uint32_t gs_counter = 0;

static void task_test_routine(void *arg)
{
	while(!task_must_stop(gs_tsk))
	{
		gs_counter++;
		task_delay_ms(1);
	}
}

TEST_GROUP(task);
TEST_GROUP_RUNNER(task)
{
	RUN_TEST_CASE(task, to_string);
	RUN_TEST_CASE(task, alloc_failure_returns_null);
	RUN_TEST_CASE(task, delete_cancel_thread);
	RUN_TEST_CASE(task, count_running_tasks);
}

TEST_SETUP(task)
{
	gs_tsk = task_create(task_test_routine, NULL, 0, 0, "test_task");
	TEST_ASSERT_NOT_NULL(gs_tsk);
}

TEST_TEAR_DOWN(task)
{
	object_delete(&gs_tsk->base);
}

TEST(task, to_string)
{
	char *string = object_to_string(&gs_tsk->base);
	TEST_ASSERT_EQUAL_STRING("task: test_task", string);
	free(string);
}

TEST(task, alloc_failure_returns_null)
{
	UnityMalloc_MakeMallocFailAfterCount(0);
	TEST_ASSERT_NULL(task_create(NULL, NULL, 0, 0, "test"));
}

TEST(task, delete_cancel_thread)
{
	task_start(gs_tsk);
	uint32_t val1 = gs_counter;
	task_delay_ms(10);
	uint32_t val2 = gs_counter;
	TEST_ASSERT_MESSAGE((val1 != val2), "Task is not running");

	task_stop(gs_tsk);
	val1 = gs_counter;
	task_delay_ms(10);
	val2 = gs_counter;
	TEST_ASSERT_MESSAGE((val1 == val2), "Task is still running");

}

TEST(task, count_running_tasks)
{
	TEST_ASSERT_EQUAL_UINT32(1, task_running_count());
	task_start(gs_tsk);
	TEST_ASSERT_EQUAL_UINT32(2, task_running_count());
	task_stop(gs_tsk);
	TEST_ASSERT_EQUAL_UINT32(1, task_running_count());
}
