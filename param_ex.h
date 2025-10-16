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

