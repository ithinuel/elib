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
#include "unity_fixture.h"
#include "tests/tests.h"
#include "os/memmgr.h"
#include "memmgr_conf.h"

/* helper functions ----------------------------------------------------------*/
void eval_not_null_aligned_and_zero_filled(void *ptr, int32_t size)
{
	TEST_ASSERT_NOT_NULL(ptr);
	TEST_ASSERT(((uintptr_t)ptr % MM_CFG_ALIGNMENT) == 0);
	for(int32_t i = 0; i < size; i++) {
		TEST_ASSERT(((uint8_t*)ptr)[i] == 0);
	}
}

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr);

TEST_GROUP_RUNNER(memmgr)
{
	RUN_TEST_CASE(memmgr, zalloc_free);
	RUN_TEST_CASE(memmgr, overflow);
}

TEST_SETUP(memmgr)
{
	mm_init();
}

TEST_TEAR_DOWN(memmgr)
{

}

/* Tests ---------------------------------------------------------------------*/
TEST(memmgr, zalloc_free)
{
	void *ptr5 = NULL;
	void *ptr4 = NULL;
	void *ptr3 = NULL;
	void *ptr2 = NULL;
	void *ptr1 = mm_zalloc(0);
	TEST_ASSERT_NULL(ptr1);

	ptr1 = mm_zalloc(32);
	ptr2 = mm_zalloc(512);
	TEST_ASSERT_NOT_EQUAL(ptr1, ptr2);
	eval_not_null_aligned_and_zero_filled(ptr1, 32);
	eval_not_null_aligned_and_zero_filled(ptr2, 512);

	memset(ptr1, 55, 32);
	memset(ptr2, 55, 512);

	mm_free(ptr1);
	mm_free(ptr2);

	ptr1 = mm_zalloc(32);
	ptr2 = mm_zalloc(131012);
	ptr3 = mm_zalloc(32);
	ptr4 = mm_zalloc(22);
	ptr5 = mm_zalloc(130976);
	TEST_ASSERT_NULL(mm_zalloc(1));

	mm_free(ptr5);
	mm_free(ptr2);
	mm_free(ptr3);
	mm_free(ptr4);
	mm_free(ptr1);
}

TEST(memmgr, overflow)
{
	uint8_t *ptr1 = mm_zalloc(35);
	eval_not_null_aligned_and_zero_filled(ptr1, 35);

	memset(ptr1, 50, 50);

	die_Expect();
	VERIFY_DIE_START
	mm_free(ptr1);
	VERIFY_DIE_END
	die_Verify();
}
