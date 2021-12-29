%define KERNEL_VERSION    %{?kern_ver}
%define WANPIPE_VER	  wanpipe
%define name              %{WANPIPE_VER}
%define version           3.5.12
%define release           0
%define	serial	 	  1
%define UTILS_DIR 	  /usr/sbin
%define ETC_DIR		  /etc
%define USR_DIR		  /usr
%define PROD_HOME  	  /etc/wanpipe
%define WANCFG_LIBS_DIR   /etc/wanpipe/lib
%define API_DIR           /etc/wanpipe/api
%define DOCS_DIR	  /usr/share/doc/wanpipe
%define USR_INCLUDE_DIR	  /usr/include
%define LIBSANGOMA_CONF   /etc/ld.so.conf.d/libsangoma.so.conf
%define PROD		  wanrouter
%define MODULES_DIR	  /lib/modules
%define META_CONF         %{PROD_HOME}/%{PROD}.rc
%define WAN_INTR_DIR      %{PROD_HOME}/interfaces
%define WAN_CONF_DIR      %{PROD_HOME}
%define PROD_CONF         %{WAN_CONF_DIR}/wanpipe1.conf
%define START_SCRIPT      S07%{PROD}
%define OLD_START         S07router
%define STOP_SCRIPT       K90%{PROD}
%define OLD_STOP          K900router
%define ROUTER_RC         %{META_CONF}
%define WANROUTER_STARTUP_SMPL    %{PROD_HOME}/samples/wanrouter
%define WANROUTER_STARTUP         /usr/sbin/wanrouter
%define NEW_IF_TYPE               NO
%define PROD_INIT                 /usr/sbin/

%define KVERSION          %{?kern_ver}
 

Summary: 	Sangoma WANPIPE package for Linux. It contains the WANPIPE kernel drivers and configuration/startup/debugging utilities for Linux.
Name: 		%{name}-%{?kern_ver}
Version: 	%{version}
Release:        %{release}
License: 	GPL
Group: 		Applications/Communications
Vendor:		Sangoma Technologies Inc.
Url:		www.sangoma.com
Group:		Networking/WAN
 

%description 
Linux Drivers for Sangoma AFT Series of cards and S Series of Cards. Wanpipe supports the following protocols, TDM Voice, Frame Relay, X25(API), PPP, Multi-link PPP, CHDLC and custom API development for WAN and Voice.

%prep

%build

%install

%clean

%postun

echo "Uninstalling WANPIPE..."

# ----------------------------------------------------------------------------
# Remove initialization scripts.
# ----------------------------------------------------------------------------
remove_init()
{
	chkconfig --del wanrouter
	rm /etc/init.d/wanrouter
}
remove_init_old()
{
        # Examine system bootstrap files.
        if [ -d /etc/rc0.d ]
        then RC_DIR=/etc
        elif [ -d /etc/rc.d/rc0.d ]
        then RC_DIR=/etc/rc.d
        else return 0
        fi
 
	echo "Removing start-up scripts..."
        rm -f $RC_DIR/rc2.d/%{START_SCRIPT}
	rm -f $RC_DIR/rc3.d/%{START_SCRIPT}
        rm -f $RC_DIR/rc4.d/%{START_SCRIPT}
        rm -f $RC_DIR/rc5.d/%{START_SCRIPT}

        rm -f $RC_DIR/rc0.d/%{STOP_SCRIPT}
        rm -f $RC_DIR/rc1.d/%{STOP_SCRIPT}
        rm -f $RC_DIR/rc6.d/%{STOP_SCRIPT}

        rm -f $RC_DIR/init.d/%{PROD}
        return 0
}

#remove start-on-boot scripts
remove_init;

%post

# ----------------------------------------------------------------------------
# Create meta-configuration file.
# ----------------------------------------------------------------------------
create_metaconf()
{
        local response
 
        # Select directory for the log file.
        if      [ -d /var/log ]; then
                LOG_FILE=/var/log/%{PROD}
        elif    [ -d /var/adm wanpipe1]; then
                LOG_FILE=/var/adm/%{PROD}
        else
                LOG_FILE=%{PROD_HOME}/%{PROD}.log
        fi
 
	# Select directory for the lock file.
        if      [ -d /var/lock/subsys ]; then
                LOCK_FILE=/var/lock/subsys/%{PROD}
        elif    [ -d /var/lock ]; then
                LOCK_FILE=/var/lock/%{PROD}
        else
                LOCK_FILE=$PROD_HOME/%{PROD}.lck
        fi

	
        cat > %{META_CONF} << ENDOFTEXT
#!/bin/sh
# wanrouter.rc	WAN router meta-configuration file.
#
#		This file defines variables used by the router shell scripts
#		and should be located in /etc/wanpipe directory.  These are:
#
#               ROUTER_BOOT	=       Boot flag (YES/NO).
#               WAN_CONF_DIR	=       Where to put wanpipe config files.
#		WAN_INTR_DIR	=	Where to put wanpipe interface files.
#               WAN_LOG		=       Where to put start-up log file.
#               WAN_LOCK	=       File used as a lock.
#		WAN_LOCK_DIR	=
#		WAN_IP_FORWARD	=	Enable IP Forwarding on startup.
#               WAN_DEVICES	=       Name of the wanpipe devices to be
#                                       loaded on 'wanrouter start'
#                                       (ex: "wanpipe1 wanpipe2 wanpipe3...")
#
#                              Note:    Name of wanpipe devices correspond
#                                       to the configuration files in
#                                       WANPIPE_CONF_DIR directory:
#                                         (ex. /etc/wanpipe/wanpipe1.conf )
#
#               Note:   This file is 'executed' by the shell script, so
#                       the usual shell syntax must be observed. 
ENDOFTEXT
 	echo "ROUTER_BOOT=YES"          >> %{META_CONF}
        echo "WAN_CONF_DIR=%{WAN_CONF_DIR}" >> %{META_CONF}
        echo "WAN_INTR_DIR=%{WAN_INTR_DIR}" >> %{META_CONF}
        echo "WAN_LOG=$LOG_FILE"     >> %{META_CONF}
        echo "WAN_LOCK=$LOCK_FILE"   >> %{META_CONF}
	echo "WAN_LOCK_DIR=/var/lock/subsys"    >> %{META_CONF}
	echo "WAN_IP_FORWARD=NO" >> %{META_CONF}           
	echo "NEW_IF_TYPE=NO"    >> %{META_CONF}
	echo "WAN_LIB_DIR=/etc/wanpipe/lib"    >> %{META_CONF}
	echo "WAN_ADSL_LIST=/etc/wanpipe/wan_adsl.list"    >> %{META_CONF}
	echo "WAN_ANNEXG_LOAD=NO"    >> %{META_CONF}
	echo "WAN_LIP_LOAD=YES"    >> %{META_CONF}
	echo "WAN_DYN_WANCONFIG=NO"    >> %{META_CONF}
	echo "WAN_SCRIPTS_DIR=/etc/wanpipe/scripts"    >> %{META_CONF}
	echo "WAN_FIRMWARE_DIR=/etc/wanpipe/firmware"    >> %{META_CONF}
	echo "WAN_DEVICES_REV_STOP_ORDER=YES"    >> %{META_CONF}
	echo "WAN_DEVICES=\"wanpipe1\""    >> %{META_CONF}

 
        return 0
}


# ----------------------------------------------------------------------------
# Install initialization scripts.
# ----------------------------------------------------------------------------
install_init()
{
	ln -s /usr/sbin/wanrouter /etc/init.d/wanrouter
	chkconfig wanrouter on
}
install_init_old()
{
	#Examine system bootstrap files.
        if [ -d /etc/rc0.d ]
        then RC_DIR=/etc
        elif [ -d /etc/rc.d/rc0.d ]
        then RC_DIR=/etc/rc.d
        else return 0
        fi
 
	PROD_INIT=%{PROD_INIT}%{PROD}

	# Install start scripts.
        [ -d $RC_DIR/rc2.d ] && ln -sf $PROD_INIT $RC_DIR/rc2.d/%{START_SCRIPT}
        [ -d $RC_DIR/rc3.d ] && ln -sf $PROD_INIT $RC_DIR/rc3.d/%{START_SCRIPT}
        [ -d $RC_DIR/rc5.d ] && ln -sf $PROD_INIT $RC_DIR/rc4.d/%{START_SCRIPT}
        [ -d $RC_DIR/rc5.d ] && ln -sf $PROD_INIT $RC_DIR/rc5.d/%{START_SCRIPT}
 
        # Install stop scripts.
        [ -d $RC_DIR/rc0.d ] && ln -sf $PROD_INIT $RC_DIR/rc0.d/%{STOP_SCRIPT}
        [ -d $RC_DIR/rc1.d ] && ln -sf $PROD_INIT $RC_DIR/rc1.d/%{STOP_SCRIPT}
        [ -d $RC_DIR/rc6.d ] && ln -sf $PROD_INIT $RC_DIR/rc6.d/%{STOP_SCRIPT}
        [ -d $RC_DIR/init.d ] && ln -sf $PROD_INIT $RC_DIR/init.d/%{PROD}
 
        return 0
}

if [ -d "/usr/local/wanrouter" ]; then
	cat <<EOM
*** Previous installation of Wanpipe detected.
    Please use /usr/sbin/wancfg instead of /usr/local/wanrouter/wancfg
    for Wanpipe configuration.
    The new configuration files will be saved in /etc/wanpipe
    and /etc/wanpipe/interfaces directories.

EOM
else
		echo 'no old wanpipe detected' > /dev/null

fi

cat <<EOM
*** Sangoma Wanpipe was successfully installed.
    Run wancfg command to configure wanpipe.
    Refer to %{DOCS_DIR} for documentation.

    Docs: README-2.config
          README-3.operation
	  README-4.debugging

    Hardware Probe: /usr/sbin/wanrouter hwprobe
    Wanpipe Config: /usr/sbin/wancfg
    Wanpipe Start : /usr/sbin/wanrouter start

EOM
#check dependancies for the new modules

depmod -ae -F /boot/System.map-%{KVERSION} %{KVERSION}
echo "Wanpipe Modules located in %{MODULES_DIR}/%{KVERSION}"   

#install start-on-boot scripts
install_init;
#create wanrouter.rc in /etc/wanpipe
#create_metaconf;

%files
%{UTILS_DIR}
%{ETC_DIR}
%{USR_DIR}
%{PROD_HOME}
%{DOCS_DIR}
%{MODULES_DIR}
%{USR_INCLUDE_DIR}


%changelog

* Mon Jun 28 2010 Nenad Corbic <ncorbic@sangoma.com> -  3.5.12
===================================================================

- Fixed Dahdi 2.3 Support
- Fixed FreeSwitch Openzap HardHDLC option for AFT cards
- Fixed wanpipemon support for non aft cards.
- Merged USB FXO code from 3.6 release
- USB FXO bug fix for 2.6.32 kernels
- Support for B800 Analog card
- Fixed alarm reporting in DAHDI/ZAPTEL

- Added Extra EC DSP Configuration Options
  HWEC_OPERATION_MODE	= OCT_NORMAL 	# OCT_NORMAL: echo cancelation enabled with nlp (default)  
										# OCT_SPEECH: improves software tone detection by disabling NLP (echo possible)
										# OCT_NO_ECHO:disables echo cancelation but allows VQE/tone functions. 
  HWEC_DTMF_REMOVAL		= NO          	# NO: default  YES: remove dtmf out of incoming media (must have hwdtmf enabled)
  HWEC_NOISE_REDUCTION	= NO 			# NO: default  YES: reduces noise on the line - could break fax
  HWEC_ACUSTIC_ECHO		= NO			# NO: default  YES: enables acustic echo cancelation
  HWEC_NLP_DISABLE		= NO			# NO: default  YES: guarantees software tone detection (possible echo)   
  HWEC_TX_AUTO_GAIN		= 0    			# 0: disable   -40-0: default tx audio level to be maintained (-20 default)
  HWEC_RX_AUTO_GAIN		= 0				# 0: disable   -40-0: default rx audio level to be maintained (-20 default)
  HWEC_TX_GAIN			= 0				# 0: disable   -24-24: db values to be applied to tx signal
  HWEC_RX_GAIN			= 0				# 0: disable   -24-24: db values to be applied to tx signal  

- Added AIS BLUE Alarm Maintenance Startup option
  Allows a port to be started in BLUE alarm.

  TE_AIS_MAINTENANCE = NO         	#NO: defualt  YES: Start port in AIS Blue Alarm and keep line down
									#wanpipemon -i w1g1 -c Ttx_ais_off to disable AIS maintenance mode
									#wanpipemon -i w1g1 -c Ttx_ais_on to enable AIS maintenance mode           
  
- Fixed Legacy XDLC compile	
- Fixed core edge case scenarios where
  potential race condition could occour.



* Thu Apr 08 2010 Nenad Corbic <ncorbic@sangoma.com> -  3.5.11
===================================================================

- Fix for 2.6.31 and higher kernels
- TDM API Analog rx gain feature
- Disabled default NOISE REDUCTION feature in hwec
  that was enabled in 3.5.9 release. 
- Updates to T1/E1 Loopback and BERT Test	


* Wed Jan 11 2010 Nenad Corbic <ncorbic@sangoma.com> -  3.5.10
===================================================================

- Release cleanup script earsed libsangoma.c during release packaging.
  I have update release procedure so this does not happen again.
  This release has no functionl differences aside from the missing
  file from 3.5.9 release.

* Wed Dec 30 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.9
===================================================================

- New logger dev feature
- Bug fix in tx fifo handler
- Dahdi 2.2 broke wanpipe rbs support.
- Fixed free run interrupt supported on V38 (A108)
- Fixed RBS signalling for E1 channel 31
- Added Front end Reset Detection
  -> Support for new A108 Firmware V40
- Fixed RTP TAP bug: Caused high system load on RTP TAP usage.
- Added excessive fifo error sanity check. Fixes random pci dma errors.
- HWEC: Increased EC VQE Delay: Fixes random fax failure due to hwec.
- HWEC: Check state before bypass enable.
- HWEC: Disable bypass on release
- HWEC: Enabled Noise Reduction by default
- HWEC: Enabled Auto Gain Control by default
- HWEC: To disable Noise Reduction and Gain control set
        -> HWEC_NOISE_REDUCTION_DISABLE=NO in [wanpipe] section of wanpipe1.conf
		To check if Noise Reduction or Gain control are set
		-> wan_ec_client wanpipe1 stats 1


* Thu Oct 02 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.8
===================================================================

- Bug fix in sangoma_prid PRI stack for FreeSwitch & Asterisk.
  There was a slow memory leak.


* Thu Sep 04 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.7
===================================================================

- New Telesoft PRI Stack Support for FreeSwitch & Asterisk
  For Asterisk: The new stack uses the existing Sangoma Media Gateway
                architecture currently used by SS7 and BRI.
                -> run: wancfg_dahdi or wancfg_zaptel to configure
				   for sangoma prid stack.

  For FreeSwitch: The new stack binds to openzap directly just like
                  current SS7 and BRI.
				-> run: wancfg_fs to configure freeswitch for
				        sangoma prid, brid, ss7.

- Fixed Tx Tristate 

- Updated yellow alarm handling for Dallas maxim cards
  (A101/2/4/8)

- Autodetect USB support so that driver will compile
  correctly on kernel without USB support

- Added DAHDI Red alarm for Analog


* Thu Aug 20 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.6
===================================================================

- Update to T1 Yellow Alarm handling.
  In some cases Yellow alarm did not turn off poperly causing
  line to stay down an card startup.
- Update configuration utility
  wancfg_fs updated for sangoma_prid configuration. Added wancfg_openzap
  for OpenZap Configuration 


* Mon Aug 17 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.5
===================================================================

- Dahdi 2.2 Support
- BRI Update - Added T1 timer for NT module
- AFT Core Update - optimized dma ring buffer usage
- TDM API - refractoring and optimization
- Updated for 2.6.30 kernel

- New firmawre feature for A101/2/5/8: Free Run Timer Interrupt 
  The AFT T1/E1 cards will now provide perfect timing to zatpel/dahdi
  even when the ports are not connected. The free run interrupt
  will be enabled when all zaptel/dahdi ports are down, or on
  inital card start. To test this feature just start a wanpipe 
  port with zaptel/dahdi and run zttest. 
  A108 firmare V38 
  A104/2/1/ firmware V36

- AFT T1/E1 front end update
  Added OOF alarm treshold, so that line does not go down
  on very first OOF alarm.

- Added module inc cound when zaptel/dahdi starts.
  So wanpipe drivers do not crash if one tries to unload 
  zaptel/dahdi before stopping wanpipe drivers.


* Thu Jul 07 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4.8
===================================================================

- Updated for B700 Dchan Critical Timeout
- Fix for FAX detect on PRI
- Updated for 2.6.21 kernel TASK QUEUE REMOVAL caused 
  unexpected behaviour.
- Updated wancfg_zaptel for fax detect

* Thu Jul 03 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4.3
===================================================================

- Added DAHDI 2.2 Support


* Thu Jul 02 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4.2
===================================================================

- AFT 64bit update
  No need for --64bit_4G flag any more. 
  The 64bit check is now down in the driver.

- TDM API
  Updated the Global TDM Device
  This device can be used to read events an all cards configured in
  TDM API mode.

- Libsangoma verion 3.1.0
  Added a function to check if hwec is supported


* Tue Jun 30 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4.1
===================================================================

- Sangoma MGD update v.1.48
  Disable hwec on data calls


* Mon Jun 29 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4
===================================================================

- E1 Voice Bug fix introduced in 3.5.3 

- Removed NOISE REDUCTION enabled by default.
  The noise reduction is disabled by default and should be
  enabled using HWEC_NOISE_REDUCTION = YES 
 
- Fixed libsangoma enable dtmf events functionality



* Tue Jun 25 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.3
===================================================================

- New Makefile build system
  Note this does not replace Setup. Makefile build system can be
  used by power users.
  Asterisk
     make dahdi DAHDI_DIR=<abs path to dahdi>
	 make install
     make zaptel ZAPDIR=<abs path to zaptel>
	 make install

  FreeSwitch
     make openzap
	 make install

  TDM API 
     make all_src
	 make install

- Updated libsangoma API
  Redesigned wait object for Linux/Windows integration.

- Turned on HWEC Noise Reduction by default
  To disable noise reduction specify
  HWEC_NOISE_REDUCTION_DISABLE=YES in [wanpipe1] section of wanpipe
  config file.

- Regression tested for FreeSwitch+OpenZAP

- Updated dma buffers in ZAPTEL and TDM API mode.
- Bug fixes for Mixed Data + Voice Mode

- Bug fix on TDM API mode. 
  Flush buffers could interfere with tx/rx data.

- Added BRI DCHAN monitor in case task is not scheduled by the
  system.  Sanity check.
- Fixed libsangoma stack overflow check that failed on some kernels.


* Fri May 08 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.2
===================================================================

- B700 PCIe cards were being displayed as PCI cards in hwprobe
- Bug fix in wancfg_zaptel 

* Thu May 07 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.1
===================================================================

- New Hardware Support
  B700 - Mixed BRI & Analog
  B600 - Analog 4FXO/FXS
  USB-FXO - USB Fxo device

- New Unified API for Linux & Windows 
  API Library - libsangoma
  Unified Voice API for Linux & Windows
  
  -More Info
  http://wiki.sangoma.com/wanpipe-api

  - SPAN mode API
  - CHAN mode API

- Unified driver for Linux & Windows
- Updated BRI Stack and Support
- New BRI A500 & B700 firmware that fixes PCI parity errors.
  On some systems A500 & B700 cards can generate parity errors.

- FreeSwitch Tested
- Update for 2.6.26 kernel

Note this is a major release. It has been fully regression
tested and stress tested in the lab and in the field.


- - END - 
