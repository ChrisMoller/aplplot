#!/usr/bin/env perl

$max = 15;

$lx = "--LX \"z←'libaplplot.so' ⎕FX 'plot'\"";
    
for ($i = 0; $i <= $max; $i++) {
    $evalstr = sprintf("--eval \"z←'bgblack; file frame-%04d.png' plot ⍳6\"", $i);
    $cmd = "apl $lx $evalstr\n";
    print "  \n";
    print $cmd;
    print "\n  \n";
    eval $cmd
}
