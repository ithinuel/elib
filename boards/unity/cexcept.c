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
#include <setjmp.h>
#include <stdlib.h>
#include <stddef.h>
#include "cexcept/cexcept.h"
#include "common/common.h"
#include "common/cexcept.h"
#include "os/memmgr.h"
#include "os/task.h"

/* Types definitions ---------------------------------------------------------*/
struct cexcept_ctx
{
	cexcept_ctx_t	*prev;
	jmp_buf		jmpbuf;
	uint8_t		state;

	cexcept_t	excpt;
	bool		excpt_is_set;
};

/* Public functions ----------------------------------------------------------*/
void cexcept_throw(const char *type, const char *message, bool is_dynamic)
{
	cexcept_ctx_t *cur = task_cexcept_get_ctx();

	if (cur == NULL) {
		die("Throw without context");
	}

	if (cur->excpt_is_set && cur->excpt.is_dynamic) {
		mm_free((void *)cur->excpt.message);
	}
	cur->excpt.type = type;
	cur->excpt.message = message;
	cur->excpt.is_dynamic = false;
	cur->excpt_is_set = true;
	longjmp(cur->jmpbuf, cur->state);
}

void *cexcept_enter_ctx(void)
{
	cexcept_ctx_t *new = mm_zalloc(sizeof(cexcept_ctx_t));
	if (new == NULL) {
		cexcept_throw("NOMEM", "no memory available to enter cexcept ctx.", false);
	}
	new->prev = task_cexcept_get_ctx();
	new->excpt_is_set = false;
	new->state = 1;
	task_cexcept_set_ctx(new);

	return new->jmpbuf;
}

cexcept_t *cexcept_catch(void)
{
	cexcept_ctx_t *ctx = task_cexcept_get_ctx();
	if (ctx == NULL) {
		die("Catch without context");
	}
	ctx->state = 2;
	ctx->excpt_is_set = false;
	return &ctx->excpt;
}

void cexcept_finally(void)
{
	cexcept_ctx_t *ctx = task_cexcept_get_ctx();
	if (ctx == NULL) {
		die("Finally without context");
	}
}


void cexcept_exit_ctx(void)
{
	bool		excpt_is_set = false;
	cexcept_t	ex = {0};
	cexcept_ctx_t	*cur = task_cexcept_get_ctx(),
			*prev = NULL;
	if (cur == NULL) {
		die("Exit without context");
	}

	excpt_is_set = cur->excpt_is_set;
	ex = cur->excpt;
	prev = cur->prev;
	task_cexcept_set_ctx(prev);

	mm_free(cur);
	if (excpt_is_set) {
		cexcept_throw(ex.type, ex.message, ex.is_dynamic);
	}
}
