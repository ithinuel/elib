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
#include "common/common.h"
#include "tests/chunk_mock.h"

/* Types ---------------------------------------------------------------------*/
typedef struct _mock_call	mock_call_t;
typedef enum
{
	mock_call_type_none,
	mock_call_type_merge,
	mock_call_type_split,
	mock_call_type_find_first_free,
	mock_call_type_validate_csize
} mock_call_type_e;

struct _mock_call
{
	mock_call_t 		*next;
	mock_call_type_e	type;
	union
	{
		struct
		{
			mm_chunk_t	*expect_this;
		} merge;
		struct
		{
			mm_chunk_t	*expect_this;
			uint16_t	expect_csize;
			bool		then_return_null;
		} split;
		struct
		{
			uint16_t	expect_csize;
			mm_chunk_t	*then_return;
		} find_first_free;
		struct
		{
			uint16_t	expect_min_csize;
			uint32_t	expect_csize;
			bool		then_return;
		} validate_csize;
	} call;
};

/* Prototypes ----------------------------------------------------------------*/
void *			unity_malloc				(size_t size);
void			unity_free				(void *ptr);

static mm_chunk_t *	mock_mm_find_first_free			(uint16_t wanted_csize);
static mm_chunk_t *	mock_mm_chunk_split			(mm_chunk_t *this,
								 uint16_t csize);
static void		mock_mm_chunk_merge			(mm_chunk_t *this);
static bool		mock_mm_validate_csize			(uint16_t min_csize,
								 uint32_t csize);

static mock_call_type_e mock_expect				(void);
static void		mock_push				(mock_call_t *new);
static void		mock_pop				(void);
static void		mock_clean				(void);

/* Variables -----------------------------------------------------------------*/
static mock_call_t *gs_mock_expect = NULL;
static mock_call_t *gs_mock_last = NULL;

static mm_chunk_split_f gs_chunk_split = NULL;
static mm_chunk_merge_f gs_chunk_merge = NULL;

/* Private definitions -------------------------------------------------------*/
static mm_chunk_t *mock_mm_find_first_free(uint16_t wanted_csize)
{
	TEST_ASSERT_MESSAGE(mock_expect() == mock_call_type_find_first_free,
			    "Unexpected call to mm_find_first_free.");
	TEST_ASSERT_EQUAL_UINT16(gs_mock_expect->call.find_first_free.expect_csize, wanted_csize);

	mm_chunk_t *ret = gs_mock_expect->call.find_first_free.then_return;
	mock_pop();
	return ret;
}

static mm_chunk_t *mock_mm_chunk_split(mm_chunk_t *this, uint16_t csize)
{
	TEST_ASSERT_MESSAGE(mock_expect() == mock_call_type_split,
			    "Unexpected call to mm_chunk_split.");

	TEST_ASSERT_EQUAL_PTR(gs_mock_expect->call.split.expect_this, this);
	TEST_ASSERT_EQUAL_UINT16(gs_mock_expect->call.split.expect_csize, csize);

	bool then_return_null = gs_mock_expect->call.split.then_return_null;
	mock_pop();
	if (then_return_null) {
		return NULL;
	}
	return gs_chunk_split(this, csize);
}

static void mock_mm_chunk_merge(mm_chunk_t *this)
{
	TEST_ASSERT_MESSAGE(mock_expect() == mock_call_type_merge,
			    "Unexpected call to mm_chunk_merge.");
	TEST_ASSERT_EQUAL_PTR(gs_mock_expect->call.merge.expect_this, this);

	mock_pop();

	gs_chunk_merge(this);
}

static bool mock_mm_validate_csize(uint16_t min_csize, uint32_t csize)
{
	TEST_ASSERT_MESSAGE(mock_expect() == mock_call_type_validate_csize,
			    "Unexpected call to mm_validate_csize.");
	TEST_ASSERT_EQUAL_UINT16(gs_mock_expect->call.validate_csize.expect_min_csize, min_csize);
	TEST_ASSERT_EQUAL_UINT32(gs_mock_expect->call.validate_csize.expect_csize, csize);
	bool ret = gs_mock_expect->call.validate_csize.then_return;
	mock_pop();
	return ret;
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

/* Definitions ---------------------------------------------------------------*/
void mock_chunk_setup(void)
{
	gs_chunk_split = mm_chunk_split;
	gs_chunk_merge = mm_chunk_merge;
	UT_PTR_SET(mm_find_first_free, mock_mm_find_first_free);
	UT_PTR_SET(mm_chunk_merge, mock_mm_chunk_merge);
	UT_PTR_SET(mm_chunk_split, mock_mm_chunk_split);
	UT_PTR_SET(mm_validate_csize, mock_mm_validate_csize);
}

void mock_chunk_verify(void)
{
	bool success = gs_mock_expect == NULL;
	mock_clean();
	TEST_ASSERT_MESSAGE(success, "Calls were still expected");
}

void mock_mm_find_first_free_ExpectAndReturn(uint16_t wanted_csize, mm_chunk_t *ret)
{
	mock_call_t *new = unity_malloc(sizeof(mock_call_t));
	if (new == NULL) {
		die("expect init failure");
	}
	new->type = mock_call_type_find_first_free;
	new->call.find_first_free.expect_csize = wanted_csize;
	new->call.find_first_free.then_return = ret;
	new->next = NULL;

	mock_push(new);
}

void mock_mm_chunk_split_ExpectAndReturn(mm_chunk_t *this, uint16_t csize, bool return_null)
{
	mock_call_t *new = unity_malloc(sizeof(mock_call_t));
	if (new == NULL) {
		die("expect init failure");
	}
	new->type = mock_call_type_split;
	new->call.split.expect_this = this;
	new->call.split.expect_csize = csize;
	new->call.split.then_return_null = return_null;
	new->next = NULL;

	mock_push(new);
}

void mock_mm_chunk_merge_Expect(mm_chunk_t *this)
{
	mock_call_t *new = unity_malloc(sizeof(mock_call_t));
	if (new == NULL) {
		die("expect init failure");
	}
	new->type = mock_call_type_merge;
	new->call.merge.expect_this = this;
	new->next = NULL;

	mock_push(new);
}

void mock_mm_validate_csize_ExpectAndReturn(uint16_t min_csize, uint32_t csize, bool then_return)
{
	mock_call_t *new = unity_malloc(sizeof(mock_call_t));
	if (new == NULL) {
		die("expect init failure");
	}
	new->type = mock_call_type_validate_csize;
	new->call.validate_csize.expect_min_csize = min_csize;
	new->call.validate_csize.expect_csize = csize;
	new->call.validate_csize.then_return = then_return;
	new->next = NULL;

	mock_push(new);
}
