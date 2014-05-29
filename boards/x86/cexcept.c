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
	bool		flying;
};

/* Public functions ----------------------------------------------------------*/
void cexcept_throw(const char *type, const char *message, bool is_dynamic)
{
	cexcept_ctx_t *cur = task_cexcept_get_ctx();

	if (cur == NULL) {
		die("Throw without context");
	}

	if (cur->flying && cur->excpt.is_dynamic) {
		mm_free((void *)cur->excpt.message);
	}
	cur->excpt.type = type;
	cur->excpt.message = message;
	cur->excpt.is_dynamic = false;
	cur->flying = true;
	longjmp(cur->jmpbuf, cur->state);
}

void *cexcept_enter_ctx(void)
{
	cexcept_ctx_t *new = mm_zalloc(sizeof(cexcept_ctx_t));
	if (new == NULL) {
		cexcept_throw("NOMEM", "no memory available to enter cexcept ctx.", false);
	}
	new->prev = task_cexcept_get_ctx();
	new->flying = false;
	new->state = 1;
	task_cexcept_set_ctx(new);

	return new->jmpbuf;
}

cexcept_t *cexcept_catch(void)
{
	cexcept_ctx_t *ctx = task_cexcept_get_ctx();
	ctx->state = 2;
	if (ctx->flying) {
		ctx->flying = false;
		return &ctx->excpt;
	}
	return NULL;
}

void cexcept_finally(void)
{
	cexcept_ctx_t *ctx = task_cexcept_get_ctx();
	if (ctx == NULL) {
		die("Finally without context");
	}
	ctx->state = 3;
}


void cexcept_exit_ctx(void)
{
	bool flying = false;
	cexcept_t ex = {0};
	cexcept_ctx_t	*cur = task_cexcept_get_ctx(),
			*prev = NULL;
	if (cur == NULL) {
		die("Exit without context");
	}

	flying = cur->flying;
	ex = cur->excpt;
	prev = cur->prev;
	task_cexcept_set_ctx(prev);

	mm_free(cur);
	if (flying) {
		cexcept_throw(ex.type, ex.message, ex.is_dynamic);
	}
}
