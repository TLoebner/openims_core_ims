#!/bin/bash
cd /opt/OpenIMSCore/ser_ims/scripts/update/
echo "Adding the mapping (civic location,serviceURN)->PSAP URI for Germany"
psql -h localhost -U postgres -d lostdb -f civic_germany.sql
echo "Adding the mapping (civic location,serviceURN)->PSAP URI for Belgium"
psql -h localhost -U postgres -d lostdb -f civic_belgium.sql
echo "Adding the mapping (civic location, serviceURN)->PSAP URI for Spain"
psql -h localhost -U postgres -d lostdb -f civic_spain.sql
