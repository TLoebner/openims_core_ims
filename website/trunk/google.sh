#!/bin/bash

forrest site 

htmls=`find build/site -name "*.html"`

for x in $htmls
do
	cat $x | sed -f sed.script > $x
done

scp -r -2 build/site/* vingarzan@shell.berlios.de:/home/groups/openimscore/htdocs/ 