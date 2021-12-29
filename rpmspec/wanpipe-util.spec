%define KERNEL_VERSION    %{?kern_ver}
%define WANPIPE_VER	  wanpipe-util
%define name              %{WANPIPE_VER}
%define version           2.3.4
%define release           13
%define	serial	 	  1
%define UTILS_DIR 	  /usr/sbin
%define UTILS_LOCAL_DIR   /usr/local/sbin
%define PROD_HOME  	  /etc/wanpipe
%define WANCFG_LIBS_DIR   /etc/wanpipe/wancfg/lib
%define API_DIR           /etc/wanpipe/api
%define DOCS_DIR	  /usr/share/doc/wanpipe
%define KERNEL_VERSION	  /
%define PROD		  wanrouter
%define META_CONF         %{PROD_HOME}/%{PROD}.rc
%define WAN_INTR_DIR      %{PROD_HOME}/interfaces
%define WAN_CONF_DIR      %{PROD_HOME}
%define PROD_CONF         %{WAN_CONF_DIR}/wanpipe1.conf
%define LIBSANGOMA_CONF   /etc/ld.so.conf.d/libsangoma.so.conf
#%define START_SCRIPT      S07%{PROD}
#%define OLD_START         S07router
#%define STOP_SCRIPT       K10%{PROD}
#%define OLD_STOP          K10router
%define ROUTER_RC         %{META_CONF}
%define WANROUTER_STARTUP_SMPL    %{PROD_HOME}/samples/wanrouter
%define WANROUTER_STARTUP         /usr/sbin/wanrouter
%define NEW_IF_TYPE               NO
%define PROD_INIT                 /usr/sbin/
 

Summary: 	Sangoma WANPIPE package for Linux. It contains WANPIPE configuration/startup/debugging utilities for Linux. This package requires the wanpipe-mod package!
Name: 		%{name}
Version: 	%{version}
Release: 	%{release}
License: 	GPL
Group: 		Applications/Communications
#Source0:	%{WANPIPE_VER}.tgz
#Source1:	bridge-utils-0.9.1.tar.gz
Vendor:		Sangoma Technologies Inc.
Url:		www.sangoma.com
Group:		Networking/WAN
 

%description 
Linux Utilities for Sangoma AFT-Series of cards and S-Series of Cards. Wanpipe supports the following protocols, TDM Voice, Frame Relay, X25(API), PPP,Multi-link PPP, CHDLC and custom API development for WAN and Voice.

Install Wanpipe-modules package for wanpipe drivers.


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

EOM

#install start-on-boot scripts
install_init;
#create wanrouter.rc in /etc/wanpipe
#create_metaconf;



%files
%{UTILS_DIR}
%{UTILS_LOCAL_DIR}
%{PROD_HOME}
%{DOCS_DIR}


%changelog

* Mon Jun 30 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-13
==================================================================== 

- Update to Ocatsic Hardware Echo Canceler Library
  Turned of the NOISE suppression because it can interfere
  with faxes. If you faxes did not work properly on 2.3.4-12
  release they will work fine with this one.

- Cleaned up the Setup installation script.


* Mon Jun 16 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-12
==================================================================== 

- Update to Octasic Hardware Echo Canceler library
  This is a very important update that affects all AFT cards
  with octasic hardware echo canceler.  The new octasic update
  fixes faxing/modem issues over octasic hwec.  The previous
  release contained a bug that limited the faxing/modem speeds
  to 26k.  The new update properly detects fax/modem and works
  with full speed of 33k fax and 56k modem.

- A200/A400 Updated
  This update fixes the offhook startup failure.
  On startup if fxs is offhook driver will start correctly

- Wanpipe Startup order changed
  The wanpipe startup scripts on bootup were previously
  set too early "S03wanrouter".  This caused unpredictable
  behaviour on some systems.  We have now moved wanrouter 
  startup on boot up to "S11wanrouter", after networking
  code.

- Zaptel Adjustable Chunk Size Feature
  Wanpipe drivers can work with 1,2,5 and 10ms 
  chunk size.  Zaptel also supports this, however
  the wct4xx driver breaks compilation when chunk
  size is changed.  ./Setup can how change the
  zaptel chunk size for you and update zaptel
  Makefiles to remove wct4xx driver out.

  Zaptel with 1ms generates 1000 interrupts per sec
  Zaptel with 10ms generates 100 interrupts per sec.

  As you can see its a drastic interrupt performance
  increase.

  NOTE: This breaks software echo cancelation, but
        its not needed since we have hwec.


* Thu Jun 14 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-11
=============================================================== 

- A101/2/4/8 (MAXIM) AFT Update IMPORTANT
  A major bug fix for AFT Maxim E1 cards for E1 CRC4 Mode.
  On some lines the E1/CRC4 mode causes line errors on 
  the telco side which results in PRI not coming up.
 
  Symptiom: E1 is up (no alarms) on local side but pri is 
 	    not coming up!  (Only in E1 CRC4 Mode)

- A101/2/4/8 (MAXIM) Mandatory Firmware Update
  An echo canceler bug has been fixed for all AFT
  MAXIM Cards A101/2/4/8dm.  New firmware version is V31.
  If you are running MAXIM cards with hwec wiht older
  firmware version you must upgrade.

- Updated SMG 
  Fixed DTMF synchronization


* Thu Jun 13 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-10
=============================================================== 

- Support for AFT 54K DDS hardware

- Support for New A301 T3/E3 Card

- Updated Maxim (A101/2/4/8) Front end
  On port shutdown properly reset the port.
  This will solve instances where on port
  shutdown, the remote end stays up.

- Updated Setup for patching Zaptel 1.2.17

- Analog Network Sync Feature
  Synchronize Analog card to a clock of a
  Digital T1/E1 port using external cable.
  Improves Faxing performance! 
  For more info: 
     http://wiki.sangoma.com/t1e1analogfaxing	


* Wed May 17 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-9
====================================================================  

- Updated Zaptel 1.2.17 DCHAN Patch

- Hardware A101D and A101DX Support

- Added Maxim register debug in wanpipemon

- Update to SMG   


* Wed Apr 23 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-8
====================================================================  

- Important: AFT TDMV (ZAPTEL) Fix
  A race condition existed on wanrouter startup.
  If echo canceler startup delayed the startup long enough,
  for T1/E1 line to come up, it was possible that zaptel
  alarms would not be cleared.

- Important: AFT T1/E1 Front End update.
  The AFT cards will not disable communications for minor
  T1/E1 alarms.  This has caused problems in the passed because
  AFT cards were too strict in interpreting T1/E1 alarms.
  

- Added AFT/56K Line Loop Test feature in wanpipemon
  After enabling digital loopback run line test to test
  the device.:
  	wanpipemon -i w1g1 -c Tadlb
	wanpipemon -i w1g1 -c Tlt   

  This option can be used with any T1/E1 AFT card as well as 56K,
  with any wanpipe configuration.
  
- Hardware support for new AFT 301 Card.
  The new T3 card has been redesigned to use common main board as
  the rest of the AFT family. However, the driver remains identical.
  
- AFT T1/E1 Improved CAS Signaling Support

- Allow original A102 config to work with A102d cards.

- TDM API updated for unlimited number of /dev/wptdm devices.

- Updated the Hardware Echo Canceler with Noise suppression.

- New TDM Zaptel/TDM API installation Makefile to be used by linux power users.
  Options:
	make #buils all utilities and kernel modules
	make install #installs utilities modules etc

- Fixed DCHAN patch for zaptel 1.2.13


  KNOWS ISSUES:
  -------------
  - All cards with Maxim/Dallas front end chips do not support RBS signalling.
    These include: A102d/A108d and new A104d with maxim/dallas chips.
    The fix is in new beta 3.1.0 release.  


* Wed Jan 31 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-7
====================================================================  

- Removed a sanity check from AFT drivers
  This check was added in 2.3.4-5 release.

  Due to some imporant fixes in 2.3.4-5 this release was
  rushed, and things like this happen.  Sorry :)


* Wed Jan 31 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-6
====================================================================

- Bugfix for AFT Hardware Echo Voice cards
  Bug introduced in 2.3.4-5 release.
 
  The new feature "persistent hwec" released in 2.3.4-5 had a bug
  that was apprent on wanpipe restarts.  
  Anyone running 2.3.4-5 release should upgrade.

  If you are running 2.3.4-5 in production to fix this problem enable
  the following option in [wanpipe] section of wanpipeX.conf.
    	TDMV_HWEC_PERSIST_DISABLE=YES
  And restart all wanpipe cards using: wanrouter restart command.
  
  Note this bug affects all AFT cards with hardware echo cancellation
  and release 2.3.4-5.

  For more info please contact Sangoma Support.

- Minor cosmetic update to wancfg_zaptel wanpipe/zaptel configurator.
 

* Mon Jan 22 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-5
====================================================================

- Updated support for A400 Analog card
  This release will recognize the new A400 
  pci card info.

- Updated A200/A400 firmware updater
  Takes into account the new A200 and A400 cards.

- AFT A301 T3/E3 Driver Update
  Fixed a possible race condition
  during startup/shutdown

- AFT A101/2 T1/E1 Driver Update
  Fixed a possible race condition
  during startup/shutdown

- AFT Hardware Echo Cancellation Persist Mode
  The new default Echo Cancellation mode for all AFT cards.
  If HWEC is enabled in wanpipe1.conf the Sangoma HWEC will
  remain enabled all the time, even if the calls are not up.
  Previously Asterisk enabled and disabled echo based on
  zapata.conf and call state.  This delay however caused minute
  echo on call startup.  In Persist mode, Sangoma HWEC will
  always be enabled, thus users will get perfect quality 100%
  of the time.

  In order to configure WANPIPE card in NON-PERSIST mode one 
  has to enable TDMV_HWEC_PERSIST_DISABLE=YES option in [wanpipeX]
  section of wanpipeX.conf.
  In this mode Asterisk will be responsible for enabling/disabling
  hardware echo canceler.

- LIP Layer Bug Fix.
  This bug fix affects all WAN protocols for all AFT cards.
  The bug was a race condition in startup code.
  On a slow enough machine it was possible for an interface
  to get stuck in disconnected mode during startup.

- Wanpipe Zaptel Configuration Utility
  The new wancfg_zaptel utility is now a default way to 
  configure Sangoma cards for Zaptel.  The wancfg_zaptel utility
  will auto detect all Sangoma cards in your machine and create
  wanpipe configuration files along with /etc/zaptel.conf.  This way
  the customer can concentrate on /etc/asterisk/zapata.conf only.

  Run: /usr/sbin/wancfg_zaptel <enter>
       Wizard like questions will lead you through whole
       configuration process.

  After wancfg_zaptel following files will be created:
	/etc/wanpipe/wanpipe*.conf # All wanpipe config files
	/etc/zaptel.con	           # Fully configured zaptel      


* Tue Jan 9 2007 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-4
====================================================================

- Critical Bug fix A200/A400 Analog Cards
  Critical bug fix in fxo sanity check control.
  All customers that are running stable 2.3.4 release
  must upgrade to 2.3.4-4.

- Updates for 2.6.18 and 2.6.19 kernels.

- Critical Bug fix for A108D and A102D cards.
  It was possible for the front end interrupt handler to
  miss-handle a pending interrupt, which would caused system 
  instability.

- TDM API Update
  Removed Zaptel dependency for A200/A400 cards when
  running in TDM API mode only.


* Tue Dec 12 2006 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-3
====================================================================

- Bug fix on A301 T3/E3 Drivers
  The T3/E3 card/drivers could get stuck in connected or disconnected
  state after which no state changes are reported. Used to happend
  on noisy lines.

- Bug Fix in A101/2 Drivers
  Mishandling of skb buffers on rx stream could cause
  unpredictable behavior on some systems. 
  This has now been fixed.

- Updated A101/2/4/8 A301 Drivers
  Changed the memory allocation scheme in non interrupt context to
  use KERNEL instead of ATOMIC. The symptoms were on low 
  memory system wanrouter start could fail due to memory 
  allocation error when starting up large number of devices.

- TDM API polling bug fix
  By default TDM API application uses rx and tx streams.
  However in tx only mode, the select would fail to wakeup.

- A200 A400 Driver Bug Fix
  If A200/A400 card are started with NO FXO/FXS modules
  a kernel error is possible on some systems.   

- Update to TDM API sample Makefiles
  added -lm library to compilation 

- Updated SMG Drivers
  Sangoma SMG 1.7 Chan Woomera 1.6


* Wed Nov 30 2006 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-2
====================================================================

- A200 Sanity Check Bug Fix
  This is an IMPORTANT update! This bug fix fixes a bug in FXO
  sanity checker that on some machine could cause instability.

- Wanpipemon A200 Voltage Check bug fix.
  wanpipemon command was showing over 50V voltage 
  the sign was not observed.



* Wed Nov 23 2006 Nenad Corbic <ncorbic@sangoma.com> - 2.3.4-1
====================================================================

- This is the first official STABLE release of 2.3.4 branch.
 
- AFT A104d 64bit bug fix
  The AFT A104d T1 channel 24 failed to initialize causing DCHAN
  not to come up on 64 bit machines.

- SMG Update 
  Major optimizations to greatly improve system load and capacity.
  Added rx tx gain control from CLI and woomera.conf
  Woomera v1.5  SMG v1.6

- Driver update for 2.6.18 kernel.

- Wanpipemon Updates
  Updated for A301 drivers.
  Minor bug fixes.

- CHDLC Protocol
  Fixed up the ignore keepalive

- WANCFG - CHDLC Protocol
  Bug fix in chdlc configuration.


* Thu Nov 02 2006 Nenad Corbic <ncorbic@sangoma.com> - beta12-2.3.4
===================================================================
- Further Analog Bug Fixes.
  Fixes the problem where fxo module gets stuck during operation.



* Wed Nov 01 2006 Nenad Corbic <ncorbic@sangoma.com> - beta11-2.3.4
===================================================================
- A102D Hardware Support
  Updated drivers for A102D Hardware
  Minimum A102D firmware version V.28

- Bug fix in Analog A200 driver.
  Every once in a while a fxo port would get stuck and fail to
  receive any more calls. This has not been fixed.

- Bug fix in Analog A200 driver.
  The lasttxhoot state could get stuck in one state.
  If you have weird analog A200 issues, please try this release.

- New SMG update

- Setup installation script update

- New A200 voltage statistics in wanpipemon

- New wanrouter hwprobe verbose option
  Displays all FXO/FXS modules installed on your system
  wanrouter hwprobe verbose


* Fri Oct 13 2006 Nenad Corbic <ncorbic@sangoma.com> - beta10-2.3.4
====================================================================

- A200 Bug fix
  Analog software echo cancellation has been broken
  in beta8-2.3.4 release.  This bug has now been fixed.
  This bug does not affect A200d cards with HWEC.

- Setup update
   Update in compilation headers that caused wan_ec_client
   to fail compilation on some machine.

* Fri Oct 6 2006 Nenad Corbic <ncorbic@sangoma.com> - beta9-2.3.4
- Bug fixes hwec that were introduced in beta8
- Disabled hw dtmf events.
- This release has been tested in production.

* Mon Oct 2 2006 Nenad Corbic <ncorbic@sangoma.com> - beta8-2.3.4
=================================================================

- New A108 HWEC Support
  Optimized TDM Drivers for A108 
  Support for A108 Hardware Echo Cancellation
  Support for A108 RefClock between ports.
  New Firmware. V27

- New A108 HWEC Support
  Optimized TDM Drivers for A108 
  Support for A108 Hardware Echo Cancellation
  Support for A108 RefClock between ports.
  New Firmware. V27

- LIP / AFT Layer Latency Optimization (LINUX Only)
  By setting txqueuelen using ifconfig to value of 1
  the LIP layer will re-configure the protocol and hw layer 
  for SINGLE tx buffer. This will improve overall latency.

- AFT (A101/2/4/8) Layer Latency Optimization
  The SINGLE_TX_BUF=YES option in [w1g1] interface 
  section, configures the AFT hardware for single hw dma buffer
  instead of a DMA chain.  This option along with above
  LIP layer latency option reduces driver/protocol latency
  to theoretical minimum.
  NOTE: In WAN/IP applications latency feature will increase
  system load.
  (5% to 20% depending on system and number of ports used) 


* Mon Jul 31 2006 Nenad Corbic <ncorbic@sangoma.com> - beta7-2.3.4
==================================================================
- A200 Driver Bug Fix.
  The A200 drivers had PULSE dialing enabled
  that can cause call outages on NON-PULSE dialing lines.
  This has now been fixed.

- Updates to ./Setup installation script
  More documentation in usage.    



* Mon Jul 24 2006 Nenad Corbic <ncorbic@sangoma.com> - beta6-2.3.4
==================================================================
- Fixed the AFT HWEC on 64bit kernels.
  The bug was caused by using 32bit version of
  the octasic API release in beta5-2.3.4.



* Fri Jul 21 2006 Nenad Corbic <ncorbic@sangoma.com> - beta5-2.3.4
==================================================================
- TDM API was broken on A101/2 Cards
  It has not been fixed.

- A108D Hardware Support
  The new A108D: 8 Port T1/E1 with onboard 246 channel 
  hardware echo canceller.

- The wanec daemon has been deprecated.
  A new wanec kernel module has been created to handle
  all octasic commands. 

- The hwec release has been deprecated.
  The hwec drivers are now part of wanpipe release.

- Bug Fix in A104 and A104D Ref Clock 
  This bug caused FAX failures when REF Clock option was enabled.

- Updates for 2.6.16 and 2.6.17 kernels
  New kernels broke previous wanpipe drivers.

- Frame Relay Update: EEK Support
  New Frame relay stack supports Cisco End to End Keepalives
  on each DLCI.

- PPP Update: Updated PPP IP negotiation. IPCP was not
  working properly agains some routers.  This has now
  been resolved.

- TDM API: HW DMTF on A104D cards.
  The TDM API and regular API interfaces now supports HW DTMF
  on A104D and A108D cards.

- Setup update: the --protocols option with --silent option did not
  select proper protocols under certail conditions.
  ./Setup install --protocols=TDM  --silent
  This option will now compile TDM protocol properly.

- ADSL Update: The ADSL drivers were failing on some machines due
  to kfree_skb error under irq.  This has now been fixed.

- AFT A104/A108 Driver updated for 64bit kernels with over 4GB 
  of memory.  The NO AUDIO problem on TDM drivers.  
  The 4GB memory caused DMA corruptions to occur which caused
  no AUDIO effect.

- SSMG Product: Sangoma Signal Media Gateway
  The ssmg product is now part of standard wanpipe.
  It will automatically get installed if XMTP2 drivers are
  selected.  The SMG product provides SS7 Support to Asterisk.

- LibSangoma: LibSangoma is part of the SSMG product, however
  it can be used separately with TDM API to develop custom
  TDM applications.  Libsangoma is now part of standard wanpipe
  release. 

* Fri Mar 04 2006 Nenad Corbic <ncorbic@sangoma.com> - beta4-2.3.4
- A108 Hardware Support
  Full support for new A108 8 Port T1/E1 Card
	
- AFT Front End T1/E1 Alarm Update/Polling
  On some embedded machines the A104D cards exhibited
  unreliable ports i.e. The port would not come up due to
  missing interrupts from the front end chip.  
  The Front End polling mechanism has been updated to solve this problem.  

- TDM API Update
  Fixed a bug in RX/TX functions.
  On some machines the kernel to user copy did not work.

- Updated HWEC and TDMAPI Udev Devices
  HWEC UDEV device: /dev/wp1ec (Major: 241)
  TDM API UDEV device: /dev/wptdm_s1c1 (Major: 242)
	
- Setup Installation Script Update
  Compilation of Zaptel during installation
  UDEV Zaptel updates
  UDEV TDM API updates

- AFT-LIP Frame Relay DLCI Statistics
  Updated Frame Relay DLCI Statistics.

- AFT-LIP PPP Update
  IPCP negotiation failed when connected to some telco.

* Wed Feb 22 2006 Nenad Corbic <ncorbic@sangoma.com> - beta3-2.3.4
- A104D HWEC Update
  Bug fix on E1 4th port channel 31 did not work.

- A104D HWEC 64Bit Fix
  The A104D HWEC now works on 64bit machines.
  Using UDEV /dev/wp1ec device to access hwec chip.
  If no UDEV one must create /dev/wp1ec using:
		mknod /dev/wp1ec c 2402 1 

- AFT Firmware Update Utility 
  Bug fix. On some systems firmware update failed. 

- TDM API Updates
  Support on AFT A101/2 Cards
  Fixed shutdown problems while channels are opened.
	  
* Wed Feb 15 2006 Nenad Corbic <ncorbic@sangoma.com> - beta2-2.3.4
- The Beta2-2.3.4 is the first official Beta release of 2.3.4.
  The 2.3.4 release has been in ALPHA for months and has gone
  through major testing process.

- A200 Remora Updates
  Updates for 24 port A200 solution
  New A200 Firmware Release V05

- A104D Update/Bug Fix 
  Echo cancellation DSP had problems on
  port 4 of A104D card.

- New A104/4D TDM API and libsangoma release
  The new TDM API replaces the old TDM API.
  Currently the TDM API is only supported on A104 and A104D cards.

  The TDM API is compiled by default in wanpipe drivers.
  Supports: 	PRI and RBS functionality.
  Sample files: /etc/wanpipe/api/tdm_api 

- Libsangoma Release
  New libsangoma source is located in 
  /etc/wanpipe/api/libsangoma directory.

  For more info and docs please refer to http://sangoma.editme.com


* Tue Jan 10 2006 Nenad Corbic <ncorbic@sangoma.com> - beta1y-2.3.4
- Driver update for 2.4.30 kernel 
- AFT A104 Driver minor update to TDM API 
  Number of dma packets used in API mode.
  To save memory. Not a functional change.

- Update to Setup install script

- Wancfg Update
  New option to configure wanpipe config file base on zaptel.conf
  syntax:  "wancfg zaptel"

* Tue Dec 15 2005 Nenad Corbic <ncorbic@sangoma.com> - beta1x-2.3.4
- Major AFT A200 Analog Updates
  Analog Hardware Echo Canceler is now supported
  Bug fixes on aLaw lines, noise problems.
	  
- Wancfg Updates

- Updates to Setup installation utility

* Mon Dec 5 2005 Nenad Corbic <ncorbic@sangoma.com> - beta1x-2.3.4
- WanCfg Update 
  New wancfg option "zaptel" that will create wanpipe
  based on /etc/zaptel.conf configuration.
  eg:
	1. Configure your zaptel first based on hardware
	2. Run: wancfg zaptel 
	   To create wanpipe config files based on zaptel.conf
        3. Run: wanrouter start
	4. Run: ztcfg -vvv
	5. Read to start Asterisk
	
- Setup Update
  Added new options in ./Setup for user to specify custom gcc	
  compiler.  

- AFT A200 Updates
  Further Analog driver updates.
 
- AFT Echo Debugging Utility
  Used to measure echo spike and debug echo problems. 


* Wed Nov 23 2005 Nenad Corbic <ncorbic@sangoma.com> - beta1x-2.3.4

- Bug fix in Analog startup
  You can now set ACTIVE_CH=ALL and by default all detected
  FXO/FXS Devices will be configured.

- Bug fix in A102 Drivers
  When both ports are running, if you stop one 
  the other one dies.
- - END - 
