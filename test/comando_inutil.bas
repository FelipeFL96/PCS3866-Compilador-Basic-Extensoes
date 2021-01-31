00 LET a = 40
01 LET b = a/5
02 PRINT “A inicial: ” a 
03 FOR i = 0 TO 20 STEP 1
04 IF a < 3 THEN 10
05 LET a = a - 2 * b
06 GOTO 08
07 PRINT “Valor corrente: ” a
08 LET j = a + 3
09 GOTO 11
10 PRINT “Valor calculado: ” j
11 NEXT i

