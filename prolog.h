
#ifdef VAR_VAL

#define MAX_NEW_CONS 120

#define UNDEF 0x7FFD
#define VAR 0x7FFC

#define mk_var(adr) (cons (VAR, *(adr)))
#define mk_var(adr) (cons (VAR, expr_int ((int) adr)))
#define pl_cut_0(k) alt_process->status |= PL_STATUS_CUT

#define pl_ccode_1(k,x) { x; }

extern struct process_list *getpl();

