
#include <stdio.h>

#define DEBUG

#define N_CONS 1000
#define N_RECUP 800
#define N_SYMBOL 500

#define FREE_CAR 0x7FFF

#define SYMBOL_LIMIT 0x7FF0

#define nil 0
#define true 1

typedef short expr;

typedef union
{
	int n;
	expr *adr;
} recup_item;

struct param_expr_info
{
	expr (*pe_tab_cons) [2];
	char *pe_tab_status;
	int pe_n_cons;
	int pe_ptr_cons;

	recup_item *pe_tab_recup;
	int pe_n_recup;
	int pe_ptr_recup;
	int pe_n_decl;

	char **pe_tab_symbol;
	int pe_n_symbol;
	int pe_ptr_symbol;
};

extern expr tab_cons [N_CONS] [2];
extern char tab_status [N_CONS];
extern int ptr_cons;

extern expr (*tab_cons) [2];
extern char *tab_status;
#define ptr_cons (*p_ptr_cons)
extern int ptr_cons;

#define N_SYMBOL 500
#define ptr_symbol (*p_ptr_symbol)
extern char **tab_symbol;
extern int ptr_symbol;

extern struct param_expr_info *param_expr;

#define tab_cons (param_expr->pe_tab_cons)
#define tab_status (param_expr->pe_tab_status)
#define ptr_cons (param_expr->pe_ptr_cons)

#define tab_recup (param_expr->pe_tab_recup)
#define n_recup (param_expr->pe_n_recup)
#define ptr_recup (param_expr->pe_ptr_recup)
#define n_decl (param_expr->pe_n_decl)

#define tab_symbol (param_expr->pe_tab_symbol)
#define n_symbol (param_expr->pe_n_symbol)
#define ptr_symbol (param_expr->pe_ptr_symbol)

#endif

#define cons_free(n) ((tab_status[n]&1) == 0)
#define free_cons(n) (tab_status[n] &= ~1)
#define take_cons(n) (tab_status[n] |= 1)

/* #define atom(x) ((x) >= 0 || ((x) & 0x7FFF) >= N_CONS) */

#define ATOM(x) ((x) >= 0 || ((x) & 0x7FFF) >= N_CONS)

#define INTP(x) (((x) >= 0 && (x) < 0x7FFF - N_SYMBOL) \
	|| ((x) & 0x7FFF) >= N_CONS)
    
#define num_symbol(x) (SYMBOL_LIMIT - (x))
#define symbol_num(n) (SYMBOL_LIMIT - (n))

#define name_symbol(x) (param_expr->pe_tab_symbol[num_symbol(x)])

#define car(x) (tab_cons[(x)&0x7FFF][0])
#define cdr(x) (tab_cons[(x)&0x7FFF][1])

#define NDF 0x7FFE

#define decl_loc(x) x=NDF; decl_expr(&x);

#define dle decl_loc

#define expr_int(x) (x)

