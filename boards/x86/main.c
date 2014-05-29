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

#include <stdbool.h>
#include "unity_fixture.h"

#include "tests/tests.h"

static void runAllTests()
{
	RUN_TEST_GROUP(memmgr);
	RUN_TEST_GROUP(cexcept);
}

static bool gs_die_expected = false;
void die(const char *reason)
{
	if (gs_die_expected) {
		gs_die_expected = false;
		longjmp(g_on_die, 1);
	} else {
		TEST_FAIL_MESSAGE(reason);
	}
}

void die_Expect(void)
{
	gs_die_expected = true;
}

void die_Verify(void)
{
	if (gs_die_expected) {
		TEST_FAIL_MESSAGE("This test should have died");
	}
}

int main(int argc, char **argv, char **arge)
{
	return UnityMain(argc, argv, runAllTests);
}
