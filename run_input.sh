#!/bin/bash
FILES=./input/test/rand_4_SAT/critical_10.7_var_clause_ratio9/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done
