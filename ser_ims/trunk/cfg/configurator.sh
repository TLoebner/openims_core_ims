#!/bin/bash

# Initialization & global vars
clear
DDOMAIN="open-ims\.test"
DSDOMAIN="open-ims\\\.test"
DEFAULTIP="127\.0\.0\.1"
CONFFILES=`ls *.cfg *.xml *.sql 2>/dev/null`

# Interaction
printf "Domain Name:"
read domainname 
printf "IP Adress:"
read ip_address

# input domain is to be slashed for cfg regexes 
slasheddomain=`echo $domainname | sed 's/\./\\\\\\\\\./g'`

printf "File to change [\"all\" for everything, \"exit\" to quit]:"

# main loop
    while read filename ;
    do
	    if [ "$filename" = "exit" ] 
	    then 
	    printf "exitting...\n"
	    break ;

		elif [ "$filename" = "all" ]
		then    
		    printf "changing: "
		   for i in $CONFFILES 
		   do
			sed -i -e "s/$DDOMAIN/$domainname/g" $i
			sed -i -e "s/$DSDOMAIN/$slasheddomain/g" $i
			sed -i -e "s/$DEFAULTIP/$ip_address/g" $i
			
			printf "$i " 
		   done 
		   echo 
		   break;

			elif [ -w $filename ] 
			then
			    printf "changing $filename \n"
			    sed -i -e "s/$DDOMAIN/$domainname/g" $filename
			    sed -i -e "s/$DSDOMAIN/$slasheddomain/g" $filename
			    sed -i -e "s/$DEFAULTIP/$ip_address/g" $filename

				else 
				printf "cannot access file $filename. skipping... \n" 
	    fi

	    printf "File to Change:"
    done 

