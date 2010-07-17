AOK Trigger Studio - 1.0.1 (16 July 2010)
Created by David Tombs (DiGiT)
cyan.spam@gmail.com
http://sourceforge.net/projects/aokts/


== Installation ==

Zip contents:
aokts.exe - the program
aokts.ini - configuration settings
readme.txt - this file
scx_format.txt - file format of AOK scenarios
zlib1.dll - contains DEFLATE algorithm
libexpatw.dll - contains XML-reading algorithms
source.zip - source code for geeks
data_aok.xml - genie data

Instructions:
1) Unzip aokts.exe, zlib1.dll, and libexpat.dll into a folder of your choice,
   no install program is necessary.
2) (Optional) Configure aokts.ini to your preference.
3) If you get any errors about DLLs, "Side-by-side assemblies", or "application
   configuration" being incorrect, download and install vcredist_x86.exe from
   the sourceforge.net page.


== How to Use ==

This program loads and saves Age of Empires II scenarios. It offers/will offer
more (and less) features than the standard AOK editor, including:
 * Placing Beta units just like any other unit
 * Placing units in 'formation' style
 * A trigger interface that allows negative tributes and damages
 * The ability to copy+paste triggers
 * Trigger templates to easily make common triggers

During loading/saving, a decompressed scenario called 'scendata.tmp' will be
created in the program directory.

If you have suggestions, comments, or bugs report them at
<https://sourceforge.net/projects/aokts/> (don't worry, there's an option to
request a feature too). Thanks for trying it out!


== System Requirements ==

HDD Space:	20MB (dependant on scenario size)
RAM usage:	Approximately 20MB.
Minimum OS:	Windows 2000 with IE 5.0

Basically, if you can run AOK, you can run AOKTS.

== Changes ==

Bugfixes:
- "Duplicate for all players" changed player if original player was GAIA
  (sf#2992333, thanks Impeached.)
- Effects could be corrupted when copy+pasting.
- Unit-selection popup didn't always show current selection when opening.
- Changing effect/condition type caused some combo boxes to have duplicate
  items. (sf#3018528)
- Couldn't read player data 3 in some rare instances. (sf#2953792)
- Trigger drag & drop placement was wrong. (sf#3002507)
- Pasting a condition into a trigger with no effects crashed. (sf#3026075,
  thanks again Impeached)
- Effects and conditions referencing non-stock unit constants corrupted
  scenario. (sf#3006947, thanks Wizardboy)

Improvements:
- Source now has dependencies (expat, zlib) included.
- Write trigtext now pre-fills a filename. But no read support yet. :(

Known Issues:
- Map copy area selections do not appear on map viewer.
- Window size is too small.


== Notes and legal ==

- Please do NOT save over your original scenarios with this program. I'd really
  hate for you to lose all your hard work with a bug!  
- The constant list previously included in this zip can be found under
  'Complete Constant Lists' in the AOKH utilities.  
- If anyone wants to port this for MacOS, the scenario reading itself is cross
  platform. Contact me if you're interested.

Copyright (C) 2010 David Tombs

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Zlib compression library Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler
Any loss or damage to your computer or work is not the responsibility of the
author.
