#!/bin/bash

killser pcscf
setkey -F
setkey -FP

/opt/OpenIMSCore/ser_ims/ser -f /opt/OpenIMSCore/pcscf.cfg -D -D
