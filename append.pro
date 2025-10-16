
append (0, L, L).
append (cons(X,A), B, cons(X,C)) :- append (A, B, C).

goal :- printstring ("""\nTest append : \n"""),
	append (A, B, cons(a,cons(b,cons(c,0)))),
	printstring ("""A = """), printexpr (A),
	printstring (""", B = """), printexpr (B),
	printstring ("""\n""").

