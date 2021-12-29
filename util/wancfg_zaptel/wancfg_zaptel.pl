#!/usr/bin/perl
# config-zaptel.pl 
# Sangoma Zaptel/TDM API/SMG Configuration Script.
#
# Copyright     (c) 2006, Sangoma Technologies Inc.
#
#               This program is free software; you can redistribute it and/or
#               modify it under the terms of the GNU General Public License
#               as published by the Free Software Foundation; either version
#               2 of the License, or (at your option) any later version.
# ----------------------------------------------------------------------------
# Jun 13,  2007  Yuan Shen      SS7 support
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
print "\n#    Sangoma Wanpipe:  Zaptel/SMG/TDMAPI Configuration Script     #";
print "\n#                           v2.4                                  #";
print "\n#                  Sangoma Technologies Inc.                      #";
print "\n#                     Copyright(c) 2007.                          #";
print "\n###################################################################\n\n";

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
my $num_non_ss7_config=0;
my $num_ss7_config=0;
my $asked_autozapata=0;
my $line_media='';
my $max_chans=0;
my $ss7_tdmvoicechans='';
my $ss7_array_length=0;

my $device_has_hwec=$FALSE;
my $device_has_normal_clock=$FALSE;
my @device_normal_clocks=("0");

my $def_femedia='';
my $def_feframe='';
my $def_feclock='';
my $def_signalling='';
my $def_switchtype='';
my $def_zapata_context='';
my $def_zapata_group='';
my $def_te_ref_clock='';

my $is_trixbox = $FALSE;
my $config_zapata = $TRUE;
my $is_smg = $FALSE;

my $current_dir=`pwd`;
chomp($current_dir);
my $cfg_dir='tmp_cfg';
my $debug_info_file="$current_dir/$cfg_dir/debug_info";
my @hwprobe=`wanrouter hwprobe verbose`;
my $zaptelprobe = `modprobe -l |grep zaptel`;

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

if ($is_trixbox==$FALSE && $is_smg==$FALSE){
        print "Would you like to generate /etc/asterisk/zapata.conf\n";
        if (&prompt_user_list(("YES","NO","")) eq 'NO'){
                $config_zapata = $FALSE;
        }
}else{
	prompt_user("\nPress <Enter> to continue");
}

if ($is_smg==$TRUE){
     $config_zapata = $FALSE;    
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
	if (!($is_smg==$TRUE  &&  ($zaptelprobe !~ /zaptel.ko/))){
		if ($num_non_ss7_config != 0){
			write_zaptel_conf();
		}
		if ($config_zapata==$TRUE){
			write_zapata_conf();
		}
		save_debug_info();
		system('clear');
		my $file_list = 1;
		print "\n###################################################################";
		print "\n#                             SUMMARY                             #";
		print "\n###################################################################\n\n";

		print("  $num_digital_devices_total T1/E1 port(s) detected, $num_digital_devices configured\n");
		print("  $num_analog_devices_total analog card(s) detected, $num_analog_devices configured\n");
	
		print "\nConfigurator has created the following files:\n";
		print "\t1. Wanpipe config files in /etc/wanpipe\n";
		$file_list++;
				
		if ($num_non_ss7_config != 0){
			print "\t$file_list. Zaptel config file /etc/zaptel.conf\n";
			$file_list++;
		}
        	if ($config_zapata==$TRUE){
			print "\t$file_list. Zapata config file $zapata_conf_file_t\n";
		}
	
        	if (($num_non_ss7_config != 0) | ($config_zapata==$TRUE)){	
			print "\n\nYour original configuration files will be saved to:\n";
			$file_list=1;
		}	
		
		if ($num_non_ss7_config != 0){
			print "\t$file_list. $zaptel_conf_file_t.bak \n";
			$file_list++;
		}
		if ($config_zapata==$TRUE){
       			print "\t$file_list. $zapata_conf_file_t.bak \n\n";
		}
	
		print "\nYour configuration has been saved in $debug_tarball.\n";
		print "When requesting support, email this file to techdesk\@sangoma.com\n\n";
		prompt_user("Press any key to continue");
		apply_changes();
	} else {
		save_debug_info();
		system('clear');
		print "\n###################################################################";
		print "\n#                             SUMMARY                             #";
		print "\n###################################################################\n\n";
		
		print("  $num_digital_devices_total T1/E1 port(s) detected, $num_digital_devices configured\n");		
		print "\nConfigurator has created the following files:\n";
		print "\t1. Wanpipe config files in /etc/wanpipe\n";
		
		print "\nYour configuration has been saved in $debug_tarball.\n";
		print "When requesting support, email this file to techdesk\@sangoma.com\n\n";
		prompt_user("Press any key to continue");
		apply_changes();
	}
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
	foreach $arg_num (0 .. $#ARGV) {
		#my $arg = $ARGV[$arg_num];
		$_ = $ARGV[$arg_num];
		if ( /^--trixbox$/){
			$is_trixbox=$TRUE;
		}
		elsif ( /^--smg$/){
			$is_smg=$TRUE;
		}

	}
	
	if ($is_trixbox==$TRUE){
		print "\nGenerating configuration files for Trixbox\n";
	}
	if ($is_smg==$TRUE){
		print "\nGenerating configuration files for SS7\n";
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
	my @modules_list = ("ztdummy","wctdm","wcfxo","wcte11xp","wct1xxp","wct4xxp","tor2","zttranscode","zaptel");
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

sub copy_config_files{
	my @wanpipe_files = split / /, $startup_string; 	
	exec_command("cp -f $current_dir/$cfg_dir/wanrouter.rc /etc/wanpipe");
	foreach my $wanpipe_file (@wanpipe_files) {
		exec_command("cp -f $current_dir/$cfg_dir/$wanpipe_file.conf /etc/wanpipe");
	}
}

sub apply_changes{
	my $asterisk_command='';
	my $asterisk_restart=$FALSE;
	my $res='';
 
	system('clear');

	if ($is_smg != $TRUE){
		print "\nZaptel and Wanpipe configuration complete: choose action\n";
		$res=&prompt_user_list("Save cfg: Restart Asterisk & Wanpipe now",
					"Save cfg: Restart Asterisk & Wanpipe when convenient",
					"Save cfg: Stop Asterisk & Wanpipe now", 
					"Save cfg: Stop Asterisk & Wanpipe when convenient",
					"Do not save cfg: Exit",
					"");
	}else{
		print "\nWanpipe configuration complete: choose action\n";
		$res=&prompt_user_list("Save cfg: Stop Asterisk & Wanpipe now", 
				       "Save cfg: Stop Asterisk & Wanpipe when convenient",
				       "Do not save cfg: Exit",
				       "");
	}
	
	if ($res =~ m/Exit/){
		print "No changes made to your configuration files\n";
		exit 0;
	}
	if ($res =~ m/now/){
		$asterisk_command='stop now';	
	} else {
		$asterisk_command='stop when convenient';
	}

	if ($res =~ m/Restart/){
		$asterisk_restart=$TRUE;
	} else {
		$asterisk_restart=$FALSE;
	}
	 
	if ($is_trixbox==$TRUE){
		exec_command("amportal stop");
		unload_zap_modules();
	} else {
		print "\nStopping Asterisk...\n";	
		exec_command("asterisk -rx \"$asterisk_command\"");
	} 

	print "\nStopping Wanpipe...\n";
	exec_command("wanrouter stop all");

	if (!($is_smg==$TRUE  &&  ($zaptelprobe !~ /zaptel.ko/))){
            	print "\nUnloading Zaptel modules...\n";
		unload_zap_modules();
	}

	print "\nRemoving old configuration files...\n";
 
        exec_command("rm -f /etc/wanpipe/wanpipe*.conf");
	
       	gen_wanrouter_rc();

	print "\nCopying new Wanpipe configuration files...\n";
	copy_config_files();
#	exec_command("cp -f $current_dir/$cfg_dir/* /etc/wanpipe");

	if (!($is_smg==$TRUE  &&  ($zaptelprobe !~ /zaptel.ko/))){
		if ($num_non_ss7_config!=0){
			print "\nCopying new Zaptel configuration file ($zaptel_conf_file_t)...\n";
   			exec_command("cp -f $zaptel_conf_file $zaptel_conf_file_t");
		}
	}  	
	
	if ($config_zapata==$TRUE | $is_trixbox==$TRUE){
		print "\nCopying new chan_zap configuration files ($zapata_conf_file_t)...\n";
		exec_command("cp -f $zapata_conf_file $zapata_conf_file_t");
	}
	if( $asterisk_restart == $TRUE ){
        	print "\nStarting Wanpipe...\n";
		exec_command("wanrouter start");

		if ( ! -e "/etc/wanpipe/scripts/start" & $is_trixbox==$FALSE ){ 
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
	exec_command("tar cvfz $debug_tarball $cfg_dir/* >/dev/null 2>/dev/null");
}

sub config_analog{
	my $a20x;
	system('clear');
	print "------------------------------------\n";
	print "Configuring analog cards [A200/A400]\n";
	print "------------------------------------\n";
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
               				$a20x = eval {new A20x(); } or die ($@);
					$a20x->card($card);
					$card->first_chan($current_zap_channel);
       					$current_zap_channel+=24;
					$num_non_ss7_config++;

					if ( $device_has_hwec==$TRUE){
						print "Will this A$1 to synchronize with an existing Sangoma T1/E1 card?\n";
						print "\n WARNING: for hardware and firmware requirements, check:\n";
						print "          http://wiki.sangoma.com/t1e1analogfaxing\n";
						
						if (&prompt_user_list(("YES","NO","")) eq 'NO'){
							$a20x->rm_network_sync('NO');
						} else {
							$a20x->rm_network_sync('YES');
						}
					} 

					print "A$1 configured on slot:$3 bus:$4 span:$devnum\n";
					prompt_user("Press any key to continue");
			 		$zaptel_conf.="#Sangoma A$1 [slot:$3 bus:$4 span:$devnum]\n";
					$zapata_conf.=";Sangoma A$1 [slot:$3 bus:$4 span:$devnum]\n";
					$startup_string.="wanpipe$devnum ";
			
	#				my $i;
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
					$device_has_normal_clock=$FALSE;
					@device_normal_clocks = ("0");
				}
				system('clear');
				my $msg ="Configuring port ".$port." on A".$card->card_model." slot:".$card->pci_slot." bus:".$card->pci_bus.".\n";
				print "\n-----------------------------------------------------------\n";
				print "$msg";
				print "-----------------------------------------------------------\n";
					
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
					my $msg= "A$1 on slot:$3 bus:$4 port:$port not configured\n";
					print "$msg";
					prompt_user("Press any key to continue");
				}else{
					if ($card->hwec_mode eq 'YES' && $device_has_hwec==$FALSE){
						$device_has_hwec=$TRUE;
					} 
			       		
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
						$max_chans = 24;
						$line_media = 'T1';
					}else{
						$a10x->fe_lcode('HDB3');
						printf ("Select framing type for %s on port %s\n", $card->card_model, $port);
						@options = ("CRC4","NCRC4");
						$def_feframe=&prompt_user_list(@options,$def_feframe);
						$a10x->fe_frame($def_feframe);
			      			$current_zap_channel+=31;
						$max_chans = 31;
						$line_media = 'E1';
					}
					my @options = ("NORMAL", "MASTER");
					printf ("Select clock for A%s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $port, $card->pci_slot, $card->pci_bus);
					$def_feclock=&prompt_user_list(@options, $def_feclock);
					$a10x->fe_clock($def_feclock);

					if ( $def_feclock eq 'NORMAL') {
						$device_has_normal_clock=$TRUE;
						 push(@device_normal_clocks, $a10x->fe_line);
					} elsif ( $def_feclock eq 'MASTER' && $device_has_normal_clock == $TRUE ){
						printf("Clock synchronisation options for %s on port %s [slot:%s bus:%s span:$devnum] \n", 
								$card->card_model, 
								$port, 
								$card->pci_slot, 
								$card->pci_bus);
						printf(" Free run: Use internal oscillator on card [default] \n");
        					printf(" Port N: Sync with clock from port N \n\n");
						
						printf("Select clock source %s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $port, $card->pci_slot, $card->pci_bus);
						$def_te_ref_clock=&get_te_ref_clock(@device_normal_clocks);
						$a10x->te_ref_clock($def_te_ref_clock);
					}
			
		     	   	
					if ($is_smg==$TRUE && ($zaptelprobe =~ /zaptel.ko/)){
						@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start", "SS7 - Sangoma Signal Media Gateway", "No Signaling (Voice Only)");
					} elsif ($is_smg==$TRUE && ($zaptelprobe !~ /zaptel.ko/)){
                                         	@options = ("SS7 - Sangoma Signal Media Gateway", "No Signaling (Voice Only)");
					} else {			
		     	   			@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start");
					}

					printf ("Select signalling type for %s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $port, $card->pci_slot, $card->pci_bus);
					$def_signalling=&prompt_user_list(@options,$def_signalling); 
					$a10x->signalling($def_signalling);

					my $ss7_chan;
					my $ss7_group_start;
					my $ss7_group_end;
					my @ss7_chan_array;
					my @ss7_sorted;
					if( $a10x->signalling eq 'SS7 - Sangoma Signal Media Gateway' ){
                                                $a10x->ss7_option(1); 

						print("Choose an option below to configure SS7 signalling channels:\n"); 
						if (&prompt_user_list("Configure consecutive signalling channels(e.g #1-#16)","Configure separate signalling channels(e.g #1,#10)","") eq 'Configure separate signalling channels(e.g #1,#10)'){
					                goto SS7CHAN;
  							while (1){
								print("\nAny other SS7 signalling channel to configure?\n");
					        		if (&prompt_user_list("YES","NO","") eq 'NO'){
				                			goto ENDSS7CONFIG;
							    	}else{
SS7CHAN:		  					$ss7_chan = prompt_user_ss7_chans('Specify the time slot for SS7 signalling(24 for T1? 16 for E1?)');
									push(@ss7_chan_array, $ss7_chan);
									$ss7_array_length = @ss7_chan_array;
									if ($ss7_array_length == $max_chans){
										   goto ENDSS7CONFIG;
									}
								}
       							}
						}else{
			                		goto SS7GROUP;
							while(1){
								print("\nAny other SS7 consecutive signalling channels to configure?\n");
								if (&prompt_user_list("YES","NO","") eq 'NO'){
                							goto ENDSS7CONFIG;
					  			}else{
SS7GROUP:								$ss7_group_start = prompt_user_ss7_chans('Consecutive signalling channels START FROM channel number');
									$ss7_group_end = prompt_user_ss7_chans('Consecutive signalling channels END AT channel number');						        
									if ($ss7_group_start > $ss7_group_end){
										print("The starting channel number is bigger than the ending one!\n");
										goto SS7GROUP;
									}
									my $i = 0;	
									for ($i = $ss7_group_start; $i <= $ss7_group_end; $i++){
                			                		      push(@ss7_chan_array, $i);
									      my @remove_duplicate;
									      @ss7_chan_array = grep(!$remove_duplicate[$_]++, @ss7_chan_array);
									      $ss7_array_length = @ss7_chan_array;				      
									      if ($ss7_array_length > $max_chans){			    										       		   		print("\nERROR : You defined more than $max_chans signalling channels in $line_media and please try to configure them again.\n\n");
									           @ss7_chan_array = ();
										   goto SS7GROUP;
									      }
						 			}
								        if ($ss7_array_length == $max_chans){
      									      goto ENDSS7CONFIG;	
									}	
								}
							}
						}

ENDSS7CONFIG:           		       @ss7_sorted = sort { $a <=> $b } @ss7_chan_array;
 		       			       print("\nYou configured the following SS7 signalling channels: @ss7_sorted\n");
			                       my $ss7_voicechans = gen_ss7_voicechans(@ss7_sorted,$max_chans);
					       $ss7_tdmvoicechans = $ss7_voicechans;					   
					       #print("Voice channels are: $ss7_voicechans\n ");
					       #if($ss7_voicechans eq ''){
					       #	print("No voice channel is configured\n");
					       #}
					       
					       if ($ss7_voicechans =~ m/(\d+)/){						    
						     $a10x->ss7_tdminterface($1);   
					       }

		                               $a10x->ss7_tdmchan($ss7_voicechans);                                             
					       $num_ss7_config++;
					}elsif ( $a10x->signalling eq 'No Signaling (Voice Only)'){
                                               $a10x->ss7_option(2); 
					       $num_ss7_config++;   
					}else{
                                               $num_non_ss7_config++;
					       if ($is_smg==$TRUE){
							 if ($config_zapata==$TRUE || $asked_autozapata > 0){
								goto Label0;
							 }
							 print "Would you like to generate /etc/asterisk/zapata.conf\n";
   							 if (&prompt_user_list(("YES","NO","")) eq 'YES'){
						                $config_zapata = $TRUE;
        						 }
							 $asked_autozapata++;
					       }
					}

Label0:					if ( $a10x->signalling eq 'PRI CPE' | $a10x->signalling eq 'PRI NET' ){    
						if ($config_zapata==$TRUE){
							printf ("Select switchtype for %s on port %s \n", $card->card_model, $port);
							$a10x->pri_switchtype(get_pri_switchtype());
						}
					}


					if( $a10x->signalling eq 'SS7 - Sangoma Signal Media Gateway' ){
						$a10x->ss7_subinterface(1);
						$a10x->gen_wanpipe_ss7_subinterfaces();
						if ($ss7_tdmvoicechans ne ''){
							$a10x->ss7_subinterface(2);
							$a10x->gen_wanpipe_ss7_subinterfaces();
						}
						my $ss7_element;
						foreach $ss7_element (@ss7_sorted){
							$a10x->ss7_sigchan($ss7_element); 
							$a10x->ss7_subinterface(3);
							$a10x->gen_wanpipe_ss7_subinterfaces();
						}					
						$a10x->gen_wanpipe_conf();
						if ($ss7_tdmvoicechans ne ''){
							$a10x->ss7_subinterface(5);
							$a10x->gen_wanpipe_ss7_subinterfaces();
						}
						foreach $ss7_element (@ss7_sorted){
							$a10x->ss7_sigchan($ss7_element); 
							$a10x->ss7_subinterface(6);
							$a10x->gen_wanpipe_ss7_subinterfaces();
						}						
					}else{
						$a10x->gen_wanpipe_conf();
					}


                                        if ($is_smg==$TRUE && $config_zapata==$FALSE){
						if ($a10x->signalling eq 'SS7 - Sangoma Signal Media Gateway'| $a10x->signalling eq 'No Signaling (Voice Only)'){
						        goto Label1;
						}
						$zaptel_conf.=$a10x->gen_zaptel_conf();
					}elsif ($is_smg==$TRUE && $config_zapata==$TRUE){
						if ($a10x->signalling eq 'SS7 - Sangoma Signal Media Gateway'| $a10x->signalling eq 'No Signaling (Voice Only)'){
							goto Label1;
						}else{
							$zaptel_conf.=$a10x->gen_zaptel_conf();
						        $a10x->card->zap_context(&get_zapata_context($a10x->card->card_model,$port));
							$a10x->card->zap_group(&get_zapata_group($a10x->card->card_model,$port,$a10x->card->zap_context));
							$zapata_conf.=$a10x->gen_zapata_conf();
						}
					}elsif ($is_trixbox==$TRUE | $config_zapata==$TRUE){
						$zaptel_conf.=$a10x->gen_zaptel_conf();
					        $a10x->card->zap_context(&get_zapata_context($a10x->card->card_model,$port));
						$a10x->card->zap_group(&get_zapata_group($a10x->card->card_model,$port,$a10x->card->zap_context));
						$zapata_conf.=$a10x->gen_zapata_conf();
					}else{
						$zaptel_conf.=$a10x->gen_zaptel_conf();
					}

Label1:					$devnum++;
					$num_digital_devices++;
					my $msg ="\n\nPort ".$port." on A".$card->card_model." configuration complete...\n";
					print "$msg";
					prompt_user("Press any key to continue");

		      		}  	
			} 

	        #####################################	
		#      for A101/2u, A101/2c         #
 		#####################################

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
					$max_chans = 24;
					$line_media = 'T1';
				}else{
                       			$a10u->fe_lcode('HDB3');
					printf ("Select framing type for %s on port %s\n", $card->card_model, $6);
					my @options = ("CRC4","NCRC4");
					$a10u->fe_frame(&prompt_user_list(@options,""));
					$current_zap_channel+=31;
					$max_chans = 31;
					$line_media = 'E1';
				}
                      		my @options = ("NORMAL", "MASTER");
				printf ("Select clock type for %s on port %s\n", $card->card_model, $a10u->fe_line);
				$a10u->fe_clock(&prompt_user_list(@options,""));			
		     	   	
				if ($is_smg==$TRUE && ($zaptelprobe =~ /zaptel.ko/)){
					@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start", "SS7 - Sangoma Signal Media Gateway", "No Signaling (Voice Only)");
				} elsif ($is_smg==$TRUE && ($zaptelprobe !~ /zaptel.ko/)){
                                       	@options = ("SS7 - Sangoma Signal Media Gateway", "No Signaling (Voice Only)");
				} else {			
	     	   			@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start");
				}

				printf ("Select signalling type for %s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $a10u->fe_line, $card->pci_slot, $card->pci_bus);
			
				$a10u->signalling(&prompt_user_list(@options,""));

				my $ss7_chan_a10u;
				my $ss7_group_start_a10u;
				my $ss7_group_end_a10u;
				my @ss7_chan_array_a10u;	
				my @ss7_sorted_a10u;
				if( $a10u->signalling eq 'SS7 - Sangoma Signal Media Gateway' ){
                                        $a10u->ss7_option(1); 

					print("Choose an option below to configure SS7 signalling channels:\n"); 
					if (&prompt_user_list("Configure consecutive signalling channels(e.g #1-#16)","Configure separate signalling channels(e.g #1,#10)","") eq 'Configure separate signalling channels(e.g #1,#10)'){
				                goto SS7CHAN_A10U;
  						while (1){
							print("\nAny other SS7 signalling channel to configure?\n");
				        		if (&prompt_user_list("YES","NO","") eq 'NO'){
				               			goto ENDSS7CONFIG_A10U;
						    	}else{
SS7CHAN_A10U:		  					$ss7_chan_a10u = prompt_user_ss7_chans('Specify the time slot for SS7 signalling(24 for T1? 16 for E1?)');
								push(@ss7_chan_array_a10u, $ss7_chan_a10u);
								$ss7_array_length = @ss7_chan_array_a10u;
								if ($ss7_array_length == $max_chans){
									   goto ENDSS7CONFIG_A10U;
								}
							}
       						}
					}else{
			               		goto SS7GROUP_A10U;
						while(1){
							print("\nAny other SS7 consecutive signalling channels to configure?\n");
							if (&prompt_user_list("YES","NO","") eq 'NO'){
                						goto ENDSS7CONFIG_A10U;
				  			}else{
SS7GROUP_A10U:							$ss7_group_start_a10u = prompt_user_ss7_chans('Consecutive signalling channels START FROM channel number');
								$ss7_group_end_a10u = prompt_user_ss7_chans('Consecutive signalling channels END AT channel number');						        
								if ($ss7_group_start_a10u > $ss7_group_end_a10u){
									print("The starting channel number is bigger than the ending one!\n");
									goto SS7GROUP_A10U;
								}
								my $i = 0;
								for ($i = $ss7_group_start_a10u; $i <= $ss7_group_end_a10u; $i++){
                		                		      push(@ss7_chan_array_a10u, $i);
								      my @remove_duplicate_a10u;
								      @ss7_chan_array_a10u = grep(!$remove_duplicate_a10u[$_]++, @ss7_chan_array_a10u);
								      $ss7_array_length = @ss7_chan_array_a10u;				      
								      if ($ss7_array_length > $max_chans){			    										       		   	print("\nERROR : You defined more than $max_chans signalling channels in $line_media and please try to configure them again.\n\n");
									   @ss7_chan_array_a10u = ();
									   goto SS7GROUP_A10U;
								      }
					    			}
								if ($ss7_array_length == $max_chans){
      									      goto ENDSS7CONFIG_A10U;	
								}
							}
						}
					}

ENDSS7CONFIG_A10U:           	       @ss7_sorted_a10u = sort { $a <=> $b } @ss7_chan_array_a10u;
 		       		       print("\nYou configure the following SS7 signalling channels: @ss7_sorted_a10u\n");
			               my $ss7_voicechans_a10u = gen_ss7_voicechans(@ss7_sorted_a10u,$max_chans);
				     
				       $ss7_tdmvoicechans = $ss7_voicechans_a10u;				      
					   
				       #print("Voice channels are: $ss7_voicechans_a10u\n ");
				       #if($ss7_voicechans_a10u eq ''){
				       #		print("No voice channel is configured\n");
				       #}
				       if ($ss7_voicechans_a10u =~ m/(\d+)/){						    
					     $a10u->ss7_tdminterface($1);   
				       }

	                               $a10u->ss7_tdmchan($ss7_voicechans_a10u);
				       $num_ss7_config++;
				}elsif ( $a10u->signalling eq 'No Signaling (Voice Only)'){
                                       $a10u->ss7_option(2); 
				       $num_ss7_config++;   
				}else{
                                       $num_non_ss7_config++;
				       if ($is_smg==$TRUE){
						 if ($config_zapata==$TRUE || $asked_autozapata > 0){
							goto Label0_A10U;
						 }
						 print "Would you like to generate /etc/asterisk/zapata.conf\n";
   						 if (&prompt_user_list(("YES","NO","")) eq 'YES'){
					                $config_zapata = $TRUE;
        					 }
						 $asked_autozapata++;
				       }
				}

Label0_A10U:			if ( $a10u->signalling eq 'PRI CPE' | $a10u->signalling eq 'PRI NET' ){    
					if ($config_zapata==$TRUE){
						printf ("Select switchtype for %s on port %s \n", $card->card_model, $port);
						$a10u->pri_switchtype(get_pri_switchtype());
					}
				}

				if( $a10u->signalling eq 'SS7 - Sangoma Signal Media Gateway' ){
					$a10u->ss7_subinterface(1);
					$a10u->gen_wanpipe_ss7_subinterfaces();
					if ($ss7_tdmvoicechans ne ''){
						$a10u->ss7_subinterface(2);
						$a10u->gen_wanpipe_ss7_subinterfaces();
					}
					my $ss7_element_a10u;
					foreach $ss7_element_a10u (@ss7_sorted_a10u){
						$a10u->ss7_sigchan($ss7_element_a10u); 
						$a10u->ss7_subinterface(3);
						$a10u->gen_wanpipe_ss7_subinterfaces();
					}					
					$a10u->gen_wanpipe_conf();
					if ($ss7_tdmvoicechans ne ''){
						$a10u->ss7_subinterface(5);
						$a10u->gen_wanpipe_ss7_subinterfaces();
					}
					foreach $ss7_element_a10u (@ss7_sorted_a10u){
						$a10u->ss7_sigchan($ss7_element_a10u); 
						$a10u->ss7_subinterface(6);
						$a10u->gen_wanpipe_ss7_subinterfaces();
					}						
				}else{
					$a10u->gen_wanpipe_conf();
				}


                                if ($is_smg==$TRUE && $config_zapata==$FALSE){
					if ($a10u->signalling eq 'SS7 - Sangoma Signal Media Gateway'| $a10u->signalling eq 'No Signaling (Voice Only)'){
					        goto Label1_A10U;
					}
					$zaptel_conf.=$a10u->gen_zaptel_conf();
				}elsif ($is_smg==$TRUE && $config_zapata==$TRUE){
					if ($a10u->signalling eq 'SS7 - Sangoma Signal Media Gateway'| $a10u->signalling eq 'No Signaling (Voice Only)'){
						goto Label1_A10U;
					}else{
						$zaptel_conf.=$a10u->gen_zaptel_conf();
					        $a10u->card->zap_context(&get_zapata_context($a10u->card->card_model,$port));
						$a10u->card->zap_group(&get_zapata_group($a10u->card->card_model,$port,$a10u->card->zap_context));
						$zapata_conf.=$a10u->gen_zapata_conf();
					}
				}elsif ($is_trixbox==$TRUE | $config_zapata==$TRUE){
					$zaptel_conf.=$a10u->gen_zaptel_conf();
				        $a10u->card->zap_context(&get_zapata_context($a10u->card->card_model,$port));
					$a10u->card->zap_group(&get_zapata_group($a10u->card->card_model,$port,$a10u->card->zap_context));
					$zapata_conf.=$a10u->gen_zapata_conf();
				}else{
					$zaptel_conf.=$a10u->gen_zaptel_conf();
				}
	

Label1_A10U:			$devnum++;
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


sub get_te_ref_clock{
	my @list_normal_clocks=@_;
	my @f_list_normal_clocks;
	my $f_port;
	foreach my $port (@list_normal_clocks) {
		if ($port eq '0'){
			$f_port = "Free run";
		} else {
			$f_port = "Port ".$port;
		}
		push(@f_list_normal_clocks, $f_port);
	}		

	my $res = &prompt_user_list(@f_list_normal_clocks, "Free run");
	my $i;
	
	for $i (0..$#f_list_normal_clocks){
		if ( $res eq @f_list_normal_clocks[$i] ){
			return @list_normal_clocks[$i];
		}
	}

	print "Internal error: Invalid reference clock\n";
	exit 1;

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
       
       	return $group;
}

sub gen_ss7_voicechans{
      my @ss7_array = @_;
      my $T1CHANS = pop(@ss7_array);
      my $count = @ss7_array;
      my $output ='';
      my $chan_in = 1;
      my $chan_de = 0;
      my $flag = 0;
      my $i = 0;
      my $j = 0;      

      while($i < $count){
	   $j = $i + 1;
           if ($ss7_array[$i] > 2 && $i == 0){
		$chan_de = $ss7_array[$i] - 1;
		$output .= "1-$chan_de";
		$flag = 1;
	   }elsif ($ss7_array[$i] == 2 && $i == 0){
		$output .= "1";
		$flag = 1;
	   }	   

           if ($ss7_array[$j] == ($ss7_array[$i] + 1) && $j < $count){
	       goto Incrementing;
           }elsif ($ss7_array[$i] == $T1CHANS && $i == ($count-1)){
               goto Incrementing;
           }

	   if ($i < ($count-1)){
	   	   $chan_in = $ss7_array[$i]+1;
	           if ($chan_in < ($ss7_array[$j]-1)){
			$chan_de = $ss7_array[$j] - 1;
			if ($flag != 0){
	   			 $output .= ".$chan_in-$chan_de";
			}else{
				$output .= "$chan_in-$chan_de";
				$flag = 1;
			}	
		   }
		   else{
			if ($flag != 0){
				$output .= ".$chan_in";
			}else{
				$output .= "$chan_in";
				$flag = 1;
			}
		   }
   	   }else{
                   $chan_in = $ss7_array[$i]+1;
	           if ($chan_in < $T1CHANS){
			$chan_de = $T1CHANS;
			if ($flag != 0){
	   			$output .= ".$chan_in-$chan_de";
			}else{
				$output .= "$chan_in-$chan_de";
				$flag = 1;
			}
		   }else{
			if ($flag != 0){
				$output .= ".$T1CHANS";
			}else{
				$output .= "$T1CHANS";
				$flag = 1;
			}
		   }
	   }

Incrementing:	   $i++;          
      }
      return $output;
}


sub prompt_user_ss7_chans{
	my @ss7_string = @_;
	my $def_ss7_group_chan = '';

	my $ss7_group_chan=&prompt_user("$ss7_string[0]",$def_ss7_group_chan);

CHECK1:	while (length($ss7_group_chan)==0 |!($ss7_group_chan =~ /^\d+$/)){
	       print("ERROR: Invalid channel number, input an integer only.\n\n");
	       $ss7_group_chan=&prompt_user("$ss7_string[0]",$def_ss7_group_chan);
	}
	if ($line_media eq 'T1'){
	          while ($ss7_group_chan>24 || $ss7_group_chan<1){
	       	        print("Invalid channel number, there are only 24 channels in T1.\n\n");
		        $ss7_group_chan=&prompt_user("$ss7_string[0]",$def_ss7_group_chan);
			goto CHECK1;
	          }
	}elsif ($line_media eq 'E1'){
	          while ($ss7_group_chan>31 || $ss7_group_chan<1){
	  	  	print("Invalid channel number, there are only 31 channels in E1.\n\n");
			$ss7_group_chan=&prompt_user("$ss7_string[0]",$def_ss7_group_chan);
			goto CHECK1;
		  }
	}
	return $ss7_group_chan;
}
