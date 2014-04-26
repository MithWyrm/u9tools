/*

Ultima 9 Sound Extraction Tool v0.001a
Copyright (C) 2000 David Churchill
Portions Copyright (C) 2000 Josef Drexler <jdrexler@julian.uwo.ca>

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


    doesn't support type 2 encoding (speech)

    information on the type 1 stereo encoding, and the decode function
    was derived from Josef Drexler's U9Decode program which can be
    retrived from http://publish.uwo.ca/~jdrexler/ultima/

*/


#include <stdio.h>     /* portable */
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "flxx.h"      /* local */


#define BUFSIZE 65536


short int coeff1[4] = { 0, 240,  460,  392 };
short int coeff2[4] = { 0,   0, -208, -220 };


struct u9snd_hdr
{
   unsigned int id;
   char         desc[0x24];
   unsigned int len,
                freq,
                bits,
                chans,
                encoding;
} __attribute__ ((packed));


struct wave_hdr
{
   char           riffid[4];
   unsigned int   rifflen;
   char           rifftype[4];

   char           fmtid[4];
   unsigned int   fmtlen;
   unsigned short format,
                  chans;
   unsigned int   freq,
                  bytespersec;
   unsigned short blockalign,
                  bitspersample;

   char           dataid[4];
   unsigned int   datalen;

} __attribute__ ((packed));


const struct wave_hdr base_wave_hdr =
{ "RIFF", 0, "WAVE", "fmt ", 16, 1, 0, 0, 0, 0, 0, "data", 0 };



FILE *write_wavehdr(char *name, unsigned int bits, unsigned int chans,
                    unsigned int freq, unsigned int datalen)
{
   FILE *fout;
   struct wave_hdr hdr;



   if ((name == NULL) || (strlen(name) == 0))
      return NULL;

   fout = fopen(name, "wb");
   if (fout == NULL)
      return NULL;


   memcpy(&hdr, &base_wave_hdr, sizeof(struct wave_hdr));

   hdr.rifflen = 8 +             /* fmt header */
                 hdr.fmtlen +    /* fmt block */
                 8 +             /* data header */
                 datalen;        /* data block */


   /* fmt block */
   hdr.chans = chans;
   hdr.freq = freq;
   hdr.blockalign = chans * (bits / 8);
   hdr.bytespersec = freq * hdr.blockalign;
   hdr.bitspersample = bits;


   /* data header */
   hdr.datalen = datalen;


   fwrite(&hdr, sizeof(struct wave_hdr), 1, fout);

   return fout;
}


short int decode(int smp, int vol, int cf1, int cf2, int p1, int p2)
{
   return (((smp << 28) >> (vol+8)) + cf1*p1 + cf2*p2 + 0x80) >> 8;
}




int extractsnd(FILE *fin, int id)
{
   struct u9snd_hdr sndhdr;
   unsigned char fname[80], *buf1;
   short int *buf2;
   FILE *fout;
   int i, len;

   short int lcf1, lcf2, rcf1, rcf2,  /* co-efficients */
             lvol, rvol,              /* volumes */
             lpr1 = 0, lpr2 = 0,      /* previous samples */
             rpr1 = 0, rpr2 = 0;


   if (flxx_getitem(fin, id) <= 0)
      return -1;

   fread(&sndhdr, sizeof(struct u9snd_hdr), 1, fin);
   sprintf(fname, "%s.wav", sndhdr.desc);


   if (sndhdr.encoding == 0)
   {
      buf1 = malloc(BUFSIZE);
      if (buf1 == NULL)
         return -ENOMEM;

      fout = write_wavehdr(fname, sndhdr.bits, sndhdr.chans,
                           sndhdr.freq, sndhdr.len);
      if (fout == NULL)
      {
         free(buf1);
         return -EIO;
      }

      len = sndhdr.len;
      while (len >= BUFSIZE)
      {
         fread(buf1, 1, BUFSIZE, fin);
         fwrite(buf1, 1, BUFSIZE, fout);
         len -= BUFSIZE;
      }
      if (len)
      {
         fread(buf1, 1, len, fin);
         fwrite(buf1, 1, len, fout);
      }

      fclose(fout);
      free(buf1);
   }
   else
   if (sndhdr.encoding == 1)
   {
      buf1 = malloc(30);
      buf2 = malloc(112);         /* (30 / 15) * 28 * 2 */
      if ((buf1 == NULL) || (buf2 == NULL))
         return -ENOMEM;

      fout = write_wavehdr(fname, sndhdr.bits, sndhdr.chans,
                           sndhdr.freq, sndhdr.len * 28 * 2 / 15);
      if (fout == NULL)
      {
         free(buf1);
         free(buf2);
         return -EIO;
      }

      len = sndhdr.len;

      if (sndhdr.chans == 1)
      {
         while (len > 0)
         {
            fread(buf1, 15, 1, fin);
            len -= 15;

            lcf1 = coeff1[buf1[0] >> 4];   lcf2 = coeff2[buf1[0] >> 4];
            lvol = buf1[0] & 0x0F;

            for (i=1; i<15; i++)
            {
               buf2[(i-1)*2] =
                  decode(buf1[i] >> 4, lvol, lcf1, lcf2, lpr1, lpr2);
               lpr2 = lpr1;
               lpr1 = buf2[(i-1)*2];

               buf2[(i-1)*2+1] =
                  decode(buf1[i] & 0x0F, lvol, lcf1, lcf2, lpr1, lpr2);
               lpr2 = lpr1;
               lpr1 = buf2[(i-1)*2+1];
            }

            fwrite(buf2, 56, 1, fout);
         }
      }
      else
      if (sndhdr.chans == 2)
      {
         while (len > 0)
         {
            fread(buf1, 30, 1, fin);
            len -= 30;

            lcf1 = coeff1[buf1[0] >> 4];   lcf2 = coeff2[buf1[0] >> 4];
            rcf1 = coeff1[buf1[0] & 0x0F]; rcf2 = coeff2[buf1[0] & 0x0F];
            lvol = buf1[1] >> 4;  rvol = buf1[1] & 0x0F;

            for (i=2; i<30; i++)
            {
               buf2[(i-2)*2] =
                  decode(buf1[i] >> 4, lvol, lcf1, lcf2, lpr1, lpr2);
               lpr2 = lpr1;
               lpr1 = buf2[(i-2)*2];

               buf2[(i-2)*2+1] =
                  decode(buf1[i] & 0x0F, rvol, rcf1, rcf2, rpr1, rpr2);
               rpr2 = rpr1;
               rpr1 = buf2[(i-2)*2+1];
            }

            fwrite(buf2, 112, 1, fout);
         }
      }

      fclose(fout);
      free(buf1);
      free(buf2);
   }


   return 0;
}



void indexsnd(FILE *fin, int id, FILE *findex)
{
   struct u9snd_hdr sndhdr;

   if (flxx_getitem(fin, id) <= 0)
      return;

   fread(&sndhdr, sizeof(struct u9snd_hdr), 1, fin);
   fprintf(findex, "%5i   %4i   %s\n",
           sndhdr.id,
           sndhdr.encoding,
           sndhdr.desc);
}


void usage(char *pname)
{
   fprintf(stderr,
           "usage: %s [-i] [-r <min>-<max>] -f <inputfile>\n"
           "       -i:  make an index, don't extract\n"
           "       -r:  work on items in the range min-max (default all items)\n"
           "       -f:  set the input file\n",
           pname);
   exit(-1);
}



int main(int argc, char *argv[])
{
   FILE *fin, *findex;
   char indexname[256], sndname[256];
   int ch, i, maxitem,
       min = -1, max = -1, makeindex = 0;


   sndname[0] = '\0';

   while ((ch=getopt(argc, argv, "r:if:")) != -1)
      switch(ch)
      {
         case 'i':  makeindex = 1;
                    break;
         case 'r':  sscanf(optarg, "%i-%i", &min, &max);
                    break;
         case 'f':  strcpy(sndname, optarg);
                    break;
         default:   usage(argv[0]);
                    break;
      }


   if (strlen(sndname) == 0)
      usage(argv[0]);


   fin = flxx_open(sndname);
   if (fin == NULL)
   {
      fprintf(stderr, "couldn't open %s\n", sndname);
      exit(-1);
   }

   if (makeindex)
   {
      strcpy(indexname, sndname);
      strcat(indexname, ".txt");
      findex = fopen(indexname, "wt");
      if (findex == NULL)
      {
         fprintf(stderr, "couldn't create %s\n", indexname);
         exit(-1);
      }
      fprintf(findex, "   id   type   description\n");
      fprintf(findex, "---------------------------------------------------\n");
   }


   maxitem = flxx_numitems(fin) - 1;

   if (min == -1)
      min = 0;
   if (max == -1)
      max = maxitem;

   if (min < 0)
      min = 0;
   else if (min > maxitem)
      min = maxitem;

   if (max < min)
      max = min;
   else if (max > maxitem)
      max = maxitem;


   for (i=min; i<=max; i++)
   {
      if (makeindex)
         indexsnd(fin, i, findex);
      else
      {
         printf("Extracting %i/%i\n", i-min+1, max-min+1);
         extractsnd(fin, i);
      }
   }


   fclose(fin);
   if (makeindex)
      fclose(findex);

   return 0;
}

