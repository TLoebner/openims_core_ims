#!/bin/bash

echo "Updating the HSS database"
cd /opt/OpenIMSCore/
bash add-imscore-user_newdb.sh -u default_psap -d
bash add-imscore-user_newdb.sh -u default_psap -a

#berlin
bash add-imscore-user_newdb.sh -u police_berlin -d
bash add-imscore-user_newdb.sh -u police_berlin -a
bash add-imscore-user_newdb.sh -u ambulance_berlin -d
bash add-imscore-user_newdb.sh -u ambulance_berlin -a
bash add-imscore-user_newdb.sh -u fire_berlin -d
bash add-imscore-user_newdb.sh -u fire_berlin -a

#de germany
bash add-imscore-user_newdb.sh -u police_de -d
bash add-imscore-user_newdb.sh -u police_de -a
bash add-imscore-user_newdb.sh -u ambulance_de -d
bash add-imscore-user_newdb.sh -u ambulance_de -a
bash add-imscore-user_newdb.sh -u fire_de -d
bash add-imscore-user_newdb.sh -u fire_de -a

#es espana
bash add-imscore-user_newdb.sh -u police_es -d
bash add-imscore-user_newdb.sh -u police_es -a
bash add-imscore-user_newdb.sh -u ambulance_es -d
bash add-imscore-user_newdb.sh -u ambulance_es -a
bash add-imscore-user_newdb.sh -u fire_es -d
bash add-imscore-user_newdb.sh -u fire_es -a

#madrid
bash add-imscore-user_newdb.sh -u police_madrid -d
bash add-imscore-user_newdb.sh -u police_madrid -a
bash add-imscore-user_newdb.sh -u ambulance_madrid -d
bash add-imscore-user_newdb.sh -u ambulance_madrid -a
bash add-imscore-user_newdb.sh -u fire_madrid -d
bash add-imscore-user_newdb.sh -u fire_madrid -a

#be
bash add-imscore-user_newdb.sh -u police_be -d
bash add-imscore-user_newdb.sh -u police_be -a
bash add-imscore-user_newdb.sh -u ambulance_be -d
bash add-imscore-user_newdb.sh -u ambulance_be -a
bash add-imscore-user_newdb.sh -u fire_be -d
bash add-imscore-user_newdb.sh -u fire_be -a

#brussels
bash add-imscore-user_newdb.sh -u police_brussels -d
bash add-imscore-user_newdb.sh -u police_brussels -a
bash add-imscore-user_newdb.sh -u ambulance_brussels -d
bash add-imscore-user_newdb.sh -u ambulance_brussels -a
bash add-imscore-user_newdb.sh -u fire_brussels -d
bash add-imscore-user_newdb.sh -u fire_brussels -a

