echo off
REM Build Prolog compiler and examples

REM Files required :
REM  - headers : coroutin.h expr.h param_ex.h prolog.h stream.h 
REM  - C sources : pltoc.c prolog.c expr.c schedule.c 
REM  - Prolog sources : append.pro member.pro parent.pro testc.pro testcut.pro

echo Build Prolog -> C translator pltoc ...
cc -g -w -o pltoc.exe pltoc.c

echo Translate and build example programs ...

for %%p in (append member parent testc testcut) do (
 echo Translate %%p.pro to %%p.c ...
 pltoc %%p.pro %%p.c
 echo Build %%p.exe ...
 cc -g -w -DVAR_VAL -o %%p.exe %%p.c prolog.c expr.c schedule.c
 echo %%p.exe built.
)

echo All programs have been built !
