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
# Jun 26   2008  2.13   Jingesh Patel	Fixed scritp for wancfg_smg and tdm_api 
# May 22   2008  2.12	Jignesh Patel 	Added confirmation check /dev/zap* for hardhdlc
# May 22   2008  2.12   Jignesh Patel 	Update zaptel module list 
# Jan 02   2008	 2.11  	David Yat Sin  	Support for per span configuration in silent mode
# Jan 02   2008	 2.10  	David Yat Sin  	Added option for BRI master clock
# Dec 15   2007	 2.9  	David Yat Sin  	Support for Zaptel hardware hdlc for Zaptel 1.4
# Nov 29   2007	 2.8  	David Yat Sin  	Support for BRI cards on Trixbox
# Aug 22   2007	 2.7  	David Yat Sin  	support for hardware DTMF option
# Aug 15   2007	 2.6  	David Yat Sin  	support for BRI cards in SMG mode	
# Jul 20,  2007  2.5 	David Yat Sin  	silent option	
# Jun 13,  2007  	Yuan Shen      	SS7 support
# Jan 15,  2007  	David Yat Sin  	support for non-trixbox installations. Major 
#					changes to script.
# Jan 8,   2007  	David Yat Sin  	script renamed to config-zaptel.pl
# Jan 5,   2007  2.1	David Yat Sin  	v2.1 fix for analog cards inserted in wrong
#					context
# Dec 12,  2006  2.0	David Yat Sin  	2.0 support for T1/E1 cards
# Oct 13,  2006  	David Yat Sin  	Added --opermode and --law option
# Oct 12,  2006  	David Yat Sin  	Initial version
# ============================================================================

system('clear');
print "\n#############################################i##########################";
print "\n#    Sangoma Wanpipe:  Zaptel/SMG/TDMAPI/BOOT Configuration Script     #";
print "\n#                             v2.13                                    #";
print "\n#                     Sangoma Technologies Inc.                        #";
print "\n#                        Copyright(c) 2008.                            #";
print "\n########################################################################\n\n";

use strict;
#use warnings;
#use diagnostics;
use Card;
use A10x;
use A10u;
use A20x;
use A50x;


my $FALSE = 1;
my $TRUE = 0;
my $zaptelprobe=' ';

my $etc_dir="";
my $include_dir="";
my $module_list="";
my $module_load="";
my $module_unload="";
my $os_type_name="";
my $dchan_str="dchan";

my $os_type_list=`sysctl -a |grep ostype`;
if ($os_type_list =~ m/Linux/){
	$os_type_name="Linux";
	$etc_dir="/etc";
	$module_load="modprobe";
	$module_unload="modprobe -r";
	$module_list="modprobe -l";
	$include_dir="/usr/include";
}elsif ($os_type_list =~ m/FreeBSD/){
	$os_type_name="FreeBSD";
	$etc_dir="/usr/local/etc";
	$module_load="kldload";
	$module_unload="kldunload";
	$module_list="kldstat";
}else{
	print("Failed to determine OS type\n");
	print("Exiting...\n");
	exit(1);
}


my $no_boot=$FALSE;
my $boot_only=$FALSE;
#HW DTMF NOT SUPPORTED in 3.2 drivers
#my $no_hwdtmf=$FALSE;
my $no_hwdtmf=$TRUE;
my $startup_string="";
my $cfg_string="";
my $first_cfg=1;
my $zaptel_conf="";
my $zapata_conf="";
my $bri_conf="";
my $woomera_conf="";
my $devnum=1;
my $current_zap_span=1;
my $current_tdmapi_span=1;
#my $current_bri_span=1;
my $current_zap_channel=1;
my $num_analog_devices=0;
my $num_analog_devices_total=0;
my $num_bri_devices=0;
my $num_bri_devices_total=0;
my $num_digital_devices=0;
my $num_digital_devices_total=0;
my $num_ss7_config=0;
my $num_zaptel_config=0;
my $line_media='';
my $max_chans=0;
my $ss7_tdmvoicechans='';
my $ss7_array_length=0;



my $device_has_hwec=$FALSE;
my $device_has_normal_clock=$FALSE;
my @device_normal_clocks=("0");
my @woomera_groups=("0");
my $bri_device_has_master_clock=$FALSE;
			
my $is_tdm_api=$FALSE;

my $def_femedia='';
my $def_feframe='';
my $def_feclock='';
my $def_bri_option='';
my $def_signalling='';
my $def_switchtype='';
my $def_zapata_context='';
my $def_woomera_context='';
my $def_zapata_group='';
my $def_te_ref_clock='';
my $def_tdmv_dchan=0;
my $def_woomera_group='';
my $def_felcode='';
my $def_feframe='';
my $def_te_sig_mode='';

my $def_hw_dtmf="YES";
my $def_tdm_law='';


my @silent_femedias;
my @silent_feframes;
my @silent_felcodes;
my @silent_tdm_laws;
my @silent_feclocks;
my @silent_signallings;
my @silent_pri_switchtypes;
my @silent_zapata_contexts;
my @silent_woomera_contexts;
my @silent_zapata_groups;
my @silent_bri_conn_types;
my @silent_woomera_groups;
my @silent_hwdtmfs;
my @silent_first_chans;
my @silent_last_chans;


my $silent_hwdtmf;
my $silent_femedia="T1";
my $silent_feframe="ESF";
#my $silent_feframe_e1="CRC4";

my $silent_felcode="B8ZS";
my $silent_tdm_law="MULAW";


my $silent_feclock="NORMAL";
my $silent_signalling="PRI CPE";
my $silent_pri_switchtype="national";
my $silent_zapata_context="from-pstn";
my $silent_woomera_context="from-pstn";
my $silent_zapata_group="0";
my $silent_te_sig_mode='CCS';

my $silent_bri_conn_type="point_to_multipoint";
my $silent_woomera_group="1";

my $silent_first_chan=1;
my $silent_last_chan=24;

my $def_bri_country="europe";
my $def_bri_operator="etsi";
my $def_bri_conn_type="Point to multipoint";

my $is_trixbox = $FALSE;
my $silent = $FALSE;
my $config_zaptel = $TRUE;
my $config_zapata = $TRUE;
my $is_smg = $FALSE;

my $tdm_api_span_num=0;
my $zaptel_installed=$FALSE;
my $modprobe_list=`$module_list`;


read_args();
check_zaptel();
if($boot_only==$TRUE){
	exit( &config_boot());
}
my $current_dir=`pwd`;
chomp($current_dir);
my $cfg_dir='tmp_cfg';
my $curdircfg="$current_dir"."/"."$cfg_dir";

unless ( -d $curdircfg ) {
	$curdircfg = "mkdir " . $curdircfg;
	system ($curdircfg); 
}


my $debug_info_file="$current_dir/$cfg_dir/debug_info";
my @hwprobe=`wanrouter hwprobe verbose`;

my $wanpipe_conf_dir="$etc_dir/wanpipe";
my $asterisk_conf_dir="$etc_dir/asterisk";

my $wanrouter_rc_template="$current_dir/templates/wanrouter.rc.template";

my $zaptel_conf_template="$current_dir/templates/zaptel.conf";
my $zaptel_conf_file="$current_dir/$cfg_dir/zaptel.conf";
my $zaptel_conf_file_t="$etc_dir/zaptel.conf";

my $zapata_conf_template="$current_dir/templates/zapata.conf";
my $zapata_conf_file="$current_dir/$cfg_dir/zapata.conf";
my $zapata_conf_file_t="$asterisk_conf_dir/zapata.conf";

my $bri_conf_template="$current_dir/templates/smg_bri.conf";
my $bri_conf_file="$current_dir/$cfg_dir/smg_bri.conf";
my $bri_conf_file_t="$wanpipe_conf_dir/smg_bri.conf";

my $woomera_conf_template="$current_dir/templates/woomera.conf";
my $woomera_conf_file="$current_dir/$cfg_dir/woomera.conf";
my $woomera_conf_file_t="$asterisk_conf_dir/woomera.conf";

my $date=`date +%F`;
chomp($date);
my $debug_tarball="$wanpipe_conf_dir/debug-".$date.".tgz";

if( $zaptel_installed == $TRUE){
	set_zaptel_hwhdlc();
}
prepare_files();
config_t1e1();
config_bri();
config_analog();
summary();
apply_changes();
config_boot();
config_ztcfg_start();
config_smg_ctrl_start();
clean_files();
print "Sangoma cards configuration complete, exiting...\n\n";


#######################################FUNCTIONS##################################################


sub set_zaptel_hwhdlc{
	print "Checking for native zaptel hardhdlc support...";
        my $cnt = 0;
        while ($cnt++ < 30) {
             if ((system("ls /dev/zap* > /dev/null 2>  /dev/null")) == 0) {
	                   goto wait_done;
                } else {
                        print "." ;
                        sleep(1);
                }
        }
	print "Error";
	print "\n\n No /dev/zap* Found on the system \n";
	printf "     Contact Sangoma Support\n";
	print " Send e-mail to techdesk\@sangoma\.com \n\n";
	exit 1;
wait_done:

	if ((system("ztcfg -t -c $current_dir/templates/zaptel.conf_test > /dev/null 2>/dev/null")==0)){
		$dchan_str="hardhdlc";
		 print "Yes \n\n";

        } else {
                print "No \n\n";
	}
}
sub config_boot{
	if($no_boot==$TRUE){
		return 1;
	}
	my $script_name="wanrouter";
	my $current_run_level=3;
	my $zaptel_start_level=9;
	my $zaptel_stop_level=92;
	my $network_start_level=10;
	my $wanrouter_start_level=8;
	my $wanrouter_stop_level=91;
	my $smg_ctrl_start_level=11;
	my $command='';
	my $rc_dir=$etc_dir;			

	if ($os_type_list =~ m/FreeBSD/){
		return 1;
	}
	my $res=`cat $etc_dir/inittab |grep id`;
	if ($res =~ /id:(\d+):initdefault/){
		$current_run_level=$1;
		print "Current boot level is $current_run_level\n";
	} else {
		print "Warning: Failed to determine init boot level, assuming 3\n";
		$current_run_level=3;
	}


	print "\nWanrouter boot scripts configuration...\n\n";
	print "Removing existing $script_name boot scripts...";
	$command="rm -f $rc_dir/rc?.d/*$script_name >/dev/null 2>/dev/null";
	if(system($command) == 0){
		print "OK\n";
	} else {
		print "Not installed\n";
	}
	if($num_ss7_config!=0){
		$script_name="smgss7_init_ctrl";
	}

	my $res='YES';
	if($silent==$FALSE){
		print ("Would you like $script_name to start on system boot?\n");
		$res= &prompt_user_list("YES","NO","");
	} 
	
	if ( $res eq 'YES'){
		#examine system bootstrap files
		$command="find ".$etc_dir."/rc0.d >/dev/null 2>/dev/null";
		if (system($command) == 0){
			$rc_dir=$rc_dir;
		} else {
			$command="find ".$etc_dir."/rc.d/rc0.d >/dev/null 2>/dev/null";
			if (system($command) == 0){
				$rc_dir=$etc_dir."/rc.d";
			} else {
				print "Failed to locate boostrap directory\n";
				print "wanrouter boot scripts will not be installed\n";
				return 1;
			}
		}

		if($zaptel_installed==$TRUE){
			print "Verifying Zaptel boot scripts...";
			#find zaptel start scripts
			$command="find $rc_dir/rc".$current_run_level.".d/*zaptel >/dev/null 2>/dev/null";
			if (system($command) == 0){
				$command="find $rc_dir/rc".$current_run_level.".d/*zaptel";
				$res=`$command`;
				if ($res =~ /.*S(\d+)zaptel/){
					$zaptel_start_level=$1;
					print "Enabled (level:$zaptel_start_level)\n";
				} else {
					print "\nfailed to parse zaptel boot level, assuming $zaptel_start_level";
				}
			} else {
				print "Not installed\n";
				$zaptel_start_level=0;
			}
			
			#find zaptel stop scripts
			print "Verifying Zaptel shutdown scripts...";
			$command="find ".$rc_dir."/rc6.d/*zaptel >/dev/null 2>/dev/null";
			if (system($command) == 0){
				$command="find ".$rc_dir."/rc6.d/*zaptel";
				$res=`$command`;
				if ($res =~ /.*K(\d+)zaptel/){
					$zaptel_stop_level=$1;
					print "Enabled (level:$zaptel_stop_level)\n";
				} else {
					print "\nfailed to parse zaptel boot level, assuming $zaptel_stop_level\n";
				}
			} else {
				print "Not installed\n";
				$zaptel_stop_level=0;
			}
			if ($zaptel_start_level != 0){
				$wanrouter_start_level=$zaptel_start_level-1;	
			}
			if ($zaptel_stop_level != 0){
				$wanrouter_stop_level=$zaptel_stop_level-1;	
			}
		}
			
		my $wanrouter_start_script='';
		if($wanrouter_start_level < 10){
			$wanrouter_start_script="S0".$wanrouter_start_level.$script_name;
		} else {
			$wanrouter_start_script="S".$wanrouter_start_level.$script_name;
		}
		my $wanrouter_stop_script='';
		if($wanrouter_stop_level < 10){
			$wanrouter_stop_script="K0".$wanrouter_stop_level.$script_name;
		} else {
			$wanrouter_stop_script="K".$wanrouter_stop_level.$script_name;
		}

		$command="find $etc_dir/init.d/$script_name >/dev/null 2>/dev/null";
		if(system($command) !=0){
			$command="install -D -m 755 /usr/sbin/$script_name $rc_dir/init.d/$script_name";
			if(system($command) !=0){
				print "Failed to install $script_name script to $rc_dir/init.d/$script_name\n";
				print "$script_name boot scripts not installed\n";
				return 1;			}
		}
		print "Enabling $script_name boot scripts ...(level:$wanrouter_start_level)\n";
		my @run_levels= ("2","3","4","5");
		foreach my $run_level (@run_levels) {
			$command="ln -sf $rc_dir/init.d/$script_name ".$rc_dir."/rc".$run_level.".d/".$wanrouter_start_script;
			if(system($command) !=0){
				print "Failed to install $script_name init script to $rc_dir/rc$run_level.d/$wanrouter_start_script\n";
				print "$script_name start scripts not installed\n";
				return 1;
			}
		}	
				
		print "Enabling $script_name shutdown scripts ...(level:$wanrouter_stop_level)\n";
		@run_levels= ("0","1","6");
		foreach my $run_level (@run_levels) {
			$command="ln -sf $rc_dir/init.d/$script_name ".$rc_dir."/rc".$run_level.".d/".$wanrouter_stop_script;
			if(system($command) !=0){
				print "Failed to install $script_name shutdown script to $rc_dir/rc$run_level.d/$wanrouter_stop_script\n";
				print "$script_name stop scripts not installed\n";
				return 1;
			}
		}	

		if($num_bri_devices != 0){
			#smg_ctrl must start after network
			print "Verifying Network boot scripts...";
			$command="find $rc_dir/rc".$current_run_level.".d/*network >/dev/null 2>/dev/null";
			if (system($command) == 0){
				$command="find $rc_dir/rc".$current_run_level.".d/*network";
				$res=`$command`;
				if ($res =~ /.*S(\d+)network/){
					$network_start_level=$1;
					print "Enabled (level:$network_start_level)\n";
				} else {
					print "\nfailed to parse network boot level, assuming $network_start_level";
				}
			} else {
				print "Not installed\n";
				$network_start_level=0;
			}
			if ($network_start_level != 0 && $network_start_level > $zaptel_start_level){
				$smg_ctrl_start_level=$network_start_level+1;	
				print "Enabling smg_ctrl start scripts...(level:$smg_ctrl_start_level)\n";		
				$command="install -D -m 755 /usr/sbin/smg_ctrl $rc_dir/init.d/smg_ctrl";
				if(system($command) !=0){
					print "Failed to install smg_ctrl start scripts to $rc_dir/init.d/smg_ctrl";
					print "smg_ctrl start scripts not installed";
					return 1;
				}
				my $smg_ctrl_start_script='';
				if($smg_ctrl_start_level < 10){
					$smg_ctrl_start_script="S0".$smg_ctrl_start_level."smg_ctrl";
				} else {
					$smg_ctrl_start_script="S".$smg_ctrl_start_level."smg_ctrl";
				}
				print "Enabling smg_ctrl boot scripts ...(level:$smg_ctrl_start_level)\n";
				my @run_levels= ("2","3","4","5");
				foreach my $run_level (@run_levels) {
					$command="ln -sf $rc_dir/init.d/smg_ctrl ".$rc_dir."/rc".$run_level.".d/".$smg_ctrl_start_script;
					if(system($command) !=0){
						print "Failed to install smg_ctrl init script to $rc_dir/rc$run_level.d/$smg_ctrl_start_script\n";
						print "smg_ctrl start scripts not installed\n";
						return 1;
					}
				}	
			}
			
		}

	}
	return 0;	
}


sub config_ztcfg_start{
	if ($num_zaptel_config ==0 || $silent==$TRUE){
		return;
	}
	my $command="find /etc/rc?.d/*zaptel >/dev/null 2>/dev/null";
	if (system($command) != 0){
		#Zaptel init scripts not installed, prompt for wanpipe_zaptel_start_script
		print ("\nWould you like to execute \'ztcfg\' each time wanrouter starts?\n");
		if (&prompt_user_list("YES","NO","") eq 'YES'){
			exec_command("cp -f $current_dir/templates/zaptel_cfg_script $wanpipe_conf_dir/scripts/start");	
		}
	}
}

sub config_smg_ctrl_start{
	if($num_bri_devices == 0){
		return;
	}
	my $res;
	if($silent==$FALSE) {
		print ("Would you like smg_ctrl to start/stop on wanrouter start?\n");
		$res = &prompt_user_list("YES","NO","");
	} else {
		$res = "YES";
	}
	
	if ($res = "YES"){	
		#if zaptel start script is in $wanpipe_conf_dir/scripts/start, do not overwrite
		my $command="find ".$wanpipe_conf_dir."/scripts/start >/dev/null 2>/dev/null";
		if (system($command) == 0){
			$command="cat ".$current_dir."/templates/smgbri_start_script_addon >>".$wanpipe_conf_dir."/scripts/start";
		} else {
			$command="cp -f ".$current_dir."/templates/smgbri_start_script ".$wanpipe_conf_dir."/scripts/start";
		}
		
		if (system($command) == 0){
			print "smgbri start script installed successfully\n";
		} else {
			print "failed to install smgbri start script\n";
		}

		$command="cp -f ".$current_dir."/templates/smgbri_stop_script ".$wanpipe_conf_dir."/scripts/stop";
		if (system($command) == 0){
			print "smgbri stop script installed successfully\n";
		} else {
		print "failed to install smgbri start script\n";
		}
	}
}

sub check_zaptel{
	if ($modprobe_list =~ /zaptel.ko/){
		$zaptel_installed=$TRUE;
	}
}

sub apply_changes{
	my $asterisk_command='';
	my $bri_command='';
	my $asterisk_restart=$FALSE;
	my $res='';

	if($silent==$FALSE) {system('clear')};
	
	if($silent==$TRUE){
		$res="Stop now";
	}elsif($is_tdm_api==$TRUE){
		print "\n Wanpipe configuration complete: choose action\n";
		$res=&prompt_user_list(	"Save cfg: Stop Wanpipe now",
					"Do not save cfg: Exit",
					"");
	}else{
		print "\nZaptel and Wanpipe configuration complete: choose action\n";
		$res=&prompt_user_list("Save cfg: Restart Asterisk & Wanpipe now",
					"Save cfg: Restart Asterisk & Wanpipe when convenient",
					"Save cfg: Stop Asterisk & Wanpipe now", 
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
	} elsif ($is_tdm_api==$FALSE ){
		if (`(pidof asterisk)` != 0 ){
			print "\nStopping Asterisk...\n";
			exec_command("asterisk -rx \"$asterisk_command\"");
			sleep 2;
			while (`(pidof asterisk)` != 0 ){
				if ($asterisk_command eq "stop now"){
					print "Failed to stop asterisk using command: \'$asterisk_command\' \n";
					my @options=("Force Stop - Send KILL signal to asterisk", "Wait - Wait for asterisk to stop", "Exit - Do not apply changes");
					my $res=&prompt_user_list(@options,"");
	
					if ( $res =~ m/Force Stop/){
						execute_command("kill -KILL \$(pidof asterisk)");
					} elsif ( $res =~ m/Exit/ ){
						exit(1);
					} else {
						print "Waiting for asterisk to stop...\n";
						sleep 5;
						exec_command("asterisk -rx \"$asterisk_command\"");
					}
				} else { 
					#stop when convenient option was selected
					print "Waiting for asterisk to stop...\n";
					sleep 3;
				}
			}





		}else {
			print "\nAsterisk is not running...\n";
		}
			
	} 

	if($num_bri_devices != 0){
		exec_command("/usr/sbin/smg_ctrl stop");
	}

	print "\nStopping Wanpipe...\n";
	exec_command("wanrouter stop all");

	if ($zaptel_installed==$TRUE){
		if($is_tdm_api==$FALSE){
			print "\nUnloading Zaptel modules...\n";
			unload_zap_modules();
		}
	}

	print "\nRemoving old configuration files...\n";

	exec_command("rm -f $wanpipe_conf_dir/wanpipe*.conf");
	
	gen_wanrouter_rc();

	print "\nCopying new Wanpipe configuration files...\n";
	copy_config_files();
	if($num_bri_devices != 0){
		print "\nCopying new sangoma_brid configuration files ($bri_conf_file_t)...\n";
		exec_command("cp -f $bri_conf_file $bri_conf_file_t");
		exec_command("cp -f $woomera_conf_file $woomera_conf_file_t");
	}
	
	if ($zaptel_installed==$TRUE){
		if($config_zaptel==$TRUE){
			if ($num_zaptel_config !=0){
				print "\nCopying new Zaptel configuration file ($zaptel_conf_file_t)...\n";
				exec_command("cp -f $zaptel_conf_file $zaptel_conf_file_t");
			}
		}
	}  	
	
	if ($config_zapata==$TRUE || $is_trixbox==$TRUE){
		if ($num_zaptel_config !=0){
			print "\nCopying new chan_zap configuration files ($zapata_conf_file_t)...\n";
			exec_command("cp -f $zapata_conf_file $zapata_conf_file_t");
		}
	}

	if( $asterisk_restart == $TRUE && $is_tdm_api==$FALSE){
		print "\nStarting Wanpipe...\n";
		exec_command("wanrouter start");

		if($num_bri_devices != 0){
			print "Loading SMG BRI...\n";
			sleep 2;
			exec_command("/usr/sbin/smg_ctrl start");
		}

		if ($num_zaptel_config != 0){	
			print "Loading Zaptel...\n";	
			sleep 2;
			exec_command("ztcfg -v");   
#			}	 
		}
		if ($is_trixbox==$TRUE){
			print "\nStarting Amportal...\n";
			exec_command("amportal start");
			sleep 2;
		}elsif($config_zapata==$TRUE){
			print "\nStarting Asterisk...\n";
			exec_command("asterisk");
			sleep 2;
	
				
			if ($num_zaptel_config != 0){	
				print "\nListing Asterisk channels...\n\n";
				exec_command("asterisk -rx \"zap show channels\"");
			}
			print "\nType \"asterisk -r\" to connect to Asterisk console\n\n";
		}else{
		}
	}	
	print "\nWanrouter start complete...\n";
}



sub save_debug_info{
	my $version=`wanrouter version`;
	chomp($version);

	my $uname=`uname -a`;
	chomp($uname);

	my $issue='';

	if ($os_type_list =~ m/Linux/){
		$issue=`cat $etc_dir/issue`;
		chomp($issue);
	}

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
	if ($os_type_list =~ m/Linux/){
		$debug_info.="\n$os_type_name distribution \"cat $etc_dir/issue\"\n"; 
		$debug_info.="$issue\n";
	}
	$debug_info.="EOF\n";
	
	open (FH,">$debug_info_file") or die "Could not open $debug_info_file";
	print FH $debug_info;
	close (FH);	
	exec_command("tar cvfz $debug_tarball $cfg_dir/* >/dev/null 2>/dev/null");
}

sub get_chan_no{
	my ($chan_name,$first_ch, $last_ch)=@_;
	
	my $res_ch=&prompt_user("\nInput the $chan_name channel for this span [$first_ch-$last_ch]\n");
	while(length($res_ch)==0 ||!($res_ch =~/(\d+)/) 
		|| $res_ch<$first_ch || $res_ch>$last_ch){
		print "Invalid channel, must be between $first_ch and $last_ch\n";
		$res_ch=&prompt_user("Input the channel for this port[$first_ch-$last_ch]");
	}
	return $res_ch;
}

sub get_zapata_context{
	my ($card_model,$card_port)=@_;
	my $context='';
	my @options = ("from-pstn", "from-internal","Custom");
	
	if($is_trixbox==$TRUE){
		@options = ("PSTN", "INTERNAL");
	}
	if ($silent==$FALSE){
		printf ("Select dialplan context for AFT-A%s on port %s\n", $card_model, $card_port);
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
	} else {
		if($#silent_zapata_contexts >= 0){
			$silent_zapata_context=pop(@silent_zapata_contexts);
		}
		$context=$silent_zapata_context;	
	}
	$def_zapata_context=$context;
	return $context;	
}


sub get_woomera_context{
        my ($group,$card_model,$card_port,$bri_type)=@_;

        my $context='';
	my @options = ("from-pstn", "from-internal","Custom");

	if($bri_type eq 'bri_nt'){
	        @options = ("from-internal","Custom");
	} elsif ($bri_type eq 'bri_te'){
	        @options = ("from-pstn","Custom");
	}

        if($is_trixbox==$TRUE){
                @options = ("PSTN", "INTERNAL");
        }
        if ($silent==$FALSE){
                printf ("\nSelect dialplan context for group:%d\n", $group);
                my $res = &prompt_user_list(@options,$def_woomera_context);
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
                                $res_context=&prompt_user("Input the context for this port",$def_woomera_context);
                        }

                        $context=$res_context;
                }elsif($res eq $def_woomera_context){
                        $context=$def_woomera_context;
                }else{
                        print "Internal error:invalid context,group\n";
                        exit 1;
                }
        } else {
		if($#silent_woomera_contexts >= 0){
			$silent_woomera_context=pop(@silent_woomera_contexts);
		}
                $context=$silent_woomera_context;
        }
        $def_woomera_context=$context;
        return $context;
}



sub gen_wanrouter_rc{
	#update wanpipe startup sequence
	my $rcfile="";
	if (!open (FH,"$wanpipe_conf_dir/wanrouter.rc")) {
		open (FH,"$wanrouter_rc_template");
	}
	while (<FH>) {
		$rcfile .= $_;
	}
	close (FH);
	open (FH,">$current_dir/$cfg_dir/wanrouter.rc");
	$rcfile =~ s/WAN_DEVICES\s*=.*/WAN_DEVICES="$startup_string"/g;
	print FH $rcfile;
	close (FH);
}

sub prepare_files{
	
	if ($is_trixbox==$TRUE){
		$zapata_conf_template="$current_dir/templates/zapata-auto.conf";
		$zapata_conf_file="$current_dir/$cfg_dir/zapata-auto.conf";
		$zapata_conf_file_t="$asterisk_conf_dir/zapata-auto.conf";
	}

	if ($silent==$FALSE){
		if ($is_trixbox==$FALSE && $is_smg==$FALSE && $is_tdm_api==$FALSE){
			print "Would you like to generate $zapata_conf_file_t\n";
			if (&prompt_user_list(("YES","NO","")) eq 'NO'){
				$config_zapata = $FALSE;
			}
		}	
	}

#remove temp files
	my $tmp_cfg_dir="$current_dir"."/"."$cfg_dir";
	if ( -d "$current_dir"."/"."$cfg_dir") {
		exec_command("rm -f $current_dir/$cfg_dir/*");
	}
#backup existing configuration files
	if ( -f $zaptel_conf_file_t ) {
		exec_command("cp -f $zaptel_conf_file_t $zaptel_conf_file_t.bak ");
	} 

	if ( -f $zapata_conf_file_t ) {
		exec_command("cp -f $zapata_conf_file_t $zapata_conf_file_t.bak");
	}
}

sub clean_files{
	exec_command("rm -rf $current_dir/$cfg_dir");
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
	my @wanpipe_files = split / /, $cfg_string; 	
	exec_command("cp -f $current_dir/$cfg_dir/wanrouter.rc $wanpipe_conf_dir");
	foreach my $wanpipe_file (@wanpipe_files) {
		exec_command("cp -f $current_dir/$cfg_dir/$wanpipe_file.conf $wanpipe_conf_dir");
	}
}

sub unload_zap_modules{
	my @modules_list = ("ztdummy","wctdm","wcfxo","wcte11xp","wct1xxp","wct4xxp","tor2","zttranscode","wcusb", "wctdm24xxp","xpp_usb","xpp" ,"wcte12xp","opvxa1200", "zaptel");

	foreach my $module (@modules_list) {	
		if ($modprobe_list =~ m/$module/){
			exec_command("$module_unload $module");
		}
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

sub summary{
	if($devnum==1){
		if ($silent==$FALSE) {system('clear')};
		print "\n###################################################################";
		print "\n#                             SUMMARY                             #";
		print "\n###################################################################\n\n";
							
		print("\n\nNo Sangoma voice compatible cards found/configured\n\n"); 
		exit 0;
	}else{
		if ($num_zaptel_config != 0 && $config_zaptel==$TRUE){
			write_zaptel_conf();
		}
		if ($num_bri_devices != 0 ){
			write_bri_conf();
			write_woomera_conf();
		}
		if ($num_zaptel_config != 0 && $config_zapata==$TRUE){
			write_zapata_conf();
		}
		save_debug_info();
		if ($silent==$FALSE) {system('clear')};
		my $file_list = 1;
		print "\n###################################################################";
		print "\n#                             SUMMARY                             #";
		print "\n###################################################################\n\n";

		print("  $num_digital_devices_total T1/E1 port(s) detected, $num_digital_devices configured\n");
		print("  $num_bri_devices_total ISDN BRI port(s) detected, $num_bri_devices configured\n");
		print("  $num_analog_devices_total analog card(s) detected, $num_analog_devices configured\n");
		
		print "\nConfigurator will create the following files:\n";
		print "\t1. Wanpipe config files in $wanpipe_conf_dir\n";
		$file_list++;
		
		if ($num_bri_devices != 0){
			print "\t$file_list. sangoma_brid config file $wanpipe_conf_dir/smg_bri\n";
			$file_list++;
		}
		
		if ($num_zaptel_config != 0){
			print "\t$file_list. Zaptel config file $zaptel_conf_file_t\n";
			$file_list++;
		}
		if ($config_zapata==$TRUE){
			print "\t$file_list. Zapata config file $zapata_conf_file_t\n";
		}
		
		if (($num_zaptel_config != 0) | ($config_zapata==$TRUE)){	
			print "\n\nYour original configuration files will be saved to:\n";
			$file_list=1;
		}	
			
		if ($num_zaptel_config != 0){
			print "\t$file_list. $zaptel_conf_file_t.bak \n";
			$file_list++;
		}
		if ($num_zaptel_config !=0 && $config_zapata==$TRUE){
			print "\t$file_list. $zapata_conf_file_t.bak \n\n";
		}
		
		print "\nYour configuration has been saved in $debug_tarball.\n";
		print "When requesting support, email this file to techdesk\@sangoma.com\n\n";
		if($silent==$FALSE){
			confirm_conf();
		}
	}
}

sub confirm_conf(){
	print "Configuration Complete! Please select following:\n";
	if(&prompt_user_list("YES - Continue", "NO - Exit" ,"") =~ m/YES/){
		return $?;
	} else {
		print "No changes made to your configuration files\n";
		print "exiting.....\n";	
		exit $?;
	}
}
sub exec_command{
	my @command = @_;
	if (system(@command) != 0 ){
		print "Error executing command:\n@command\n\n";
		if($silent==$FALSE){
			print "Would you like to continue?\n";
			if(&prompt_user_list("No - exit", "YES", "No") eq 'YES'){
				return $?;
			} else {
				exit $?;
			}
		}
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
		$_ = $ARGV[$arg_num];
		if( /^--trixbox$/){
			$is_trixbox=$TRUE;
		}elsif( /^--install_boot_script/){
			$boot_only=$TRUE;	
		}elsif ( /^--tdm_api/){
			$is_tdm_api=$TRUE;
		}elsif ( /^--smg$/){
			$is_smg=$TRUE;
		}elsif ( /^--no_boot$/){
			$no_boot=$TRUE;		
		}elsif ( /^--no_hwdtmf$/){
			$no_hwdtmf=$TRUE;
		}elsif ( /^--silent$/){
			$silent=$TRUE;
		}elsif ( /^--no-zapata$/){
			$config_zapata=$FALSE;
		}elsif ( /^--no-zaptel$/){
			$config_zaptel=$FALSE;
		}elsif ( $_ =~ /--zapata_context=(\w+)/){
			$silent_zapata_context=substr($_,length("--zapata_context="));
			push(@silent_zapata_contexts, $silent_zapata_context);
		}elsif ( $_ =~ /--zapata_group=(\d+)/){
			$silent_zapata_group=$1;
			push(@silent_zapata_groups, $silent_zapata_group);
		}elsif ( $_ =~ /--woomera_context=(\w+)/){
			$silent_woomera_context=substr($_,length("--woomera_context="));
			push(@silent_woomera_contexts, $silent_woomera_context);
		}elsif ( $_ =~ /--woomera_group=(\d+)/){
			$silent_woomera_group=$1;
			push(@silent_woomera_groups, $silent_woomera_group);
		}elsif ( $_ =~ /--fe_media=(\w+)/){
			$silent_femedia=$1;
			if(!($silent_femedia eq 'T1' || $silent_femedia eq 'E1')){
				printf("Invalid value for fe_media, should be T1/E1\n");
				exit(1);
			} else {	
				push(@silent_femedias, $silent_femedia);
				if($silent_femedia eq 'E1'){
					if(!($silent_feframe eq 'CRC4' || $silent_feframe eq 'NCRC4')){
						$silent_feframe='CRC4';
					}
					if(!($silent_felcode eq 'HDB3' || $silent_felcode eq 'AMI')){
						$silent_felcode='HDB3';
					}
				}
			}
		}elsif ( $_ =~ /--fractional_chans=(\d+)-(\d+)/ ){
			$silent_first_chan=$1;
			$silent_last_chan=$2;
			push(@silent_first_chans, $silent_first_chan);
			push(@silent_last_chans, $silent_last_chan);
		}elsif ( $_ =~ /--hw_dtmf=(\w+)/){
			$silent_hwdtmf=$1;
			if(!($silent_hwdtmf eq 'YES' || $silent_hwdtmf eq 'NO')){
				printf("Invalid value for hw_dtmf, should be YES/NO\n");
				exit(1);
			} else {	
				push(@silent_hwdtmfs, $silent_hwdtmf);
			}
		}elsif ( $_ =~ /--fe_lcode=(\w+)/){
			$silent_felcode=$1;
			if(!($silent_felcode eq 'B8ZS' || $silent_felcode eq 'HDB3' || $silent_felcode eq 'AMI')){
				printf("Invalid value for fe_lcode, should be B8ZS/HDB3/AMI \n");
				exit(1);
			} else {	
				push(@silent_felcodes, $silent_felcode);
			}			
		}elsif ( $_ =~ /--fe_frame=(\w+)/){
			$silent_feframe=$1;
			if(!($silent_feframe eq 'ESF' || $silent_feframe eq 'D4' || $silent_feframe eq 'CRC4' || $silent_feframe eq 'NCRC4')){
				printf("Invalid value for fe_frame, should be ESF/D4/CRC4/NCRC4\n");
				exit(1);
			} else {	
				push(@silent_feframes, $silent_feframe);
			}	
		}elsif ( $_ =~ /--tdm_law=(\w+)/){
			$silent_tdm_law=$1;
			if(!($silent_tdm_law eq 'MULAW' || $silent_tdm_law eq 'ALAW')){
				printf("Invalid value for tdm_law, should be MULAW/ALAW\n");
				exit(1);
			} else {	
				push(@silent_tdm_laws, $silent_tdm_law);
			}
		}elsif ( $_ =~ /--fe_clock=(\w+)/){
			$silent_feclock=$1;
			if(!($silent_feclock eq 'NORMAL' || $silent_feclock eq 'MASTER' )){
				printf("Invalid value for fe_clock, should be NORMAL/MASTER\n");
				exit(1);
			} else {	
				push(@silent_feclocks, $silent_feclock);
			}			
		}elsif ( $_ =~ /--signalling=(\w+)/){
			my $tmp_signalling=$1;
			if ($tmp_signalling eq 'em'){
				$silent_signalling="E & M";
			}elsif ($tmp_signalling eq 'em_w'){
				$silent_signalling="E & M Wink";
			}elsif ($tmp_signalling eq 'pri_cpe'){
				$silent_signalling="PRI CPE";
			}elsif ($tmp_signalling eq 'pri_net'){
				$silent_signalling="PRI NET";
			}elsif ($tmp_signalling eq 'fxs_ls'){
				$silent_signalling="FXS - Loop Start";
			}elsif ($tmp_signalling eq 'fxs_gs'){
				$silent_signalling="FXS - Ground Start";
			}elsif ($tmp_signalling eq 'fxs_ks'){
				$silent_signalling="FXS - Kewl Start";
			}elsif ($tmp_signalling eq 'fxo_ls'){
				$silent_signalling="FXO - Loop Start";
			}elsif ($tmp_signalling eq 'fxo_gs'){
				$silent_signalling="FXO - Ground Start";
			}elsif ($tmp_signalling eq 'fxo_ks'){
				$silent_signalling="FXO - Kewl Start";
			}else{
				printf("Invalid option for --signalling, options are\n");
				printf("\t pri_cpe/pri_net/em/em_w\n");
				printf("\t fxo_ls/fxo_gs/fxo_ks\n");
				printf("\t fxs_ls/fxs_gs/fxs_ks\n");
				exit(1);
			}
			
			push(@silent_signallings, $silent_signalling);
						
		}elsif ( $_ =~ /--pri_switchtype=(\w+)/){
			my $tmp_switchtype=$1;
			if ($tmp_switchtype eq 'national'){
				$silent_pri_switchtype="national"
			}elsif ($tmp_switchtype eq 'dms100'){
				$silent_pri_switchtype="dms100"
			}elsif ($tmp_switchtype eq '4ess'){
				$silent_pri_switchtype="4ess"
			}elsif ($tmp_switchtype eq '5ess'){
				$silent_pri_switchtype="5ess"
			}elsif ($tmp_switchtype eq 'euroisdn'){
				$silent_pri_switchtype="euroisdn"
			}elsif ($tmp_switchtype eq 'ni1'){
				$silent_pri_switchtype="ni1"
			}elsif ($tmp_switchtype eq 'qsig'){
				$silent_pri_switchtype="qsig"
			} else {
				printf("Invalid option for --pri_switchtype, options are\n");
				printf("\t national/dms100/4ess/5ess/euroisdn/ni1/qsig");
				exit(1);
			}
			push(@silent_pri_switchtypes, $silent_pri_switchtype);
		}elsif ( $_ =~ /--conf_dir=(.*)/){
			$etc_dir=$1;
			if (! -d $etc_dir){	
				printf("Error: directory $1 does not exist\n");
				exit(1);
			}
		}else {
			printf("Error: Unrecognized parameter \"$_\" \n");
			exit(1);

		}
	}
	@silent_femedias = reverse(@silent_femedias);
	@silent_feframes = reverse(@silent_feframes);
	@silent_felcodes = reverse(@silent_felcodes);
	@silent_tdm_laws = reverse(@silent_tdm_laws);
	@silent_feclocks = reverse(@silent_feclocks);
	@silent_signallings = reverse(@silent_signallings);
	@silent_pri_switchtypes = reverse(@silent_pri_switchtypes);
	@silent_zapata_contexts = reverse(@silent_zapata_contexts);
	@silent_woomera_contexts = reverse(@silent_woomera_contexts);
	@silent_zapata_groups = reverse(@silent_zapata_groups);
	@silent_hwdtmfs = reverse(@silent_hwdtmfs);
	@silent_first_chans = reverse(@silent_first_chans);
	@silent_last_chans = reverse(@silent_last_chans);

	if ($is_trixbox==$TRUE){
		print "\nGenerating configuration files for Trixbox\n";
	}
	if ($is_smg==$TRUE){
		print "\nGenerating configuration files for Sangoma Media Gateway\n";
	}
	if ($is_tdm_api==$TRUE){
		$config_zapata = $FALSE;    
	}

}

#------------------------------BRI FUNCTIONS------------------------------------#
sub get_bri_country {
	$def_bri_country = "europe"; 
	return $def_bri_country;
}

sub get_woomera_group{
	if($silent==$TRUE){
		if($#silent_woomera_groups >= 0){
			$silent_woomera_group=pop(@silent_woomera_groups);
		}
		return $silent_woomera_group;
	}

	my $group;
	my $res_group=&prompt_user("\nInput the group for this port\n",$def_woomera_group);
	while(length($res_group)==0 |!($res_group =~/(\d+)/)| $res_group eq '0'){
		print "Invalid group, input an integer greater than 0\n";
		$res_group=&prompt_user("Input the group for this port",$def_woomera_group);
	}
	$group=$res_group;
	$def_woomera_group=$group;
	return $def_woomera_group;
}


sub get_bri_operator {
#warning returning ETSI
	$def_bri_operator = "etsi";
	return $def_bri_operator;



	my @options = ( "European ETSI Technical Comittee", "France Telecom VN2", "France Telecom VN3",
			"France Telecom VN6", "Deutsche Telekom 1TR6", "British Telecom ISDN2",
			"Belgian V1", "Sweedish Televerket", "ECMA 143 QSIG");

	my @options_val = ("etsi", "ft_vn2", "ft_vn3", "ft_vn6", "dt_1tr6", "bt_isdn2", "bg_vi", "st_swd", "ecma_qsi");

	my $res = &prompt_user_list(@options,$def_switchtype);
	
	my $i;
	for $i (0..$#options){
		if ( $res eq @options[$i] ){
			$def_bri_operator=@options[$i];
			return @options_val[$i]; 
		}
	}	
	print "Internal error: Invalid PRI switchtype\n";
	exit 1;
}

sub write_bri_conf{
	my $bri_file="";
	open(FH, "$bri_conf_template") or die "cannot open $bri_conf_template";
	while (<FH>) {
		$bri_file .= $_;
	}
	close (FH);
	$bri_file=$bri_file.$bri_conf;	
	open(FH, ">$bri_conf_file") or die "cannot open $bri_conf_file";
		print FH $bri_file;
	close(FH);	
}

sub write_woomera_conf{
	my $woomera_file="";
	open(FH, "$woomera_conf_template") or die "cannot open $woomera_conf_template";
	while (<FH>) {
		$woomera_file .= $_;
	}
	close (FH);
	$woomera_file=$woomera_file.$woomera_conf;	
	open(FH, ">$woomera_conf_file") or die "cannot open $woomera_conf_file";
		print FH $woomera_file;
	close(FH);	
}


sub get_bri_conn_type{
	my ($port)=@_;
	
	if($silent==$TRUE){
		if($#silent_bri_conn_types >= 0){
			$silent_bri_conn_type=pop(@silent_bri_conn_types);
		}
		return $silent_bri_conn_type;
	}
        printf("\nSelect connection type for port %d\n", $port);
	my $conn_type;
	
	my @options = ( "Point to multipoint", "Point to point");
	my @options_val = ("point_to_multipoint", "point_to_point");

	my $res = &prompt_user_list(@options,$def_bri_conn_type);
	
	my $i;
	for $i (0..$#options){
		if ( $res eq @options[$i] ){
			$def_bri_conn_type=@options[$i];
			return @options_val[$i]; 
		}
	}	
	print "Internal error: Invalid BRI connection type\n";
	exit 1;
}




sub config_bri{
	if($is_smg!=$TRUE && $is_trixbox==$FALSE){
		return;
	}
	my $a50x;
	if (!$first_cfg && $silent==$FALSE) {
		system('clear');
	}
	$first_cfg=0;
	print "------------------------------------\n";
	print "Configuring ISDN BRI cards [A500]\n";
	print "------------------------------------\n";
	my $skip_card=$FALSE;
	$zaptel_conf.="\n";
	$zapata_conf.="\n";
	foreach my $dev (@hwprobe) {
		if ( $dev =~ /A(\d+)(.*):.*SLOT=(\d+).*BUS=(\d+).*PORT=(\w+).*HWEC=(\w+).*/){
			$skip_card=$FALSE;
			if ($1 eq '500'){
				my $card = eval {new Card(); } or die ($@);

				$card->current_dir($current_dir);
				$card->cfg_dir($cfg_dir);
				$card->device_no($devnum);
				$card->card_model($1);
				$card->pci_slot($3);
				$card->pci_bus($4);
				
				my $hwec=0;
				if($6 gt 0){
					$hwec=1;	
				}
				if ($hwec==0){
					$card->hwec_mode('NO');
				}else{
					$card->hwec_mode('YES');
				}

				if ($card->card_model eq '500'){
					$num_bri_devices_total++;
					if($5 eq '1'){
						$bri_device_has_master_clock=$FALSE;
					}
					if ($silent==$FALSE) {
						system('clear');
						print "\n-----------------------------------------------------------\n";
						print "A$1 detected on slot:$3 bus:$4\n";
						print "-----------------------------------------------------------\n";
					}
				
select_bri_option:
					if($silent==$FALSE){
						print "\nWould you like to configure AFT-A$1 port $5 on slot:$3 bus:$4\n";
						my @options=("YES", "NO", "Exit");
						$def_bri_option=&prompt_user_list(@options,$def_bri_option);
					} else {
						$def_bri_option="YES";
					}

					
					if($def_bri_option eq 'YES'){
						$skip_card=$FALSE;
						$bri_conf.="\n;Sangoma AFT-A$1 port $5 [slot:$3 bus:$4 span:$current_tdmapi_span]";
					}elsif($def_bri_option eq 'NO'){
						$skip_card=$TRUE;
					}else{
						print "Exit without applying changes?\n";
						if (&prompt_user_list(("YES","NO","YES")) eq 'YES'){
							print "No changes made to your configuration files.\n\n";
							exit 0;
						} else {
							goto select_bri_option;
						}
					}
					if ($skip_card==$FALSE){
						$startup_string.="wanpipe$devnum ";			
						$cfg_string.="wanpipe$devnum ";			
						$a50x = eval {new A50x(); } or die ($@);
						$a50x->card($card);
						$a50x->fe_line($5);
						$devnum++;
						$num_bri_devices++;
						$card->tdmv_span_no($current_tdmapi_span);
						$current_tdmapi_span++;
					}else{
						print "A$1 on slot:$3 bus:$4 port:$5 not configured\n";
						prompt_user("Press any key to continue");
					}

				}				
			}
		}elsif (($dev =~ /(\d+):NT/ | 
	 		$dev =~ /(\d+):TE/ )& 
	 		$skip_card==$FALSE ){
	 		
	 		
	 		my $context="";
	 		my $group="";
			my $bri_pos=$a50x->card->tdmv_span_no;
			
	 		printf("\nConfiguring port %d on AFT-A%d [slot:%d bus:%d span:%d]\n", $a50x->fe_line(), $a50x->card->card_model(), $a50x->card->pci_slot(), $a50x->card->pci_bus(), $current_tdmapi_span-1);
			my $conn_type=get_bri_conn_type($a50x->fe_line());
			my $country=get_bri_country();
			my $operator=get_bri_operator();
			if($is_trixbox==$TRUE && $silent==$FALSE){
				if ( $dev =~ /(\d+):NT/ ){
					$context="from-internal";
					$group=1;
				} else {
					$context="from-zaptel";
					$group=2;
				}
			} else {
				$group=get_woomera_group();
				#if a context has already been assigned to this group, do not prompt for options
				foreach my $f_group (@woomera_groups) {
					if($f_group eq $group){
						$context="WOOMERA_NO_CONFIG";
					}
				}			
				if(!($context eq "WOOMERA_NO_CONFIG")){
					if ( $dev =~ /(\d+):NT/ ){	
						$context=get_woomera_context($group, $a50x->card->card_model(),$a50x->fe_line(),'bri_nt');
					} else {
					 	$context=get_woomera_context($group, $a50x->card->card_model(),$a50x->fe_line(),'bri_te');
					}
					push(@woomera_groups, $group);

				}
			}
			$a50x->gen_wanpipe_conf();
			if ( $dev =~ /(\d+):NT/ ){	
				$bri_conf.=$a50x->gen_bri_conf($bri_pos,"bri_nt", $group, $country, $operator, $conn_type);
			} else {
				$bri_conf.=$a50x->gen_bri_conf($bri_pos,"bri_te", $group, $country, $operator, $conn_type);
			}
			if(!($context eq "WOOMERA_NO_CONFIG")){
				$woomera_conf.=$a50x->gen_woomera_conf($group, $context);
			}

		}
	}
	if($num_bri_devices_total!=0){
		print("\nISBN BRI card configuration complete\n\n");
	} else {
		print("\nNo Sangoma ISDN BRI cards detected\n\n");
	}
	if($silent==$FALSE){
		prompt_user("Press any key to continue");
	}
}





#------------------------------T1/E1 FUNCTIONS------------------------------------#

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


sub  prompt_user_hw_dchan{
	my ($card_model, $port, $port_femedia) = @_;
	my $res_dchan='';
	my $dchan;

	printf("Hardware HDLC can be performed on the data channel.\n");
prompt_hw_dchan:
	my $res_dchan = &prompt_user("Select the data channel on A$card_model, port:$port, select 0 for unused.\n","0");
	while(length($res_dchan)==0 |!($res_dchan =~/(\d+)/)){
		print "Invalid channel, input an integer (0 for unused).\n";
		$res_dchan=&prompt_user("Select the data channel","0");
	}
	if ( $res_dchan < 0){
		printf("Error: channel cannot have negative value\n");
		$res_dchan='';
		goto prompt_hw_dchan;
	}
	if ( $port_femedia eq 'T1' && $res_dchan > 24){
		printf("Error: only 24 channels available on T1\n");
		$res_dchan='';
		goto prompt_hw_dchan;
	}elsif ($port_femedia eq 'E1' && $res_dchan > 31){
		printf("Error: only 31 channels available on E1\n");
		$res_dchan='';
		goto prompt_hw_dchan;
	}
	if ($res_dchan == 0){
		printf("HW HDLC channel not used\n");
	}else{
		printf("HW HDLC channel set to:%d\n", $res_dchan);
	}
	return $res_dchan;
}


sub get_zapata_group{
	my ($card_model,$card_port,$context)=@_;
	my $group='';
	if($silent==$TRUE){
		if($#silent_zapata_groups >= 0){
			$silent_zapata_group=pop(@silent_zapata_groups);
		}
		$silent_zapata_group =
		return $silent_zapata_group;
	}
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



sub prompt_hw_dtmf {
#HW DTMF not supported in the 3.2 drivers
#	return "NO";
	if( $no_hwdtmf == $TRUE){
		return "NO";
	}
	print("Would you like to enable hardware DTMF detection?\n");
	my @options = ("YES","NO");
	$def_hw_dtmf = prompt_user_list(@options, $def_hw_dtmf);
	return $def_hw_dtmf;
}

sub prompt_tdm_law {
	print("Which codec will be used?\n");
	my @options = ("MULAW - North America","ALAW - Europe");
	my @options_val = ("MULAW", "ALAW");
	my $res = &prompt_user_list(@options, $def_tdm_law);
	my $i;
	for $i (0..$#options){
		if ( $res eq @options[$i] ){
			$def_tdm_law=@options[$i];
			return @options_val[$i]; 
		}
	}
	print "Internal error: Invalid TDM LAW type\n";
	exit 1;
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
			}else{
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
	
Incrementing:	
		$i++;
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






sub config_t1e1{
	if (!$first_cfg && $silent==$FALSE) {
		system('clear');
	}
	print "---------------------------------------------\n";
	print "Configuring T1/E1 cards [A101/A102/A104/A108]\n";
	print "---------------------------------------------\n";
	
	foreach my $dev (@hwprobe) {
		if ( $dev =~ /A(\d+)(.*):.*SLOT=(\d+).*BUS=(\d+).*CPU=(\w+).*PORT=(\w+).*/){

		  	if ( ! ($1 eq '200' | $1 eq '400' | $1 eq '500') ){
				#do not count analog devices
				$num_digital_devices_total++;
			}
			my $card = eval {new Card(); } or die ($@);
			$card->current_dir($current_dir);
			$card->cfg_dir($cfg_dir);
			$card->device_no($devnum);
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
				if (!$first_cfg && $silent==$FALSE) {
					system('clear');
				}
				if (($6 eq '1' || $6 eq 'PRI') && $5 eq 'A'){
					print "A$1 detected on slot:$3 bus:$4\n";
					$device_has_normal_clock=$FALSE;
					@device_normal_clocks = ("0");
				}
				$first_cfg=0;
				if($silent==$FALSE){
					my $msg ="Configuring port ".$port." on A".$card->card_model." slot:".$card->pci_slot." bus:".$card->pci_bus.".\n";
					print "\n-----------------------------------------------------------\n";
					print "$msg";
					print "-----------------------------------------------------------\n";
				}
				my $fe_media = '';
				if ($silent==$TRUE){
					if($#silent_femedias >= 0){
						$silent_femedia=pop(@silent_femedias);
					}
					
					$fe_media = $silent_femedia;
				} else {
					printf ("\nSelect media type for AFT-A%s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $port, $card->pci_slot, $card->pci_bus);
					my @options = ("T1", "E1", "Unused","Exit");
					$fe_media = prompt_user_list( @options, $def_femedia);
				}
	
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
				$cfg_string.="wanpipe$devnum ";
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
					$max_chans = 24;
					$line_media = 'T1';
			
					if(!($def_felcode eq 'B8ZS' || $def_felcode eq 'AMI')){
						$def_felcode='B8ZS';
					}
					if(!($def_feframe eq 'ESF' || $def_feframe eq 'D4')){
						$def_feframe='ESF';
					}

		
					if ($silent==$FALSE){
						printf ("Configuring port %s on AFT-A%s as: %s, coding:%s, framing:%s.\n", 
											$port,
											$card->card_model, 
											$fe_media,
											$def_felcode,
											$def_feframe,
											$port);
						my @options = ("YES - Keep these settings", "NO  - Configure line coding and framing");
						my $res = &prompt_user_list(@options, "YES");
						if ($res =~ m/NO/){
							printf("Select line coding for port %s on %s\n", $port, $card->card_model);
							my @options = ("B8ZS", "AMI");
							$def_felcode= &prompt_user_list(@options, $def_felcode);
								
							
							printf("Select framing for port %s on %s\n", $port, $card->card_model);
							my @options = ("ESF", "D4");
							$def_feframe= &prompt_user_list(@options, $def_feframe);
						}


					} else {
						if($#silent_felcodes >= 0){
							$silent_felcode=pop(@silent_felcodes);
						}					
						$def_felcode=$silent_felcode;

						if(!($def_felcode eq 'B8ZS' || $def_felcode eq 'AMI')){
							$def_felcode='B8ZS';
						}
						
						if($#silent_feframes >= 0){
							$silent_feframe=pop(@silent_feframes);
						}			
						$def_feframe=$silent_feframe;

						if(!($def_feframe eq 'ESF' || $def_feframe eq 'D4')){
							$def_feframe='ESF';
						}
					}


				}else{ #fe_media = E1
					$max_chans = 31;
					$line_media = 'E1';
					
			
					if(!($def_felcode eq 'HDB3' || $def_felcode eq 'AMI')){
						$def_felcode='HDB3';
					}
					if(!($def_feframe eq 'CRC4' || $def_feframe eq 'NCRC4' || $def_feframe eq 'UNFRAMED')){
						$def_feframe='CRC4';
					}

					if ($silent==$FALSE){
						printf ("Configuring port %s on %s as %s, line coding:%s, framing:%s \n", 
											$port,
											$card->card_model, 
											$fe_media,
											$def_felcode,
											$def_feframe);
						my @options = ("YES - Keep these settings", "NO  - Configure line coding and framing");
						my $res = &prompt_user_list(@options, "YES");
						if ($res =~ m/NO/){
							printf("Select line coding for port %s on %s\n", $port, $card->card_model);
							my @options = ("HDB3", "AMI");
							$def_felcode= &prompt_user_list(@options, $def_felcode);
								
							
							printf("Select framing for port %s on %s\n", $port, $card->card_model);
							my @options = ("CRC4", "NCRC4","UNFRAMED");
							$def_feframe = &prompt_user_list(@options, $def_feframe);

#							printf("Select signalling mode for port %s on %s\n", $port, $card->card_model);
#							my @options = ("CCS - Clear channel signalling ", "CAS");
#							$def_te_sig_mode = &prompt_user_list(@options, $def_te_sig_mode);
								
						}
						if ($def_signalling eq 'TDM API'){	
							printf("Select signalling mode on AFT-A%s port %s\n",$card->card_model, $port);
							my @options=("CCS","CAS");
							$def_te_sig_mode=&prompt_user_list(@options, $def_te_sig_mode);
						} elsif ($def_signalling eq 'PRI CPE' | 
						         $def_signalling eq 'PRI NET' | 
							 $def_signalling eq 'SS7 - Sangoma Signal Media Gateway'|
							 $def_signalling eq 'No Signaling (Voice Only)'){

							$def_te_sig_mode="CCS";
						} else {
							$def_te_sig_mode="CAS";
						}


					} else {
						if($#silent_felcodes >= 0){
							$silent_felcode=pop(@silent_felcodes);
						}					
						$def_felcode=$silent_felcode;
						if(!($def_felcode eq 'HDB3' || $def_felcode eq 'AMI')){
							$def_felcode='HDB3';
						}
						
						if($#silent_feframes >= 0){
							$silent_feframe=pop(@silent_feframes);
						}			
						$def_feframe=$silent_feframe;
						if(!($def_feframe eq 'CRC4' || $def_feframe eq 'NCRC4' || $def_feframe eq 'UNFRAMED')){
							$def_feframe='CRC4';
						}
					}


				}
				$a10x->fe_lcode($def_felcode);
				$a10x->fe_frame($def_feframe);
				if($silent==$FALSE){
					my @options = ("NORMAL", "MASTER");
					printf ("Select clock for AFT-A%s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $port, $card->pci_slot, $card->pci_bus);
					$def_feclock=&prompt_user_list(@options, $def_feclock);
				} else {
					if($#silent_feclocks >= 0){
						$silent_feclock=pop(@silent_feclocks);
					}
					$def_feclock=$silent_feclock;
				}
				
				$a10x->fe_clock($def_feclock);
				if ( $def_feclock eq 'NORMAL') {
					$device_has_normal_clock=$TRUE;
					push(@device_normal_clocks, $a10x->fe_line);
				} elsif ( $def_feclock eq 'MASTER' && $device_has_normal_clock == $TRUE ){
					printf("Clock synchronization options for %s on port %s [slot:%s bus:%s span:$devnum] \n", 
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


								
				if ($silent==$FALSE){
					if ($card->hwec_mode eq "YES"){
						$card->hw_dtmf(&prompt_hw_dtmf());
					} else {
						$card->hw_dtmf("NO");
					}
				} else {
					if($#silent_hwdtmfs >= 0){
						$silent_hwdtmf=pop(@silent_hwdtmfs);
					}
					if ($card->hwec_mode eq "YES" && $no_hwdtmf==$FALSE){
						$card->hw_dtmf($silent_hwdtmf);
					} else {
						$card->hw_dtmf("NO");
					}
				} 


				
				my @options="";	
				if ($is_smg==$TRUE && $zaptel_installed==$TRUE){
					@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start", "SS7 - Sangoma Signal Media Gateway", "No Signaling (Voice Only)");
				} elsif ($is_smg==$TRUE && $zaptel_installed==$FALSE){
					@options = ("SS7 - Sangoma Signal Media Gateway", "No Signaling (Voice Only)");
				} elsif ($is_tdm_api==$TRUE){
					$def_signalling="TDM API";
				} else {			
					@options = ("PRI CPE", "PRI NET", "E & M", "E & M Wink", "FXS - Loop Start", "FXS - Ground Start", "FXS - Kewl Start", "FX0 - Loop Start", "FX0 - Ground Start", "FX0 - Kewl Start");
				}
				if ($silent==$FALSE){
					if( $is_tdm_api == $FALSE){
						printf ("Select signalling type for AFT-A%s on port %s [slot:%s bus:%s span:$devnum]\n", $card->card_model, $port, $card->pci_slot, $card->pci_bus);
						$def_signalling=&prompt_user_list(@options,$def_signalling); 
					}
				} else {
					if($#silent_signallings >= 0){
						$silent_signalling=pop(@silent_signallings);
					}
					$def_signalling=$silent_signalling;
				}
				$a10x->signalling($def_signalling);
				
				if ($def_signalling eq 'TDM API'){	
					printf("Select signalling mode for port %s on %s\n", $port, $card->card_model);
					my @options=("CCS","CAS");
					$def_te_sig_mode=&prompt_user_list(@options, $def_te_sig_mode);
				} elsif ($def_signalling eq 'PRI CPE' | 
				         $def_signalling eq 'PRI NET' | 
					 $def_signalling eq 'SS7 - Sangoma Signal Media Gateway'|
					 $def_signalling eq 'No Signaling (Voice Only)'){

					$def_te_sig_mode="CCS";

				} else {
					$def_te_sig_mode="CAS";
				}
				$a10x->te_sig_mode($def_te_sig_mode);

				my $ss7_chan;
				my $ss7_group_start;
				my $ss7_group_end;
				my @ss7_chan_array;
				my @ss7_sorted;

				if( $a10x->signalling eq 'SS7 - Sangoma Signal Media Gateway' ){
					$a10x->ss7_option(1); 
					print("Choose an option below to configure SS7 signalling channels:\n");
					my @options =("Configure individual signalling channels(e.g #1,#10)", 
						      "Configure consecutive signalling channels(e.g   #1-#16)");
					my $res = &prompt_user_list(@options, "");
					if ( $res eq 'Configure individual signalling channels(e.g #1,#10)'){
						goto SS7CHAN;
						while (1){
							print("\nAny other SS7 signalling channel to configure?\n");
							if (&prompt_user_list("YES","NO","") eq 'NO'){
								goto ENDSS7CONFIG;
							}else{
SS7CHAN:
								$ss7_chan = prompt_user_ss7_chans('Specify the channel for SS7 signalling(24 for T1? 16 for E1?)');
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
SS7GROUP:					
								$ss7_group_start = prompt_user_ss7_chans('Consecutive signalling channels START FROM channel number');
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
								
									if ($ss7_array_length > $max_chans){	
										print("\nERROR : You defined more than $max_chans signalling channels in $line_media and please try to configure them again.\n\n");

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

ENDSS7CONFIG:
					
					@ss7_sorted = sort { $a <=> $b } @ss7_chan_array;

					print("\nYou configured the following SS7 signalling channels: @ss7_sorted\n");
					my $ss7_voicechans = gen_ss7_voicechans(@ss7_sorted,$max_chans);
					$ss7_tdmvoicechans = $ss7_voicechans;

					if ($ss7_voicechans =~ m/(\d+)/){
						$a10x->ss7_tdminterface($1);
					}
	
					$a10x->ss7_tdmchan($ss7_voicechans);

					$num_ss7_config++;
					$card->tdmv_span_no($current_tdmapi_span);

					#wanrouter start/stop for signalling span is controlled by ss7boxd
					#$startup_string.="wanpipe$devnum "; 
					$current_tdmapi_span++;
				}elsif ( $a10x->signalling eq 'No Signaling (Voice Only)'){
					$a10x->ss7_option(2);
					$num_ss7_config++;   
					$card->tdmv_span_no($current_tdmapi_span);
					$startup_string.="wanpipe$devnum ";
					$current_tdmapi_span++;

				}elsif ($a10x->signalling eq 'TDM API'){
					if ($a10x->te_sig_mode eq "CAS"){
						$a10x->hw_dchan('0');
					} else {
						$a10x->hw_dchan(&prompt_user_hw_dchan($card->card_model, $port, $a10x->fe_media));
					}
					$card->tdmv_span_no($current_tdmapi_span);
					$startup_string.="wanpipe$devnum ";
					$current_tdmapi_span++;
				}else{
					$num_zaptel_config++;
					$card->tdmv_span_no($current_zap_span);
					$startup_string.="wanpipe$devnum ";
					$current_zap_span++;
					$current_zap_channel+=$max_chans;
				}


	
				if ( $a10x->signalling eq 'PRI CPE' | $a10x->signalling eq 'PRI NET' ){    
					if ($silent==$FALSE && $config_zapata==$TRUE){
						printf ("Select switchtype for AFT-A%s on port %s \n", $card->card_model, $port);
						$a10x->pri_switchtype(get_pri_switchtype());	
					} else {
						if($#silent_pri_switchtypes >= 0){
							$silent_pri_switchtype=pop(@silent_pri_switchtypes);
						}			
						$def_feframe=$silent_feframe;
						$a10x->pri_switchtype($silent_pri_switchtype);
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
					if (!($a10x->signalling eq 'SS7 - Sangoma Signal Media Gateway'
						| $a10x->signalling eq 'No Signaling (Voice Only)')){
						$zaptel_conf.=$a10x->gen_zaptel_conf($dchan_str);
					}
				}elsif ($is_smg==$TRUE && $config_zapata==$TRUE){
					if (!($a10x->signalling eq 'SS7 - Sangoma Signal Media Gateway'| $a10x->signalling eq 'No Signaling (Voice Only)')){
					
						$zaptel_conf.=$a10x->gen_zaptel_conf($dchan_str);
						$a10x->card->zap_context(&get_zapata_context($a10x->card->card_model,$port));
						$a10x->card->zap_group(&get_zapata_group($a10x->card->card_model,$port,$a10x->card->zap_context));
						$zapata_conf.=$a10x->gen_zapata_conf();
					}
				}elsif ($is_trixbox==$TRUE | $config_zapata==$TRUE){
					if($silent==$FALSE){
						printf ("Configuring port %s on %s as a full %s\n", $a10x->fe_line(), $a10x->card->card_model(),$a10x->fe_media());
						my $res=&prompt_user_list("YES - Use all channels", "NO  - Configure for fractional","YES");
						if ($res =~ m/NO/){
							my $max_chan=0;
							if($a10x->fe_media eq 'T1'){
								$max_chan=24;
							} else {
								$max_chan=31;
							}
							my $first_chan = &get_chan_no("first",1,$max_chan-1);
							my $last_chan = &get_chan_no("last",$first_chan+1,$max_chan);
							
							$a10x->frac_chan_first($first_chan);
							$a10x->frac_chan_last($last_chan);
						}
					} else {
						if($#silent_first_chans >= 0){
							$silent_first_chan = pop(@silent_first_chans);
							$silent_last_chan = pop(@silent_last_chans);
						}
						
						if($silent_first_chan != 0){
							$a10x->frac_chan_first($silent_first_chan);
							$a10x->frac_chan_last($silent_last_chan);
						}	
					}

					$zaptel_conf.=$a10x->gen_zaptel_conf($dchan_str);
					$a10x->card->zap_context(&get_zapata_context($a10x->card->card_model,$port));
					$a10x->card->zap_group(&get_zapata_group($a10x->card->card_model,$port,$a10x->card->zap_context));
					$zapata_conf.=$a10x->gen_zapata_conf();
				}elsif ($is_tdm_api == $FALSE){
					$zaptel_conf.=$a10x->gen_zaptel_conf($dchan_str);
				}
					$devnum++;
					$num_digital_devices++;
					my $msg ="\nPort ".$port." on AFT-A".$card->card_model." configuration complete...\n";
					print "$msg";
					if($silent==$FALSE){
						prompt_user("Press any key to continue");
					}

				}  	
	

			} 

		}
	}
	if($num_digital_devices_total!=0){
		print("\nT1/E1 card configuration complete.\n");
		if($silent==$FALSE){
			prompt_user("Press any key to continue");
		}
		$first_cfg=0;
	}
#	close SCR;
}


#------------------------------ANALOG FUNCTIONS------------------------------------#


sub config_analog{
	if($is_tdm_api==$TRUE){
		return;
	}
	my $a20x;
	if (!$first_cfg && $silent==$FALSE) {
		system('clear');
	}
	$first_cfg=0;
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
			$card->device_no($devnum);
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
				if($silent==$FALSE) {
					system('clear');
					print "\n-----------------------------------------------------------\n";
					print "A$1 detected on slot:$3 bus:$4\n";
					print "-----------------------------------------------------------\n";
				}
				if($is_trixbox==$FALSE){
					if ($silent==$FALSE){
						print "\nWould you like to configure AFT-A$1 on slot:$3 bus:$4\n";
						if (&prompt_user_list(("YES","NO","")) eq 'NO'){
							$skip_card=$TRUE;	
						}
					}
				}
				if ($skip_card==$FALSE){
					
					$a20x = eval {new A20x(); } or die ($@);
					$a20x->card($card);
					$card->first_chan($current_zap_channel);

					if ( $device_has_hwec==$TRUE && $silent==$FALSE){
						print "Will this AFT-A$1 synchronize with an existing Sangoma T1/E1 card?\n";
						print "\n WARNING: for hardware and firmware requirements, check:\n";
						print "          http://wiki.sangoma.com/t1e1analogfaxing\n";
						
						if (&prompt_user_list(("NO","YES","")) eq 'NO'){
							$a20x->rm_network_sync('NO');
						} else {
							$a20x->rm_network_sync('YES');
						}
					} 

					if ($silent==$FALSE){
						if ($card->hwec_mode eq "YES"){
							$card->hw_dtmf(&prompt_hw_dtmf());
						} else {
							$card->hw_dtmf("NO");
						}
						$a20x->tdm_law(&prompt_tdm_law());
					} else {
						if($#silent_hwdtmfs >= 0){
							$silent_hwdtmf=pop(@silent_hwdtmfs);
						}
				
						if ($card->hwec_mode eq "YES" && $no_hwdtmf==$FALSE){
							$card->hw_dtmf($silent_hwdtmf);
						} else {
							$card->hw_dtmf("NO");
						}
						
						if($#silent_tdm_laws >= 0){
							$silent_tdm_law=pop(@silent_tdm_laws);
						}						
						
						$a20x->tdm_law($silent_tdm_law);
					} 

					
					print "A$1 configured on slot:$3 bus:$4 span:$current_zap_span\n";
					$zaptel_conf.="#Sangoma A$1 [slot:$3 bus:$4 span:$current_zap_span] <wanpipe".$a20x->card->device_no.">\n";
					$zapata_conf.=";Sangoma A$1 [slot:$3 bus:$4 span:$current_zap_span]  <wanpipe".$a20x->card->device_no.">\n";
					$startup_string.="wanpipe$devnum ";
					$cfg_string.="wanpipe$devnum ";
					
					if($silent==$FALSE){
						prompt_user("Press any key to continue");
					}
			
					$current_zap_channel+=24;
					my $i;
					$devnum++;
					$num_analog_devices++;
					$num_zaptel_config++;
					$card->tdmv_span_no($current_zap_span);
					$current_zap_span++;
					$a20x->gen_wanpipe_conf();

				}else{
					print "A$1 on slot:$3 bus:$4 not configured\n";
					prompt_user("Press any key to continue");
				}
			}
		}elsif ( $dev =~ /(\d+):FXS/ & $skip_card==$FALSE){
			my $zap_pos = $1+$current_zap_channel-25;
			if($silent==$TRUE){
				if($#silent_zapata_contexts >= 0){
					$silent_zapata_context=pop(@silent_zapata_contexts);
				}
				$a20x->card->zap_context($silent_zapata_context);
			} else {
				$a20x->card->zap_context("from-internal");
			}
			$a20x->card->zap_group("1");
			$zaptel_conf.=$a20x->gen_zaptel_conf($zap_pos,"fxo");
			$zapata_conf.=$a20x->gen_zapata_conf($zap_pos,"fxo");
		}elsif ( $dev =~ /(\d+):FXO/ & $skip_card==$FALSE){
			my $zap_pos = $1+$current_zap_channel-25;
			if($silent==$TRUE){
				if($#silent_zapata_contexts >= 0){
					$silent_zapata_context=pop(@silent_zapata_contexts);
				}
				$a20x->card->zap_context($silent_zapata_context);
			} else {
				$a20x->card->zap_context("from-zaptel");
			}
			$a20x->card->zap_group("0");
			$zaptel_conf.=$a20x->gen_zaptel_conf($zap_pos,"fxs");
			$zapata_conf.=$a20x->gen_zapata_conf($zap_pos,"fxs");
		}
	}
	if($num_analog_devices_total!=0){
		print("\nAnalog card configuration complete\n\n");
		if( $silent==$FALSE){
			prompt_user("Press any key to continue");
		}
	}

}

