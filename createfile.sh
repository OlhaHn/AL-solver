#!/bin/bash
FILES=./input/test/rand_4_SAT/critical_10.7_var_clause_ratio9/*
FILE=output

if [ -f "$FILE" ]; then
    echo "$FILE exist"
else
    touch $FILE
    echo -n "SAT;" >> $FILE
    for f in $FILES
    do
    echo -n "$f;" >> $FILE
    done
fi
echo "" >> $FILE
echo -n "v10000;" >> $FILE
for f in $FILES
do
  #echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f >> $FILE
done
