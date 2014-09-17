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
#include "common/common.h"
#include "os/memmgr.h"
#include "tests/memmgr_mock.h"
#include "unity_fixture.h"

/* Macro definitions ---------------------------------------------------------*/

/* Type definitions ----------------------------------------------------------*/
typedef struct _mock_call	mock_call_t;
typedef enum
{
	mock_call_type_none,
	mock_call_type_alloc,
	mock_call_type_zalloc,
	mock_call_type_free,
} mock_call_type_e;

struct _mock_call
{
	mock_call_t 		*next;
	mock_call_type_e	type;
	union
	{
		struct
		{
			bool		do_expect;
			uint32_t	expect_size;
			bool		then_return_set;
			void		*then_return;
		} alloc;
		struct
		{
			void		*expect_ptr;
		} free_;
	} call;
};

/* Prototypes ----------------------------------------------------------------*/
void *			unity_malloc				(size_t size);
void			unity_free				(void *ptr);

static void *		mock_mm_alloc				(uint32_t size);
static void		mock_mm_alloc_expect_internal		(bool do_expect,
								 uint32_t expect_size,
								 bool then_return_set,
								 void *then_return);
static void		mock_mm_free				(void *ptr);

static mock_call_type_e mock_expect				(void);
static void		mock_push				(mock_call_t *new);
static void		mock_pop				(void);
static void		mock_clean				(void);

/* Variables -----------------------------------------------------------------*/
static mm_alloc_f gs_mm_alloc = NULL;
static mm_free_f gs_mm_free = NULL;

static mock_call_t *gs_mock_expect = NULL;
static mock_call_t *gs_mock_last = NULL;

static uint32_t gs_setup_counter = 0;

/* Private Functions definitions ---------------------------------------------*/
static void *mock_mm_alloc(uint32_t size)
{
	TEST_ASSERT_MESSAGE(mock_expect() == mock_call_type_alloc,
			    "Unexpected call to mm_alloc.");

	if (gs_mock_expect->call.alloc.do_expect) {
		TEST_ASSERT_EQUAL_UINT32(gs_mock_expect->call.alloc.expect_size, size);
	}
	void *ptr = gs_mock_expect->call.alloc.then_return;
	if (!gs_mock_expect->call.alloc.then_return_set) {
		ptr = gs_mm_alloc(size);
	}

	mock_pop();
	return ptr;
}

static void mock_mm_alloc_expect_internal(bool do_expect, uint32_t expect_size,
					  bool then_return_set, void *then_return)
{
	mock_call_t *new = unity_malloc(sizeof(mock_call_t));
	if (new == NULL) {
		die("expect init failure");
	}
	new->type = mock_call_type_alloc;
	new->call.alloc.do_expect = do_expect;
	new->call.alloc.expect_size = expect_size;
	new->call.alloc.then_return_set = then_return_set;
	new->call.alloc.then_return = then_return;
	new->next = NULL;

	mock_push(new);
}

static void mock_mm_free(void *ptr)
{
	TEST_ASSERT_MESSAGE(mock_expect() == mock_call_type_free,
			    "Unexpected call to mm_free.");
	TEST_ASSERT_EQUAL_PTR(gs_mock_expect->call.free_.expect_ptr, ptr);
	mock_pop();
}

static mock_call_type_e mock_expect(void)
{
	if (gs_mock_expect == NULL) {
		return mock_call_type_none;
	}
	return gs_mock_expect->type;
}

static void mock_push(mock_call_t *new)
{
	if (gs_mock_expect == NULL) {
		gs_mock_expect = new;
	}

	if (gs_mock_last != NULL) {
		gs_mock_last->next = new;
	}
	gs_mock_last = new;
}

static void mock_pop(void)
{
	mock_call_t *next = gs_mock_expect->next;
	unity_free(gs_mock_expect);
	gs_mock_expect = next;
	if (gs_mock_expect == NULL) {
		gs_mock_last = NULL;
	}
}

static void mock_clean(void)
{
	while (gs_mock_expect != NULL) {
		mock_pop();
	}
}

/* Functions definitions -----------------------------------------------------*/
void mock_memmgr_setup(void)
{
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, gs_setup_counter,
			"API is not clean, missing a call to mock_memmgr_verify"),
	gs_setup_counter++;

	gs_mm_alloc = mm_alloc;
	gs_mm_free = mm_free;
	mm_alloc = mock_mm_alloc;
	mm_free = mock_mm_free;
}

void mock_mm_alloc_Expect(uint32_t size)
{
	mock_mm_alloc_expect_internal(true, size, false, NULL);
}

void mock_mm_alloc_ExpectAndReturn(uint32_t size, void *ptr)
{
	mock_mm_alloc_expect_internal(true, size, true, ptr);
}

void mock_mm_alloc_IgnoreAndReturn(void *ret)
{
	mock_mm_alloc_expect_internal(false, 0, true, ret);
}

void mock_mm_free_Expect(void *ptr)
{
	mock_call_t *new = unity_malloc(sizeof(mock_call_t));
	if (new == NULL) {
		die("expect init failure");
	}
	new->type = mock_call_type_free;
	new->call.free_.expect_ptr = ptr;
	new->next = NULL;

	mock_push(new);
}

void mock_memmgr_verify(void)
{
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, gs_setup_counter,
			"Unexpected call to mock_memmgr_verify"),
	gs_setup_counter--;

	bool success = gs_mock_expect == NULL;
	mock_clean();
	mm_alloc = gs_mm_alloc;
	mm_free = gs_mm_free;
	TEST_ASSERT_MESSAGE(success, "Calls were still expected.");
}
