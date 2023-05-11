#!/bin/bash

str="hello"
echo $str

var1=5
var2=6

total=$var1+$var2
echo $total

total1=`expr $var1 + $var2`
echo $total1

var=$str$var1
echo $var
