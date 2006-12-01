#!/usr/bin/perl 
#   written by Umut Emin aka jsbach
#   umut.emin at fokus dot fraunhofer dot de
#
#
#

use warnings ;
use strict ;


use Cwd ;
use File::Find ;
use Archive::Tar ;
use Net::SSH::Perl ;
use Net::SSH::Perl::Auth ;
use Net::SCP qw(scp iscp) ;
use IO::Compress::Gzip qw(gzip $GzipError) ;
use Date::Calc qw(:all) ;




my @DIRECTORIES = ( "CDiameterPeer","FHoSS","JavaDiameterPeer","ser_ims" ) ;
#my @DIRECTORIES = ( "JavaDiameterPeer") ;
my $SVNSERVER = "shell.berlios.de" ;
my $LOGINNAME = "jsbach" ;
my $ROOT_LOCAL = '' ;
my $ROOT_REMOTE = "/home/groups/openimscore/htdocs/" ;
my $URI = "http://svn.berlios.de/svnroot/repos/openimscore/" ;
my $SNAPSHOT = "snapshots/" ;
my $DOCS = "docs/" ;
my $DOXCONST = "/trunk/doxygen/" ;
my $UPLOAD = 0 ;
my ($whereami,$doxdir) ;
my $revnumber_actual  ;
my $revnumber_found ;

my $scp_client= new Net::SCP ;
#my $remote_path = $ROOT_REMOTE.$SNAPSHOT."snappy" ;
#my $ssh_con = Net::SSH::Perl->new($SVNSERVER ,	protocol => '2,1' ,
#						cipher => '3des-cbc',
#						ciphers => '3des-cbc,blowfish-cbc,arcfour',
#						port => '22',
#						debug => 1 
#						interactive => '0' ,
#						privileged => '0' ,
#						identity_files => [ "~/.ssh/id_rsa" ]
#						compression => '0' ,
#						compression_level => '6' ,
#						use_pty => '0'  ,
#						options => [] 
#						) ;
#my $auth = Net::SSH::Perl::Auth->new('DSA',$ssh_con) ;
#$auth->authenticate ;
#$ssh_con->login($LOGINNAME,'') ;
#my($stdout,$stderr,$exit) = $ssh_con->cmd("ls ") ;

#print "valid auth" if $auth->authenticate ;
#print "this is stdout",$stdout,"\nthis is stderr", $stderr,"\n" ;


#my $svn_agent = SVN::Agent->load({ 
my ($year,$month,$day) = Parse_Date(`/bin/date`) ;

my $rellocalname = "/snapshot".$year.$month.$day."/" ;
$ROOT_LOCAL = &Cwd::cwd().$rellocalname  ;
print $ROOT_LOCAL,"\n" ;

    if(-e $ROOT_LOCAL) {
    rmdir($ROOT_LOCAL) ;

    } else {
	mkdir($ROOT_LOCAL,0775) or die "Cannot make dir $!" ;
	chdir($ROOT_LOCAL) ;
#	$scp_client->scp($ROOT_LOCAL,"$LOGINNAME\@$SVNSERVER:$ROOT_REMOTE$SNAPSHOT") ; 
#	$scp_client->scp($ROOT_LOCAL,"$LOGINNAME\@$SVNSERVER:$ROOT_REMOTE$DOCS") ; 
	my $rvalue  ;
	my $real_archive ;
	my @temp ;

	    foreach my $element (@DIRECTORIES) 
	    {

	    $rvalue = system("svn export $URI$element 2>$element\\exportErrors.txt 1>$element\\export.txt " ) ;
	    open(FH,"$element"."export.txt") or die "cannot open export $element\\export.txt, $!";
	    @temp = <FH> ;   
	    close(FH) ;
		while( my $line = pop(@temp)){if($line =~ m/.*revision\s(\d+)/){ $revnumber_actual = $1;}} 
		if($rvalue)
		{
		} else {

		my @files ;
	        my @final_array;
		    my $ret_value = system("ssh -l $LOGINNAME $SVNSERVER -C ls -lh $ROOT_REMOTE$SNAPSHOT$element* >lsremote_$element.txt") ;
		    
		    open(FH,"lsremote_$element.txt") or die "cannot open lsremote_$element.txt" ;
		    @temp = <FH> ;
		    close(FH) ;

			    my $line = pop(@temp) ;
			    if($line =~ m/.*r0*(\d+)\.tgz/){$revnumber_found = $1; print "FOUND REVISION NUMBER:$revnumber_found\n" ;} 

			if( $revnumber_actual > $revnumber_found || !defined($revnumber_found) )
			{
			    find(sub{push @files,$File::Find::name},"$ROOT_LOCAL$element") ;
			    @final_array = map { s/$ROOT_LOCAL(.*)/$1/g ; $_ } @files ;

#				foreach my $kinky(@final_array) { print "$kinky\n" ; } 
			    
				if($revnumber_actual < 10 ){ $revnumber_actual = "000".$revnumber_actual ; }
				if($revnumber_actual >= 10 && $revnumber_actual < 100) { $revnumber_actual = "00".$revnumber_actual ;}
				if($revnumber_actual >=100 && $revnumber_actual < 1000) {$revnumber_actual = "0".$revnumber_actual ;}

			    my $tarfile = "$element"."$year"."$month".$day.".r$revnumber_actual" ;

			    Archive::Tar->create_archive("$tarfile",0,@final_array) ;
			    gzip "$tarfile" => "$tarfile.tgz" or die "gunzip failed with $!" ;
			    unlink("$tarfile") ; 
				
			    $scp_client->scp("$tarfile.tgz", "$LOGINNAME\@$SVNSERVER:$ROOT_REMOTE$SNAPSHOT") ; 

			    $whereami = &Cwd::cwd() ;
			    $doxdir = $whereami."/".$element.$DOXCONST ;
			    chdir($doxdir) ;

			    system("doxygen doxygen.config 2>> $ROOT_LOCAL.errors") ;
			    $scp_client->scp($doxdir."html","$LOGINNAME\@$SVNSERVER:$ROOT_REMOTE$DOCS$element") ;
			    system("ssh -l jsbach $SVNSERVER \'cd $ROOT_REMOTE$DOCS && chmod -R 775 $element\'") ;
			}
		}
	    chdir($ROOT_LOCAL) ;
	    $doxdir = '' ;
	    }
    }
