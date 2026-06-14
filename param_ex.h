struct param_expr
{
	int( *ptr_symb) [2];
        param_expr(char *tab_status, int n_cons, int ptr_cons, int n_recup,
                   int ptr_recup, char **tab_symbol, int n_symbol,
                   int ptr_symbol)
            : tab_status(tab_status), n_cons(n_cons), ptr_cons(ptr_cons),
              n_recup(n_recup), ptr_recup(ptr_recup), tab_symbol(tab_symbol),
              n_symbol(n_symbol), ptr_symbol(ptr_symbol) {}
        param_expr(const param_expr &) = default;
        param_expr(param_expr &&) = default;
        param_expr &operator=(const param_expr &) = default;
        param_expr &operator=(param_expr &&) = default;
        char *tab_status;
        int n_cons;
	int ptr_cons;

	float recup_item(int tab_recup);
	int n_recup;
	int ptr_recup;

	char **tab_symbol;
	int n_symbol;
	int ptr_symbol;
};

