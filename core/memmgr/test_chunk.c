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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "unity_fixture.h"
#include "tests/tests.h"
#include "memmgr/chunk.h"

/* helper functions ----------------------------------------------------------*/

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(mm_chunk);

TEST_GROUP_RUNNER(mm_chunk)
{
	RUN_TEST_CASE(mm_chunk, validate);
	RUN_TEST_CASE(mm_chunk, split);
	RUN_TEST_CASE(mm_chunk, merge);
	RUN_TEST_CASE(mm_chunk, aggregate);
}

TEST_SETUP(mm_chunk)
{
}

TEST_TEAR_DOWN(mm_chunk)
{
}

/* Tests ---------------------------------------------------------------------*/
TEST(mm_chunk, validate)
{
	TEST_FAIL();
}

TEST(mm_chunk, split)
{
	TEST_FAIL();
}

TEST(mm_chunk, merge)
{
	TEST_FAIL();
}

TEST(mm_chunk, aggregate)
{
	TEST_FAIL();
}

