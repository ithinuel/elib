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
#include <stddef.h>
#include "tests/common_mock.h"
#include "tests/memmgr_mock.h"
#include "unity_fixture.h"

/* Macro definitions ---------------------------------------------------------*/

/* Type definitions ----------------------------------------------------------*/

/* Prototypes ----------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Private Functions definitions ---------------------------------------------*/
/* Functions definitions -----------------------------------------------------*/
TEST_GROUP(memmgr_mock);

TEST_GROUP_RUNNER(memmgr_mock)
{
	RUN_TEST_CASE(memmgr_mock, free_expect_alloc_failure);
	RUN_TEST_CASE(memmgr_mock, alloc_expect_alloc_failure);
}

TEST_SETUP(memmgr_mock)
{

}

TEST_TEAR_DOWN(memmgr_mock)
{

}

TEST(memmgr_mock, free_expect_alloc_failure)
{
	UnityMalloc_MakeMallocFailAfterCount(0);

	die_Expect("expect init failure");
	VERIFY_DIE_START
	mock_mm_free_Expect(NULL);
	VERIFY_DIE_END
	die_Verify();
}

TEST(memmgr_mock, alloc_expect_alloc_failure)
{
	UnityMalloc_MakeMallocFailAfterCount(0);

	die_Expect("expect init failure");
	VERIFY_DIE_START
	mock_mm_alloc_Expect(0);
	VERIFY_DIE_END
	die_Verify();

	die_Expect("expect init failure");
	VERIFY_DIE_START
	mock_mm_alloc_ExpectAndReturn(0, NULL);
	VERIFY_DIE_END
	die_Verify();
}
