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
#include "unity_fixture.h"
#include "tests/mock_memmgr.h"
#include "os/memmgr.h"
#include "os/mutex.h"

/*----------------------------------------------------------------------------*/
static mutex_t *gs_mtx = NULL;

/* Test group definitions ----------------------------------------------------*/
TEST_GROUP(mutex);

TEST_GROUP_RUNNER(mutex)
{
	RUN_TEST_CASE(mutex, create);
}

TEST_SETUP(mutex)
{
	mock_memmgr_setup();
	gs_mtx = mutex_new(false);

}

TEST_TEAR_DOWN(mutex)
{
	object_delete(&gs_mtx->base);
}

/* Tests ---------------------------------------------------------------------*/
TEST(mutex, create)
{
}
