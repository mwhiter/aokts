AOK Trigger Studio
------------------

**AOK Trigger Studio** is an open-source
utility for Age of Empires II Scenario Designers that serves as a
supplement to the standard AOK editor. It allows a designer to do things
that just aren't possible and are just very time consuming with the
standard editor. It is written in C++ using the standard Win32 API.

What AOKTS Can Do:

* Decompress a scenario into a format readable by geeks.
* Place units anywhere on a map, including on the same tile as another unit.
* Place Beta units just like any other unit.
* Allow negative tributes and damages.
* Export AI files included in a scenario.
* Copy, Cut, and Paste triggers.
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
* Activate AOKTS and select "Save As (hex)" from the Scenario menu.
  Specify the file to save. All your changes to scendata.tmp will be
  saved to the scenario. No changes made in AOKTS will be saved.

Credit
------

### Created by David Tombs (DiGiT) ###
DIGIT thanks the following people: Berserker Jerker, zyxomma, geebert,
scenario_t_c, and iberico! Geebert gets extra credit for his great
reports when I release a new version!

### Since 2013 ###
JustTesting1234 aka. BlankName and myself (Shane Mulligan) (steam name
E/X mantis) were working on separate versions then found each other
towards the end of the year and have shared all our code!
I would like to thank the following people for providing outstanding
feedback! Gallas_HU (voobly), [MMCrew]jizzy, Rewaider and Lord Basse.

TODO and Pressing Bugs
----------------------
See [todo.md](todo.md)

License
-------
**GNU GPLv2** or later; see [legal/gpl-2.0.txt](legal/gpl-2.0.txt) or [legal/gpl-3.0.txt](legal/gpl-3.0.txt).