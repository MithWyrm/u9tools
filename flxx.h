
/* FLX eXtractor header file */


#include <stdio.h>
#include <string.h>


FILE *flxx_open(char *fname);
int flxx_numitems(FILE *fp);
int flxx_getitem(FILE *fp, int id);


