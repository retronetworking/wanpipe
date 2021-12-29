#!/bin/sh

PATTERNS="*.txt *.c *.h *.sh *.sample"

for PATTERN in $PATTERNS
do
    find . -name "$PATTERN" -prune -exec dos2unix \{\} \;
#    find . -name "$PATTERN" -prune -exec svn propset svn:eol-style LF \{\} \;
	echo "-----------------"
done

echo
echo "don't forget to execute 'svn commit'."
