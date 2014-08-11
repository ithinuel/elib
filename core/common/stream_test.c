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
#include "common/stream.h"

/* Helper prototypes ---------------------------------------------------------*/
static int32_t test_stream_read(stream_t *this, uint8_t *buffer, uint32_t len);
static int32_t test_stream_write(stream_t *this, uint8_t *buffer, uint32_t len);

/* Test variables ------------------------------------------------------------*/
static object_ops_t gs_obj_ops = {
	.delete = NULL,
	.to_string = NULL
};
static stream_ops_t gs_strm_ops = {
	.read = test_stream_read,
	.write = test_stream_write,
};
static stream_t gs_strm = {
	.base.ops = &gs_obj_ops,
	.ops = &gs_strm_ops,
	.on_receive = NULL
};

static uint8_t gs_buffer[32] = {0};

/* Helper definitions --------------------------------------------------------*/
static int32_t test_stream_read(stream_t *this, uint8_t *buffer, uint32_t len)
{
	TEST_ASSERT_EQUAL_PTR(&gs_strm, this);
	if (buffer != NULL) {
		TEST_ASSERT_EQUAL_PTR(buffer, gs_buffer);
	}
	return len;
}

static int32_t test_stream_write(stream_t *this, uint8_t *buffer, uint32_t len)
{
	TEST_ASSERT_EQUAL(&gs_strm, this);
	if (buffer != NULL) {
		TEST_ASSERT_EQUAL_PTR(buffer, gs_buffer);
	}
	return len;
}

/* Test group ----------------------------------------------------------------*/
TEST_GROUP(stream);

TEST_GROUP_RUNNER(stream)
{
	RUN_TEST_CASE(stream, null_stream_does_no_harm);
	RUN_TEST_CASE(stream, null_ops_does_no_harm);
	RUN_TEST_CASE(stream, null_read_does_no_harm);
	RUN_TEST_CASE(stream, null_write_does_no_harm);
	RUN_TEST_CASE(stream, read_write);
}

TEST_SETUP(stream)
{
	gs_strm_ops.read = test_stream_read;
	gs_strm_ops.write = test_stream_write;
	gs_strm.ops = &gs_strm_ops;
}

TEST_TEAR_DOWN(stream)
{

}

/* Test cases ----------------------------------------------------------------*/
TEST(stream, null_stream_does_no_harm)
{
	TEST_ASSERT_EQUAL_INT32(-1, stream_read(NULL, NULL, 0));
	TEST_ASSERT_EQUAL_INT32(-1, stream_write(NULL, NULL, 0));
}

TEST(stream, null_ops_does_no_harm)
{
	gs_strm.ops = NULL;
	TEST_ASSERT_EQUAL_INT32(-1, stream_read(&gs_strm, NULL, 0));
	TEST_ASSERT_EQUAL_INT32(-1, stream_write(&gs_strm, NULL, 0));
}

TEST(stream, null_read_does_no_harm)
{
	gs_strm_ops.read = NULL;
	TEST_ASSERT_EQUAL_INT32(-1, stream_read(&gs_strm, NULL, 0));
	TEST_ASSERT_EQUAL_INT32(0, stream_write(&gs_strm, NULL, 0));
}

TEST(stream, null_write_does_no_harm)
{
	gs_strm_ops.write = NULL;
	TEST_ASSERT_EQUAL_INT32(0, stream_read(&gs_strm, NULL, 0));
	TEST_ASSERT_EQUAL_INT32(-1, stream_write(&gs_strm, NULL, 0));
}

TEST(stream, read_write)
{
	TEST_ASSERT_EQUAL_INT32(25, stream_read(&gs_strm, gs_buffer, 25));
	TEST_ASSERT_EQUAL_INT32(25, stream_write(&gs_strm, gs_buffer, 25));
}
