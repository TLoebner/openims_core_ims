#!/bin/bash
#
# Proxy-CSCF SA for Outgoing Replies ( UC <- PS )
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

spi_uc=$5

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

if [ "$mod" = "tunnel" ]
then
	tunnel=$pcscf-$ue
fi

setkey -c << EOF
spdadd $pcscf[$port_ps] $ue[$port_uc] tcp -P out ipsec $prot/$mod/$tunnel/unique:4 ;
spdadd $pcscf[$port_ps] $ue[$port_uc] udp -P out ipsec $prot/$mod/$tunnel/unique:4 ;
add $pcscf $ue $prot $spi_uc -m $mod -u 4 -E $ealg $ck -A  $alg $ik ;
EOF
