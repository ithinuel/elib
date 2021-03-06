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

#include <stdlib.h>
#include <stdbool.h>
#include "unity_fixture.h"
#include "os/memmgr.h"
#include "os/system.h"
#include "mcp/mcp.h"

void die(const char *reason)
{
	TEST_FAIL_MESSAGE(reason);
}

int main(int argc, char **argv, char **arge)
{
	system_boot(&g_mcp_entry);
	return 0;
}
