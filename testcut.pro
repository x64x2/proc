
letter (a).
letter (b).
letter (c).
letter (d).
letter (e).
letter (f).
letter (g).
letter (h).
letter (i).

voyelle (a).
voyelle (e).
voyelle (i).

consonne (X) :- letter (X), non_voyelle (X).

non_voyelle (X) :- voyelle (X), cut, fail.
non_voyelle (X).

goal :- consonne (X), printexpr (X).

