#!/bin/bash

projects=(ser_ims FHoSS JavaDiameterPeer)
#projects=(JavaDiameterPeer)

for x in $(seq 0 $((${#projects[@]}-1)))
do
	y=${projects[$x]}
	echo "Doing: "$y 
	rm -rf $y*
	#rev="Exported revision 19."
	rev=`svn export http://svn.berlios.de/svnroot/repos/openimscore/$y/trunk $y|grep "Exported revision"`
	
	rev=${rev:18}
	rev=${rev[@]//./}
	echo $rev
	
	tar -czvf $y.$rev.tgz $y
	
	cd $y/doxygen
	doxygen doxygen.config
	mv html $y
	chmod -R g+w $y

	scp -2 -r $y vingarzan@shell.berlios.de:/home/groups/openimscore/htdocs/docs/
	ssh vingarzan@shell.berlios.de chmod -R g+w /home/groups/openimscore/htdocs/docs/$y
	
	cd ../..
	
	scp -2 $y.$rev.tgz vingarzan@shell.berlios.de:/home/groups/openimscore/htdocs/snapshots/
	ssh vingarzan@shell.berlios.de chmod -R g+w /home/groups/openimscore/htdocs/snapshots/$y.$rev.tgz
done
