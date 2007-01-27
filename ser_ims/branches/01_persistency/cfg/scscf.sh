#!/bin/bash

killser scscf
/opt/OpenIMSCore/ser_ims/ser -f /opt/OpenIMSCore/scscf.cfg -D -D

ipcs -s