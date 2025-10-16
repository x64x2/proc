
lettre (a).
lettre (b).
lettre (c).
lettre (d).
lettre (e).
lettre (f).
lettre (g).
lettre (h).
lettre (i).

voyelle (a).
voyelle (e).
voyelle (i).

consonne (X) :- lettre (X), non_voyelle (X).

non_voyelle (X) :- voyelle (X), cut, fail.
non_voyelle (X).

goal :- consonne (X), printexpr (X).

