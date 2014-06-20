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
#include <stdio.h>
#include "unity_fixture.h"
#include "common/cexcept.h"

#include "tests/tests.h"
#include "tests/mock_memmgr.h"

/* Helper functions and variables --------------------------------------------*/
static bool did_try_start = false;
static bool did_try_end = false;
static bool did_catch_start = false;
static bool did_catch_end = false;
static bool did_finally_start = false;
static bool did_finally_end = false;

void do_something(bool throw)
{
	if (throw) {
		Throw("Test", "woops", false);
	}
}

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(cexcept);

TEST_GROUP_RUNNER(cexcept) {
	RUN_TEST_CASE(cexcept, Throw_alone);
	RUN_TEST_CASE(cexcept, Try_Fail_to_alloc);
	RUN_TEST_CASE(cexcept, Catch_without_ctx);
	RUN_TEST_CASE(cexcept, Finally_without_ctx);
	RUN_TEST_CASE(cexcept, EndTry_without_ctx)

	RUN_TEST_CASE(cexcept, Try_EndTry);
	RUN_TEST_CASE(cexcept, Try_Throw_EndTry);
	RUN_TEST_CASE(cexcept, Try_Catch_EndTry);
	RUN_TEST_CASE(cexcept, Try_Throw_Catch_EndTry);
	RUN_TEST_CASE(cexcept, Try_Catch_Throw_EndTry);
	RUN_TEST_CASE(cexcept, Try_Throw_Catch_Throw_EndTry);
	RUN_TEST_CASE(cexcept, Try_Catch_Finally_EndTry);

	RUN_TEST_CASE(cexcept, Try_Throw_Catch_Finally_EndTry);
	RUN_TEST_CASE(cexcept, Try_Catch_Throw_Finally_EndTry);
	RUN_TEST_CASE(cexcept, Try_Catch_Finally_Throw_EndTry);
	RUN_TEST_CASE(cexcept, Try_Throw_Catch_Throw_Finally_EndTry);
	RUN_TEST_CASE(cexcept, Try_Throw_Catch_Finally_Throw_EndTry);
	RUN_TEST_CASE(cexcept, Try_Catch_Throw_Finally_Throw_EndTry);
	RUN_TEST_CASE(cexcept, Try_Throw_Catch_Throw_Finally_Throw_EndTry);

	RUN_TEST_CASE(cexcept, Try_Finally_EndTry);
	RUN_TEST_CASE(cexcept, Try_Throw_Finally_EndTry);
	RUN_TEST_CASE(cexcept, Try_Finally_Throw_EndTry);
	RUN_TEST_CASE(cexcept, Try_Throw_Finally_Throw_EndTry);

}

TEST_SETUP(cexcept)
{
	mock_memmgr_setup();
	did_try_start = false;
	did_try_end = false;
	did_catch_start = false;
	did_catch_end = false;
	did_finally_start = false;
	did_finally_end = false;
}

TEST_TEAR_DOWN(cexcept) {

}

/* Tests ---------------------------------------------------------------------*/
TEST(cexcept, Throw_alone)
{
	die_Expect("Throw without context");

	VERIFY_DIE_START
	do_something(true);
	VERIFY_DIE_END

	die_Verify();
}
TEST(cexcept, Try_Fail_to_alloc)
{
//	mm_Expect(false);
}
TEST(cexcept, Catch_without_ctx)
{
	die_Expect("Catch without context");
	VERIFY_DIE_START
	switch(1)
	{
	case 0:
	{
		do {}
	Catch {

	}
	while(0);
		break;
	}
	}
	VERIFY_DIE_END

	die_Verify();
}
TEST(cexcept, Finally_without_ctx)
{
	jmp_buf __buf;

	die_Expect("Finally without context");
	VERIFY_DIE_START
	switch(1)
	{
	case 0:
	{
		do {}
	Finally {

	}
	while(0);
		break;
	}
	}
	VERIFY_DIE_END

	die_Verify();
}
TEST(cexcept, EndTry_without_ctx)
{
	die_Expect("Exit without context");
	VERIFY_DIE_START
	{
	switch(1)
	{
	case 0:
	{
		do {}
	EndTry
	VERIFY_DIE_END

	die_Verify();
}

TEST(cexcept, Try_EndTry)
{
	Try {
		did_try_start = true;
		do_something(false);
		did_try_end = true;
	}
	EndTry
	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_TRUE(did_try_end);
}
TEST(cexcept, Try_Throw_EndTry)
{
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(true);
		did_try_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_FALSE(did_try_end);
	die_Verify();
}

TEST(cexcept, Try_Catch_EndTry)
{
	Try {
		did_try_start = true;
		do_something(false);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		TEST_ASSERT_EQUAL_STRING("Test", cexcept_type(e));
		TEST_ASSERT_EQUAL_STRING("woops", cexcept_message(e));
		did_catch_end = true;
	}
	EndTry

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_TRUE(did_try_end);
	TEST_ASSERT_FALSE(did_catch_start);
	TEST_ASSERT_FALSE(did_catch_end);
}
TEST(cexcept, Try_Throw_Catch_EndTry)
{
	Try {
		did_try_start = true;
		do_something(true);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		TEST_ASSERT_EQUAL_STRING("Test", cexcept_type(e));
		TEST_ASSERT_EQUAL_STRING("woops", cexcept_message(e));
		did_catch_end = true;
	}
	EndTry

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_FALSE(did_try_end);
	TEST_ASSERT_TRUE(did_catch_start);
	TEST_ASSERT_TRUE(did_catch_end);
}
TEST(cexcept, Try_Catch_Throw_EndTry)
{
	Try {
		did_try_start = true;
		do_something(false);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		TEST_ASSERT_EQUAL_STRING("Test", cexcept_type(e));
		TEST_ASSERT_EQUAL_STRING("woops", cexcept_message(e));
		did_catch_end = true;
	}
	EndTry

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_TRUE(did_try_end);
	TEST_ASSERT_FALSE(did_catch_start);
	TEST_ASSERT_FALSE(did_catch_end);
}
TEST(cexcept, Try_Throw_Catch_Throw_EndTry)
{
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(true);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		TEST_ASSERT_EQUAL_STRING("Test", cexcept_type(e));
		TEST_ASSERT_EQUAL_STRING("woops", cexcept_message(e));
		do_something(true);
		did_catch_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_FALSE(did_try_end);
	TEST_ASSERT_TRUE(did_catch_start);
	TEST_ASSERT_FALSE(did_catch_end);
	die_Verify();
}

TEST(cexcept, Try_Catch_Finally_EndTry)
{
	bool did_try_start = false;
	bool did_try_end = false;
	bool did_catch_start = false;
	bool did_catch_end = false;
	bool did_finally_start = false;
	bool did_finally_end = false;

	Try {
		did_try_start = true;
		do_something(false);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		did_catch_end = true;
	}
	Finally {
		did_finally_start = true;
		did_finally_end = true;
	}
	EndTry

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_TRUE(did_try_end);
	TEST_ASSERT_FALSE(did_catch_start);
	TEST_ASSERT_FALSE(did_catch_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_TRUE(did_finally_end);
}
TEST(cexcept, Try_Throw_Catch_Finally_EndTry)
{
	Try {
		did_try_start = true;
		do_something(true);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		TEST_ASSERT_EQUAL_STRING("Test", cexcept_type(e));
		TEST_ASSERT_EQUAL_STRING("woops", cexcept_message(e));
		do_something(false);
		did_catch_end = true;
	}
	Finally {
		did_finally_start = true;
		did_finally_end = true;
	}
	EndTry

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_FALSE(did_try_end);
	TEST_ASSERT_TRUE(did_catch_start);
	TEST_ASSERT_TRUE(did_catch_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_TRUE(did_finally_end);
}
TEST(cexcept, Try_Catch_Throw_Finally_EndTry)
{
	Try {
		did_try_start = true;
		do_something(false);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		do_something(true);
		did_catch_end = true;
	}
	Finally {
		did_finally_start = true;
		did_finally_end = true;
	}
	EndTry

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_TRUE(did_try_end);
	TEST_ASSERT_FALSE(did_catch_start);
	TEST_ASSERT_FALSE(did_catch_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_TRUE(did_finally_end);
}
TEST(cexcept, Try_Catch_Finally_Throw_EndTry)
{
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(false);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		do_something(false);
		did_catch_end = true;
	}
	Finally {
		did_finally_start = true;
		do_something(true);
		did_finally_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_TRUE(did_try_end);
	TEST_ASSERT_FALSE(did_catch_start);
	TEST_ASSERT_FALSE(did_catch_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_FALSE(did_finally_end);
	die_Verify();
}
TEST(cexcept, Try_Throw_Catch_Throw_Finally_EndTry)
{
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(true);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		TEST_ASSERT_EQUAL_STRING("Test", cexcept_type(e));
		TEST_ASSERT_EQUAL_STRING("woops", cexcept_message(e));
		do_something(true);
		did_catch_end = true;
	}
	Finally {
		did_finally_start = true;
		did_finally_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_FALSE(did_try_end);
	TEST_ASSERT_TRUE(did_catch_start);
	TEST_ASSERT_FALSE(did_catch_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_TRUE(did_finally_end);
	die_Verify();
}
TEST(cexcept, Try_Throw_Catch_Finally_Throw_EndTry)
{
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(true);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		TEST_ASSERT_EQUAL_STRING("Test", cexcept_type(e));
		TEST_ASSERT_EQUAL_STRING("woops", cexcept_message(e));
		did_catch_end = true;
	}
	Finally {
		did_finally_start = true;
		do_something(true);
		did_finally_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_FALSE(did_try_end);
	TEST_ASSERT_TRUE(did_catch_start);
	TEST_ASSERT_TRUE(did_catch_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_FALSE(did_finally_end);
	die_Verify();
}
TEST(cexcept, Try_Catch_Throw_Finally_Throw_EndTry)
{
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(false);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		TEST_ASSERT_EQUAL_STRING("Test", cexcept_type(e));
		TEST_ASSERT_EQUAL_STRING("woops", cexcept_message(e));
		do_something(true);
		did_catch_end = true;
	}
	Finally {
		did_finally_start = true;
		do_something(true);
		did_finally_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_TRUE(did_try_end);
	TEST_ASSERT_FALSE(did_catch_start);
	TEST_ASSERT_FALSE(did_catch_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_FALSE(did_finally_end);
	die_Verify();
}
TEST(cexcept, Try_Throw_Catch_Throw_Finally_Throw_EndTry)
{
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(true);
		did_try_end = true;
	}
	Catch {
		did_catch_start = true;
		TEST_ASSERT_EQUAL_STRING("Test", cexcept_type(e));
		TEST_ASSERT_EQUAL_STRING("woops", cexcept_message(e));
		do_something(true);
		did_catch_end = true;
	}
	Finally {
		did_finally_start = true;
		do_something(true);
		did_finally_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_FALSE(did_try_end);
	TEST_ASSERT_TRUE(did_catch_start);
	TEST_ASSERT_FALSE(did_catch_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_FALSE(did_finally_end);
	die_Verify();
}

TEST(cexcept, Try_Finally_EndTry)
{
	Try {
		did_try_start = true;
		do_something(false);
		did_try_end = true;
	}
	Finally {
		did_finally_start = true;
		do_something(false);
		did_finally_end = true;
	}
	EndTry

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_TRUE(did_try_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_TRUE(did_finally_end);
}
TEST(cexcept, Try_Throw_Finally_EndTry)
{
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(true);
		did_try_end = true;
	}
	Finally {
		did_finally_start = true;
		do_something(false);
		did_finally_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_FALSE(did_try_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_TRUE(did_finally_end);
	die_Verify();
}
TEST(cexcept, Try_Finally_Throw_EndTry) {
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(false);
		did_try_end = true;
	}
	Finally {
		did_finally_start = true;
		do_something(true);
		did_finally_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_TRUE(did_try_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_FALSE(did_finally_end);
	die_Verify();
}
TEST(cexcept, Try_Throw_Finally_Throw_EndTry) {
	die_Expect("Throw without context");

	VERIFY_DIE_START
	Try {
		did_try_start = true;
		do_something(true);
		did_try_end = true;
	}
	Finally {
		did_finally_start = true;
		do_something(true);
		did_finally_end = true;
	}
	EndTry
	VERIFY_DIE_END

	TEST_ASSERT_TRUE(did_try_start);
	TEST_ASSERT_FALSE(did_try_end);
	TEST_ASSERT_TRUE(did_finally_start);
	TEST_ASSERT_FALSE(did_finally_end);
	die_Verify();
}


