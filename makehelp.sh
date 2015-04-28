#!/bin/bash
pandoc -s -f markdown -t rtf -o README.rtf README.md
cat help/header.rtf >  help/AOKTS.RTF
cat README.rtf | sed '1,2d' | sed '$d' >> help/AOKTS.RTF
cat help/footer.rtf >> help/AOKTS.RTF
