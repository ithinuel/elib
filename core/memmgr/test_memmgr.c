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
#include "os/memmgr.h"

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr);

TEST_GROUP_RUNNER(memmgr)
{
	RUN_TEST_CASE(memmgr, alloc);
}

TEST_SETUP(memmgr)
{
	mm_init();
}

TEST_TEAR_DOWN(memmgr)
{

}

/* Tests ---------------------------------------------------------------------*/
TEST(memmgr, alloc)
{
	//void *ptr = mm_alloc(0);
	//TEST_ASSERT_NULL(ptr);
}
