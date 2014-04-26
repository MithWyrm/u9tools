/*

Ultima 9 Texture Viewer
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

*/


#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <allegro.h>
#include "flxx.h"



void exit_err(char *fmt, char *param)
{
   set_gfx_mode(GFX_TEXT, 80, 25, 0, 0);
   fprintf(stderr, fmt, param);
   exit(-1);
}



int *load_palette(void)
{
   int *pal;
   FILE *fpal;



   fpal = fopen("ankh.pal", "rb");
   if (fpal == NULL)
      exit_err("error opening ankh.pal%s\n", "");

   if ( ( pal = malloc(256*sizeof(int)) ) == NULL)
      exit_err("no memory for pallete!%s\n", "");

   fread(pal, 4, 256, fpal);
   fclose(fpal);

   return pal;
}



void show_texture(FILE *fp, int *pal, int id)
{
   unsigned char *img;
   int width, height, len;
   static int x = 0, y = 0;
   int i, j, col;



/*   clear(screen); */

   len = flxx_getitem(fp, id);
   if (len <= 0)
      return;

   fread(&width, 4, 1, fp);
   fread(&height, 4, 1, fp);
   img = malloc(width * height);
   if (img == NULL)
      exit_err("out of memory%s\n", "");

   fseek(fp, len - width * height - 8, SEEK_CUR);
   fread(img, 1, width * height, fp);

   for (i=0; i<height; i++)
      for (j=0; j<width; j++)
      {
         col = pal[img[j*width+i]];
         putpixel(screen, x+i,   y+j,   col);
      }


   x += width;
   if ((x+width) > SCREEN_W)
   {
      x = 0;
      y += height;
      if ((y+height) > SCREEN_H)
         y = 0;
   }

   free(img);
}



int main(int argc, char *argv[])
{
   FILE *fp;
   int *pal;
   int curritem = 0, numitems;
   char ch = '\0';
   int done = 0;


   allegro_init();


   if (argc != 2)
      exit_err("usage: %s <texturefile>\n", argv[0]);


   fp = flxx_open(argv[1]);
   if (fp == NULL)
      exit_err("error opening %s\n", argv[1]);
   numitems = flxx_numitems(fp);


   set_color_depth(24);
   if (set_gfx_mode(GFX_AUTODETECT, 1024, 768, 0, 0) < 0)
      exit_err("error initialising graphics:\n   %s\n", allegro_error);
   pal = load_palette();



   while (flxx_getitem(fp, curritem) == 0)
   {
      curritem++;
      if (curritem > numitems)
         exit_err("no non-zero items found!\n", "");
   }

   while (!done)
   {
      show_texture(fp, pal, curritem);
      ch = getch();
      switch (ch)
      {
         case 0x1B:  done = 1;
                     break;

         case ',':   curritem--;
                     if (curritem < 0)
                        curritem = numitems - 1;
                     while (flxx_getitem(fp, curritem) == 0)
                     {
                        curritem--;
                        if (curritem < 0)
                           curritem = numitems - 1;
                     }
                     break;

         case '.':   curritem++;
                     if (curritem >= numitems)
                        curritem = 0;
                     while (flxx_getitem(fp, curritem) == 0)
                     {
                        curritem++;
                        if (curritem >= numitems)
                           curritem = 0;
                     }
                     break;
      }
   }

   free(pal);
   fclose(fp);

   return 0;
}

