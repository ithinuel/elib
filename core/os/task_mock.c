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

#include <stddef.h>
#include "unity_fixture.h"
#include "os/task.h"
#include "tests/task_mock.h"

typedef struct _task_delay_call_t task_delay_call_t;

struct _task_delay_call_t
{
	int32_t			expect_ms;
	delegate_f		then_cbk;
	task_delay_call_t	*next;
};

void *unity_malloc(size_t size);
void unity_free(void *ptr);

static void mock_task_delay_ms(int32_t ms);

static task_delay_call_t *gs_first = NULL;
static task_delay_call_t *gs_last = NULL;

static void mock_task_delay_ms(int32_t ms)
{
	TEST_ASSERT_NOT_NULL_MESSAGE(gs_first, "Unexpected call to task_delay_ms");
	TEST_ASSERT_EQUAL_INT32(gs_first->expect_ms, ms);
	if (gs_first->then_cbk != NULL) {
		gs_first->then_cbk();
	}

	task_delay_call_t *next = gs_first->next;
	unity_free(gs_first);
	gs_first = next;
	if (gs_first == NULL) {
		gs_last = NULL;
	}
}

static void task_delay_ms_ExpectthenCbk(uint32_t ms, delegate_f cbk)
{
	task_delay_call_t *new = unity_malloc(sizeof(task_delay_call_t));
	new->expect_ms = ms;
	new->then_cbk = cbk;
	new->next = NULL;

	if (gs_first == NULL) {
		gs_first = new;
	}
	if (gs_last != NULL) {
		gs_last->next = new;
	}
	gs_last = new;
}

void task_delay_ms_setup(void)
{
	UT_PTR_SET(task_delay_ms, mock_task_delay_ms);
}

void task_delay_ms_ExpectNthenCbk(uint32_t ms, uint32_t times, delegate_f cbk)
{
	if (times == 0) {
		return;
	}

	for (int i = 0; i < (times-1); i++) {
		task_delay_ms_ExpectthenCbk(ms, NULL);
	}
	task_delay_ms_ExpectthenCbk(ms, cbk);
}

void task_delay_ms_verify(void)
{
	TEST_ASSERT_NULL_MESSAGE(gs_last, "missing call to task_delay_ms");
}
