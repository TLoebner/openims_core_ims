#!/bin/bash
#

FILE=icscf.sql

mysqldump icscf -d -B --add-drop-table --add-drop-database >$FILE
mysqldump icscf -t -B  >>$FILE

echo "# DB access rights" >>$FILE
echo "grant delete,insert,select,update on icscf.* to icscf@localhost identified by 'heslo';" >>$FILE
echo "grant delete,insert,select,update on icscf.* to provisioning@localhost identified by 'provi';" >>$FILE



FILE=sip2ims.sql

mysqldump sip2ims -d -B --add-drop-table --add-drop-database >$FILE
mysqldump sip2ims -t  >>$FILE

echo "# DB access rights" >>$FILE
echo "grant delete,insert,select,update on sip2ims.* to sip2ims@localhost identified by 'heslo';" >>$FILE
echo "grant delete,insert,select,update on sip2ims.* to provisioning@localhost identified by 'provi';" >>$FILE


