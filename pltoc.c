
/* Analyse syntaxique Prolog :

	programme ::= clause *
	clause ::= enonce corps "."
	corps ::=
	corps ::= ":-" enonce ("," enonce )*
	enonce ::= ident args
	args ::=
	args ::= "(" expr ("," expr) * ")"
	expr ::= variable
	expr ::= entier
	expr ::= enonce

   Representation d'un programme Prolog
	liste de predicats

*/

/* #define TRACE */

#include <stdio.h>
#include <errno.h>
#include "stream.h"

#define IDENT_SIZE 32
#define MAX_ARGS 16

#define PLTYPE_INT 1
#define PLTYPE_VAR 2
#define PLTYPE_SYMB 3
#define PLTYPE_STRING 4

#define MAX_STRING 1000

#define cput(put,c) ((*((put)->f)) ((put)->p, (c)))

#define RESET_NX "\t\tfor (i=0; i<pnx; i++)\n\t\t\tnx[i] = 0;\n\t\tpnx=0;\n"

#define STRING_DELIM '"'

struct param_get_file
{
	FILE *fd;
};

int f_get_file (struct param_get_file *p)
{
int c;
	c = getc (p->fd);
	printf ("%c[7m", 0x1B);
	putchar (c);
	printf ("%c[0m", 0x1B);
	return c;
}

struct param_put_file
{
	FILE *fd;
};

f_put_file (struct param_put_file *p, char c)
{
	if (putc (c, p->fd) == EOF)
	{
		fprintf (stderr, "Error writing output file\n");
		exit (-1);
	}
}

/* cput (struct put_fnct *put, char c) */
sput (struct put_fnct *put, char *s)
{
	while (*s)
		cput (put, *s++);
}

typedef struct plexpr
{
	char type;
	char *s;
	int n;
	char name[IDENT_SIZE];
	struct plexpr *args[MAX_ARGS];
} *plexpr;

struct expr_list
{
	plexpr first;
	struct expr_list *next;
};

struct clause_list
{
	plexpr args[MAX_ARGS];
	struct expr_list *corps;
	struct clause_list *next;
};

struct pred_list
{
	char name[IDENT_SIZE];
	int n_args;
	struct clause_list *clauses;
	struct pred_list *next;
};

print_plexpr (plexpr x)
{
int i;
	switch (x->type)
	{
		case PLTYPE_INT:
			printf ("%d", x->n);
			break;
		case PLTYPE_STRING:
			printf ("\"%s\"", x->s);
			break;
		case PLTYPE_VAR:
			printf ("%s", x->name);
			break;
		case PLTYPE_SYMB:
			printf ("%s", x->name);
			if (x->n)
			{
				printf (" (");
				print_plexpr (x->args[0]);
				for (i=1; i<x->n; i++)
				{
					printf (", ");
					print_plexpr (x->args[i]);
				}
				printf (")");
			}
			break;
		default:
			printf ("???");


	}

}

print_program (struct pred_list *programme)
{
struct clause_list *clause;
int i;
struct expr_list *l;
	for ( ; programme != NULL; programme = programme->next)
	{
		for (clause = programme->clauses;
			clause != NULL;
			clause = clause->next)
		{
			printf ("%s", programme->name);
			if (programme->n_args)
			{
				printf (" (");
				print_plexpr (clause->args[0]);
				for (i=1; i<programme->n_args; i++)
				{
					printf (", ");
					print_plexpr (clause->args[i]);
				}
				printf (")");
			}
			if (clause->corps)
			{
				printf (" :-\n\t");
				print_plexpr (clause->corps->first);
				for (l=clause->corps->next;
					l != NULL;
					l = l->next)
				{
					printf (",\n\t");
					print_plexpr (l->first);
				}
			}
			printf (".\n");
		}
	}
}

char buf [1000];

gen_expr (plexpr x, struct put_fnct *put)
{
int i;
	switch (x->type)
	{
		case PLTYPE_INT:
			sprintf (buf, "%d", x->n);
			sput (put, buf);
			break;
		case PLTYPE_STRING:
			sprintf (buf, "%s", x->s);
			sput (put, buf);
			break;
		case PLTYPE_VAR:
			sprintf (buf, "var_%s", x->name);
			sput (put, buf);
			break;
		case PLTYPE_SYMB:
			sprintf (buf, "nx[pnx++] = cons (symbol(\"%s\"),\n", x->name);
			sput (put, buf);
			for (i=0; i<x->n; i++)
			{
				sput (put, "\t\t\tnx[pnx++] = cons (");
				gen_expr (x->args[i], put);
				sput (put, ",\n");
			}
			sput (put, "\t\t\t0"); /* nil */
			for (i=0; i<x->n; i++)
				sput (put, ")");
			sput (put, ")");
			break;
		default:
			sprintf (buf, "Error /* Invalid type %d */", x->type);
			sput (put, buf);
	}
}

struct expr_list *aj_var (struct expr_list *vars, plexpr x)
{
struct expr_list *vars1, *ptr;

	if (vars == NULL)
	{
		vars1 = malloc (sizeof(*vars1));
		if (vars1 == NULL)
		{
			fprintf (stderr, "Insufficient memory to allocate variable list\n");
			exit (-1);
		}
		vars1->first = x;
		vars1->next = NULL;
		return vars1;
	}
	else
	{
		for (ptr = vars; ptr->next != NULL; ptr = ptr->next)
		{
			if (!strncmp (ptr->first->name, x->name, IDENT_SIZE-1))
				return vars;
		}
		if (!strncmp (ptr->first->name, x->name, IDENT_SIZE-1))
			return vars;

		vars1 = malloc (sizeof(*vars1));
		if (vars1 == NULL)
		{
			fprintf (stderr, "Insufficient memory to allocate variable list\n");
			exit (-1);
		}
		vars1->first = x;
		vars1->next = NULL;
		ptr->next = vars1;
		return vars;
	}
}

struct expr_list *aj_vars (struct expr_list *vars, plexpr x)
{
int i;
struct expr_list *vars1;
	switch (x->type)
	{
		case PLTYPE_INT:
		case PLTYPE_STRING:
			return vars;
		case PLTYPE_SYMB:
			vars1 = vars;
			for (i=0; i<x->n; i++)
				vars1 = aj_vars (vars1, x->args[i]);
			return vars1;
		case PLTYPE_VAR:
			return aj_var (vars, x);
		default:
			fprintf (stderr, "Invalid type %d\n", x->type);
			exit (-1);
	}
}

void gen_clause (struct clause_list *clause, int n_args, struct put_fnct *put)
{
struct expr_list *l, *vars, *var;
int i;

	sput (put, "\t/* clause */\n");

	vars = NULL;

	for (i=0; i<n_args; i++)
		vars = aj_vars (vars, clause->args[i]);
	for (l=clause->corps; l!=NULL; l=l->next)
		vars = aj_vars (vars, l->first);

	for (var=vars; var!=NULL; var=var->next)
	{
		sprintf (buf, "\texpr val_%s, var_%s;\n",
				var->first->name, var->first->name);
		sput (put, buf);
	}

	sput (put, "\t\talt_process = getpl (k) -> alt;\n");

	for (var=vars; var!=NULL; var=var->next)
	{
		sprintf (buf, "\t\tdle(val_%s) dle(var_%s)\n",
				var->first->name, var->first->name);
		sput (put, buf);
	}

	for (var=vars; var!=NULL; var=var->next)
	{
		sprintf (buf, "\t\tval_%s=UNDEF; var_%s=mk_var(&val_%s);\n",
				var->first->name, var->first->name,
				var->first->name);
		sput (put, buf);
	}

	sput (put, "#ifdef TRACE\n");
	for (i=0; i<n_args; i++)
	{
		sprintf (buf, "\t\tprintf (\"\\n\\ta%d = \"); print_expr (a%d);\n",
				i, i);
		sput (put, buf);
	}
	sput (put, "#endif\n");

	for (i=0; i<n_args; i++)
	{
		sput (put, "\t\tunify (k, ");
		gen_expr (clause->args[i], put);
		sprintf (buf, ", a%d);\n", i);
		sput (put, buf);
		sput (put, RESET_NX);
	}

	for (l=clause->corps; l!=NULL; l=l->next)
	{
		if (!strcmp (l->first->name, "c_code"))
		{
			sput (put, "\t/* C code inclusion */\n\t");
			sput (put, l->first->args[0]->s);
			sput (put, "\n\t/* End of C code inclusion */\n");
		}
		else
		{
			sprintf (buf, "\t\tpl_%s_%d (k",
				l->first->name, l->first->n);
			sput (put, buf);
			for (i=0; i<l->first->n; i++)
			{
				sput (put, ", ");
				gen_expr (l->first->args[i], put);
			}
			sput (put, ");\n");
			sput (put, RESET_NX);
		}
	}

	for (i=0; i<n_args; i++)
	{
		sprintf (buf, "\t\tunify (k, a%d, ", i);
		sput (put, buf);
		gen_expr (clause->args[i], put);
		sput (put, ");\n");
		sput (put, RESET_NX);
	}

	sput (put, "#ifdef TRACE\n");
	for (i=0; i<n_args; i++)
	{
		sprintf (buf, "\t\tprintf (\"\\n\\ta%d = \"); print_expr (a%d);\n",
				i, i);
		sput (put, buf);
	}
	sput (put, "#endif\n");

}

void gen_pred (struct pred_list *pred, struct put_fnct *put)
{
int i;
struct clause_list *clause;

	sprintf (buf, "void pl_%s_%d (struct coroutine *k",
			pred->name, pred->n_args);
	sput (put, buf);
	for (i=0; i<pred->n_args; i++)
	{
		sprintf (buf, ", expr a%d", i);
		sput (put, buf);
	}
	sput (put, ")\n{\nexpr nx[MAX_NEW_CONS];\n");
	sput (put, "int pnx, i;\nstruct process_list *alt_process;\n");
	sput (put, "\tpnx = 0;\n\tbegin_decl ();\n");
	for (i=0; i<pred->n_args; i++)
	{
		sprintf (buf, "\tdecl_expr (&a%d);\n", i);
		sput (put, buf);
	}
	/*
	sput (put, "\tfor (i=0; i<MAX_NEW_CONS; i++)\n\t{\n");
	sput (put, "\t\tnx[i] = 0;\n");
	sput (put, "\t\tdle (nx[i]);\n\t}\n");
	*/
	sput (put, "\tfor (i=0; i<MAX_NEW_CONS; i++)\n");
	sput (put, "\t\tdle (nx[i]);\n");

	sput (put, "#ifdef TRACE\n");
	for (i=0; i<pred->n_args; i++)
	{
		sprintf (buf, "\tprintf (\"\\n%s: a%d = \"); print_expr (a%d);\n",
				pred->name, i, i);
		sput (put, buf);
	}
	sput (put, "#endif\n");

	for (clause = pred->clauses;
		clause != NULL;
		clause = clause->next)
	{
		sput (put, "\tif (alt (k, 1, 0))\n\t{\n");
		/* sput (put, "\talt_process = getpl() -> alt;\n\t{"); */
		gen_clause (clause, pred->n_args, put);
		/* sput (put, "\t}\n\t} else\n"); */
		sput (put, "\t} else\n");
	}
	sput (put, "\tend (k);\n\tfree_expr ();\n}\n");

}

void gen_code (struct pred_list *programme, struct put_fnct *put)
{
	sprintf (buf, "/* C program translated from Prolog code */\n");
	sput (put, buf);
	sput (put, "#include \"coroutin.h\"\n");
	sput (put, "#include \"expr.h\"\n");
/*	sput (put, "#define MAX_NEW_CONS 50\n");
	sput (put, "#define UNDEF 0x7FFD\n");*/
	sput (put, "#include \"prolog.h\"\n");
	for ( ; programme != NULL; programme = programme->next)
		gen_pred (programme, put);
	sprintf (buf, "\n/* End of translation */\n");
	sput (put, buf);
}

/* example:
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

		unify (k, a, nil);
		unify (k, b, var_l);
		unify (k, c, var_l);
	}
	else
	/* append ([X|A], B, [X|C]) :- append (A, B, C) */
	if (alt (k, 1, 0))
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

		unify (k, XA, a);
		unify (k, _B, b);
		unify (k, XC, c);

		append (k, _A, _B, _C);

		unify (k, a, XA);
		unify (k, b, _B);
		unify (k, c, XC);
	} else
	end (k);
	free_expr ();
}


append (struct coroutine *k,
	expr a, expr b, expr c)
{
#ifndef OLD
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
#ifndef OLD
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
struct pred_list *aj_clause (struct pred_list *programme, char *name,
				int n_args, struct clause_list *clause)
{
struct pred_list *programme1;
struct clause_list *clause1;
	if (programme == NULL)
	{
		programme1 = malloc (sizeof(*programme1));
		if (programme1 == NULL)
		{
			perror ("Insufficient memory\n");
			exit (-1);
		}
		strncpy (programme1->name, name, IDENT_SIZE);
		programme1->n_args = n_args;
		programme1->clauses = clause;
		programme1->next = NULL;
		return programme1;
	}
	else if (!strncmp (programme->name, name, IDENT_SIZE) &&
		 programme->n_args == n_args)
	{
		for (clause1 = programme->clauses;
			clause1->next != NULL;
			clause1 = clause1->next);
		clause1->next = clause;
		return programme;
	}
	else
	{
		programme->next = aj_clause (programme->next, name, n_args,
			clause);
		return programme;
	}
}

struct get_chr
{
	int c;
	struct get_fnct *next;
	/* int (*next) ();
	   void *p; */
	int flag_string;
};

int blank (char c)
{
	return (c==' ' || c=='\t' || c=='\n' || c=='\r');
}

void next (struct get_chr *get)
{
	do
		get->c = (*(get->next->f)) (get->next->p);
	while (!(get->flag_string) && blank(get->c));
}

int aspl_str (struct get_chr *get, char *s)
{
char c;
	for (;;)
	{
		if (!*s)
			return 1;
		if (get->c != *s)
			return 0;
		next (get);
		s++;
	}
}

int aspl_ident (struct get_chr *get, char *name)
{
int i;
	i = 0;
	if (!(/*('A' <= get->c && get->c <= 'Z') ||*/
	      ('a' <= get->c && get->c <= 'z')))
		return 0;
	for (;;)
	{
		if (i < IDENT_SIZE - 1)
			name[i++] = get->c;
		next (get);
		if (!(('A' <= get->c && get->c <= 'Z') ||
		      ('a' <= get->c && get->c <= 'z') ||
		      ('0' <= get->c && get->c <= '9') ||
		      get->c == '_'))
		{
			name[i] = 0;
			return 1;
		}
	}
}

int aspl_variable (struct get_chr *get, char *name)
{
int i;
	i = 0;
	if (!(('A' <= get->c && get->c <= 'Z') || get->c == '_'/* ||
	      ('a' <= get->c && get->c <= 'z')*/))
		return 0;
	for (;;)
	{
		if (i < IDENT_SIZE - 1)
			name[i++] = get->c;
		next (get);
		if (!(('A' <= get->c && get->c <= 'Z') ||
		      ('a' <= get->c && get->c <= 'z') ||
		      ('0' <= get->c && get->c <= '9') ||
		      get->c == '_'))
		{
		      name[i] = 0;
		      return 1;
		}
	}
}

int aspl_entier (struct get_chr *get, int *val)
{
int signe;
	*val = 0;
	signe = 1;
	if (get->c == '-')
	{
		next (get);
		signe = -signe;
	}
	if (! ('0' <= get->c && get->c <= '9'))
		return 0;
	for (;;)
	{
		*val = *val * 10 + get->c - '0';
		next (get);
		if (! ('0' <= get->c && get->c <= '9'))
		{
			*val *= signe;
			return 1;
		}
	}
}

int aspl_string (struct get_chr *get, char **s)
{
char buf [MAX_STRING+1];
int i;
	next (get);
	i = 0;
	get->flag_string = 1;
	for (;;)
	{
		if (get->c == EOF)
			return 0;
		if (get->c == STRING_DELIM)
		{
			next (get);
			if (get->c == EOF)
				return 0;
			if (get->c == STRING_DELIM)
				goto store;
			break;
		}
	store:
		if (i < MAX_STRING - 1)
			buf[i++] = get->c;
		next (get);
	}
	buf[i] = 0;
	*s = malloc (i+1);
	if (*s == NULL)
	{
		fprintf (stderr, "Insufficient memory to allocate string\n");
		exit (-1);
	}
	strcpy (*s, buf);
	get->flag_string = 0;
	return 1;
}

int aspl_expr (struct get_chr *get, plexpr *x)
{
	if ('a' <= get->c && get->c <= 'z')
		return aspl_enonce (get, x);
	*x = malloc (sizeof(**x));
	if (*x == NULL)
	{
		printf ("Insufficient memory\n");
		exit (-1);
	}
	if (('A' <= get->c && get->c <= 'Z') || get->c == '_')
	{
		(*x)->type = PLTYPE_VAR;
		return aspl_variable (get, (*x)->name);
	}
	if (get->c == STRING_DELIM)
	{
		(*x)->type = PLTYPE_STRING;
		return aspl_string (get, &((*x)->s));
	}
	(*x)->type = PLTYPE_INT;
	return aspl_entier (get, &((*x)->n));
}

int aspl_args (struct get_chr *get, plexpr x)
{
	x->n = 0;
	if (get->c != '(')
		return 1;
	aspl_str (get, "(");
	aspl_expr (get, &(x->args[x->n++]));
	for (;;)
	{
		if (!aspl_str (get, ","))
			break;
		if (!aspl_expr (get, &(x->args[x->n++])))
			return 0;
	}
	return aspl_str (get, ")");
}

int aspl_enonce (struct get_chr *get, plexpr *x)
{
	*x = malloc (sizeof(**x));
	if (*x == NULL)
	{
		printf ("Insufficient memory\n");
		exit (-1);
	}
	(*x)->type = PLTYPE_SYMB;
	if (!aspl_ident (get, (*x)->name))
		return 0;
	return aspl_args (get, *x);
}

int aspl_corps (struct get_chr *get, struct expr_list **corps)
{
plexpr enonce;
struct expr_list *ptr;
	*corps = NULL;
	if (get->c != ':')
		return 1;
	*corps = malloc (sizeof(**corps));
	if (*corps == NULL)
	{
		printf ("Insufficient memory\n");
		exit (-1);
	}
	ptr = *corps;
	if (!aspl_str (get, ":-"))
		return 0;
	if (!aspl_enonce (get, &((*corps)->first)))
		return 0;
	for (;;)
	{
		if (get->c != ',')
		{
			ptr->next = NULL;
			return 1;
		}
		next (get);
		ptr->next = malloc (sizeof(*(ptr->next)));
		if (ptr->next == NULL)
		{
			printf ("Insufficient memory\n");
			exit (-1);
		}
		ptr = ptr->next;
		if (!aspl_enonce (get, &(ptr->first)))
			return 0;
	}
}

int aspl_clause (struct get_chr *get, char *name, int *n_args,
	struct clause_list **clause)
{
plexpr tete;
int i;
	if (!aspl_enonce (get, &tete))
		return 0;
	print_plexpr (tete);
	strncpy (name, tete->name, IDENT_SIZE);
	*n_args = tete->n;
	*clause = malloc (sizeof(**clause));
	if (*clause == NULL)
	{
		printf ("Insufficient memory\n");
		exit (-1);
	}
	(*clause)->next = NULL;
	for (i=0; i<tete->n; i++)
	{
		(*clause)->args[i] = tete->args[i];
		print_plexpr ((*clause)->args[i]);
	}
	if (!aspl_corps (get, &((*clause)->corps)))
		return 0;
	return aspl_str (get, ".");
}

int aspl_programme (struct get_chr *get, struct pred_list **programme)
{
struct clause_list *clause;
char name[IDENT_SIZE];
int n_args;
/*	while (aspl_clause (get)); */
	*programme = NULL;
	for (;;)
	{
		if (get->c == EOF)
			return 1;
		if (!aspl_clause (get, name, &n_args, &clause))
			return 0;
		print_program (*programme);
		*programme = aj_clause (*programme, name, n_args, clause);
		print_program (*programme);
	}
}

int aspl (struct get_fnct *get, struct pred_list **programme)
{
struct get_chr getchr;
/*struct pred_list *programme;*/
	/* getchr.c = (*(get->f)) (get->p); */
	getchr.next = get;
	getchr.flag_string = 0;
	next (&getchr);
	return aspl_programme (&getchr, programme);
}

int main (int argc, char *argv[])
{
struct get_fnct get;
struct param_get_file p_get_file;
int status;
struct pred_list *programme;
struct put_fnct put;
struct param_put_file p_put_file;

	if (argc != 3)
	{
		printf ("usage: %s source.pro output.c\n", argv[0]);
		exit (-1);
	}
	get.f = f_get_file;
	get.p = &p_get_file;

	p_get_file.fd = fopen (argv[1], "r");
	if (p_get_file.fd == NULL)
	{
		perror (argv[1]);
		return errno;
	}

	status = aspl (&get, &programme);

	if (status)
	{
		printf ("\nProgramme correct\n");
		print_program (programme);

		put.f = f_put_file;
		put.p = &p_put_file;
		p_put_file.fd = fopen (argv[2], "w");
		if (p_put_file.fd == NULL)
		{
			perror (argv[2]);
			return errno;
		}
		gen_code (programme, &put);
	}
	else
		printf ("<- Erreur syntaxique\n");
}
