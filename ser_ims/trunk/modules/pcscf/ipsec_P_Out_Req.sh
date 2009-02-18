#!/bin/bash
#
# Proxy-CSCF SA for Outgoing Replies ( US <- PC )
#
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
# \author Laurent Etiemble laurent.etiemble -at- inexbee -dot- com
# \author Mamadou Diop mamadou.diop -at- inexbee -dot- com
#

# Strip unwanted characters that surrounds IPv6 addresses
ue=`echo $1 | tr -d "'[]"`
port_us=$2
# Strip unwanted characters that surrounds IPv6 addresses
pcscf=`echo $3 | tr -d "'[]"`
port_pc=$4

spi_us=$5

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
spdadd $pcscf[$port_pc] $ue[$port_us] tcp -P out ipsec $prot/$mod/$tunnel/unique:3 ;
spdadd $pcscf[$port_pc] $ue[$port_us] udp -P out ipsec $prot/$mod/$tunnel/unique:3 ;
add $pcscf $ue $prot $spi_us -m $mod -u 3 -E $ealg $ck -A  $alg $ik ;
EOF
