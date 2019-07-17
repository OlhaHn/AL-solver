#!/bin/bash
FILES=./input/test_one_folder/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done
