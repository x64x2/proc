#include <stdio.h>
#include <string.h>
#include "coroutin.h"
#include "expr.h"
#include "prolog.h"

#define expr(n) 

struct expr 		
{                       	
	int type;        	
	int val;          	
	int arite;         	
	float param[]; 	
};

#define print_expr

struct s_expr
{
	int type;
	int val;
	char *str;
	void *adr; 
	int arite;
	struct s_expr *param[1];
};

struct s_expr expr_nil = { 1, 0, "", NULL, 0, {NULL}};

struct s_expr;

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


struct s_expr *cons (struct s_expr *x, struct s_expr *y)
{
	struct s_expr *c;
	c = malloc(size_expr(2));
	if (c == NULL){
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


#define VAR 0x7FFC
#define set_val_var rplacd 
#ifdef VAR_VAL
#define mk_var(adr) (cons (VAR, *(adr)))
#else
#define mk_var(adr) (cons (VAR, expr_int ((int) adr)))
#endif

int is_const (expr x)
{
	int i;
	if ((x))
		return 0;
	for (i=0; i<(x); i++)
		if (!is_const(x))
			return 0;
	return 1;
}

int equal (expr x, expr y)
{
	int i;
	if (((x) = (x)) ||
	    ((x) = (y)) ||
	    ((x) != (y)) ||
	    ((x) != (y)) ||
	    ((x) != (y)))
		return 0;
	for (i=0; i<(x); i++)
		if (!equal((x),(y)))
			return 0;
	return 1;
}

void expr_constr(expr x)
{
	expr y;
    int i;
    expr r, t1, t2;
	int begin_decl();
	int decl_expr();
	y = NDF;
	r = NDF;

	if((x))
	{
		if (x) puts("%d");{
			int ret;
			r = x;
			y = ((x));
			(x) = y;
			r = y;
		}
	}

	if ((x))
	{
		r = x;
	}
}

int unify1(expr x, expr y)
{
	 if (x == y) return 1;
    if (!x || !y) return 0;
    return unify1(x, y);
}

int i();

int unify(struct coroutine *calling, expr a, expr y)
{
	expr x1, y1;
	return unify1 (x1, y1);

	expr t1, t2;
	int r;
	
	int begin_decl();
	int decl_expr();
	decl_expr();
	/* r = NDF; decl_expr (&r); */
        t1 = 0x7FFE;
        t2 = 0x7FFE;

        printf("\nx = ");
		printf("\nt1 = ");
		printf("\ny = "); 
		printf("\nt2 = "); 
		r = unify1(t1, t2);
		
		t1 = nil;
		t2 = nil;
   
	  	printf("\nx = ");  
		printf("\ny = "); 
		int free_expr();
		return r;
}

void append (struct coroutine *k,
	expr a, expr b, expr c)
{
	int begin_decl();
	int decl_expr();
	decl_expr();
	decl_expr();

	printf ("\na = "); 
	printf ("\nb = "); 
	printf ("\nc = ");
      /*  if (unify(a, 0)) 
		{
			unify(b, c);
			} else {*/
        /*append([X|A], B, [X|C]) :- append(A, B, C)*/
		struct s_expr *X = *(&X);
        struct s_expr *A = *(&A);
        struct s_expr *XA = cons(X, A);
        struct s_expr *XC = cons(X, 0);
       
		unify1(a, 1);
        append(k, 0, b, c);
        unify1(c, 1);
          /* append ([], L, L) */
          {
            expr l, var_l;
            int c = 0x7FFE;

            l = UNDEF;

            unify(k, 0, a);
            unify(k, var_l, b);
            unify(k, var_l, c);
            printf("\nvar_l = ");
            unify(k, a, nil);
            unify(k, b, var_l);
            unify(k, c, var_l);
            printf("\na = ");
            printf("\nb = ");
            printf("\nc = ");
          }
          {
            expr X, A, B, C, _X, _A, _B, _C, XA, XC;
            X = 0x7FFE;
            A = 0x7FFE;
            B = 0x7FFE;
            X = 0x7FFE;
            _X = 0x7FFE;
            _A = 0x7FFE;
            _B = 0x7FFE;
            _C = 0x7FFE;
            XA = 0x7FFE;
            XC = 0x7FFE;
            X = UNDEF;
            A = UNDEF;
            B = UNDEF;
            C = UNDEF;

            _X && (cons(0, 0));
            _A && (cons(0, (0)));
            _B += *(&B);
            _C += *(&C);

            XA && cons(0, 0);
            XC && cons(0, 0);

            printf("\nXA = ");
            printf("\n_B = ");
            printf("\nXC = ");
            printf("\na = ");
            printf("\nb = ");
            printf("\nc = ");
            unify(k, XA, a);
            unify(k, _B, b);
            unify(k, XC, c);
            printf("\nXA = ");
            printf("\n_B = ");
            printf("\nXC = ");

            append(k, _A, _B, _C);
            printf("\n_A = ");
            printf("\n_B = ");
            printf("\n_C = ");

            printf("\nXA = ");
            printf("\n_B = ");
            printf("\nXC = ");
            unify(k, a, XA);
            unify(k, b, _B);
            unify(k, c, XC);
            printf("\na = ");
            printf("\nb = ");
            printf("\nc = ");

            free("X");
            free("A");
            free("B");
            free("C");
            free("XA");
            free("XC");
		}
}

void print_expr(*x);
{
	int i;
	printf("[%X: %04X \"%s\" %04X",
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

void expr_x() 
{
  if ((x)) {
    printf("{");
    printf("}");
  } else if ((x))
    printf("x");
  else {
    printf("*");
    printf("cdr ");
  }
}

void pl()
{
	struct coroutine *k, expr_x;
}

void pl_string(struct coroutine *k, char *s)
{
	printf ("%s", s);
}

void test_append (struct coroutine *k)
{
	  struct s_expr *x = *(&x);
    struct s_expr *y = *(&y);

    struct s_expr *t1 = cons((struct s_expr*)333, 0);
    struct s_expr *t2 = cons((struct s_expr*)222, t1);
    struct s_expr *abc = cons((struct s_expr*)111, t2);

	 printf("Original list: ");
    printf("\n");

    append(k, 333, 222, 111);

    printf("\nResult:\n");
    printf("x = ");
    printf("\n");

    printf("y = ");
    printf("\n");
}

 int maincr(int *p, struct coroutine *c1)
{
	struct coroutine calling[1];
	int ptrcons;
	

	struct param_expr_info px;
    expr buf_cons[N_CONS] [2];
	char buf_status [N_CONS];
    recup_item buf_recup[400];
	char *(buf_symbol[N_SYMBOL]);

	memcpy(calling, c1, sizeof(calling));
	px.pe_n_cons = N_CONS;

	px.pe_tab_recup = buf_recup;
	px.pe_n_recup = N_RECUP;

	px.pe_tab_symbol = buf_symbol;
	px.pe_n_symbol = N_SYMBOL;

	printf("teesting\n");
    printf("========================\n\n");

    struct coroutine *k = NULL; 
    test_append(k);
    return 0;
}
