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

#include "tests/common_mock.h"

jmp_buf g_on_die;
static char *gs_expected_cause = NULL;

static void runAllTests()
{
	RUN_TEST_GROUP(mm_chunk);
	RUN_TEST_GROUP(memmgr);
	RUN_TEST_GROUP(cexcept);
	RUN_TEST_GROUP(object);
	RUN_TEST_GROUP(mutex);
	RUN_TEST_GROUP(spinlock);
	RUN_TEST_GROUP(memmgr_mock);
}

void die(const char *reason)
{
	if (gs_expected_cause != NULL) {
		TEST_ASSERT_EQUAL_STRING(gs_expected_cause, reason);
		gs_expected_cause= NULL;
		longjmp(g_on_die, 1);
	} else {
		TEST_FAIL_MESSAGE(reason);
		/* test fail may return on  teardown */
		exit(1);
	}
}

void die_Expect(char *expected_cause)
{
	gs_expected_cause = expected_cause;
}

void die_Verify(void)
{
	if (gs_expected_cause != NULL) {
		TEST_FAIL_MESSAGE("This test should have died");
	}
}

int main(int argc, char **argv, char **arge)
{
	return UnityMain(argc, argv, runAllTests);
}
