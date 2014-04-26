/*

Ultima 9 Books to HTML Converter
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


    Utility to dump books-??.flx to HTML
    NB: Might not work for languages other than en.

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "flxx.h"



void dump_book(FILE *fin, FILE *findex, int id)
{
   FILE *fbook;
   char fname[80],
        *buf, *curr;
   unsigned int len;
   int lastfont = 0;



   sprintf(fname, "%04i.html", id);
   fbook = fopen(fname, "wt");
   if (fbook == NULL)
   {
      fprintf(stderr, "couldn't create %s\n", fname);
      exit(-1);
   }


   fread(&len, 4, 1, fin);
   buf = malloc(len);
   if (buf == NULL)
   {
      fprintf(stderr, "out of memory\n");
      exit(-1);
   }
   fread(buf, 1, len, fin);


   fprintf(fbook, "<HTML><HEAD><TITLE>%s</TITLE></HEAD>\n", buf);
   fprintf(fbook, "<BODY>\n");

   fprintf(findex, "<A HREF=%s>%s</A><BR>\n", fname, buf);


   free(buf);
   fread(&len, 4, 1, fin);
   buf = malloc(len + 1);
   if (buf == NULL)
   {
      fprintf(stderr, "out of memory\n");
      exit(-1);
   }
   fread(buf, 1, len, fin);
   buf[len] = '\0';

   curr = buf;
   while (*curr)
   {
      if (*curr == '`')
         switch (*(++curr))
         {
            case 'p':  fprintf(fbook, "\n<P>\n");
                       break;

            case 'f':  curr++;
                       if (lastfont != 0)
                          fprintf(fbook, "</H%i>", lastfont);
                       lastfont = 6 - atoi(curr);
                       fprintf(fbook, "<H%i>", lastfont);
                       break;
         }
      else
         switch (*curr)
         {
            case 0x0A:  fprintf(fbook, "<BR>\n");
                        break;

            case 0x0D:  break;

            default:    putc(*curr, fbook);
         }
      curr++;
   }
   free(buf);

   if (lastfont != 0)
      fprintf(fbook, "</H%i>", lastfont);
   fprintf(fbook, "</BODY></HTML>\n");
   fclose(fbook);
}



int main(int argc, char *argv[])
{
   FILE *fout, *fin;
   int i, max;



   if (argc != 2)
   {
      fprintf(stderr, "usage: %s <books filename>\n", argv[0]);
      exit(-1);
   }

   fin = flxx_open(argv[1]);
   if (fin == NULL)
   {
      fprintf(stderr, "couldn't open %s\n", argv[1]);
      exit(-1);
   }


   if (mkdir("u9-books", S_IWUSR) != 0)
   {
      fprintf(stderr, "unable to create u9-books directory\n");
      exit(-1);
   }
   chdir("u9-books");
   fout = fopen("index.html", "wt");
   if (fout == NULL)
   {
      fprintf(stderr, "couldn't create index.html\n");
      exit(-1);
   }


   fprintf(fout, "<HTML><HEAD><TITLE>U9 Books</TITLE></HEAD>\n");
   fprintf(fout, "<BODY>\n<H1>U9 Books</H1>\n");


   max = flxx_numitems(fin);

   for (i=0; i<max; i++)
      if (flxx_getitem(fin, i) > 0)
         dump_book(fin, fout, i);


   fprintf(fout, "</BODY></HTML>\n");
   fclose(fout);
   fclose(fin);

   return 0;
}

