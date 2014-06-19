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

#include "unity_fixture.h"
#include "tests/tests.h"
#include "os/memmgr.h"
#include "memmgr_conf.h"

/* helper functions ----------------------------------------------------------*/
void eval_not_null_aligned_and_filled(void *ptr, int32_t size, uint8_t val, char *msg)
{
	TEST_ASSERT_NOT_NULL(ptr);
	TEST_ASSERT(((uintptr_t)ptr % MM_CFG_ALIGNMENT) == 0);
	for(int32_t i = 0; i < size; i++) {
		if (((uint8_t *)ptr)[i] != val) {
			TEST_FAIL_MESSAGE(msg);
		}
	}
}

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr);

TEST_GROUP_RUNNER(memmgr)
{
	RUN_TEST_CASE(memmgr, zalloc_free);
	RUN_TEST_CASE(memmgr, overflow);
	RUN_TEST_CASE(memmgr, realloc_expand);
	RUN_TEST_CASE(memmgr, realloc_shrink);
	RUN_TEST_CASE(memmgr, infos);
}

TEST_SETUP(memmgr)
{
	mm_init();
}

TEST_TEAR_DOWN(memmgr)
{
	mm_check();
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
	eval_not_null_aligned_and_filled(ptr1, 32, 0, "zalloc does not fill with 0");
	eval_not_null_aligned_and_filled(ptr2, 512, 0, "zalloc does not fill with 0");

	memset(ptr1, 55, 32);
	memset(ptr2, 55, 512);

	mm_free(ptr1);
	mm_free(ptr2);

	ptr1 = mm_zalloc(31);
	TEST_ASSERT_NOT_NULL(ptr1);
	ptr2 = mm_zalloc(130877);
	TEST_ASSERT_NOT_NULL(ptr2);
	ptr3 = mm_zalloc(29);
	TEST_ASSERT_NOT_NULL(ptr3);
	ptr4 = mm_zalloc(22);
	TEST_ASSERT_NOT_NULL(ptr4);
	ptr5 = mm_zalloc(131048);
	TEST_ASSERT_NOT_NULL(ptr5);
	TEST_ASSERT_NULL(mm_zalloc(1));

	mm_free(ptr5);
	mm_free(ptr2);
	mm_free(ptr3);
	mm_free(ptr4);
	mm_free(ptr1);

	ptr1 = mm_zalloc(32);
	ptr2 = mm_zalloc(512);
	ptr3 = mm_zalloc(128);
	ptr4 = mm_zalloc(12);

	mm_free(ptr2);

	ptr2 = mm_zalloc(128);

	mm_free(ptr3);
	mm_free(ptr2);
	mm_free(ptr4);
	mm_free(ptr1);
}

TEST(memmgr, overflow)
{
	uint8_t *ptr1 = mm_zalloc(35);
	eval_not_null_aligned_and_filled(ptr1, 35, 0, "zalloc does not fill with 0");

	memset(ptr1, 50, 50);

	die_Expect();
	VERIFY_DIE_START
	mm_free(ptr1);
	VERIFY_DIE_END
	die_Verify();

	/* reset for tear down mm_check */
	mm_init();
}

TEST(memmgr, realloc_expand)
{
	void *ptr1 = mm_zalloc(11);
	memset(ptr1, 'a', 11);
	ptr1 = mm_realloc(ptr1, 50);
	eval_not_null_aligned_and_filled(ptr1, 11, 'a', "data has been lost");
	memset(ptr1, 'a', 50);
	ptr1 = mm_realloc(ptr1, 100);
	eval_not_null_aligned_and_filled(ptr1, 50, 'a', "data has been lost");
	memset(ptr1, 'a', 100);

	void *ptr2 = mm_zalloc(12);
	ptr1 = mm_realloc(ptr1, 200);
	eval_not_null_aligned_and_filled(ptr1, 100, 'a', "data has been lost");

	TEST_ASSERT_NULL(mm_realloc(ptr1, 2*1024*1024));

	mm_free(ptr2);
	mm_free(ptr1);
}

TEST(memmgr, realloc_shrink)
{
	void *ptr1 = mm_zalloc(1024);
	void *ptr2 = mm_zalloc(12);
	void *ptr3 = mm_zalloc(1024);
	void *ptr4 = mm_zalloc(12);

	mm_free(ptr1);
	memset(ptr3, 'a', 1024);
	ptr3 = mm_realloc(ptr3, 512);
	memset(ptr3, 'a', 512);

	mm_free(ptr2);
	mm_free(ptr3);
	mm_free(ptr4);
}

TEST(memmgr, infos)
{
	uint32_t base = MM_CFG_HEAP_SIZE/(UINT15_MAX*MM_CFG_ALIGNMENT);
	TEST_ASSERT_EQUAL_INT(base, mm_nb_chunk());

	mm_stats_t *stats = mm_calloc(3, sizeof(mm_stats_t));
	mm_allocator_set(stats);
	TEST_ASSERT_NOT_NULL(stats);
	TEST_ASSERT_EQUAL_INT(base+1, mm_nb_chunk());

	mm_chunk_info(stats, base+1);
	for (int32_t i = 0; i < (base+1); i++) {
		if (i == 0) {
			TEST_ASSERT_TRUE(stats[i].allocated);
			TEST_ASSERT_EQUAL_UINT32(sizeof(mm_stats_t)*(base+1), stats[i].size);
		} else {
			TEST_ASSERT_FALSE(stats[i].allocated);
			TEST_ASSERT_EQUAL_UINT32(0, stats[i].size);
		}
	}
	mm_free(stats);
	TEST_ASSERT_EQUAL_INT(base, mm_nb_chunk());
}
