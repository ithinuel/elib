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
#include "utils/cstring.h"


/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(cstring);

TEST_GROUP_RUNNER(cstring)
{
	RUN_TEST_CASE(cstring, dup_null_is_ok);
	RUN_TEST_CASE(cstring, dup_can_be_freed);
}

TEST_SETUP(cstring)
{
	unity_mock_setup();
}

TEST_TEAR_DOWN(cstring)
{

}

/* Tests ---------------------------------------------------------------------*/
TEST(cstring, dup_null_is_ok)
{
	TEST_ASSERT_NULL(cstring_dup(NULL));
}

TEST(cstring, dup_can_be_freed)
{
	char *s = cstring_dup("Bonjour");
	TEST_ASSERT_EQUAL_STRING("Bonjour", s);
	mm_free(s);
}
