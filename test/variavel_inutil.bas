00 LET fib = 1
01 LET ant1 = 0
02 PRINT fib
03 FOR i = 0 TO 20 STEP 1
04 LET ant2 = ant1
05 LET ant1 = fib
06 LET aux = ant1 + ant2
07 LET fib = ant1 + ant2
08 PRINT fib
09 NEXT i
