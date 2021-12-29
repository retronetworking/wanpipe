#!/usr/bin/perl
$| = 1;

use Net::FTP;

$ip="64.82.90.99";

$file="A*.BIN";

$dir="linux/current_wanpipe/firmware";

$ftp = Net::FTP->new($ip, Passive => 1, Debug => 0);

print "\nLogging into ftp.sangoma.com ... ";

$ftp->login("anonymous",'me@here.there' ) || die "\nFailed to login to ftp.sangoma.com!\n\n";
#$ftp->login("michael",'sangoma') || die "\nFailed to login!\n\n";

print "Done\n\n";

$ftp->binary;  

#$ftp->cwd($dir) || die "\nFailed to chdir $dir\n";

@files=$ftp->dir($dir) || die "\nFailed to list directory $dir\n";

if (!defined(@files)){
	die "\nFailed to list directory $dir\n";
}	

print "\nGot  file list\n";   

foreach $filename (@files) {
	print "\nDownloading AFT Firmware $filename ...  ";
	$ftp->get($filename) || die "\nFailed to get file $filename from $ip\:/$dir\n\n";
  	print "Done\n";
}

$ftp->quit;               

print "\nWanpipe AFT Firmware Files Downloaded from ftp.sangoma.com\n\n";


exit (0);

