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

#include <stdlib.h>
#include <stdbool.h>
#include "unity_fixture.h"

#include "mcp/mcp.h"

static void mcp_entry(void);

system_entry_t g_mcp_entry  =
{
	.entry = mcp_entry,
	.stack_size = 512,
	.priority = 1
};

static void runAllTests()
{
	RUN_TEST_GROUP(mm_chunk);
	RUN_TEST_GROUP(memmgr);
	RUN_TEST_GROUP(cexcept);
	RUN_TEST_GROUP(object);
	RUN_TEST_GROUP(mutex);
	RUN_TEST_GROUP(spinlock);
	RUN_TEST_GROUP(stream);
	RUN_TEST_GROUP(task);
	RUN_TEST_GROUP(task_mock);
}

static void mcp_entry(void)
{
	UnityMain(0, NULL, runAllTests);
}
