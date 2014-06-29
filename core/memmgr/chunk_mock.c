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
typedef struct mock_chunk_merge_call_t	mock_chunk_merge_call_t;
struct mock_chunk_merge_call_t
{
	mm_chunk_t		*expect_this;
	mock_chunk_merge_call_t	*next;
};

/* Prototypes ----------------------------------------------------------------*/
void *			unity_malloc				(size_t size);
void			unity_free				(void *ptr);

static mm_chunk_t *	mock_mm_find_first_free			(uint16_t wanted_csize);
static mm_chunk_t *	mock_mm_chunk_split			(mm_chunk_t *this,
								 uint16_t csize);
static void		mock_mm_chunk_merge			(mm_chunk_t *this);

/* Variables -----------------------------------------------------------------*/
static struct
{
	bool		expect_call;
	uint16_t	expect_wanted_csize;
	mm_chunk_t 	*then_return;
} gs_mock_find_first_free_ctx;

static struct
{
	bool		expect_call;
	mm_chunk_t 	*expect_this;
	uint16_t	expect_csize;
	bool		return_null;
} gs_mock_chunk_split_ctx;

static mock_chunk_merge_call_t *gs_mock_chunk_merge_expect = NULL;

static mm_chunk_split_f gs_chunk_split = NULL;
static mm_chunk_merge_f gs_chunk_merge = NULL;

/* Private definitions -------------------------------------------------------*/
static mm_chunk_t *mock_mm_find_first_free(uint16_t wanted_csize)
{
	TEST_ASSERT_MESSAGE(gs_mock_find_first_free_ctx.expect_call, "Unexpected call to mm_find_first_free.");
	TEST_ASSERT_EQUAL_UINT16(gs_mock_find_first_free_ctx.expect_wanted_csize, wanted_csize);
	gs_mock_find_first_free_ctx.expect_call = false;
	return gs_mock_find_first_free_ctx.then_return;
}

static mm_chunk_t *mock_mm_chunk_split(mm_chunk_t *this, uint16_t csize)
{
	TEST_ASSERT_MESSAGE(gs_mock_chunk_split_ctx.expect_call, "Unexpected call to mm_chunk_split.");
	gs_mock_chunk_split_ctx.expect_call = false;

	TEST_ASSERT_EQUAL_PTR(gs_mock_chunk_split_ctx.expect_this, this);
	TEST_ASSERT_EQUAL_UINT16(gs_mock_chunk_split_ctx.expect_csize, csize);

	if (gs_mock_chunk_split_ctx.return_null) {
		return NULL;
	}
	return gs_chunk_split(this, csize);
}

static void mock_mm_chunk_merge(mm_chunk_t *this)
{
	TEST_ASSERT_NOT_NULL_MESSAGE(gs_mock_chunk_merge_expect, "Unexpected call to mm_chunk_merge.");
	TEST_ASSERT_EQUAL_PTR(gs_mock_chunk_merge_expect->expect_this, this);

	mock_chunk_merge_call_t *next = gs_mock_chunk_merge_expect->next;
	unity_free(gs_mock_chunk_merge_expect);
	gs_mock_chunk_merge_expect = next;

	gs_chunk_merge(this);
}

/* Definitions ---------------------------------------------------------------*/
void mock_chunk_setup(void)
{
	gs_mock_find_first_free_ctx.expect_call = false;
	gs_mock_chunk_split_ctx.expect_call = false;
	gs_mock_chunk_merge_expect = NULL;

	gs_chunk_split = mm_chunk_split;
	gs_chunk_merge = mm_chunk_merge;
	UT_PTR_SET(mm_find_first_free, mock_mm_find_first_free);
	UT_PTR_SET(mm_chunk_merge, mock_mm_chunk_merge);
	UT_PTR_SET(mm_chunk_split, mock_mm_chunk_split);
}

void mock_chunk_verify(void)
{
	TEST_ASSERT_FALSE_MESSAGE(gs_mock_find_first_free_ctx.expect_call,
				  "A Call to mm_chunk_find_first_free was expected.");
	TEST_ASSERT_FALSE_MESSAGE(gs_mock_chunk_split_ctx.expect_call,
				  "A Call to mm_chunk_split was expected.");
	TEST_ASSERT_NULL_MESSAGE(gs_mock_chunk_merge_expect,
				  "A Call to mm_chunk_merge was expected.");
}

void mock_mm_find_first_free_ExpectAndReturn(uint16_t wanted_csize, mm_chunk_t *ret)
{
	gs_mock_find_first_free_ctx.expect_call = true;
	gs_mock_find_first_free_ctx.expect_wanted_csize = wanted_csize;
	gs_mock_find_first_free_ctx.then_return = ret;
}

void mock_mm_chunk_split_ExpectAndReturn(mm_chunk_t *this, uint16_t csize, bool return_null)
{
	gs_mock_chunk_split_ctx.expect_call = true;
	gs_mock_chunk_split_ctx.expect_this = this;
	gs_mock_chunk_split_ctx.expect_csize = csize;
	gs_mock_chunk_split_ctx.return_null = return_null;
}

void mock_mm_chunk_merge_Expect(mm_chunk_t *this)
{
	mock_chunk_merge_call_t *new = unity_malloc(sizeof(mock_chunk_merge_call_t));
	if (new == NULL) {
		die("expect init failure");
	}
	new->expect_this = this;
	new->next = NULL;

	if (gs_mock_chunk_merge_expect == NULL) {
		gs_mock_chunk_merge_expect = new;
	} else {
		mock_chunk_merge_call_t *last = gs_mock_chunk_merge_expect;
		while (last->next != NULL) {
			last = last->next;
		}
		last->next = new;
	}
}
