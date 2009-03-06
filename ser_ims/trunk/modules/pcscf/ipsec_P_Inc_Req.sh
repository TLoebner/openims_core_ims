#!/bin/bash
#
# Proxy-CSCF SA for Incoming Requests ( UC -> PS )
#
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
# \author Laurent Etiemble laurent.etiemble -at- inexbee -dot- com
# \author Mamadou Diop mamadou.diop -at- inexbee -dot- com
#

# Strip unwanted characters that surrounds IPv6 addresses
ue=`echo $1 | tr -d "'[]"`
port_uc=$2
# Strip unwanted characters that surrounds IPv6 addresses
pcscf=`echo $3 | tr -d "'[]"`
port_ps=$4

spi_ps=$5

ealg=$6
ck=$7
alg=$8
ik=$9

prot=${10}
mod=${11}

if [ "$6" = "null" ]
then
	ck=""
fi

if [ "$mod" = "tun" ]
then
	mod="tunnel"
	tunnel=$ue-$pcscf
else
	mod="transport"
	tunnel=""
fi

setkey -c << EOF
spdadd $ue[$port_uc] $pcscf[$port_ps] tcp -P in ipsec $prot/$mod/$tunnel/require ;
spdadd $ue[$port_uc] $pcscf[$port_ps] udp -P in ipsec $prot/$mod/$tunnel/require ;
add $ue $pcscf $prot $spi_ps -m $mod -E $ealg $ck -A $alg $ik ;
EOF
