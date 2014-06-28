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
#include "common/object.h"
#include "os/memmgr.h"
#include "tests/memmgr_mock.h"

/* Test helpers --------------------------------------------------------------*/
/* functions's declarations */
static void 		test_object_delete		(object_t *self);
static char *		test_object_to_string		(object_t *self);

/* variables's definitions  */
static object_t *gs_obj = NULL;
static object_ops_t gs_obj_opts = {
	.delete = test_object_delete,
	.to_string = test_object_to_string,
};

/* functions's definitions */
static void test_object_delete(object_t *self)
{
	mm_free(self);
}

static char *test_object_to_string(object_t *self)
{
	static const char to_string[] = "object";
	char *str = mm_zalloc(sizeof(to_string) + 1);
	if (str != NULL) {
		strcpy(str, to_string);
	}
	return str;
}

/* Test group ----------------------------------------------------------------*/
TEST_GROUP(object);

TEST_GROUP_RUNNER(object)
{
	RUN_TEST_CASE(object, to_string);
	RUN_TEST_CASE(object, null_object_does_no_harm);
	RUN_TEST_CASE(object, null_ops_does_no_harm);
	RUN_TEST_CASE(object, null_delete_does_no_harm);
	RUN_TEST_CASE(object, null_to_string_does_no_harm);
}

TEST_SETUP(object)
{
	mock_memmgr_setup();
	gs_obj = mm_zalloc(sizeof(object_t));
	gs_obj->ops = &gs_obj_opts;
}

TEST_TEAR_DOWN(object)
{
	object_delete(gs_obj);
}

/* Test cases ----------------------------------------------------------------*/
TEST(object, to_string)
{
	char *str = object_to_string(gs_obj);
	TEST_ASSERT_EQUAL_STRING("object", str);
	mm_free(str);
}

TEST(object, null_object_does_no_harm)
{
	object_delete(NULL);
	object_to_string(NULL);
}

TEST(object, null_ops_does_no_harm)
{
	gs_obj->ops = NULL;
	object_delete(gs_obj);
	TEST_ASSERT_NULL(object_to_string(gs_obj));
	gs_obj->ops = &gs_obj_opts;
}

TEST(object, null_delete_does_no_harm)
{
	gs_obj_opts.delete = NULL;
	object_delete(gs_obj);
	gs_obj_opts.delete = test_object_delete;
}

TEST(object, null_to_string_does_no_harm)
{
	gs_obj_opts.to_string = NULL;
	TEST_ASSERT_NULL(object_to_string(gs_obj));
	gs_obj_opts.to_string = test_object_to_string;
}
