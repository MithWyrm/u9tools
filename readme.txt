Ultima 9 Tool Suite
Copyright (C) 2000 David Churchill
Copyright (C) 2014 Christopher Thielen

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

The original author can be contacted by electronic mail at froods@alphalink.com.au
The original version can be found at http://home.alphalink.com.au/~froods/

The current author can be contacted via GitHub.
GitHub contact information and the latest version can be found at https://github.com/cthielen/u9tools/

Ultima 9 is probably a registered trademark of Origin Systems, Inc.
The author of this program is in no way affiliated with Origin Systems.


The unconverted tools in this package use the funky Allegro graphics library available at
http://www.talula.demon.co.uk/allegro/ and were compiled with DJGPP, a free compiler for the x86 platform, get it at http://www.delorie.com/

The updated tools use SDL (libsdl.org) and were compiled using the GNU GCC compiler.

---------
Credits
---------
The format of the type 1 encoding in the sound files was determined by Josef Drexler.
Many thanks to him as we can now extract the wonderful music from this game.  Get
Josef's U9Decode program at http://publish.uwo.ca/~jdrexler/ultima/



                         -------------------------------
                          Message from Original Release
                         -------------------------------

Hi,
   This is the first release of a set of tools to extract various pieces of 
data from the Ultima 9 data files.  Not much is completed at this stage, and
functionality is minimal.  I mostly wrote the tools to confirm my guesses as
to the data formats.  What I've discovered so far can be found in the *format.txt
files that should have come with this package.  I only have access to a DOS
compiler (DJGPP), and don't have a great deal of experience programming in C
(most of my experience is in Pascal).  If someone wants to take the information
in these text files and produce a browser of some kind that would be great.

If you've got DJGPP and Allegro installed, you should be able to make everything
by running mkall.bat (yup, still need to read up on makefiles too...), but fear
not, precompiled exe's are included.  Note that this is all distributed under
the GPL (see COPYING), and that there is no warranty - that said, it's all 
worked fine for me ;)

Please don't hesitate to send any comments to froods@alphalink.com.au

Have fun,
		Dave.




Note: Most of these programs perform little to no error checking - and will
      overwrite files that already exist without warning.  It would probably
      be wise to copy the data file you're working on to a new directory, and
      make sure you run the program from that directory.


What's Included
---------------

u9xt.exe <filename.flx> <id number> <output file>

The flx files in static and sound are just lots of little files all globbed into
one.  This program will extract the id'th (zero based) item from filename.flx
and dump it info output file.


u9books.exe <bookfile.flx>

The books file (eg. books-en.flx) contains all the books in the game, as well
as the shops (ooh, I see some cheap goods in the near future), and journal entries
for major items.  Running this will create a u9-books directory under the current
directory and dump a heap of html files in it.  Once it's finished (takes a little
time, be patient), simply open up index.html.  Note that the formatting codes
in the books file aren't translated very well yet...


u9snd.exe [-i] [-r <min>-<max>] -f <inputfile>

This program will extract all sounds except speech (type 2 encoded) from the sound
files (music.flx, speech.flx, sfx.flx).  Use -i to make the program only create an
index, -r to set the range of items to work on (get id numbers from the index), -f
to set the filename.  Only -f is required, by default the program will extract 
_all_ sounds in a particualar file.  Be sure you have a heap of space free!


u9txv.exe <texturefilename>

This program will display the textures in a texture file (only texture8.* at the
moment).  It requires 1024x768x16 to run, although this is easily changed if
you can recompile it.  Use the '<' and '>' keys (well, actually ',' and '.') to
move to new textures - it's probably better to use '>', as it currently displays
the textures sequentially across the screen.  To exit simply hit escape.


u9bmp.exe [gfx-mode-id]

This program will display the images contained in bitmap16.flx, which must be
in the current directory.  gfx-mode-id is optional; to get a list of valid
values run u9bmp with -? as a parameter.  A video card capable of a 16-bit 
resolution is needed for this.  Yours probably is, but might still not work.
If you have problems try installing a VESA BIOS driver (like the Scitech 
Display Doctor), otherwise contact me and I see if I can help out.
NB: The function to export the images to .bmp files creates 16-bit colour
    bitmap files.  AFAIK the format it produces conforms to the standard,
    and at the least mspaint will read them.  If you have trouble loading
    them into your favourite image manipulation program, try loading them
    in Paint and then saving them from there - better yet, try to fix the
    bug in my .bmp code :P


Use the following keys to browse the images:

'v' - foward 1       'f' - back 1
'c' - foward 10      'd' - back 10
'x' - foward 100     's' - back 100
'z' - foward 1000    'a' - back 1000

<cr> - dump current image(s) to .bmp files (see note above).

' ' (space) - toggle mask (removes transparency info, try this if an image looks
              whacked).
esc - exit




Other Notes of Interest:
------------------------

The texture* files in static only relate to particular maps.

Each map has a terrain, fixed, and nonfixed, all with common extensions.

There are a /heap/ of test maps!

The terrain appears to simply be a height map.

To achieve caves, rock arches you can walk under - objects are used (this can
easily be seen by setting object distance to near, terrain distance to far).

The bitmap* files have coresponding sdinfo* files, there is a bitmap-
sdinfo pair for each type of texture: 8-bit (sh), 16-bit (16), and
compressed (C).

text.flx and *brk* contain the conversation and other text, text.dat seems to 
be an index to these (try setting your language to the extension of each of the
*brk* files in turn, in options.ini!)

yiqccube.dat is probably a colour cube for a colour compression scheme used
at least on 3dfx cards.

The file 00000409.256 is a 256 colour bitmap in windows format - simply change
the extension to .bmp
