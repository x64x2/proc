
grandparent(A,C) :- parent(A,B), parent(B,C).

parent(alphonse,barnabe).
parent(alphonse,brigitte).
parent(barnabe,catherine).
parent(brigitte,claude).
parent(brigitte,chloe).
parent(didier,evelyne).

goal :- grandparent(alphonse,X), printexpr(X), printstring ("""\n""").

