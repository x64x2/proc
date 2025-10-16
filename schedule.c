
/* Coroutines */
#include <stdio.h>
#include <setjmp.h>
#include "coroutin.h"

#define NACR_MESSAGE "\nNo more result\n"
#define OPTIONS 0 /* 1 pour traces */
#define end end_cr
#define NREGS 18

struct coroutine
{
    jmp_buf *calling;
    jmp_buf *env;
};

int options = 1;

typedef __int64 myjmp_buf[NREGS];


__int64 mysetjmp (myjmp_buf env)
{
	asm (R"(
		movq	16(%rbp), %rax
		addq	$8, %rax
		movq	%rbx, (%rax)
	
		movq	16(%rbp), %rax
		addq	$16, %rax
		movq	%rcx, (%rax)
		
		movq	16(%rbp), %rax
		addq	$24, %rax
		movq	%rdx, (%rax)

		movq	16(%rbp), %rax
		addq	$32, %rax
		movq	%rsi, (%rax)
	
		movq	16(%rbp), %rax
		addq	$40, %rax
		movq	%rdi, (%rax)

		movq	16(%rbp), %rax
		addq	$64, %rax
		movq	%r8, (%rax)

		movq	16(%rbp), %rax
		addq	$72, %rax
		movq	%r9, (%rax)

		movq	16(%rbp), %rax
		addq	$80, %rax
		movq	%r10, (%rax)
	
		movq	16(%rbp), %rax
		addq	$88, %rax
		movq	%r11, (%rax)

		movq	16(%rbp), %rax
		addq	$96, %rax
		movq	%r12, (%rax)

		movq	16(%rbp), %rax
		addq	$104, %rax
		movq	%r13, (%rax)

		movq	16(%rbp), %rax
		addq	$112, %rax
		movq	%r14, (%rax)

		movq	16(%rbp), %rax
		addq	$120, %rax
		movq	%r15, (%rax)

		// Stack frame : 
		// %rsp ------------------> ...
		// ...
		// %rbp ------------------> %rbp caller
		// %rbp+8                   %rip caller
		// %rbp+16 = %rsp caller -> ...
		// ...
		// %rbp caller -----------> ...
		// ...
		
		// %rsp caller = %rsp + 2 words = %rsp+16
		movq	16(%rbp), %rax
		addq	$48, %rax
		movq	%rbp, (%rax)
		addq $16, (%rax)
		
		// %rbp caller = (%rbp)
		movq	16(%rbp), %rax
		addq	$56, %rax
		movq	%rbp, %rbx
		movq (%rbx), %rbx
		movq	%rbx, (%rax)
		
		// %rip caller = (%rbp + 1 word) = (%rbp+8)
		movq	16(%rbp), %rax
		addq	$128, %rax
		movq	%rbp, %rbx
		addq $8, %rbx 
		movq (%rbx), %rbx
		movq	%rbx, (%rax)		

		)");
		return 0;
}

void mylongjmp (myjmp_buf env, __int64 value)
{
__int64 r;
	r = env[1];
	r = env[2];
	r = env[3];
	asm (R"(
		movq	16(%rbp), %rax
		movq	8(%rax), %rax
		movq	%rax, %rbx

		movq	16(%rbp), %rax
		movq	16(%rax), %rax
		movq	%rax, %rcx

		movq	16(%rbp), %rax
		movq	24(%rax), %rax
		movq	%rax, %rdx

		movq	16(%rbp), %rax
		movq	32(%rax), %rax
		movq	%rax, %rsi
		
		movq	16(%rbp), %rax
		movq	40(%rax), %rax
		movq	%rax, %rdi
		
		movq	16(%rbp), %rax
		movq	64(%rax), %rax
		movq	%rax, %r8
		
		movq	16(%rbp), %rax
		movq	72(%rax), %rax
		movq	%rax, %r9
		
		movq	16(%rbp), %rax
		movq	80(%rax), %rax
		movq	%rax, %r10
		
		movq	16(%rbp), %rax
		movq	88(%rax), %rax
		movq	%rax, %r11

		movq	16(%rbp), %rax
		movq	96(%rax), %rax
		movq	%rax, %r12

		movq	16(%rbp), %rax
		movq	104(%rax), %rax
		movq	%rax, %r13
		
		movq	16(%rbp), %rax
		movq	112(%rax), %rax
		movq	%rax, %r14
		
		movq	16(%rbp), %rax
		movq	120(%rax), %rax
		movq	%rax, %r15

		// %rsp
		movq	16(%rbp), %rax
		movq	48(%rax), %rax
		movq	%rax, %rsp

		// %rbp
		movq	16(%rbp), %rax
		movq	56(%rax), %rax
		movq	%rax, %rbx
	
		// %rip
		movq	16(%rbp), %rax
		movq	128(%rax), %rax
		push	%rax
		
		// result = value
		movq	24(%rbp), %rax
		// %rbp
		movq	%rbx, %rbp 
		ret

	)");
	
}

void *jmpval;

int *getsp ()
{
int *a;
	a = (int *)&a;
	a += 3;
	return a;
}

int call_with_stack (int *stack, int (*f)(), int x)
{
int *sp;
	sp = getsp();
	{
		int buf[sp-stack];
		return (*f) (x);
	}
}

void *start_coroutine (struct coroutine *cr,
    void *(*f) (/* void *p, struct coroutine *cr */),
    void *p, int *stack)
{
int x, y;
struct coroutine *calling;
int *_SP;
int test;
    calling = malloc (sizeof(*calling));
    x = mysetjmp (*(cr->calling));
    if (x == 0)
    {
        calling->calling = cr->env;
        calling->env = cr->calling;
        //test = 123;
        //_SP = stack;
        //test = 456;
        /*
		_SP = getsp();
        {
            int buf[_SP-stack];
            y = (*f) (p, calling);
			return y;
        }
		*/
		asm (R"(
			movq	40(%rbp), %rax
			movq	%rax, %rsp
		)");
		y = (*f) (p, calling);
		asm (R"(
			movq	%rbp, %rsp
			subq	$64, %rsp
		)");
		
    }
    else
	return jmpval;
}

void *call_coroutine (struct coroutine *cr, void *entree)
{
int x;
#if (OPTIONS & 1)
    printf ("call_coroutine jump to:\n");
    print_jmp_buf (*(cr->env));
#endif
    x = mysetjmp (*(cr->calling));
#if (OPTIONS & 1)
    printf ("call_coroutine: x = %d, ", x);
    if (x == 0)
	printf ("saved env of calling:\n");
    else
	printf ("returned to env:\n");
    print_jmp_buf (*(cr->calling));
#endif
    if (x == 0)
    {
		jmpval = entree;			
		mylongjmp (*(cr->env), 1);
		printf ("Error calling coroutine\n");
    }
    else
		return jmpval;
}

int coroutine1 (char *p, struct coroutine *calling)
{
void *x;
    printf ("Lancement coroutine avec parametre <%s>\n", p);
    x = call_coroutine (calling, "1er parametre coroutine->appelant");
    printf ("Appelant retourne <%s> la 1ere fois\n", x);
    x = call_coroutine (calling, "2eme parametre coroutine->appelant");
    printf ("Appelant retourne <%s> la 2eme fois\n", x);
    x = call_coroutine (calling, "3eme parametre coroutine->appelant");
}

test_coroutine ()
{
struct coroutine cr;
myjmp_buf calling, env;
#define STACK_SIZE 500
#define STACK_BOTTOM 50
int stack[STACK_SIZE+STACK_BOTTOM];
void *x;
    cr.calling = &calling;
    cr.env = &env;
    x = start_coroutine (&cr, coroutine1, "Parametre lancement",
	    stack + STACK_SIZE);
    printf ("Lancement retourne <%s>\n", x);
    x = call_coroutine (&cr, "1er appel");
    printf ("1er appel retourne <%s>\n", x);
    x = call_coroutine (&cr, "2eme appel");
    printf ("2eme appel retourne <%s>\n", x);
    exit(0);
}

struct requete
{
    jmp_buf *env;
    int op;
    int p[5];
};

#define OP_ALT 1

#define OP_SCHEDULE 3
#define OP_TERMINATE 4

#define OP_END 8

#define OP_RECEIVE 5
#define OP_SEND 6

#define OP_GETPL 7
#define OP_GETCTX 9
#define OP_SETCTX 10

#define OP_COPYPL 12

int alt (struct coroutine *calling, int a, int b)
{
int x;
/*
struct
{
    int a;
    int b;
} ab;
*/
struct requete r;
struct requete *r1;

    r.op = OP_ALT;
    r.p[0] = a;
    r.p[1] = b;
    r1 = call_coroutine (calling, &r);
    calling->calling = r1->env;
    return r1->p[0];
}

end (struct coroutine *calling)
{
struct requete r;
	r.op = OP_END;
	call_coroutine (calling, &r);
}

int schedule (struct coroutine *calling)
{
struct requete r;
/* int x; */
struct requete *r1;
    r.op = OP_SCHEDULE;
    r1 = call_coroutine (calling, &r);
    calling->calling = r1->env;
    return r1;
}

struct process_list *getpl (struct coroutine *calling)
{
struct requete r;
struct requete *r1;
	r.op = OP_GETPL;
	r1 = call_coroutine (calling, &r);
	calling->calling = r1->env;
	return r1->p[0];
}

cut (struct coroutine *calling)
{
struct process_list *p;
	p = getpl (calling);
	p->alt->status |= PL_STATUS_CUT;
}

int get_prio (struct coroutine *calling)
{
struct process_list *pl;
	pl = getpl (calling);
	return pl->prio;
}

set_prio (struct coroutine *calling, int prio)
{
struct process_list *pl;
	pl = getpl (calling);
	pl->prio = prio;
}

int getctx (struct coroutine *calling, struct process_list *ctx)
{
struct requete r;
struct requete *r1;
	r.op = OP_GETCTX;
	r.p[0] = ctx;
	r1 = call_coroutine (calling, &r);
	calling->calling = r1->env;
	return r1->p[1];
}

int setctx (struct coroutine *calling, struct process_list *ctx, int x)
{
struct requete r;
	r.op = OP_SETCTX;
	r.p[0] = ctx;
	r.p[1] = x;
	call_coroutine (calling, &r);
	printf ("Error: return after setctx\n");
	exit (0);
}

struct process_list
{
	/* struct coroutine cr; */
	jmp_buf env;
	struct requete r;
	int stack[STACK_SIZE+STACK_BOTTOM];
	struct process_list *prev, *next;
};

struct canal
{
	char flag; /* 0 -> des coroutines ayant fait un receive
				attendent un send
		     1 -> des coroutines ayant fait un send attendent
				un receive */
	char prio; /* 0 -> ajouter a la fin
		      1 -> par ordre de priorite */
	struct process_list *file;
};

typedef struct {
	unsigned	j_sp;
	unsigned	j_ss;
	unsigned	j_flag;
	unsigned	j_cs;
	unsigned	j_ip;
	unsigned	j_bp;
	unsigned	j_di;
	unsigned	j_es;
	unsigned	j_si;
	unsigned	j_ds;
}	jmp_buf[1];

print_stack (int bp, int n)
{

int i;

	for (i=0; i<n && bp && (bp!=-1); i++)
	{
		printf ("   %2d:      %04X    %04X    %u %u\n",
			i, ((int *) bp) [0], ((int *) bp) [1],
                            ((int *) bp) [0], ((int *) bp) [1]);
		bp = ((int *) bp) [0];
	}
}

print_env (myjmp_buf env)
{
	printf (" SP=%04X BP=%04X IP=%04X\n",
		env->j_sp, env->j_bp, env->j_ip);
	print_stack (env->j_bp, 20);
}

int f (int x, int x1)
{
int y;
char buf[16];
int a, b, c;
int status;
	status = setjmp (env);
	printf ("status = %d\n", status);
	print_env (env);
	if (status)
		return;
	longjmp (env, 0xF00D);

	&a; &b; &c;
	y = a;
	y = x + 0x321;
	return y;
}

tframe ()
{
int x, y;
	x = 0x1234;
	y = f (x, 0x4567);
	printf ("0x%X\n", y);
}

print_jmp_buf (myjmp_buf env)
{
int i;
char *format;

	format = " SP=%04X SS=%04X %04X CS=%04X IP=%04X BP=%04X DI=%04X ES=%04X SI=%04X DS=%04X\n";
	printf (format,
		env->j_sp, env->j_ss, env->j_flag, env->j_cs, env->j_ip,
		env->j_bp, env->j_di, env->j_es, env->j_si, env->j_ds);
	for (i=0; i<10; i++)
		printf (" %u", ((unsigned int *) env) [i]);
	printf (" - adr %p\n", env);
	print_env (env);
}

send (struct coroutine *cr, struct canal *cnl, int x)
{
struct process_list *item;
	if (cnl->file == NULL)
	{
		cnl->flag = 1;
		item = malloc (sizeof (*item));
		cnl->file = item;

	}

}

int send (struct coroutine *cr, struct canal *cnl, int x)
{
struct requete r, *r1;
int status;
	r.op = OP_SEND;
	r.p[0] = cnl;
	r.p[1] = x;
	r1 = call_coroutine (cr, &r);
	cr->calling = r1->env;
	return r1;
}

int receive (struct coroutine *cr, struct canal *cnl)
{
struct requete r;
struct requete *r1;
	r.op = OP_RECEIVE;
	r.p[0] = cnl;
	r1 = call_coroutine (cr, &r);
	cr->calling = r1->env;
	return r1->p[1];
}

int copy_pl (struct coroutine *cr)
{
struct requete r;
struct requete *r1;
	r.op = OP_COPYPL;
	r1 = call_coroutine (cr, &r);
	cr->calling = r1->env;
	return (int)r1;
}

copy_process_item (struct process_list *dst, struct process_list *src)
{
	memcpy (dst, src, sizeof(*dst));
	dst->r.env = &(dst->env);
}

struct process_list *copy_process_list (struct process_list *pl, int options)
{
struct process_list *copy, *src, *dst, *prev;
	copy = malloc (sizeof (*copy));
	if (copy == NULL)
	{
	insuf_mem:
		printf ("Insufficient memory for copying process list\n");
		exit (-1);
	}
	copy_process_item (copy, pl);
	src = pl->next;
	prev = copy;
	for (;;)
	{
		if (src == pl || src == NULL)
		{
			if (options & OPT_PRIO)
				prev->next = NULL;
			else
				prev->next = copy;
			copy->prev = prev;
			return copy;
		}
		dst = malloc (sizeof(*pl));
		if (dst == NULL)
			goto insuf_mem;
		copy_process_item (dst, src);
		prev->next = dst;
		dst->prev = prev;
		prev = dst;
		src = src->next;
	}
	/* pl->r.env = &(pl->env); */
}

int testalt (void *p, struct coroutine *c1);

int testalt1 (void *p, struct coroutine *calling1)
{
struct coroutine calling[1];
int x, y;
struct canal *cnl;
    memcpy (calling, calling1, sizeof(calling));
    cnl = malloc (sizeof (*cnl));
    cnl->file = NULL;
    x = alt (calling, 111, 222);
    printf ("x = %d\n", x);

    if (x == 111)
    {
	send (calling,  cnl, 333);
	printf ("apres send, x = %d\n", x);
	end (calling);
	printf ("apres end, x = %d\n", x);
    }
    else
    {
	y = receive (calling, cnl);
	printf ("%d received\n", y);
    }
    schedule (calling);
    printf ("apres schedule, x = %d\n", x);

    schedule (calling);
    printf ("apres 2eme schedule, x = %d\n", x);

    end (calling);
    printf ("apres end, x = %d\n");
    exit (0);
}

#define MAX_CR 50

int old_scheduler (int (*f) (), void *p)
{
struct coroutine cr[MAX_CR];
myjmp_buf calling/*[MAX_CR]*/, env[MAX_CR];
int stack[MAX_CR][STACK_SIZE+50];
int astack[STACK_SIZE+50];
int ncr;
struct requete *r;
struct requete tr[MAX_CR];
int acr; /* active coroutine */
int status_cr[MAX_CR];
int nacr; /* activable coroutines */
int i;
struct canal *cnl;
struct process_list *item;

    printf ("%d\n", &acr); 
    &acr; 
    acr = 0x1234; 

    for (i=0; i<MAX_CR; i++)
	status_cr[i] = 0;

    ncr = 1;
    nacr = 1;
    acr = 0;
    cr[0].calling = &calling/*[0]*/;
    cr[0].env = &env[0];
    status_cr[0] = 1;

    r = start_coroutine (&cr[0], f, p, astack + STACK_SIZE);

    for (;;)
    {
	switch (r->op)
	{
	    case OP_SCHEDULE:
		memcpy (stack[acr], astack, sizeof(stack[acr]));
	    search_next:
		do	/* search next activable coroutine */
		{
			acr++;
			if (acr >= MAX_CR)
				acr = 0;
		} while ((status_cr[acr] & 1) == 0);
		memcpy (astack, stack[acr], sizeof(astack));
		break;
	    case OP_ALT:
	    new_process:
		if (nacr >= MAX_CR)
		{
		    printf ("Too many processes\n");
		    exit (-1);
		}
		for (i=0; status_cr[i] & 1; i++);
		status_cr[i] |= 1;
		cr[i/*ncr*/].calling = &calling/*[ncr]*/;
		cr[i].env = &env[ncr];
		/*memcpy (cr[ncr].calling, cr[acr].calling,
		    sizeof(*(cr[ncr].calling)));*/
		memcpy (cr[i].env, cr[acr].env,
		    sizeof(*(cr[i].env)));
		/* copie pile ? */
		memcpy (stack[i], astack, sizeof(stack[i]));
		memcpy (&tr[acr], r, sizeof(tr[acr]));
		memcpy (&tr[i], r, sizeof(tr[i]));
		tr[i].p[0] = r->p[1];
		nacr ++;
		break;

	    case OP_SEND:
		cnl = (struct canal *) (r->p[0]);
		if (cnl->file == NULL)
		/* empty file : initialize with coroutine */
		{
			cnl->flag = 1;
			item = malloc (sizeof (*item));
			cnl->file = item;
		queue_item:
			memcpy (&(item->env), cr[acr].env, sizeof (myjmp_buf));
			memcpy (&(item->r), r, sizeof(struct requete));
			memcpy (&(item->stack), astack, sizeof(astack));
			item->next = NULL;
			goto desactivate;
		}
		else if (cnl->flag)
		/* other coroutines waiting for receive, add it at tail */
		{
		struct process_list *p;
		add_tail:
			for (p = cnl->file; p->next != NULL; p = p->next);
			item = malloc (sizeof (*item));
			p->next = item;
			goto queue_item;
		}
		else
		/* realize operation */
		{
		struct process_list *p;
		if (nacr == MAX_CR)
		{
		    printf ("Too many processes\n");
		    exit (-1);
		}
		for (i=0; status_cr[i] & 1; i++);
		status_cr[i] |= 1;
		cr[i].calling = &calling;
		cr[i].env = &env[ncr];
		memcpy (cr[i].env, &(cnl->file->env),
		    sizeof(*(cr[i].env)));
		memcpy (stack[i], &(cnl->file->stack), sizeof(stack[i]));
		memcpy (&tr[acr], r, sizeof(tr[acr]));
		/* tr[acr].p[1] = cnl->file.r.p[1]; */
		memcpy (&tr[i], r, sizeof(tr[i]));
		tr[i].p[1] = cnl->file->r.p[1];
		p = cnl->file;
		cnl->file = cnl->file->next;
		free (p);
		nacr ++;

		break;

		}

	    case OP_RECEIVE:
		cnl = (struct canal *) (r->p[0]);
		if (cnl->file == NULL)
		/* empty file : initialize with coroutine */
		{
			cnl->flag = 0;
			item = malloc (sizeof (*item));
			cnl->file = item;
			goto queue_item;
		}
		else if (!cnl->flag)
		/* other coroutines waiting for receive, add it at tail */
		{
			goto add_tail;
		}
		else
		/* realize operation */
		{
		struct process_list *p;
			/* memcpy (&tr[acr], r, sizeof (struct requete));
			   tr[acr].p[1] = cnl->file->r.p[1]; */

		if (nacr == MAX_CR)
		{
		    printf ("Too many processes\n");
		    exit (-1);
		}
		for (i=0; status_cr[i] & 1; i++);
		status_cr[i] |= 1;
		cr[i].calling = &calling;
		cr[i].env = &env[ncr];
		memcpy (cr[i].env, &(cnl->file->env) /* cr[acr].env */,
		    sizeof(*(cr[i].env)));
		memcpy (stack[i], &(cnl->file->stack) /*astack*/, sizeof(stack[i]));
		memcpy (&tr[acr], r, sizeof(tr[acr]));
		tr[acr].p[1] = cnl->file->r.p[1];
		memcpy (&tr[i], r, sizeof(tr[i]));
		/* tr[i].p[0] = r->p[1]; */
		nacr ++;
		p = cnl->file;
		cnl->file = cnl->file->next;
		free (p);
		nacr ++;
		break;


		}

	    case OP_END:
	    desactivate:
		status_cr[acr] &= ~1;
		nacr--;
		if (nacr == 0)
		{
			/* printf ("No activable coroutine\n"); */
			printf (NACR_MESSAGE);
			return -1;
		}
		goto search_next;
	    case OP_TERMINATE:
		return r->p[0];
	}
	r = call_coroutine (&cr[acr], (int)&tr[acr]);

    }

}

insert_process (struct process_list *p, struct process_list *pl)
{
struct process_list *pl1, *prev;
	if (pl == NULL)
	{
		p->next = NULL;
		return p;
	}
	if (p->prio >= pl->prio)
	{
		p->next = pl;
		return p;
	}
	prev = pl;
	pl1 = pl->next;
	for (;;)
	{
		if (pl1 == NULL)
		{
			prev->next = p;
			p->prev = prev;
			p->next = NULL;
			return pl;
		}
		if (p->prio >= pl1->prio)
		{
			prev->next = p;
			p->prev = prev;
			p->next = pl1;
			pl1->prev = p;
			return pl;
		}
		prev = pl1;
		pl1 = pl1->next;
	}
}

typedef struct process_list *process_list;

process_list alloc_process (int *np, int stack_size)
{
process_list p;
	p = malloc (sizeof (struct process_list) + stack_size);
	if (p == NULL)
	{
		printf ("Too many processes (%d)\n", *np);
		exit (0);
	}
	(*np)++;
	/* printf (" %d ", *np); */
	return p;
}

free_process (process_list p, int *np)
{
	free (p);
	(*np)--;
}

process_list alloc_process1 (int *np, int stack_size)
{
process_list p;
int ns, na, fs;
	ns = (sizeof (struct process_list) + stack_size) / 16 + 1;
	_BX = ns;
	_AH = 0x48;
	geninterrupt (0x21);
	na = _BX;
	fs = _AX;
	if (na < ns)
	{
		printf ("Too many processes (%d)\n", *np);
		exit (0);
	}
	(*np)++;
	p = MK_FP (fs, 0);
	return p;
}

free_precess1 (process_list p, int *np)
{
	_ES = (long)p>>16;
	_AH = 0x49;
	geninterrupt (0x21);
	(*np)--;
}
*/

int scheduler (int (*f) (), void *p, int *astack, int stack_size,
	int options)
{
process_list pl, pp;
struct requete *r;
/* int astack [STACK_SIZE + STACK_BOTTOM]; */
int flag_send;
process_list item;
struct canal *cnl;
myjmp_buf calling;
struct coroutine cr;
process_list ctx;
int *sp;
int np;

	pl = malloc (sizeof(*pl)+stack_size);
	if (pl == NULL)
	{
		printf ("Insufficient memory\n");
		exit (0);
	}
	np = 1;
	*/
	np = 0;
	pl = alloc_process (&np, stack_size);
	/* printf ("alloc 0x%X\n", pl); */
	if (options & OPT_PRIO)
	{
		pl->next = NULL;
	}
	else
	{
		pl->next = pl;
	}
	pl->prev = pl;
	pl->prio = 0;
	pl->alt = pl;
	pl->status = 0;

	cr.calling = &calling;
	cr.env = &(pl->env);

	r = start_coroutine (&cr, f, p, (int *)((char *)astack + stack_size));

	for (;;)
	{
		flag_send = 0;
		switch (r->op)
		{
			case OP_SCHEDULE:
				memcpy (pl->stack, astack, sizeof(pl->stack)+stack_size);
							   /* sizeof(pl->stack) == STACK_BOTTOM */
			search_next:
				if (options & OPT_PRIO)
				{
					pl = insert_process (pl, pl->next);
				}
				else
					pl = pl->next;
				memcpy (astack, pl->stack, sizeof(pl->stack)+stack_size);
				pl->r.env = &(pl->env);
				break;

			case OP_ALT:
			new_process:
				pp = pl->next;
				pl->next = alloc_process (&np, stack_size);
				memcpy (&(pl->r), r, sizeof(pl->r));
				/* memcpy (&(pl->stack), &astack, sizeof(pl->stack)); */
				memcpy (pl->stack, astack, sizeof(pl->stack)+stack_size);
				memcpy (pl->next, pl, sizeof(*(pl->next))+stack_size);
				pl->next->next = pp;
				pl->next->prev = pl;
				pl->next->next->prev = pl->next;
				pl->next->r.p[0] = r->p[1];
				pl->r.env = &(pl->env);
				pl->next->r.env = &(pl->next->env);
				pl->alt = pl->next;
				pl->next->alt = pl;
				pl->status &= ~PL_STATUS_ALT;
				pl->next->status |= PL_STATUS_ALT;
				if (options & OPT_PRIO)
					pl = insert_process (pl,
						insert_process (pl->next,
							pl->next->next));
				break;

			case OP_GETPL:
				pl->r.p[0] = pl;
				pl->r.env = &(pl->env);
				break;

			case OP_GETCTX:
				ctx = (process_list)(r->p[0]);
				memcpy (ctx, pl, sizeof(ctx)+stack_size);
				memcpy (ctx->stack, astack,
					sizeof(ctx->stack)+stack_size);
				memcpy (&(ctx->r), r, sizeof(ctx->r));
				pl->r.p[1] = 0;
				pl->r.env = &(pl->env);
				break;

			case OP_SETCTX:
				ctx = (process_list)(r->p[0]);
				memcpy (&(pl->env), &(ctx->env), sizeof(pl->env));
				memcpy (&(pl->r), &(ctx->r), sizeof(pl->r));
				pl->r.p[1] = r->p[1];
				pl->r.env = &(pl->env);
				memcpy (astack, ctx->stack,
					sizeof(astack)+stack_size);
				break;

			case OP_SEND:
				flag_send = 1;
			case OP_RECEIVE:
				memcpy (&(pl->r), r, sizeof(pl->r));
				memcpy (pl->stack, astack,
						sizeof(pl->stack)+stack_size);
				cnl = (struct canal *) (r->p[0]);
				if (cnl->file == NULL)
				{
					cnl->flag = flag_send;
					item = malloc (sizeof(*item)+stack_size);
					cnl->file = item;
				queue_item:
					memcpy (item, pl, sizeof(*item)+stack_size);
					item->next = NULL;
					goto desactivate;
				}
				else if (cnl->flag == flag_send)
				{
				process_list p;
				add_tail:
					for (p = cnl->file; p->next != NULL;
						p = p->next);
					item = malloc (sizeof(*item)+stack_size);
					p->next = item;
					goto queue_item;
				}
				else
				{
					pp = pl->next;
					pl->next = alloc_process (&np, stack_size);
					/* memcpy (pl->next, pl,
						sizeof(*(pl->next))); */
					memcpy (pl->next, cnl->file,
						sizeof(*(pl->next))+stack_size);
					/* *(pl->next) = *(cnl->file); */
					pl->next->next = pp;
					pl->next->prev = pl;
					pl->next->next->prev = pl->next;
					/* pl->next->r.p[1] = cnl->file->r.p[1]; */
					if (flag_send)
						pl->next->r.p[1] = r->p[1];
					else
						pl->r.p[1] = cnl->file->r.p[1];
					pl->r.env = &(pl->env);
					pl->next->r.env = &(pl->next->env);
					if (options & OPT_PRIO)
					    pl = insert_process (pl,
						insert_process (pl->next,
						    pl->next->next));
					p = cnl->file;
					cnl->file = cnl->file->next;
					free (p);

				}
				break;

			case OP_COPYPL:
				pl = copy_process_list (pl, options);
				pl->r.env = &(pl->env);
				break;

			case OP_END:
			desactivate:
				if (pl->next == pl || pl == NULL)
				{
					/* printf ("No activable coroutine\n"); */
					printf (NACR_MESSAGE);
					return -1;
				}
				pp = pl;
				pl = pl->next;
				pl->prev = pp->prev;
				if (!(options & OPT_PRIO))
					pl->prev->next = pl;
				/* printf ("free 0x%X\n", pp); */
				/* free (pp); np--; */
				free_process (pp, &np);
				/* memcpy (&astack, &(pl->stack), sizeof(astack)); */
				memcpy (astack, pl->stack, sizeof(pl->stack)+stack_size);
				break;

			case OP_TERMINATE:
				return r->p[0];

			default:
				printf ("Unknown request 0x%X\n", r->op);
				return -1;

		}
		if (pl->status & PL_STATUS_CUT)
			goto desactivate;
		cr.calling = &calling;
		cr.env = &(pl->env);
		r = call_coroutine (&cr /*pl->env*/, &(pl->r));
	}
}

main ()
{
int stack [600 + STACK_BOTTOM];
int maincr ();
struct param_scheduler p;
 test_coroutine (); 
  new_scheduler (testalt, 0); 
    scheduler (testalt, 0); 
	p.stack_size = sizeof(stack)-STACK_BOTTOM*sizeof(int);
	scheduler (maincr, 0, stack, sizeof(stack)-STACK_BOTTOM*sizeof(int));
}


