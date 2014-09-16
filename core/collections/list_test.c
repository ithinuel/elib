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
#include "collections/list.h"
#include "common/common.h"
#include "tests/memmgr_mock.h"
#include "unity_fixture.h"

/* Helpers -------------------------------------------------------------------*/
typedef struct
{
	list_node_t	node;
	uint32_t	value;
} test_list_items_t;

static list_t *gs_list = NULL;
static const test_list_items_t gsc_item_1 = { .value = 1 };
static const test_list_items_t gsc_item_2 = { .value = 2 };
static const test_list_items_t gsc_item_3 = { .value = 3 };
static test_list_items_t gs_item_1;
static test_list_items_t gs_item_2;
static test_list_items_t gs_item_3;

/* Helper --------------------------------------------------------------------*/
#define TEST_ASSERT_CHAIN(list, prev, cur, next)	\
			eval_chain(list, prev, cur, next, __LINE__)
void eval_chain(list_t *list, test_list_items_t *prev, test_list_items_t *cur, test_list_items_t *next, UNITY_LINE_TYPE line)
{
	if (list == NULL) {
		UNITY_TEST_ASSERT_NULL(cur->node.owner, line, "Owner Expected NULL");
	} else {
		UNITY_TEST_ASSERT_EQUAL_PTR(list, cur->node.owner, line, "Owner Expected Non-NULL");
	}
	if (prev == NULL) {
		UNITY_TEST_ASSERT_NULL(cur->node.prev, line, "Expected NULL");
	} else {
		UNITY_TEST_ASSERT_EQUAL_PTR(&prev->node, cur->node.prev, line, NULL);
	}
	if (next == NULL) {
		UNITY_TEST_ASSERT_NULL(cur->node.next, line, "Expected NULL");
	} else {
		UNITY_TEST_ASSERT_EQUAL_PTR(&next->node, cur->node.next, line, NULL);
	}
}

/* Group ---------------------------------------------------------------------*/
TEST_GROUP(list);

TEST_GROUP_RUNNER(list)
{
	RUN_TEST_CASE(list, null_on_alloc_failure);
	RUN_TEST_CASE(list, push_back_null_does_no_harm);
	RUN_TEST_CASE(list, push_back_0_item);
	RUN_TEST_CASE(list, push_back_1_item);
	RUN_TEST_CASE(list, push_back_many_item);
	RUN_TEST_CASE(list, push_back_already_listed);
	RUN_TEST_CASE(list, pop_front_null_does_no_harm);
	RUN_TEST_CASE(list, pop_front_0_item);
	RUN_TEST_CASE(list, pop_front_1_item);
	RUN_TEST_CASE(list, pop_front_n_item);
}

TEST_SETUP(list)
{
	gs_item_1 = gsc_item_1;
	gs_item_2 = gsc_item_2;
	gs_item_3 = gsc_item_3;
	
	gs_list = list_create();
	TEST_ASSERT_NOT_NULL(gs_list);
}

TEST_TEAR_DOWN(list)
{
	object_delete(&gs_list->base);
	gs_list = NULL;
}

TEST(list, null_on_alloc_failure)
{
	mock_memmgr_setup();
	mock_mm_alloc_IgnoreAndReturn(NULL);
	TEST_ASSERT_NULL(list_create());
	mock_memmgr_verify();
}

TEST(list, push_back_null_does_no_harm)
{
	TEST_ASSERT_FALSE(list_push_back(NULL, NULL));
}

TEST(list, push_back_0_item)
{
	TEST_ASSERT_FALSE(list_push_back(gs_list, NULL));
}

TEST(list, push_back_1_item)
{
	TEST_ASSERT_TRUE(list_push_back(gs_list, &gs_item_1.node));
}

TEST(list, push_back_many_item)
{
	TEST_ASSERT_TRUE(list_push_back(gs_list, &gs_item_1.node));
	TEST_ASSERT_CHAIN(gs_list, NULL, &gs_item_1, NULL);

	TEST_ASSERT_TRUE(list_push_back(gs_list, &gs_item_2.node));
	TEST_ASSERT_CHAIN(gs_list, NULL, &gs_item_1, &gs_item_2);
	TEST_ASSERT_CHAIN(gs_list, &gs_item_1, &gs_item_2, NULL);

	TEST_ASSERT_TRUE(list_push_back(gs_list, &gs_item_3.node));
	TEST_ASSERT_CHAIN(gs_list, NULL, &gs_item_1, &gs_item_2);
	TEST_ASSERT_CHAIN(gs_list, &gs_item_1, &gs_item_2, &gs_item_3);
	TEST_ASSERT_CHAIN(gs_list, &gs_item_2, &gs_item_3, NULL);
}

TEST(list, push_back_already_listed)
{
	TEST_ASSERT_TRUE(list_push_back(gs_list, &gs_item_1.node));
	TEST_ASSERT_CHAIN(gs_list, NULL, &gs_item_1, NULL);

	list_t *list = list_create();
	TEST_ASSERT_FALSE(list_push_back(list, &gs_item_1.node));
	TEST_ASSERT_CHAIN(gs_list, NULL, &gs_item_1, NULL);
	object_delete(&list->base);
}

TEST(list, pop_front_null_does_no_harm)
{
	TEST_ASSERT_NULL(list_pop_front(NULL));
}

TEST(list, pop_front_0_item)
{
	TEST_ASSERT_NULL(list_pop_front(gs_list));
}

TEST(list, pop_front_1_item)
{
	list_push_back(gs_list, &gs_item_1.node);
	list_push_back(gs_list, &gs_item_2.node);
	list_push_back(gs_list, &gs_item_3.node);

	TEST_ASSERT_EQUAL_PTR(&gs_item_1.node, list_pop_front(gs_list));
	TEST_ASSERT_CHAIN(NULL, NULL, &gs_item_1, NULL);
	TEST_ASSERT_CHAIN(gs_list, NULL, &gs_item_2, &gs_item_3);
	TEST_ASSERT_CHAIN(gs_list, &gs_item_2, &gs_item_3, NULL);
}

TEST(list, pop_front_n_item)
{
	list_push_back(gs_list, &gs_item_1.node);
	list_push_back(gs_list, &gs_item_2.node);
	list_push_back(gs_list, &gs_item_3.node);

	TEST_ASSERT_EQUAL_PTR(&gs_item_1.node, list_pop_front(gs_list));
	TEST_ASSERT_CHAIN(NULL, NULL, &gs_item_1, NULL);
	TEST_ASSERT_CHAIN(gs_list, NULL, &gs_item_2, &gs_item_3);
	TEST_ASSERT_CHAIN(gs_list, &gs_item_2, &gs_item_3, NULL);

	TEST_ASSERT_EQUAL_PTR(&gs_item_2.node, list_pop_front(gs_list));
	TEST_ASSERT_CHAIN(NULL, NULL, &gs_item_2, NULL);
	TEST_ASSERT_CHAIN(gs_list, NULL, &gs_item_3, NULL);

	TEST_ASSERT_EQUAL_PTR(&gs_item_3.node, list_pop_front(gs_list));
	TEST_ASSERT_CHAIN(NULL, NULL, &gs_item_3, NULL);
}
