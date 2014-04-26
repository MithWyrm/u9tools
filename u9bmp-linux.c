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
#include <errno.h>
#include <ctype.h>
#include "flxx.h"

#include "SDL.h"

#define SCREEN_W 800
#define SCREEN_H 600


#define BITMAP_NAME "bitmap16.flx"
#define NUM_MODES 3

SDL_Surface *screen = NULL;


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

SDLKey getch(void) {
  SDL_Event event;
  SDLKey key;
  char quit = 0;

  while( !quit && SDL_WaitEvent( &event ) ){
    switch( event.type ){
	  case SDL_QUIT:
		key = SDLK_ESCAPE;
		quit = 1;
		break;
      case SDL_KEYUP:
		key = event.key.keysym.sym;
        quit = 1;
        break;

      default:
        break;
    }
  }

  return key;
}


/* Taken from http://sdl.beuc.net/sdl.wiki/Pixel_Access */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	if(x < 0) return;
	if(y < 0) return;
	if(x > SCREEN_W - 1) return;
	if(y > SCREEN_H - 1) return;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

void clear(SDL_Surface *surface) {
   SDL_Rect rect;

   rect.x = 0;
   rect.y = 0;
   rect.w = surface->w;
   rect.h = surface->h;
   SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 0, 0, 0));
}


void exit_err(char *fmt, char *param)
{
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
   unsigned short *img = NULL;
   int width, height, frame, maxframe;
   unsigned int ofs, len;
   int x = 0, y = 0, i, j, col;
   char fname[80] = {0};

   if (flxx_getitem(fp, id) <= 0)
      return;

   fseek(fp, 0x08, SEEK_CUR);
   fread(&maxframe, 4, 1, fp);

   fprintf(stdout, "show_texture: item has %d frames\n", maxframe);

   for (frame=0; frame<maxframe; frame++)
   {
      flxx_getitem(fp, id);

      fseek(fp, 0x10 + frame * 8, SEEK_CUR);
	  fprintf(stdout, "show_texture: frame %d, seeked to 0x%x (%ld)\n", frame, (unsigned int)ftell(fp), ftell(fp));
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
      if (img == NULL) {
         exit_err("out of memory%s\n", "");
      }

      fseek(fp, 8 + 4 * height, SEEK_CUR);
      fread(img, 1, width * height * 2, fp);

	SDL_LockSurface(screen);
      for (j=0; j<height; j++)
         for (i=0; i<width; i++)
         {
            col = img[j*width+i] & mask;
            putpixel(screen, x+i, y+j, col);
         }
	SDL_UnlockSurface(screen);

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

   SDL_Flip(screen);

	fprintf(stdout, "show_texture: finished.\n");
}



int main(int argc, char *argv[])
{
   FILE *fp = NULL;
   int curritem = 0, numitems;
   unsigned short mask = 0xFFFF;
   SDLKey key;
   int done = 0,
       wind = 0,          /* not as in air */
       refresh = 1;

   fprintf(stdout, "Opening BITMAP_NAME...\n");
   fp = flxx_open(BITMAP_NAME);
   if (fp == NULL) {
      exit_err("error opening %s\n", BITMAP_NAME);
   } else {
      fprintf(stdout, "BITMAP_NAME opened successfully.\n");
   }
   numitems = flxx_numitems(fp);


   if(SDL_Init(SDL_INIT_VIDEO) < 0) {
      fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
      exit(-1);
   } else {
      fprintf(stdout, "SDL initialized successfully.\n");
   }

   screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 16, 0);
   if(screen == NULL) {
      fprintf(stderr, "Could not set video mode: %s\n", SDL_GetError());
      exit(-1);
   }
   //allegro_init();
   //set_color_depth(16);
   //if (set_gfx_mode(GFX_AUTODETECT, gfx_modes[mode].w, gfx_modes[mode].h,
   //    0, 0) < 0)
   //   exit_err("error initialising graphics:\n   %s\n", allegro_error);


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
         //textprintf(screen, font, 10, SCREEN_H - 20, 0xFFFF,
         //           "img %i/%i | mask 0x%X", curritem, numitems-1, mask);
         fprintf(stdout, "img %i/%i | mask 0x%X\n", curritem, numitems-1, mask);
		fflush(stdout);
         refresh = 0;
      }

      key = getch();
      switch (key)
      {
		// quit program
         case SDLK_ESCAPE:
			done = 1;
            break;

		// cycle through the images
         case SDLK_RIGHT:   wind =  1;       break;
         case SDLK_LEFT:   wind = -1;       break;
         case SDLK_c:   wind =  10;      break;
         case SDLK_d:   wind = -10;      break;
         case SDLK_x:   wind =  100;     break;
         case SDLK_s:   wind = -100;     break;
         case SDLK_z:   wind =  1000;    break;
         case SDLK_a:   wind = -1000;    break;

			// hit 'f' to dump
         case SDLK_f:  clear(screen);
                     show_texture(fp, curritem, mask, 1);
                     //textprintf(screen, font, 10, SCREEN_H - 20, 0xFFFF,
                     //           "dumped.");
                     fprintf(stdout, "dumped.\n");
                     break;

		// flip the mask
         case SDLK_SPACE:   mask ^= 0x8400;  /* flip b/w 0xffff and 0x7bff */
                     refresh = 1;
                     break;
		 default:
			break;
      }

      if (wind != 0)
      {
         curritem += wind;

         if (curritem < 0)
            curritem += numitems;
         if (curritem >= numitems)
            curritem -= numitems;

		 fprintf(stdout, "looking for item %d\n", curritem);

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

		fprintf(stdout, "ending up on item %d (assuming junk inbetween?)\n", curritem);

         wind = 0;
         refresh = 1;
      }

   }

   fclose(fp);

   SDL_Quit();

   return 0;
}

