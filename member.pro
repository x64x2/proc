
member (X, cons(X,L)).
member (X, cons(Y,L)) :- member (X, L).

goal :- member (b, cons (a, cons (X, cons (c, 0)))),
	printexpr (X),
	printstring ("""\n""").


