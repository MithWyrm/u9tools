/*

Ultima 9 eXTractor
Copyright (C) 2000 David Churchill

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

The author can be contacted by electronic mail at froods@alphalink.com.au
The most recent version can be found at http://home.alphalink.com.au/~froods/

Ultima 9 is probably a registered trademark of Origin Systems, Inc.
The author of this program is in no way affiliated with Origin Systems.


    Usage: uxt <filename.flx> <id number> <output filename>

*/


#include <stdio.h>
#include <stdlib.h>
#include "flxx.h"

#define BUF_SIZE 32768


int main(int argc, char *argv[])
{
   FILE *fp, *fout;
   int len;
   char buf[BUF_SIZE];


   if (argc != 4)
   {
      fprintf(stderr,
              "usage: %s <filename.flx> <id number> <output filename>\n",
              argv[0]);
      exit(-1);
   }

   fp = flxx_open(argv[1]);
   if (fp == NULL)
   {
      fprintf(stderr, "error opening %s\n", argv[1]);
      exit(-1);
   }

   printf("extracting item %i (max %i)\n",
           atoi(argv[2]), flxx_numitems(fp)-1);

   len = flxx_getitem(fp, atoi(argv[2]));
   if (len < 0)
   {
      fprintf(stderr, "error extracting id %i, possibly non-existant\n",
              atoi(argv[2]));
      exit(-1);
   }


   fout = fopen(argv[3], "wb");
   if (fout == NULL)
   {
      fprintf(stderr, "error writing %s\n", argv[3]);
      exit(-1);
   }

   while (len > BUF_SIZE)
   {
      fread(buf, BUF_SIZE, 1, fp);
      fwrite(buf, BUF_SIZE, 1, fout);
      len -= BUF_SIZE;
   }

   fread(buf, len, 1, fp);
   fwrite(buf, len, 1, fout);

   fclose(fp);
   fclose(fout);

   return 0;
}

