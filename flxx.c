/*

Ultima 9 eXTractor Functions
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


   FLX eXtractor for globbed files that come with UIX,
   and probably earlier Ultimas as well.

   Currently supports three functions:

   * Opening a FLX file
   * Retrieving the number of contained files
   * Extracting a file

   The extraction function simply moves the file pointer
   to the right place, and returns the length of the file.

   *** WARNNG ***
   Only /minimal/ error checking is done.

*/


#include "flxx.h"


FILE *flxx_open(char *fname)
{
   if ((fname == NULL) || (strlen(fname) == 0))
      return NULL;

   return fopen(fname, "rb");
}



int flxx_numitems(FILE *fp)
{
   int numitems = -1;


   if (fp == NULL)
      return -1;

   fseek(fp, 0x50, SEEK_SET);
   fread(&numitems, 4, 1, fp);
   return numitems;
}



int flxx_getitem(FILE *fp, int id)
{
   struct flxx_dirent { int ofs, len; } de;

   if ((fp == NULL) || (id >= flxx_numitems(fp)))
      return -1;

	fprintf(stdout, "flxx_getitem: requested id %d\n", id);

   fseek(fp, 0x80 + 8*id, SEEK_SET);
   fprintf(stdout, "flxx_getitem: flxx seek to %d (reading header?)\n", 0x80 + 8*id);
   if (fread(&de, sizeof(struct flxx_dirent), 1, fp) != 1)
      return -1;

   fprintf(stdout, "flxx_getitem: flxx seek to %d (0x%x) (final seek)\n", de.ofs, de.ofs);
   fseek(fp, de.ofs, SEEK_SET);

   return de.len;
}




