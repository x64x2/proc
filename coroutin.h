#include <setjmp.h>
#include <stdlib.h>

#define NREGS 18
#define OPT_PRIO 1
#define end end_cr
#define PL_STATUS_ALT 1
#define PL_STATUS_CUT 2
#define STACK_SIZE /* 500 */ 30000
#define STACK_BOTTOM 50

typedef __int64 myjmp_buf[NREGS];

struct coroutine
{
	myjmp_buf *calling;
	myjmp_buf *env;
};

struct requete
{
	myjmp_buf *env;
	int op;
	void *p[5];
};

struct process_list
{
	struct process_list *prev, *next, *alt;
	int status;
	int prio;
	myjmp_buf env;
	struct requete r;
	int stack_size;
	int stack [STACK_BOTTOM];
};

struct canal
{
	char flag, prio;
	struct process_list *file;
};

struct param_scheduler
{
	int stack_size;
};


