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
#include "tests/common_mock.h"
#include "tests/chunk_mock.h"
#include "tests/chunk_test_tools.h"
#include "memmgr/chunk.h"
#include "os/memmgr.h"
#include "memmgr_conf.h"

/* helpers -------------------------------------------------------------------*/
#define			DEFAULT_SIZE		(11)

static void *		mock_alloc		(uint32_t size);
static void		mock_alloc_Expect	(uint32_t size, void *ret);
static void *		mock_zalloc		(uint32_t size);
static void		mock_alloc_Setup	(void);
static void		mock_zalloc_Setup	(void);
static void		mock_Verify		(void);

static uint8_t gs_heap[1024*1024];
static struct
{
	bool	 expect_call;
	uint32_t expect_size;
	void	 *then_return;
} gs_mock_alloc;
static struct
{
	bool	 expect_call;
	uint32_t expect_size;
	void	 *then_return;
} gs_mock_zalloc;

static void *mock_alloc(uint32_t size)
{
	TEST_ASSERT_MESSAGE(gs_mock_alloc.expect_call, "Unexpected call to mock_alloc");
	TEST_ASSERT_EQUAL_UINT32(gs_mock_alloc.expect_size, size);
	gs_mock_alloc.expect_call = false;
	return gs_mock_alloc.then_return;
}

static void *mock_zalloc(uint32_t size)
{
	TEST_ASSERT_MESSAGE(gs_mock_zalloc.expect_call, "Unexpected call to mock_zalloc");
	TEST_ASSERT_EQUAL_UINT32(gs_mock_zalloc.expect_size, size);
	gs_mock_zalloc.expect_call = false;
	return gs_mock_zalloc.then_return;
}

static void mock_alloc_Expect(uint32_t size, void *ret)
{
	gs_mock_alloc.expect_call = true;
	gs_mock_alloc.expect_size = size;
	gs_mock_alloc.then_return = ret;
}

static void mock_zalloc_Expect(uint32_t size, void *ret)
{
	gs_mock_zalloc.expect_call = true;
	gs_mock_zalloc.expect_size = size;
	gs_mock_zalloc.then_return = ret;
}

static void mock_alloc_Setup(void)
{
	UT_PTR_SET(mm_alloc, mock_alloc);
	gs_mock_alloc.expect_call = false;
}

static void mock_zalloc_Setup(void)
{
	UT_PTR_SET(mm_zalloc, mock_zalloc);
	gs_mock_zalloc.expect_call = false;
}

static void mock_Verify(void)
{
	TEST_ASSERT_FALSE_MESSAGE(gs_mock_alloc.expect_call, "Missing call to mm_alloc");
	TEST_ASSERT_FALSE_MESSAGE(gs_mock_zalloc.expect_call, "Missing call to mm_zalloc");
}

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(memmgr);

TEST_GROUP_RUNNER(memmgr)
{
	RUN_TEST_GROUP(memmgr_alloc);
	RUN_TEST_GROUP(memmgr_free);
	RUN_TEST_GROUP(memmgr_realloc);

	RUN_TEST_CASE(memmgr, allocator_set);
	RUN_TEST_CASE(memmgr, allocator_set_null_does_not_hurt);

	RUN_TEST_CASE(memmgr, _zalloc);
	RUN_TEST_CASE(memmgr, zalloc_alloc_failed);

	RUN_TEST_CASE(memmgr, _calloc);
	RUN_TEST_CASE(memmgr, calloc_zalloc_failed);

	RUN_TEST_CASE(memmgr, init);
	RUN_TEST_CASE(memmgr, check);
}

TEST_SETUP(memmgr)
{
	chunk_test_state_t a_state[] = {{128, false}, {128, false}};
	chunk_test_prepare(a_state, 2);
}

TEST_TEAR_DOWN(memmgr)
{
	chunk_test_clear();
	mock_Verify();
}

/* Tests ---------------------------------------------------------------------*/
TEST(memmgr, allocator_set)
{
	void *ptr = mm_alloc(DEFAULT_SIZE);
	void *lr = __builtin_return_address(0);
	mm_chunk_t *chnk = mm_tochunk(ptr);

	mm_allocator_set(ptr, lr);

	mm_chunk_validate(chnk);
	TEST_ASSERT_EQUAL_PTR(lr, chnk->allocator);

	mm_allocator_set(ptr, NULL);
	mm_chunk_validate(chnk);
	TEST_ASSERT_EQUAL_PTR(NULL, chnk->allocator);
}

TEST(memmgr, allocator_set_null_does_not_hurt)
{
	mm_allocator_set(NULL, NULL);
}

TEST(memmgr, _zalloc)
{
	uint8_t *ptr = mm_alloc(DEFAULT_SIZE);

	mock_alloc_Setup();
	mock_alloc_Expect(DEFAULT_SIZE, ptr);
	TEST_ASSERT_EQUAL_PTR(ptr, mm_zalloc(DEFAULT_SIZE));
	for (int32_t i = 0; i < DEFAULT_SIZE; i++) {
		TEST_ASSERT_EQUAL_UINT8(0, ptr[i]);
	}
}

TEST(memmgr, zalloc_alloc_failed)
{
	mock_alloc_Setup();
	mock_alloc_Expect(DEFAULT_SIZE, NULL);
	TEST_ASSERT_NULL(mm_zalloc(DEFAULT_SIZE));
}


TEST(memmgr, _calloc)
{
	void *ptr = mm_zalloc(DEFAULT_SIZE*sizeof(uint32_t));

	mock_zalloc_Setup();
	mock_zalloc_Expect(DEFAULT_SIZE*sizeof(uint32_t), ptr);
	TEST_ASSERT_EQUAL_PTR(ptr, mm_calloc(DEFAULT_SIZE, sizeof(uint32_t)));
}

TEST(memmgr, calloc_zalloc_failed)
{
	mock_zalloc_Setup();
	mock_zalloc_Expect(DEFAULT_SIZE*sizeof(uint32_t), NULL);
	TEST_ASSERT_NULL(mm_calloc(DEFAULT_SIZE, sizeof(uint32_t)));
}

TEST(memmgr, init)
{
	chunk_test_state_t a_expect[] = {
			{9, true}, {UINT15_MAX-9, false},
			{UINT15_MAX, false}, {UINT15_MAX, false},
			{UINT15_MAX, false}, {UINT15_MAX, false},
			{UINT15_MAX, false}, {UINT15_MAX, false},
			{UINT15_MAX, false}, {8, false}};
	UT_PTR_SET(g_first, gs_heap);

	mm_init(gs_heap, 1024*1024);
	chunk_test_verify(a_expect, 10);
}

TEST(memmgr, check)
{
	mm_init(gs_heap, 1024*1024);
	mm_check();
}
