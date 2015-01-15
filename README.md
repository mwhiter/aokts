Trigger Studio
------------------

**Trigger Studio**, otherwise known as AOK Trigger Studio or AOKTS, is
an open-source utility for Age of Empires II Scenario Designers that
serves as a supplement to the in-game editor. It equips designers to do
things that aren't possible or are very time consuming with the standard
editor. It is written in C++ using the standard Win32 API.

Visit AoK Heaven to give feedback!
[AoK Heaven](http://aok.heavengames.com/blacksmith/showfile.php?fileid=12103&amp;f=&amp;st=40&amp;ci=)

You can:

* Decompress a scenario for editing with a hex editor.
* Convert scenarios between different versions of the game.
* Manipulate unit type, rotation, position, elevation, garrison, ID number etc. to enable scenario tricks such as controlling another player's units, invincible units, invisible units, putting fire on the map, unlocking beta version units to name a few.
* Allow negative (silent) tributes and negative damages (for buffing units).
* Export/View and edit AI files included in a scenario.
* Copy, Cut, Paste and Rearrange groups of triggers.
* Copy, Cut, Paste, Move and Duplicate map units, terrain, elevation and/or triggers.
* Map displays all units, terrains, elevation and triggers and can zoom and save as a bitmap file.
* Group units with custom lists, other than the 4 standard AOK groups.
* Create a scenario from scratch.
* And much, much more...

How to decompress or compress raw data

You have two options:

* Open AOKTS and click the appropriate menu item under "Tools". Select
  the source and destination file.
* Run AOKTS from the Windows Run dialog. Use -c to compress and -u to
  decompress. After that, type the path to the source file, then the
  path to the destination file.

How to save hex-edited data to a scenario.

* Open the scenario you want to hex edit in AOKTS.
* Leave AOKTS open and open scendata.tmp in your preferred hex editor.
* Make desired changes to scendata.tmp, save, and close the hex editor.
* Activate AOKTS and select "Save scendata.tmp to SCX..." from the Scenario menu.
  Specify the file to save. All your changes to scendata.tmp will be
  saved to the scenario. No changes made in AOKTS will be saved.

Credit
------

### Created by David Tombs (DiGiT) ###
DIGIT thanks the following people: Berserker Jerker, zyxomma, geebert,
scenario_t_c, and iberico! Geebert gets extra credit for his great
reports when I release a new version!

### Version 1.0.2 by danielpereira ###
* Area value for Change Object Name, Patrol effects and for Own Fewer
* objects condition working.
* New effects show proper names, and can be viewed and edited properly
* Reverse conditions working (Just change the value of "Reverse" in conditions from -1 to -256)

### Version 1.1 by JustTesting1234 ###
* Fixed trigger names extra characters on copy-paste.
* Fixed list of disables (aof only).
* LudiKRIS (480x480) map size support added (aof only).
* Added Lock Teams checkbox (aof only).

### Version 1.2 by E/X mantis ###
I would like to thank the following people! Gallas_HU (voobly), [MMCrew]jizzy, Rewaider and Lord Basse.

TODO and Pressing Bugs
----------------------
See [todo.md](todo.md)

License
-------
**GNU GPLv2** or later; see [legal/gpl-2.0.txt](legal/gpl-2.0.txt) or [legal/gpl-3.0.txt](legal/gpl-3.0.txt).