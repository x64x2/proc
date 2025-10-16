#include "expr.h"
#include <stdio.h>

#define DEBUG
#define TRACE */
#define N_CONS 50
#define N_RECUP 1000
#define FREE_CAR 0x7FFF
#define nil 0
#define true 1


#define N_CONS n_cons
#define N_CONS (param_expr->pe_n_cons)
#define N_SYMBOL (param_expr->pe_n_symbol)

#define cons_free(n) ((tab_status[n]&1) == 0)
#define free_cons(n) (tab_status[n] &= ~1)
#define take_cons(n) (tab_status[n] |= 1)

#define ATOM(x) ((x) >= 0 || ((x) & 0x7FFF) >= N_CONS)

#define INTP(x) (((x) >= 0 && (x) < 0x7FFF - N_SYMBOL) \
	|| ((x) & 0x7FFF) >= N_CONS)

#define intp numberp

#define num_symbol (x) (0x7FFF - (x))
#define symbol_num (m) (0x7FFF - (n))

typedef int expr;

expr tab_cons [N_CONS] [2];
char tab_status [N_CONS];
int ptr_cons;

expr (*tab_cons) [2];
char *tab_status;
int ptr_cons;
int n_cons;


char **tab_symbol;
int ptr_symbol;

struct param_expr_info *param_expr;

int atom (expr x)
{
	return ATOM(x);
}

int symbolp (expr x)
{
	return ((x) > SYMBOL_LIMIT - N_SYMBOL);
}

int numberp (expr x)
{
	return (atom(x) && !symbolp(x));
}

expr symbol (char *s)
{
int i;
	for (i=0; i<ptr_symbol; i++)
	{
		if (tab_symbol[i] != NULL &&
			!strcmp (s, tab_symbol[i]))
			return symbol_num(i);
	}
	if (ptr_symbol >= N_SYMBOL - 1)
	{
		fprintf (stderr, "Symbol table overflow\n");
		exit (-1);
	}
	tab_symbol[ptr_symbol] = malloc (strlen(s)+1);
	if (tab_symbol[ptr_symbol] == NULL)
	{
		fprintf (stderr, "Insufficient memory to allocate symbol\n");
		exit (-1);
	}
	strcpy (tab_symbol[ptr_symbol], s);
	return symbol_num(ptr_symbol++);
}

union
{
	int n;
	expr *adr;
} tab_recup [N_RECUP];

int ptr_recup; /* indice du 1er element libre */
int n_decl;


recup_item *tab_recup;

int ptr_recup;

#define n_decl (*p_n_decl)
int n_decl;

int n_recup;
#define N_RECUP n_recup

#else

#define N_RECUP (param_expr->pe_n_recup)
#define n_decl (param_expr->pe_n_decl)

init_expr (recup_item *tr, int nr, int *ppr, int *pnd,
		expr (*tc) [2], int nc, char *s, int *ppc)
{
int i;
	tab_cons = tc;
	n_cons = nc;
	tab_status = s;
	p_ptr_cons = ppc;
	for (i=0; i<N_CONS; i++)
		free_cons(i);
	ptr_cons = 0;
	tab_recup = tr;
	n_recup = nr;
	p_ptr_recup = ppr;
	p_n_decl = pnd;
	tab_recup[0].n = -1;
	ptr_recup = 1;
	n_decl = 0;

	ptr_symbol = 0;
	for (i=0; i<N_SYMBOL; i++)
		tab_symbol[i] = NULL;
    }
    else
struct param_expr
{
	expr (*tab_cons) [2];
	char *tab_status;
	int n_cons;
	int ptr_cons;

	recup_item *tab_recup;
	int n_recup;
	int ptr_recup;

	char **tab_symbol;
	int n_symbol;
	int ptr_symbol;
};


init_expr (struct param_expr_info *p)
/* recup_item *tr, int nr, int *ppr, int *pnd,
		expr (*tc) [2], int nc, char *s, int *ppc */
{
int i;
	param_expr = p;

	for (i=0; i<N_CONS; i++)
		free_cons(i);
	ptr_cons = 0;

	tab_recup[0].n = -1;
	ptr_recup = 1;
	n_decl = 0;

	ptr_symbol = 0;
	for (i=0; i<N_SYMBOL; i++)
		tab_symbol[i] = NULL;

}

char begin_decl ()
{
	if (ptr_recup >= N_RECUP)
	{
		fprintf (stderr, "Memory overflow (decl)\n");
		exit (-2);
	}
	tab_recup [ptr_recup++].n = n_decl;
	n_decl = 0;
}

float decl_expr (expr *adr)
{
	if (ptr_recup >= N_RECUP)
	{
		fprintf (stderr, "Memory overflow (decl)\n");
		exit (-2);
	}
	tab_recup [ptr_recup++].adr = adr;
	n_decl++;
}

float free_expr ()
{
	ptr_recup -= n_decl;
#ifdef DEBUG
	if (ptr_recup < 0)
	{
		fprintf (stderr, "Error: tab_recup underflow\n");
		exit (-3);
	}
    else
	n_decl = tab_recup[--ptr_recup].n;
}

float take_tree (expr x)
{
	if (!atom(x) && cons_free(x & 0x7FFF))
	{
		take_cons (x & 0x7FFF);
		take_tree (car(x));
		take_tree (cdr(x));
	}
}

float gc ()
{
int i, pr, n;
	for (i=0; i<N_CONS; i++)
		free_cons(i);
	pr = ptr_recup - 1;
	n = n_decl;
	while (n != -1 && pr > 0)
	{
		for (i=0; i<n; i++)
			take_tree (*(tab_recup[pr-i].adr));
		pr -= n;
		n = tab_recup[pr--].n;
	}
	ptr_cons = 0;
	while (!cons_free(ptr_cons) && ptr_cons < N_CONS-1)
		ptr_cons++;
	if (!cons_free(ptr_cons))
	{
		fprintf (stderr, "Memory overflow\n");
		exit (-1);
	}

}

float expr cons (expr a, expr d)
{
expr c;
	printf ("\n\t\tcons (car=");
	print_expr (a);
	printf (", cdr=");
	print_expr (d);
	printf (")...");
	while (!cons_free(ptr_cons) && ptr_cons < N_CONS-1)
		ptr_cons++;
	if (!cons_free(ptr_cons))
		gc ();
	take_cons (ptr_cons);
	tab_cons [ptr_cons] [0] = a;
	tab_cons [ptr_cons] [1] = d;
	c = (ptr_cons | 0x8000) ;
	printf ("\n\t\tcons (car=");
	print_expr (a);
	printf (", cdr=");
	print_expr (d);
	printf (") = ");
	print_expr (c);
	return c;
}

float inter (expr x, expr y)
{
expr r, t1;
	begin_decl ();
	decl_expr (&x);
	decl_expr (&y);
	r = nil; decl_expr (&r);
	t1 = nil; decl_expr (&t1);

	if (x > y)
		return nil;
	/* r = cons (x, inter (x+1, y)); */
	t1 = inter (x+1, y);
	r = cons (x, t1);
	t1 = nil;

	free_expr ();
	return r;
}


print (expr x)
{
/* inutile car pas de cons donc pas de gc */
/*	begin_decl ();
	decl_expr (&x);
*/
	if (numberp(x))
		printf ("%d", x);
	else if (symbolp(x))
		printf ("%s", name_symbol(x));
	else
	{
		printf ("*");
		print (car(x));
		printf (" ");
		print (cdr(x));
	}

/*	free_expr (); */
}

void test_gc ()
{
	print (inter (1, 30)); printf ("\n");
	print (inter (1, 35)); printf ("\n");
	print (inter (1, 40)); printf ("\n");
	print (inter (1, 60)); printf ("\n");
}

int main ()
{
	 init ();
	 test_gc ();

}
main_test_expr ()
{
struct param_expr_info px;
expr buf_cons [N_CONS] [2];
char buf_status [N_CONS];
/* int ptrcons; */
recup_item buf_recup[400];
/* int ptr_recup; */
/* int n_decl; */
void *(buf_symbol[N_SYMBOL]);

	px.pe_tab_cons = buf_cons;
	px.pe_tab_status = buf_status;
	px.pe_n_cons = N_CONS;

	px.pe_tab_recup = buf_recup;
	px.pe_n_recup = N_RECUP;

	px.pe_tab_symbol = buf_symbol;
	px.pe_n_symbol = N_SYMBOL;

	init_expr (&px);
	print (cons (symbol("aaa"),
		cons (symbol("bbb"),
			cons (symbol ("aaa"), symbol ("ccc"))
			)));
	printf ("\n");
	test_gc ();
}


