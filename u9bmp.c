/*

Ultima 9 Bitmap Viewer
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
#include <string.h>
#include <conio.h>
#include <errno.h>
#include <ctype.h>
#include <allegro.h>
#include "flxx.h"


#define BITMAP_NAME "bitmap16.flx"
#define NUM_MODES 3


struct bmp16hdr_struct
{
   char  id1, id2;
   int   filesize;
   int   res,       /* reserved */
         dataofs,
         hdrsize,
         width,
         height;
   short planes,
         bpp;
   int   comp,
         datasize,
         hres,
         vres,
         colors,
         impcolors,
         mask_red,
         mask_green,
         mask_blue;
} __attribute__ ((packed));


struct bmp16hdr_struct base_hdr =
{ 'B', 'M', 0, 0, 0x42, 0x28, 0, 0, 1, 16, 3, 0, 0, 0, 0, 0,
   0xF800, 0x07E0, 0x001F };
/* base_hdr contains the correct values for a 16-bit colour
   bitmap.  fields that need to be filled in are filesize, width,
   height, datasize. */



void exit_err(char *fmt, char *param)
{
   set_gfx_mode(GFX_TEXT, 80, 25, 0, 0);
   fprintf(stderr, fmt, param);
   exit(-1);
}



int write_bmp16(char *name, char *img, int width, int height)
{
   FILE *fo;
   int padlen = 0, i;
   char padding[4] = {'\0', '\0', '\0', '\0'};
   struct bmp16hdr_struct bmphdr;


   fo = fopen(name, "rb");
   if (fo != NULL)
   {
      fclose(fo);
      return -EEXIST;
   }

   fo = fopen(name, "wb");
   if (fo == NULL)
      return -errno;


   if ((width*2) % 4)                /* each scanline must end on a 4-byte */
      padlen = 4 - ((width*2) % 4);  /* boundary */

   memcpy(&bmphdr, &base_hdr, sizeof(struct bmp16hdr_struct));
   bmphdr.width = width;
   bmphdr.height = height;
   bmphdr.datasize = height * (width * 2 + padlen);
   bmphdr.filesize = sizeof(struct bmp16hdr_struct) + bmphdr.datasize;

   fwrite(&bmphdr, sizeof(struct bmp16hdr_struct), 1, fo);

   img += (height - 1) * width * 2;

   for (i=0; i<height; i++)
   {
      fwrite(img, width * 2, 1, fo);
      if (padlen)
         fwrite(padding, padlen, 1, fo);
      img -= width * 2;
   }


   fclose(fo);
   return 0;
}



void show_texture(FILE *fp, int id, unsigned short mask, int dump)
{
   unsigned short *img;
   int width, height, frame, maxframe;
   unsigned int ofs, len;
   int x = 0, y = 0, i, j, col;
   char fname[80];


   if (flxx_getitem(fp, id) <= 0)
      return;

   fseek(fp, 0x08, SEEK_CUR);
   fread(&maxframe, 4, 1, fp);

   for (frame=0; frame<maxframe; frame++)
   {
      flxx_getitem(fp, id);
      fseek(fp, 0x10 + frame * 8, SEEK_CUR);
      fread(&ofs, 4, 1, fp);
      fread(&len, 4, 1, fp);
      fseek(fp, ofs - 0x18 - frame * 8, SEEK_CUR);

      fseek(fp, 4, SEEK_CUR);
      fread(&width, 4, 1, fp);
      if ((frame != 0) && (x + width > SCREEN_W))
      {
         x = 0;
         y += height + 2;
      }
      fread(&height, 4, 1, fp);

      img = malloc(width * height * 2);
      if (img == NULL)
         exit_err("out of memory%s\n", "");

      fseek(fp, 8 + 4 * height, SEEK_CUR);
      fread(img, 1, width * height * 2, fp);

      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
         {
            col = img[j*width+i] & mask;
            putpixel(screen, x+i, y+j, col);
         }

      if (dump)
      {
         for (i=0; i<height*width; i++)
            img[i] &= mask;
         sprintf(fname, "u9-%04i-%03i.bmp", id, frame);
         write_bmp16(fname, (char *)img, width, height);
      }


      free(img);

      x += width + 2;
   }
}



int main(int argc, char *argv[])
{
   FILE *fp;
   int curritem = 0, numitems, i;
   unsigned short mask = 0xFFFF;
   char ch = '\0';
   int done = 0,
       wind = 0,          /* not as in air */
       mode = 0,
       refresh = 1;

   struct gfx_modes_struct { int w,h; } gfx_modes[NUM_MODES] =
      { {640, 480}, {800, 600}, {1024, 768} };


   if (argc != 0)
   {
      if ( (strchr(argv[1], '?') != NULL) ||
           (strchr(argv[1], 'h') != NULL) ||
           (strchr(argv[1], 'H') != NULL) )
      {
         fprintf(stderr, "usage: %s [gfx-mode-id]\n", argv[0]);
         fprintf(stderr, "gfx-mode-id is one of:\n");
         for (i=0; i<NUM_MODES; i++)
            fprintf(stderr, "  %i => %ix%ix16 %s\n",
                    i, gfx_modes[i].w, gfx_modes[i].h,
                    (i == 0) ? "(default)" : "");
         exit(-1);
      }

      mode = atoi(argv[1]);
      if ((mode < 0) || (mode >= NUM_MODES))
         mode = 0;
   }


   fp = flxx_open(BITMAP_NAME);
   if (fp == NULL)
      exit_err("error opening %s\n", BITMAP_NAME);
   numitems = flxx_numitems(fp);


   allegro_init();
   set_color_depth(16);
   if (set_gfx_mode(GFX_AUTODETECT, gfx_modes[mode].w, gfx_modes[mode].h,
       0, 0) < 0)
      exit_err("error initialising graphics:\n   %s\n", allegro_error);


   while (flxx_getitem(fp, curritem) <= 0)
   {
      curritem++;
      if (curritem > numitems)
         exit_err("no non-zero items found!!\n", "");
   }



   while (!done)
   {
      if (refresh)
      {
         clear(screen);
         show_texture(fp, curritem, mask, 0);
         textprintf(screen, font, 10, SCREEN_H - 20, 0xFFFF,
                    "img %i/%i | mask 0x%X", curritem, numitems-1, mask);
         refresh = 0;
      }

      ch = toupper(getch());
      switch (ch)
      {
         case 0x1B:  done = 1;
                     break;

         case 'V':   wind =  1;       break;
         case 'F':   wind = -1;       break;
         case 'C':   wind =  10;      break;
         case 'D':   wind = -10;      break;
         case 'X':   wind =  100;     break;
         case 'S':   wind = -100;     break;
         case 'Z':   wind =  1000;    break;
         case 'A':   wind = -1000;    break;

         case 0x0D:  clear(screen);
                     show_texture(fp, curritem, mask, 1);
                     textprintf(screen, font, 10, SCREEN_H - 20, 0xFFFF,
                                "dumped.");
                     break;

         case ' ':   mask ^= 0x8400;  /* flip b/w 0xffff and 0x7bff */
                     refresh = 1;
                     break;
      }

      if (wind != 0)
      {
         curritem += wind;

         if (curritem < 0)
            curritem += numitems;
         if (curritem >= numitems)
            curritem -= numitems;

         while (flxx_getitem(fp, curritem) <= 0)
         {
            if (wind < 0)
               curritem--;
            else
               curritem++;

            if (curritem < 0)
               curritem = numitems - 1;
            if (curritem >= numitems)
               curritem = 0;
         }

         wind = 0;
         refresh = 1;
      }

   }

   fclose(fp);

   return 0;
}

