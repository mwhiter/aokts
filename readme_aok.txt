AOK Trigger Studio - 1.0.0 (06 April 2010)
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
- Unreliability in loading Genie data (data_aok.xml).
- Memory leaks and crashes in trigger handling.
- Loading crashed when decompressed scenario data is unexpected.
- "<<" (clear) button didn't work in Disables editor.
- Bad-trigger "X" icon looked really gimpy.
- Trigger editor tree's edit controls (for rename) took TAB keypresses.
- Effect editor cleared resource type for Send Tribute effect.
- Removal of single item in Disables editor could later cause a crash.
- Condition editor didn't load resource type combo box.
- Unit editor could crash when user selected a different unit group.
- Scenario could be corrupted if Play Sound effects had long filenames.
- Falsely logged a warning when loading a scenario with an instructions bitmap.
- Unit selector box (for conditions/effects) did not pre-select player.
- Hiding and then re-showing map view stopped it from updating.
- Errors during scenario saving usually resulted in crashes.
- Embedded files in scenario were not properly skipped when not included.
- Map viewer did not refresh after doing a map paste.
- Scenarios with triggers with no description but "Display as Objective" set
  did not save properly. (Just re-save with 1.0.0 to fix.)
- Embedded files were not written during scenario save.
- Changing effect/condition type didn't work if you cleared.
- Leaked (sometimes lots of) memory during scenario save.
- aokts.ini would sometimes be saved to Scenario directory.

Improvements:
- Greatly improved source code structure for you curious types.
- Source code compiles with modern tools, i.e., Visual Studio 2005.
- Moving units via map clicks add 0.5 to position to emulate AOK.
- Error reports from scenario loading are more detailed.
- Genie data (data_aok.xml) supports Unicode.
- Condition, Effect editors are larger for better user-friendliness.
- Pressing RETURN opens appropriate Condition or Effect editor.
- Slightly more detailed aokts.log during scenario load.
- File -> "Write trigtext..." dumps triggers as text. WARNING: the format is
  similar BUT NOT THE SAME AS Jatayu's java program; don't try to use them
  interchangeably!
- Saving scenarios after deleting lots of triggers was very slow; sped it up.
- Map viewer automatically expands when loading a scenario, no more scrolling.

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
