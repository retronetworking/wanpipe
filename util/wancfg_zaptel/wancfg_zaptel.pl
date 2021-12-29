#!/usr/bin/perl
# config-zaptel.pl 
# Sangoma Zaptel Configuration Script for Zaptel.
#
# Copyright     (c) 2006, Sangoma Technologies Inc.
#
#               This program is free software; you can redistribute it and/or
#               modify it under the terms of the GNU General Public License
#               as published by the Free Software Foundation; either version
#               2 of the License, or (at your option) any later version.
# ----------------------------------------------------------------------------
# Jan 15,  2007  David Yat Sin  support for non-trixbox installations. Major 
#				changes to script.
# Jan 8,   2007  David Yat Sin  script renamed to config-zaptel.pl
# Jan 5,   2007  David Yat Sin  v2.1 fix for analog cards inserted in wrong
#				context
# Dec 12,  2006  David Yat Sin  v2.0 support for T1/E1 cards
# Oct 13,  2006  David Yat Sin  Added --opermode and --law option
# Oct 12,  2006  David Yat Sin  Initial version
# ============================================================================

system('clear');
print "\n###################################################################";
print "\n#         Sangoma Wanpipe/Zaptel Configuration Script             #";
print "\n#                           v2.2                                  #";
print "\n#                  Sangoma Technologies Inc.                      #";
print "\n#                     Copyright(c) 2006.                          #";
print "\n###################################################################\n\n";

print "\nNote: This script provides Zaptel configuration for Sangoma";
print "\n      cards. Use the Wancfg Utility for more advanced options. \n\n";	

use Switch;
use strict;
#use warnings;
#use diagnostics;
use Card;
use A10x;
use A10u;
use A20x;

my $FALSE = 1;
my $TRUE = 0;


my $startup_string="";
my $zaptel_conf="";
my $zapata_conf="";
my $devnum=1;
my $current_zap_channel=1;
my $num_analog_devices=0;
my $num_analog_devices_total=0;
my $num_digital_devices=0;
my $num_digital_devices_total=0;

my $def_femedia='';
my $def_feframe='';
my $def_feclock='';
my $def_signalling='';
my $def_switchtype='';
my $def_zapata_context='';
my $def_zapata_group='';

my $is_trixbox = $FALSE;
my $config_zapata = $TRUE;

my $current_dir=`pwd`;
chomp($current_dir);
my $cfg_dir='tmp_cfg';
my $debug_info_file="$current_dir/$cfg_dir/debug_info";
my @hwprobe=`wanrouter hwprobe verbose`;
read_args();
my $zaptel_conf_template="$current_dir/templates/zaptel.conf";
my $zaptel_conf_file="$current_dir/$cfg_dir/zaptel.conf";
my $zaptel_conf_file_t="/etc/zaptel.conf";

my $zapata_conf_template="$current_dir/templates/zapata.conf";
my $zapata_conf_file="$current_dir/$cfg_dir/zapata.conf";
my $zapata_conf_file_t="/etc/asterisk/zapata.conf";
my $date=`date +%F`;
chomp($date);
my $debug_tarball="/etc/wanpipe/debug-".$date.".tgz";

if ($is_trixbox==$TRUE){
	$zapata_conf_template="$current_dir/templates/zapata-auto.conf";
	$zapata_conf_file="$current_dir/$cfg_dir/zapata-auto.conf";
	$zapata_conf_file_t="/etc/asterisk/zapata-auto.conf";
}
if ($is_trixbox==$FALSE){
	print "Would you like to generate /etc/asterisk/zapata.conf\n";
	if (&prompt_user_list(("YES","NO","")) eq 'NO'){
       		$config_zapata = $FALSE; 
	}
}

prepare_files();
config_digital();
config_analog();

if($devnum==1){
	system('clear');
	print "\n###################################################################";
	print "\n#                             SUMMARY                             #";
	print "\n###################################################################\n\n";
                                                  
	print("\n\nNo Sangoma voice compatible cards found/configured\n\n"); 
	exit 0;
}else{
	write_zaptel_conf();
	if ($config_zapata==$TRUE){
		write_zapata_conf();
	}
	save_debug_info();
	system('clear');
	print "\n###################################################################";
	print "\n#                             SUMMARY                             #";
	print "\n###################################################################\n\n";

	print("  $num_digital_devices_total T1/E1 port(s) detected, $num_digital_devices configured\n");
	print("  $num_analog_devices_total analog card(s) detected, $num_analog_devices configured\n");
	
	print "\nConfigurator has created the following files:\n";
	print "\t1. Wanpipe config files in /etc/wanpipe\n";
	print "\t2. Zaptel config file /etc/zaptel.conf\n";
        if ($config_zapata==$TRUE){
		print "\t3. Zapata config file $zapata_conf_file_t\n";
	}
	
	print "\n\nYour original configuration files will be saved to:\n";
	print "\t1. $zaptel_conf_file_t.bak \n";
	if ($config_zapata==$TRUE){
       		print "\t2. $zapata_conf_file_t.bak \n\n";
	}
	
	print "\nYour configuration has been saved in $debug_tarball.\n";
	print "When requesting support, email this file to techdesk\@sangoma.com\n\n";
	prompt_user("Press any key to continue");
	apply_changes();
}



#------------------------------FUNCTIONS------------------------------------#
sub exec_command{
	my @command = @_;
	if (system(@command) != 0 ){
		print "Error executing command:\n@command\n\n";
	}
	return $?;
}

sub prompt_user{
	my($promptString, $defaultValue) = @_;
	if ($defaultValue) {
	      print $promptString, "def=\"$defaultValue\": ";
	} else {
	      print $promptString, ": ";
	}

	$| = 1;               # force a flush after our print
	$_ = <STDIN>;         # get the input from STDIN (presumably the keyboard)
	chomp;
	if ("$defaultValue") {
	      return $_ ? $_ : $defaultValue;    # return $_ if it has a value
	} else {
	      return $_;
	}
}

sub prompt_user_list{
	my @list = @_;
	my $defaultValue = @list[$#list];
	
	my $i;
	my $valid = 0;
	for $i (0..$#list-1) {
         	printf(" %s\. %s\n",$i+1, @list[$i]);
	}
	while ($valid == 0){
		my $list_sz = $#list;
		print "[1-$list_sz";	
		if ( ! $defaultValue eq ''){
			print ", ENTER=\'$defaultValue\']:";	
		}else{
			print "]:";
		}
		$| = 1;               
       	 	$_ = <STDIN>;         
		chomp;
	       	if ( $_ eq '' & ! $defaultValue eq ''){
			print "\n";
	       		return $defaultValue;
		}
		      	
		if ( $_ =~ /(\d+)/ ){
			if ( $1 > $list_sz) {
	       	       	  	print "\nError: Invalid option, input a value between 1 and $list_sz \n";
			} else {
				print "\n";
		       		return @list[$1-1] ;
			}
		} else {
			print "\nError: Invalid option, input an integer\n";
		}
	}
}
sub read_args {
	my $arg_num;
	foreach $arg_num (0..$#ARGV) {
		my $arg = $ARGV[$arg_num];
		switch ($arg) {
			case( /^--trixbox/){
				$is_trixbox=$TRUE;
			}
		}
	}
	if ($is_trixbox==$TRUE){
		print "\nGenerating configuration files for Trixbox\n";
	}
}

sub get_pri_switchtype {
    	my @options = ( "National ISDN 2", "Nortel DMS100", "AT&T 4ESS", "Lucent 5ESS", "EuroISDN", "Old National ISDN 1", "Q.SIG" );
	my @options_val = ( "national", "dms100", "4ess", "5ess", "euroisdn", "ni1", "qsig" );
	my $res = &prompt_user_list(@options,$def_switchtype);
	my $i;
	for $i (0..$#options){
		if ( $res eq @options[$i] ){
			$def_switchtype=@options[$i];
			return @options_val[$i]; 
		}
	}	
	print "Internal error: Invalid PRI switchtype\n";
	exit 1;
}

sub unload_zap_modules{
	my @modules_list = ("ztdummy","wctdm","wcfxo","wcte11xp","wct1xxp","wct4xxp","tor2","zaptel");
	foreach my $module (@modules_list) {
		exec_command("modprobe -r $module");
	}
}

sub write_zaptel_conf{
	my $zp_file="";
	open(FH, "$zaptel_conf_template") or die "cannot open $zaptel_conf_template";
        while (<FH>) {
       		$zp_file .= $_;
	}
	close (FH);
	$zp_file=$zp_file.$zaptel_conf;	
	open(FH, ">$zaptel_conf_file") or die "cannot open $zaptel_conf_file";
        	print FH $zp_file;
	close(FH);	
}

sub write_zapata_conf{
	my $zp_file="";
	open(FH, "$zapata_conf_template") or die "cannot open $zapata_conf_template";
        while (<FH>) {
       		$zp_file .= $_;
	}
	close (FH);
	
	$zp_file.=$zapata_conf;	
	
	open(FH, ">$zapata_conf_file") or die "cannot open $zapata_conf_file";
        	print FH $zp_file;
	close(FH);	
}

sub apply_changes{
#	if ($is_trixbox==$FALSE){
#		print "\nThe following lines will be used in your $zaptel_conf_file_t\n";
#		print "\n-----------------------------------------------------------------\n";
#		print "\n$zaptel_conf";
#		print "\n-----------------------------------------------------------------\n";
#		prompt_user("Press any key to continue");
	
#		if ($config_zapata==$TRUE){
#	       		print "\nThe following lines will be used in your $zapata_conf_file_t\n";
#			print "\n-----------------------------------------------------------------\n";
#			print "\n$zapata_conf";
#			print "\n-----------------------------------------------------------------\n";
#			prompt_user("Press any key to continue");
#		}
#	}
	my $asterisk_command='';
	my $res='';
	system('clear');
	print "\nZaptel and Wanpipe configuration complete: choose action\n";
	$res=&prompt_user_list("Save cfg: Restart Asterisk & Wanpipe now","Save cfg: Restart Asterisk & Wanpipe when convenient","Do not save cfg: Exit","");
	if( $res =~ m/exit/){
		print "No changes made to your configuration files\n";
		exit 0;
	}elsif($res =~ m/Restart/){
		$asterisk_command='stop now';	
	}else{
		$asterisk_command='stop when convenient';
	}

#	print "Your original configuration files have been saved to:\n";
#	print "\t1. $zaptel_conf_file_t.bak \n";
#	if ($config_zapata==$TRUE){
#       		print "\t2. $zapata_conf_file_t.bak \n\n";
#	}

	if ($is_trixbox==$TRUE){
		exec_command("amportal stop");
		unload_zap_modules();
	}else{
		print "\nStopping Asterisk...\n";	
		exec_command("asterisk -rx \"$asterisk_command\"");
	} 
	print "\n\nStopping Wanpipe...\n";
	exec_command("wanrouter stop all");
	
	print "\nUnloading Zaptel modules...\n";
	unload_zap_modules();

	print "\nRemoving old configuration files...\n";
	exec_command("rm -f /etc/wanpipe/wanpipe*.conf");
	
       	gen_wanrouter_rc();

	print "\nCopying new Wanpipe configuration files...\n";
	exec_command("cp -f $current_dir/$cfg_dir/* /etc/wanpipe");
	
	print "\nCopying new Zaptel configuration file ($zaptel_conf_file_t)...\n";
	exec_command("cp -f $zaptel_conf_file $zaptel_conf_file_t");
	
	if ($config_zapata==$TRUE | $is_trixbox==$TRUE){
		print "\nCopying new chan_zap configuration files ($zapata_conf_file_t)...\n";
		exec_command("cp -f $zapata_conf_file $zapata_conf_file_t");
	}
        print "\nStarting Wanpipe...\n";
	exec_command("wanrouter start");

	if ( ! -e "/etc/wanpipe/scripts/start" & $is_trixbox==$FALSE){ 
       		print "Loading Zaptel...\n";	
		sleep 2;
		exec_command("ztcfg -v");   
		print ("\nWould you like to execute \'ztcfg\' each time wanrouter starts?\n");
		if (&prompt_user_list("YES","NO","") eq 'YES'){
			exec_command("cp -f /etc/wanpipe/samples/wanpipe_zaptel_start /etc/wanpipe/scripts/start");	
			
		}	 
	}
	if ($is_trixbox==$TRUE){
     		print "\nStarting Amportal...\n";
		exec_command("amportal start");
		sleep 2;
	}elsif($config_zapata==$TRUE){
     		print "\nStarting Asterisk...\n";
		exec_command("asterisk");
		sleep 2;
		
		print "\nListing Asterisk channels...\n\n";
		exec_command("asterisk -rx \"zap show channels\"");
		print "\nType \"asterisk -r\" to connect to Asterisk console\n\n";
	}else{
        	print "\nProceed to your Asterisk chan_zap configuration (/etc/asterisk/zapata.conf)\n\n";
	}
	exit 0;
}

sub gen_wanrouter_rc{
	#update wanpipe startup sequence
	my $rcfile="";
	open (FH,"$current_dir/templates/rc.template");
	while (<FH>) {
        	$rcfile .= $_;
	}
	close (FH);
	open (FH,">$current_dir/$cfg_dir/wanrouter.rc");
	$rcfile =~ s/WPSTARTUP/$startup_string/g;
	print FH $rcfile;
	close (FH);
}

sub prepare_files{
	#remove temp files
	exec_command("rm -f $current_dir/$cfg_dir/*");

	#backup existing configuration files
	exec_command("cp -f $zaptel_conf_file_t $zaptel_conf_file_t.bak ");
       	exec_command("cp -f $zapata_conf_file_t $zapata_conf_file_t.bak");
	
}

sub save_debug_info{
	my $version=`wanrouter version`;
	chomp($version);

	my $uname=`uname -a`;
	chomp($uname);

	my $issue=`cat /etc/issue`;
	chomp($issue);

	my $debug_info="\n"; 
	$debug_info.="===============================================================\n";
	$debug_info.="                   SANGOMA DEBUG INFO FILE                     \n";
        $debug_info.="                   Generated on $date                          \n";
	$debug_info.="===============================================================\n";
	
	$debug_info.="\n\nwanrouter hwprobe\n";
	$debug_info.="@hwprobe\n";	
	$debug_info.="\nwanrouter version\n";
	$debug_info.="$version\n";	
	$debug_info.="\nKernel \"uname -a\"\n";
	$debug_info.="$uname\n";
       	$debug_info.="\nLinux distribution \"cat /etc/issue\"\n"; 
	$debug_info.="$issue\n";
	$debug_info.="EOF\n";
	
        open (FH,">$debug_info_file") or die "Could not open $debug_info_file";
       	print FH $debug_info;
       	close (FH);	
			
	
#	print "\n\nGenerating debug tarball $debug_tarball \n";
	exec_command("tar cvfz $debug_tarball $cfg_dir/* >/dev/null 2>/dev/null");

#	print "\nEmail $debug_tarball to techdesk\@sangoma.com";
#	print "\nwhen requesting support.\n";	
#	prompt_user("Press any key to continue");
	
}

sub config_analog{
	my $a20x;
	system('clear');
	print "------------------------------------\n";
	print "Configuring analog cards [A200/A400]\n";
	print "------------------------------------\n";
#	prompt_user("Press any key to continue");
	my $skip_card=$FALSE;
	$zaptel_conf.="\n";
	$zapata_conf.="\n";
	foreach my $dev (@hwprobe) {
		if ( $dev =~ /(\d+).(\w+\w+).*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+).*PORT=(\w+).*HWEC=(\d+)/){
			$skip_card=$FALSE;
			my $card = eval {new Card(); } or die ($@);
			$card->current_dir($current_dir);
			$card->cfg_dir($cfg_dir);
			$card->span_no($devnum);
			$card->card_model($1);
			$card->pci_slot($3);
			$card->pci_bus($4);
			$card->fe_cpu($5);
			my $hwec=$7;
			if ($hwec==0){
				$card->hwec_mode('NO');
			}
			else{
				$card->hwec_mode('YES');
			}
       			if ($card->card_model eq '200' | $card->card_model eq '400'){
				$num_analog_devices_total++;
				system('clear');
				print "\n-----------------------------------------------------------\n";
				print "A$1 detected on slot:$3 bus:$4\n";
				print "-----------------------------------------------------------\n";
				if($is_trixbox==$FALSE){
					print "\nWould you like to configure A$1 on slot:$3 bus:$4\n";
					if (&prompt_user_list(("YES","NO","")) eq 'NO'){
						$skip_card=$TRUE;	
			       		} 
				}
				if ($skip_card==$FALSE){
					print "A$1 configured on slot:$3 bus:$4 span:$devnum\n";
					prompt_user("Press any key to continue");
			 		$zaptel_conf.="#Sangoma A$1 [slot:$3 bus:$4 span:$devnum]\n";
					$zapata_conf.=";Sangoma A$1 [slot:$3 bus:$4 span:$devnum]\n";
					$startup_string.="wanpipe$devnum ";
			
               				$a20x = eval {new A20x(); } or die ($@);
					$a20x->card($card);
					$card->first_chan($current_zap_channel);
       					$current_zap_channel+=24;
					my $i;
					$a20x->gen_wanpipe_conf();
					$devnum++;
					$num_analog_devices++;
				}else{
					print "A$1 on slot:$3 bus:$4 not configured\n";
					prompt_user("Press any key to continue");
				}
			}
		}elsif ( $dev =~ /(\d+):FXS/ & $skip_card==$FALSE){
	       		my $zap_pos = $1+$current_zap_channel-25;
			$a20x->card->zap_context("from-internal");
			$a20x->card->zap_group("1");
			$zaptel_conf.=$a20x->gen_zaptel_conf($zap_pos,"fxo");
			$zapata_conf.=$a20x->gen_zapata_conf($zap_pos,"fxo");
		}elsif ( $dev =~ /(\d+):FXO/ & $skip_card==$FALSE){
	       		my $zap_pos = $1+$current_zap_channel-25;
			$a20x->card->zap_context("from-zaptel");
			$a20x->card->zap_group("0");
			$zaptel_conf.=$a20x->gen_zaptel_conf($zap_pos,"fxs");
			$zapata_conf.=$a20x->gen_zapata_conf($zap_pos,"fxs");
		}
	}

	
	if($num_analog_devices_total!=0){
		print("\nAnalog card configuration complete\n\n");
		prompt_user("Press any key to continue");
	}
}

sub config_digital{
	system('clear');
	print "---------------------------------------------\n";
	print "Configuring T1/E1 cards [A101/A102/A104/A108]\n";
	print "---------------------------------------------\n";
#	prompt_user("Press any key to continue");
	
	foreach my $dev (@hwprobe) {
		if ( $dev =~ /A(\d+)(.*):.*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+).*PORT=(\w+).*/){
                     
		  	if ( ! ($1 eq '200' | $1 eq '400') ){
				#do not count analog devices
				$num_digital_devices_total++;
			}
			my $card = eval {new Card(); } or die ($@);
			$card->current_dir($current_dir);
			$card->cfg_dir($cfg_dir);
			$card->span_no($devnum);
			$card->card_model($1);
			$card->pci_slot($3);
			$card->pci_bus($4);
			$card->fe_cpu($5);
			my $hwec=0;
			if ( $dev =~ /HWEC=(\d+)/){
				if($1 gt 0){
					$hwec=1;
				}
			}
			if ( $dev =~ /A(\d+)(.*):.*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+).*PORT=(\w+).*/){
	
			my $abc=0;
	
}
			if ($hwec==0){
				$card->hwec_mode('NO');
			}else{
				$card->hwec_mode('YES');
			}
			my $port=$6;
			if ($6 eq 'PRI') {
				$port=$5;
			} 

			if ( $card->card_model eq '101' |  
                             $card->card_model eq '102' |  
                             $card->card_model eq '104' |  
                             $card->card_model eq '108' ){
				system('clear');
				if (($6 eq '1' || $6 eq 'PRI') && $5 eq 'A'){
                               		print "A$1 detected on slot:$3 bus:$4\n";
			#		prompt_user("Press any key to continue");
			#		$def_femedia='';
		#			$def_feframe='';
		#			$def_feclock='';
	#				$def_signalling='';
#					$def_switchtype='';
#					$def_zapata_context='';
#					$def_zapata_group='';
				}
				system('clear');
				my $msg ="Configuring port ".$port." on A".$card->card_model." slot:".$card->pci_slot." bus:".$card->pci_bus.".\n";
				print "\n-----------------------------------------------------------\n";
				print "$msg";
				print "-----------------------------------------------------------\n";
		#		prompt_user("Press any key to continue");
					
				printf ("\nSelect media type for A%s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $port, $card->pci_slot, $card->pci_bus);
				my @options = ("T1", "E1", "Unused","Exit");
				my $fe_media = prompt_user_list( @options, $def_femedia);
				if ( $fe_media eq 'Exit'){
					print "Exit without applying changes?\n";
					if (&prompt_user_list(("YES","NO","YES")) eq 'YES'){
						print "No changes made to your configuration files.\n\n";
						exit 0;
					}
				}elsif ( $fe_media eq 'Unused'){
					$def_femedia=$fe_media;	
			#		system('clear');
					my $msg= "A$1 on slot:$3 bus:$4 port:$port not configured\n";
					print "$msg";
					prompt_user("Press any key to continue");
				}else{
			       		$def_femedia=$fe_media; 
					$startup_string.="wanpipe$devnum ";
					my $a10x;
		       	
					if ($1 !~ m/104/ && $2 !~ m/SH/) {	
						$a10x = eval {new A10u(); } or die ($@);
						$a10x->card($card);
						if ($5 eq "A") { 
							$a10x->fe_line("1");
						} else {
							$a10x->fe_line("2");
						}
					} else {
						$a10x = eval {new A10x(); } or die ($@);
				                if ($dev =~ /A(\d+)(.*):.*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+).*PORT=(\w+).*/){
							my $abc=0;
						}
						$a10x->card($card);
						$a10x->fe_line($6);
					}

			       		$card->first_chan($current_zap_channel);
					$a10x->fe_media($fe_media);
					if ( $fe_media eq 'T1' ){
		      				$current_zap_channel+=24;
					}else{
						$a10x->fe_lcode('HDB3');
						printf ("Select framing type for %s on port %s\n", $card->card_model, $port);
						@options = ("CRC4","NCRC4");
						$def_feframe=&prompt_user_list(@options,$def_feframe);
						$a10x->fe_frame($def_feframe);
			      			$current_zap_channel+=31;
					}
					my @options = ("NORMAL", "MASTER");
					printf ("Select clock for A%s on port %s \n", $card->card_model, $port);
					$def_feclock=&prompt_user_list(@options, $def_feclock);
					$a10x->fe_clock($def_feclock);
			
		     	   		@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start");
					printf ("Select signalling type for %s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $port, $card->pci_slot, $card->pci_bus);
				       	$def_signalling=&prompt_user_list(@options,$def_signalling); 
					$a10x->signalling($def_signalling);

					if ( $a10x->signalling eq 'PRI CPE' | $a10x->signalling eq 'PRI NET' ){
						if ( $config_zapata==$TRUE){
							printf ("Select switchtype for %s on port %s \n", $card->card_model, $port);
							$a10x->pri_switchtype(get_pri_switchtype());
						}
					}
					$a10x->gen_wanpipe_conf();
					$zaptel_conf.=$a10x->gen_zaptel_conf();
					if ($is_trixbox==$TRUE | $config_zapata==$TRUE){
						$a10x->card->zap_context(&get_zapata_context($a10x->card->card_model,$port));
						$a10x->card->zap_group(&get_zapata_group($a10x->card->card_model,$port,$a10x->card->zap_context));
						$zapata_conf.=$a10x->gen_zapata_conf();
					}
					$devnum++;
					$num_digital_devices++;
			#		system('clear');
					my $msg ="Port ".$port." on A".$card->card_model." configuration complete...\n";
					print "$msg";
					prompt_user("Press any key to continue");
		      		}  	
			} 	
		#for A101/2u, A101/2c
       		}elsif ( $dev =~ /A(\d+).*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+)/){
	
			die "SHOULD NOT BE HERE";
			
			system('clear');	
			$num_digital_devices_total++;
       			my $card = eval {new Card(); } or die ($@);
			$card->current_dir($current_dir);
			$card->cfg_dir($cfg_dir);
       		   	$card->span_no($devnum);
			$card->card_model($1);
			$card->pci_slot($2);
			$card->pci_bus($3);
			$card->fe_cpu($4);
			my $port;
		       	if( $4 eq 'A' ){
                      		print "\n\nSangoma single/dual T1/E1 card detected on slot:$2 bus:$3\n";
		      		print "Select configuration options:\n";
		#		prompt_user("Press any key to continue");
#				$def_femedia='';
#				$def_feframe='';
#				$def_feclock='';
#				$def_signalling='';
#				$def_switchtype='';
#				$def_zapata_context='';
#				$def_zapata_group='';
		       		
				$port=1;
			}else{
		       		$port=2;
			}	
			printf ("\nSelect media type for A%s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $port, $card->pci_slot, $card->pci_bus);
			my @options = ("T1", "E1", "Unused", "Exit");
			my $fe_media = &prompt_user_list(@options,"");
			if ( $fe_media eq 'Exit'){
				print "Exit without applying changes?\n";
				if (&prompt_user_list(("YES","NO","YES")) eq 'YES'){
					print "No changes made to your configuration files\n";
					exit 0;
				}
			}elsif ( $fe_media eq 'Unused'){
				print "A$1 on slot:$2 bus:$3 port:$4 not configured\n";
		       	}else{
				$num_digital_devices++;
		       		$startup_string.="wanpipe$devnum ";
		       		my $a10u = eval {new A10u(); } or die ($@);
		       		$a10u->card($card);
		       		$card->first_chan($current_zap_channel);
				$a10u->fe_line($port);
				$a10u->fe_media($fe_media);
				if ( $fe_media eq 'T1' ){
		      			$current_zap_channel+=24;
				}else{
                       			$a10u->fe_lcode('HDB3');
					printf ("Select framing type for %s on port %s\n", $card->card_model, $6);
					my @options = ("CRC4","NCRC4");
					$a10u->fe_frame(&prompt_user_list(@options,""));
					$current_zap_channel+=31;
				}
                      		my @options = ("NORMAL", "MASTER");
				printf ("Select clock type for %s on port %s\n", $card->card_model, $a10u->fe_line);
				$a10u->fe_clock(&prompt_user_list(@options,""));
		
				@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start");
				printf ("Select signalling type for %s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $a10u->fe_line, $card->pci_slot, $card->pci_bus);
				
				
				if ( $a10u->signalling eq 'PRI CPE' | $a10u->signalling eq 'PRI NET' ){
					if ( $config_zapata==$TRUE){
						printf ("Select switchtype for %s on port %s \n", $card->card_model, $port);
						$a10u->pri_switchtype(&get_pri_switchtype());
					}
				}
				$a10u->signalling(&prompt_user_list(@options,""));
				$a10u->gen_wanpipe_conf();
				$zaptel_conf.=$a10u->gen_zaptel_conf();
				if ($is_trixbox==$TRUE | $config_zapata==$TRUE){
					$a10u->card->zap_context(&get_zapata_context($a10u->card->card_model,$port));
					$a10u->card->zap_group(&get_zapata_group($a10u->card->card_model,$port,$a10u->card->zap_context));
					$zapata_conf.=$a10u->gen_zapata_conf();
				}
				$devnum++;
				$num_digital_devices++;
				print "Port $port configuration complete\n";
				prompt_user("Press any key to continue");
			}
		}
	}
	if($num_digital_devices_total!=0){
		print("\nT1/E1 card configuration complete.\n");
		prompt_user("Press any key to continue");
	}
	close SCR;
}

sub get_zapata_context{
	my ($card_model,$card_port)=@_;
	my $context='';
	my @options = ("from-pstn", "from-internal","Custom");
	
	if($is_trixbox==$TRUE){
		@options = ("PSTN", "INTERNAL");
	}
	printf ("Select dialplan context for A%s on port %s\n", $card_model, $card_port);
	my $res = &prompt_user_list(@options,$def_zapata_context);
	if($res eq "PSTN"){
		$context="from-zaptel";
	}elsif($res eq "INTERNAL"| $res eq "from-internal"){
		$context="from-internal";
	}elsif($res eq "from-pstn"){
		$context="from-pstn";
	}elsif($res eq "Custom"){
		my $res_context=&prompt_user("Input the context for this port");
		while(length($res_context)==0){
			print "Invalid context, input a non-empty string\n";
			$res_context=&prompt_user("Input the context for this port",$def_zapata_context);
		}
		$context=$res_context;
	}elsif($res eq $def_zapata_context){
		$context=$def_zapata_context;
	}else{
		print "Internal error:invalid context,group\n";
		exit 1;
	}
	$def_zapata_context=$context;
#	if($is_trixbox==$FALSE){
#		print "\n Use \"[$context]\" in your /etc/asterisk/extensions.conf to";
#		print "\n configure your dialplan for incoming calls on this port.\n";
#	}
	return $context;	
}
sub get_zapata_group{
	my ($card_model,$card_port,$context)=@_;
	my $group='';
	if($is_trixbox==$TRUE){
		if($context eq "from-zaptel"){
			$group=0;	
			return $group;
		}elsif($context eq "from-internal"){
                	$group=1;
			return $group;
		}else{
                	print "Internal error:invalid group for Trixbox\n";
		}	
	}else{
		if($context eq "from-pstn"){
                	$group=0;
		}elsif($context eq "from-internal"){
                	$group=1;
		}else{
			my $res_group=&prompt_user("\nInput the group for this port\n",$def_zapata_group);
        		while(length($res_group)==0 |!($res_group =~/(\d+)/)){
				print "Invalid group, input an integer.\n";
				$res_group=&prompt_user("Input the group for this port",$def_zapata_group);
			}
			$group=$res_group;
			$def_zapata_group=$group;
		}      
	}
#	if($is_trixbox==$FALSE){
#		print "\n Use \"exten=>s,1,Dial(Zap/g$group/\${EXTEN})\" in your";
#		print "\n /etc/asterisk/extensions.conf to dial out on this port.\n";
#	}
       
       	return $group;
}
