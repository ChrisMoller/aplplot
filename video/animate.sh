#!/bin/sh

xx=
qfx="⎕FX"

for i in {1..5}
do
  printf -v fn  "'file frame-%03d.png; bgwhite'" $i 
  #cmd=apl --LX \"'libaplplot.so' ⎕FX 'plot'\" --eval \"$fn\" plot stuff" 
  #cmd="apl --LX \"'libaplplot.so' ⎕FX 'plot'\"--eval \"$fn\"' plot sine $i+⍳12\""
  #cmd="apl --LX \"'libaplplot.so' ⎕FX 'plot'\"--eval \"$fn plot sine $i+⍳12\""
  #cmd="apl --LX \"'libaplplot.so' $qfx 'plot'\" --eval \"⍳4\""
  args="--LX \"'libaplplot.so' $qfx 'plot'\" --eval ⍳8"
  cmd="apl $args --eval 2○($i+⍳5)÷6"
  #echo $cmd
  echo apl  $args 
  exec apl  $args 
done
