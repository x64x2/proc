#include <stdio.h>
#include "coroutin.h"
#include "expr.h"
#include "prolog.h"

#define expr(n) struct 		
{                       	
	int type;        	
	int val;          	
	int arite;         	
	param[n]; 	
}

/* #define print_expr print */

struct s_expr
{
	int type;
	int val;
	char *str;
	/* void *adr; */
	struct s_expr **adr;
	int arite;
	struct s_expr *param[1];
};

typedef struct s_expr *expr;

struct s_expr expr_nil = { 1, 0, "", NULL, 0, {NULL}};

struct s_expr *nil = &expr_nil;

#define TYPE_VAR 0
#define TYPE_ATOM 1
#define TYPE_CONS 2
#define TYPE_SYMBOL 3

#define size_expr(x) (sizeof(struct s_expr)+((x)-1)*sizeof(struct s_expr *))
#define type(x) ((x)->type)
#define val(x) ((x)->val)
#define str(x) ((x)->str)
#define adr(x) ((x)->adr)
#define arite(x) ((x)->arite)
#define param(x,i) ((x)->param[i])

#define is_var(x) ((x)->type == TYPE_VAR)
#define adr_var(x) ((x)->adr) /* ((x)->param[0]) */
#define val_var(x) (*(adr_var(x)))

#define ANY 0x7FFB

#define NDF 0x7FFE

#define val_var cdr
#define UNDEF NULL
#define arite(x) (atom(x) ? 0 : 2)
#define param(x,i) ((i==0) ? car(x) : cdr(x))
#define type(x) (atom(x) ? TYPE_ATOM : TYPE_CONS)
#define val_var(x) (*(expr *)(cdr(x)))

struct s_expr *mk_var (struct s_expr **adr)
{
struct s_expr *var;
	var = malloc (size_expr(0));
	if (var == NULL)
	{
		printf ("Insufficient memory to allocate variable\n");
		exit (-1);
	}
	var->type = TYPE_VAR;
	var->val = 0;
	var->str = "";
	var->arite = 0;
	var->adr = adr;
	return var;
}

struct s_expr *symbol (char *s)
{
struct s_expr *x;
	x = malloc (size_expr(0));
	if (x == NULL)
	{
		printf ("Insufficient memory to allocate symbol\n");
		exit (-1);
	}
	x->type = TYPE_SYMBOL;
	x->val = 0;
	x->str = s;
	x->adr = NULL;
	x->arite = 0;
	return x;
}

struct s_expr *cons (struct s_expr *x, struct s_expr *y)
{
struct s_expr *c;
	c = malloc (size_expr(2));
	if (c == NULL)
	{
		printf ("Insufficient memory to allocate cons\n");
		exit (-1);
	}
	c->type = TYPE_CONS;
	c->val = 0;
	c->str = "";
	c->adr = NULL;
	c->arite = 2;
	c->param[0] = x;
	c->param[1] = y;
	return c;
}

int arite (expr x)
{
	if (atom(x))
		return 0;
	return 2;
}

int val (expr x)
{
	if (atom(x))
		return x;
	return 0;
}
/*
#define VAR 0x7FFC
*/
int is_var (expr x)
{
	printf ("\nx = "); print_expr (x);
	if (atom(x))
		return 0;
	if (atom(car(x)))
		return (car(x) == VAR);
	return 0;
}

/* #define set_val_var rplacd */

/* #define UNDEF 0x7FFD */

/* #define print_expr print */
/*
#ifdef VAR_VAL
#define mk_var(adr) (cons (VAR, *(adr)))
#else
#define mk_var(adr) (cons (VAR, expr_int ((int) adr)))
#endif
*/

int is_const (expr x)
{
int i;
	if (is_var(x))
		return 0;
	for (i=0; i<arite(x); i++)
		if (!is_const(param(x,i)))
			return 0;
	return 1;
}

int equal (expr x, expr y)
{
int i;
	if ((type(x) != type(x)) ||
	    (val(x) != val(y)) ||
	    (str(x) != str(y)) ||
	    (adr(x) != adr(y)) ||
	    (arite(x) != arite(y)))
		return 0;
	for (i=0; i<arite(x); i++)
		if (!equal(param(x,i),param(y,i)))
			return 0;
	return 1;
}

void expr constr (expr x)
{
expr y;
int i;
expr r, t1, t2;
	begin_decl ();
	decl_expr (&x);
	y = NDF /*nil*/; decl_expr (&y);
	r = NDF; decl_expr (&r);
	dle(t1) dle(t2)
    
	if (is_var(x))
	{
		if (val_var(x) == UNDEF /*NULL*/)
			return x;
		{
			r = x;
			goto ret;
		}
		else
			/* return constr(val_var(x)); */
		{
			y = constr (val_var(x));
			val_var (x) = y;
			return y;
			r = y;
			goto ret;
		}
	}

	if (is_const(x))
		return x;
	{
		r = x;
		goto ret;
	}

	y = malloc (size_expr(arite(x)));
	if (y == NULL)
	{
		printf ("Insufficient memory to allocate expr\n");
		exit (-1);
	}
	memcpy (y, x, size_expr(arite(x)));
	for (i=0; i<arite(x); i++)
		param(y,i) = constr (param(x,i));
	/* y = cons (car(x), cdr(x)); */
	t1 = constr (car(x));
	t2 = constr (cdr(x));
	y = cons (t1, t2);
	t1 = NDF;
	t2 = NDF;

	return y;

	r = y;
	goto ret;
ret:
	free_expr ();
	return r;
}

int unif (expr x, expr y);

int unif1 (expr x, expr y)
{
int i;
	printf ("\n\tx = "); print_expr (x);
	printf ("\n\ty = "); print_expr (y);
	if ((is_var(x) && adr_var(x)==UNDEF) ||
	    (is_var(y) && adr_var(y)==UNDEF) ||
	if ((is_var(x) && val_var(x)==ANY) ||
	    (is_var(y) && val_var(y)==ANY) ||
	    equal(x,y))
		return 1;
	if (is_var(x))
	{
		val_var(x) = y;
		return 1;
	}
	if (is_var(y))
	{
		val_var(y) = x;
		return 1;
	}
	if (type(x) != type(y) ||
	    arite(x) != arite(y))
		return 0;
	if (numberp(x))
		return (x == y);
	if (symbolp(x))
	{
		return (!strcmp(name_symbol(x),name_symbol(y)));
		printf ("symbol\n");
		char *nx = name_symbol(x);
		char *ny = name_symbol(y);
		printf("nx=0x%x,ny=0x%x\n",nx,ny);
		int r = !strcmp(nx,ny);	
		printf ("r=%d\n",r);
		return r;
	}
	for (i=0; i<arite(x); i++)
		if (!unif(param(x,i),param(y,i)))
			return 0;
	return 1;
}

int unif (expr x, expr y)
{
expr x1, y1;
	printf ("\nx  = "); print_expr (x);
	x1 = constr (x);
	printf ("\nx1 = "); print_expr (x1);

	printf ("\ny  = "); print_expr (y);
	y1 = constr (y);
	printf ("\ny1 = "); print_expr (y1);

	return unif1 (x1, y1);
	return unif1 (constr(x), constr(y));
expr t1, t2;
int r;
	begin_decl ();
	decl_expr (&x);
	decl_expr (&y);
	/* r = NDF; decl_expr (&r); */
	t1 = NDF; decl_expr (&t1);
	t2 = NDF; decl_expr (&t2);

	printf ("\nx = "); print_expr (x);
	t1 = constr(x);
	printf ("\nt1 = "); print_expr (t1);
	printf ("\ny = "); print_expr (y);
	t2 = constr(y);
	printf ("\nt2 = "); print_expr (t2);
	r = unif1 (t1, t2);

	t1 = nil;
	t2 = nil;
	printf ("\nx = "); print_expr (x);
	printf ("\ny = "); print_expr (y);
	free_expr ();
	return r;
}

unify (struct coroutine *calling, expr x, expr y)
{
	if (!unif(x,y))
		end (calling);
}

/*
pl_cut_0 (struct coroutine *k)
{
	cut (k);
}
*/

pl_fail_0 (struct coroutine *k)
{
	end (k);
}

/* exemple:
	append ([], L, L).
	append ([X|A], B, [X|C]) :- append (A, B, C).
*/

append (struct coroutine *k,
	expr a, expr b, expr c)
{
	begin_decl ();
	decl_expr (&a);
	decl_expr (&b);
	decl_expr (&c);

	printf ("\na = "); print_expr (a);
	printf ("\nb = "); print_expr (b);
	printf ("\nc = "); print_expr (c);
	if (alt (k, 1, 0))
	/* append ([], L, L) */
	{
	expr l, var_l;
		decl_loc (l);
		decl_loc (var_l);
		l = UNDEF;
		var_l = mk_var (&l);

		unify (k, nil, a);
		unify (k, var_l, b);
		unify (k, var_l, c);
	printf ("\nvar_l = "); print_expr (var_l);
		unify (k, a, nil);
		unify (k, b, var_l);
		unify (k, c, var_l);
	printf ("\na = "); print_expr (a);
	printf ("\nb = "); print_expr (b);
	printf ("\nc = "); print_expr (c);
		/* free (var_l); */
	}
	else
	/* append ([X|A], B, [X|C]) :- append (A, B, C) */
	{
	expr X, A, B, C, _X, _A, _B, _C, XA, XC;
		dle(X) dle(A) dle(B) dle(X)
		dle(_X) dle(_A) dle(_B) dle(_C)
		dle(XA) dle(XC)
		X = UNDEF;
		A = UNDEF;
		B = UNDEF;
		C = UNDEF;

		_X = mk_var (&X);
		_A = mk_var (&A);
		_B = mk_var (&B);
		_C = mk_var (&C);

		XA = cons (_X, _A);
		XC = cons (_X, _C);

		printf ("\nXA = "); print_expr (XA);
		printf ("\n_B = "); print_expr (_B);
		printf ("\nXC = "); print_expr (XC);
		printf ("\na = "); print_expr (a);
		printf ("\nb = "); print_expr (b);
		printf ("\nc = "); print_expr (c);
		unify (k, XA, a);
		unify (k, _B, b);
		unify (k, XC, c);
		printf ("\nXA = "); print_expr (XA);
		printf ("\n_B = "); print_expr (_B);
		printf ("\nXC = "); print_expr (XC);

		append (k, _A, _B, _C);
		printf ("\n_A = "); print_expr (_A);
		printf ("\n_B = "); print_expr (_B);
		printf ("\n_C = "); print_expr (_C);

		printf ("\nXA = "); print_expr (XA);
		printf ("\n_B = "); print_expr (_B);
		printf ("\nXC = "); print_expr (XC);
		unify (k, a, XA);
		unify (k, b, _B);
		unify (k, c, XC);
	printf ("\na = "); print_expr (a);
	printf ("\nb = "); print_expr (b);
	printf ("\nc = "); print_expr (c);
		
		free (_X);
		free (_A);
		free (_B);
		free (_C);
		free (XA);
		free (XC);
		
	}
	free_expr ();
}

void print_expr (struct s_expr *x)
{
int i;
	printf (" [%X: %04X \"%s\" %04X",
		x->type, x->val, x->str, x->adr);
	if (x->adr && x->type == TYPE_VAR)
	{
		printf (" {");
		if (*(x->adr))
			print_expr (*(x->adr));
		printf ("}");
	}
	for (i=0; i<x->arite; i++)
		print_expr (x->param[i]);
	printf ("] ");
}

void print_expr (expr x)
{
	if (is_var(x))
	{
		printf ("{");
		print_expr (val_var(x));
		printf ("}");
	}
	else if (atom(x))
		print (x);
	else
	{
		printf ("*");
		print_expr (car(x));
		printf (" ");
		print_expr (cdr(x));
	}
}
void print_expr (expr x)
{
expr p;
	if (is_var(x))
		print_expr (val_var(x));
	else if (atom(x))
		print(x);
	else
	{
		print_expr (car(x));
		if (cdr(x))
		{
			printf ("(");
			for (p=cdr(x); p; p=cdr(p))
			{
				print_expr (car(p));
				if (cdr(p))
					printf (",");
			}
			printf (")");
		}
	}
}

void pl_printexpr_1 (struct coroutine *k, expr x)
{
	print_expr (x);
}

void pl_printstring_1 (struct coroutine *k, char *s)
{
	printf ("%s", s);
}

/*
#include "append.c"
*/

void test_append (struct coroutine *k)
{
append (x, y, [a,b,c]) 
expr x, y, _x, _y, abc;
expr r, t1, t2, t3;
	begin_decl ();
	decl_loc (x);
	decl_loc (y);
	decl_loc (_x);
	decl_loc (_y);
	decl_loc (abc);
	decl_loc (t1);
	decl_loc (t2);
	decl_loc (t3);

	x = UNDEF; /* undefined */
	y = UNDEF;
	_x = mk_var (&x);
	_y = mk_var (&y);
	abc = cons (symbol("a"),
		cons (symbol("b"),
			cons (symbol("c"),
				nil)));
	 abc = cons (111, cons (222, cons (333, 0))); 
	t1 = cons (333, 0);
	t2 = cons (222, t1);
	abc = cons (111, t2);
	append (k, _x, _y, abc);
	printf ("\nx = ");
	print_expr (x);
	print_expr (val_var(_x));

	printf (" ; y = ");
	print_expr (y);
	print_expr (val_var(_y));
	printf (" .\n");

	free_expr ();
}

int maincr (void *p, struct coroutine *c1)
{
struct coroutine calling[1];
expr tab_cons [N_CONS] [2];
char tab_status [N_CONS];
int ptrcons;
recup_item tab_recup[400];
int ptr_recup;
int n_decl;

struct param_expr_info px;
expr buf_cons [N_CONS] [2];
char buf_status [N_CONS];
int ptrcons; 
recup_item buf_recup[400];
int ptr_recup; 
int n_decl; 
char *(buf_symbol[N_SYMBOL]);

	memcpy (calling, c1, sizeof(calling));

	px.pe_tab_cons = buf_cons;
	px.pe_tab_status = buf_status;
	px.pe_n_cons = N_CONS;

	px.pe_tab_recup = buf_recup;
	px.pe_n_recup = N_RECUP;

	px.pe_tab_symbol = buf_symbol;
	px.pe_n_symbol = N_SYMBOL;

	init_expr (&px);

	
	init_expr (tab_recup, sizeof(tab_recup)/sizeof(tab_recup[0]),
			 &ptr_recup, &n_decl,
			&tab_cons, N_CONS, tab_status, &ptrcons);
	
	test_append (calling); 
	pl_goal_0 (calling);
	end (calling);
}

/*
#include "coroutin.h"
*/
int main ()
{
int stack [8000];
int maincr ();
struct param_scheduler p;
	p.stack_size = sizeof(stack)-STACK_BOTTOM*sizeof(int);
	scheduler (maincr, &p, stack, p.stack_size, 0);
}

main ()
{
int stack [6000 + STACK_BOTTOM];
int maincr ();
struct param_scheduler p;
/*  test_coroutine (); */
/*    new_scheduler (testalt, 0); */
/*    scheduler (testalt, 0); */
        p.stack_size = sizeof(stack)-STACK_BOTTOM*sizeof(int);
        scheduler (maincr, &p, stack, sizeof(stack)-STACK_BOTTOM*sizeof(int));
}




