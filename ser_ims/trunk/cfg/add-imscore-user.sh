#!/bin/sh
# 
# $Id$
# 
# add-imscore-user.sh
# Version: 0.1
# Released: 02/02/07
# Author: Sven Bornemann -at- materna de
#
# Script for generating two SQL scripts for creating/deleting IMS Core users 
# in the HSS and the SIP2IMS gateway tables.
#
# Example for creating user 'brooke' with password 'brooke' for realm 
# 'open-ims.test':
#
# # ./add-imscore-user.sh brooke
# Successfully wrote add-user-brooke.sql
# Successfully wrote delete-user-brooke.sql
# 
# Apply scripts with
# # mysql -u root -p < add-user-brooke.sql
# or 
# # mysql -u root -p < delete-user-brooke.sql
# 
# After applying the add script, you should be able # to register with IMS Core 
# with SIP clients (e.g. as 'brooke') via SIP2IMS. Use delete script for removing 
# the user from IMS Core database tables.
# 
# Usage for add-imscore-user.sh:
# 
# add-imscore-user.sh user [realm password]
# Default for realm is 'open-ims.test'
# Default for password is value of user
# 
# Known limits:
# * IMS Core installation in /opt/OpenIMSCore required.
# * Password is limited to 16 characters.
# 

Usage()
{
    echo "ERROR: Invalid parameters"
    echo "$0 user [realm password]"
    echo "Default for realm is 'open-ims.test'"
    echo "Default for password is value of user"
    exit -1
}

[ -z "$1" ] && Usage
PASSWORD=$3
REALM=$2
[ -z "$PASSWORD" ] && PASSWORD=$1
[ -z "$REALM" ] && REALM=open-ims.test

KEY=`/opt/OpenIMSCore/ser_ims/utils/gen_ha1/gen_ha1 $1@$REALM $REALM $PASSWORD`

CREATE_SCRIPT="add-user-$1.sql"
DELETE_SCRIPT="delete-user-$1.sql"
SED_SCRIPT="s/<USER>/$1/g"

echo -n $PASSWORD > password.txt
ENCODED_PASSWORD=`hexdump -C < password.txt|cut -b 10-60|sed 's/ //g'|cut -b 1-32`00000000000000000000000000000000
ENCODED_PASSWORD=`echo $ENCODED_PASSWORD|cut -b 1-32`

CREATE_SCRIPT_TEMPLATE="insert into hssdb.imsu(name) values ('<USER>_imsu');

--add Private Identity

--Add <USER>@$REALM
insert into hssdb.impi(
        impi_string,
        imsu_id,
        imsi,
        scscf_name,
        s_key,
        chrg_id,
        sqn)
values( '<USER>@$REALM',
        (select imsu_id from hssdb.imsu where hssdb.imsu.name='<USER>_imsu'),
        '<USER>_ISDN_User_part_ID',
        'sip:scscf.$REALM:6060',
        '$ENCODED_PASSWORD',
        (select chrg_id from hssdb.chrginfo where hssdb.chrginfo.name='default_chrg'),
        '000000000000');

--add Public Identity
insert into hssdb.impu(sip_url, tel_url, svp_id) values ('sip:<USER>@$REALM', '', (select svp_id from hssdb.svp where hssdb.svp.name='default_sp'));

--add Public Identity to Private Identity
insert into hssdb.impu2impi(impi_id, impu_id) values ((select impi_id from hssdb.impi where hssdb.impi.impi_string='<USER>@$REALM'), (select impu_id from hssdb.impu where hssdb.impu.sip_url='sip:<USER>@$REALM'));

--add roaming network
insert into hssdb.roam(impi_id, nw_id) values((select impi_id from hssdb.impi where hssdb.impi.impi_string='<USER>@$REALM'), (select nw_id from hssdb.networks where hssdb.networks.network_string='$REALM'));

-- add SIP2IMS credentials
insert into sip2ims.credentials values ('<USER>', '_none', '$REALM', '$PASSWORD',1,'','$KEY',(select imsu_id from hssdb.imsu where hssdb.imsu.name='<USER>_imsu'));"


DELETE_SCRIPT_TEMPLATE="delete from hssdb.roam where impi_id = (select impi_id from hssdb.impi where hssdb.impi.impi_string='<USER>@$REALM');

delete from hssdb.impu2impi where impi_id = (select impi_id from hssdb.impi where hssdb.impi.impi_string='<USER>@$REALM');

delete from hssdb.impi where imsu_id = (select imsu_id from hssdb.imsu where hssdb.imsu.name='<USER>_imsu');

delete from hssdb.impu where sip_url = 'sip:<USER>@$REALM';

delete from hssdb.imsu where name = '<USER>_imsu';

delete from sip2ims.credentials where uid = (select imsu_id from hssdb.imsu where hssdb.imsu.name='<USER>_imsu');"

# Create SQL add script
echo "$CREATE_SCRIPT_TEMPLATE" | sed $SED_SCRIPT > $CREATE_SCRIPT 
if [ $? -ne 0 ]; then
    echo "Failed to write $CREATE_SCRIPT"
    exit -1
fi
echo "Successfully wrote $CREATE_SCRIPT"

# Create SQL delete script
echo "$DELETE_SCRIPT_TEMPLATE" | sed $SED_SCRIPT > $DELETE_SCRIPT
if [ $? -ne 0 ]; then
    echo "Failed to write $DELETE_SCRIPT"
    exit -1
fi
echo "Successfully wrote $DELETE_SCRIPT"
