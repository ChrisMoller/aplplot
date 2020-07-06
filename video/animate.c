#include <stdio.h>
#include <stdlib.h>

int
main (int ac, char *av[])
{
  if (ac < 1) return 1;
  int c = atoi (av[1]);
  int i;
#define BFR_SZ 256
  char bfr[BFR_SZ];
  for (i = 0; i < c; i++) {
    snprintf (bfr, BFR_SZ,
	      "apl --LX \"z←'libaplplot.so' ⎕FX 'plot'\" --eval \"z←'bgblack; file frame%04d.png' plot 2○(%d+⍳128)÷6\"", i,i);
    int rc = system (bfr);
    fprintf (stderr, "%d\n", i);
  }
}
