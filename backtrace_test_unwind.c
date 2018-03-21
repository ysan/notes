// gcc backtrace_test_unwind.c -rdynamic -lunwind -ldl

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include <execinfo.h>

void my_backtrace (void) {
	unw_cursor_t cursor;
	unw_context_t context;
	unw_word_t offset;
	unw_word_t pc;
	char fname [64] = {0};
    int nCnt = 0;

	unw_getcontext (&context);
	unw_init_local (&cursor, &context);

	while (unw_step (&cursor) > 0) {
		unw_get_reg (&cursor, UNW_REG_IP, &pc);

		fname[0] = 0x00;
		(void) unw_get_proc_name (&cursor, fname, sizeof(fname), &offset);

		Dl_info info;
		dladdr ((void*)pc, &info);

		fprintf (stdout, "%s : backtrace [%d] %s(%s+0x%lx) [%p]\n",
					__func__, nCnt, info.dli_fname, fname, offset, (void*)pc);
		nCnt ++;
	}
}

void hoge2 () {
	my_backtrace ();
}

void hoge1 () {
	hoge2 ();
}

int main (void)
{
	hoge1 ();
	exit (EXIT_SUCCESS);
}
