%define KERNEL_VERSION    %{?kern_ver}
%define WANPIPE_VER	  wanpipe-util
%define name              %{WANPIPE_VER}
%define version           3.3.6
%define release           0
%define	serial	 	  1
%define ETC_DIR 	  /etc
%define USR_DIR 	  /usr
%define UTILS_DIR 	  /usr/sbin
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
        chkconfig --add wanrouter
	chkconfig wanrouter on
}

# ----------------------------------------------------------------------------
# Enable MGD and BRI  log for A500-BRI
# ----------------------------------------------------------------------------
enable_smg_log()
{
if [ -e  /etc/syslog.conf ]; then
        eval "grep "local2.*sangoma_mgd" /etc/syslog.conf" > /dev/null 2> /dev/null
        if [ $? -ne 0 ]; then
                eval "grep "local2" /etc/syslog.conf " > /dev/null 2> /dev/null
                if [ $? -eq 0 ]; then
                        echo
                        echo "Warning : local2 is already used in syslog.conf"
                        echo
                fi
                echo -e "\n# Sangoma Media Gateway log" > tmp.$$
                echo -e "local2.*                /var/log/sangoma_mgd.log\n" >> tmp.$$
                eval "cat /etc/syslog.conf tmp.$$ > tmp1.$$"
                \cp -f tmp1.$$ /etc/syslog.conf
                eval "/etc/init.d/syslog restart" > /dev/null 2>/dev/null
        fi
        eval "grep "local3.*sangoma_bri" /etc/syslog.conf" > /dev/null 2> /dev/null
        if [ $? -ne 0 ]; then
                eval "grep "local3" /etc/syslog.conf " > /dev/null 2> /dev/null
                if [ $? -eq 0 ]; then
                        echo
                        echo "Warning : local3 is already used in syslog.conf"
                        echo
                fi
                echo -e "\n# Sangoma BRI Daemon (smg_bri)  log" > tmp.$$
                echo -e "local3.*                /var/log/sangoma_bri.log\n" >> tmp.$$
                eval "cat /etc/syslog.conf tmp.$$ > tmp1.$$"
                \cp -f tmp1.$$ /etc/syslog.conf
                eval "/etc/init.d/syslog restart" > /dev/null 2> /dev/null
        fi
else
        echo "Warning: /etc/syslog.conf not found"
fi

if [ -f tmp1.$$ ]; then
        rm -f  tmp1.$$
fi
if [ -f tmp.$$ ]; then
        rm -f  tmp.$$
fi

echo "Ok"
echo

echo "Checking logrotate ..."
eval "type logrotate" > /dev/null 2> /dev/null
if [ $? -ne 0 ]; then
        echo "Error: Logrotate not found !"
fi

if [ -e /etc/logrotate.d ] && [ -e /etc/logrotate.d/syslog ]; then

        eval "grep sangoma_mgd /etc/logrotate.d/syslog" > /dev/null 2> /dev/null
        if [ $? -ne 0 ]; then
                eval "sed -e 's/messages/messages \/var\/log\/sangoma_mgd.log/' /etc/logrotate.d/syslog >tmp2.$$ 2>/dev/null"
                eval "cp -f tmp2.$$ /etc/logrotate.d/syslog"
                eval "logrotate -f /etc/logrotate.d/syslog"
                if [ $? -ne 0 ]; then
                        echo "Error: logrotate restart failed!";
                        exit 1;
                fi
                echo "Logrotate is being changed and restarted!"
        else
                echo "Logrotate is configured!"
        fi

else
        echo "Error: Logrotate dir: /etc/logrotate.d not found !"
fi
echo "OK."
echo

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
enable_smg_log;


%files
%{ETC_DIR}
%{USR_DIR}
%{UTILS_DIR}
%{PROD_HOME}
%{DOCS_DIR}


%changelog


* Wed Apr 4 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.6
======================================================================

- BRI Driver bug fixes
  Secured the bri restart logic to prevent possible
  race conditions.

- BRI Stack update
  Bug Fix: bri stack update fixes reconnect on etsi lines
  Feature: group outbound calls skip disconnected lines

- T3 update
  Fixed dma issues on some machines when tx/rx mixed
  voip and data traffic.

- Hardware Probe Verbose Update
  Analog and BRI cards now display PCI/PCIe info on
  wanrouter hwrobe verbose.


* Wed Mar 27 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.5
======================================================================


- Removed debugging out of firmware update utility
- Updated firmware bin files
- Updated firmware update script.

- No Functional Changes from 3.3.4

* Wed Mar 26 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.4
====================================================================== 

- BRI TE auto clocking feature - Bug fix
   This feature failed on on some machines with multiple TE BRI modules.
   This bug would cause modules to loose sync. Bug introduced in 3.3.3
   release.

* Tue Mar 25 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.3
====================================================================== 

-  BRI TE auto clocking feature. 
    Where a connected TE port is elected
    as the telco clock recovery port for the whole card.  If that TE
    port goes down, another connected TE port is found to provide
    telco recovery clock to the card.  If no connected TE ports are found
    then internal oscillator is used. 
    -> This option is fully automatic no configuration options needed.

-  BRI Zaptel Clock Source
    Since BRI does not interface to zaptel, it acts as ZT DUMMY to
    provide zaptel reliable timing.  One has to configure
    TDMV_DUMMY=YES in [wanpipe1] section of wanpipe1.conf

-  A200/A400 Remora Relax CFG
    If one module fails during operation the wanpipe driver by default
    fails to load.  With this option wanpipe driver
    will allow the card to come up with a broken module so that
    customer will be able to continue working until the module is replaced.
    RM_RELAX_CFG=YES in [wanpipe1] section of wanpipe1.conf



* Fri Feb 14 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.1
====================================================================== 

- Added DTR API for Serial S514X Card
  Ability to read and set the DTR/RTS on serial cards throught the API.
  Sample code in wanpipe-3.3.2.1/api/chdlc/chdlc_modem_cmd.c


* Wed Feb 12 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2
====================================================================== 

- Support for A500 hardware support with NetBricks BRI Stack
- Major A500 driver updates and fixes
- Serial A142/A144 hardware support
- AFT A056 56K hardware support 

- Support for HW DTMF

- Updates for AFT PMC and MAXIM framers
  PMC - lowered LOS sensitivity
        Fixes fake up/down state changes on
        started inactive lines.

  MAXIM - lowered sensistivy
          Fixes cable cross talk on 8 port cards.
        - Enabled Unframed E1
        - Enabled Tri-State Mode
        - Fixed loopback commands

- Updated loopback commands for AFT Maxim cards

- Updated for AstLinux
  The make file can now build all WAN and Voice Protocols
  
- Updated legacy protocols for new front end architecture

- 


* Fri Feb 01 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.p8
====================================================================== 

- wancfg_zaptel now asks for the default_tei value for 
- BRI cards in TE mode

- Fix for HWEC not being enabled when non-consecutive modules are using 
- in BRI cards

* Fri Feb 01 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.p4
====================================================================== 

- Fixed AFT memory leak
  Memory leak introduced in 3.3 release
- Fixed AFT 56K bug
  Bug introduced in 3.3 releae


* Fri Feb 01 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.p3
====================================================================== 

- Fix bug in BRI protocol for fast local hangups.

* Mon Jan 18 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.2.p1
====================================================================== 

- Bug fix in Hardware EC code for E1.
  Bug introduced in 3.3 release.


* Mon Jan 18 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.1
==================================================================== 


* Mon Jan 16 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.22
==================================================================== 

- BRI protocol:Increased internal timer that could cause issue in systems with
- more than 8 BRI spans

* Mon Jan 15 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.21
==================================================================== 

- BRI protocol:Fix for smg_brid daemon crashing on race condition
- BRI protocol:default_tei parameter is not ignored when using point to 
- multipoint anymore

* Mon Jan 14 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.20
====================================================================  

- BRI protocol: Additional prefix options. 
- BRI protocol: Check is caller ID number is all digits on incoming calls
- Sangoma MGD: Removed dynamic user period causing skb panics
- chan_woomera: Fixed issue with rxgain and txgain values set to 0 if 
- coding not set in woomera.conf
- wancfg_zaptel: Support for fractional T1/E1 spans.
- wancfg_zaptel: fix issue BRI always being configured as bri_net introduced in v3.3.0.19

* Mon Jan 07 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.19
====================================================================  

- Support for national/international prefix in BRI stack

* Mon Jan 07 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.18
====================================================================  

- Changed Makefile in wanpipe/api/fr causing compilation errors 


* Thu Dec 20 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.17
====================================================================  

- Fix for smg_ctrl boot script starting before network services on some systems
- Support for language parameter in chan_woomera

* Thu Dec 20 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.16
====================================================================  

- Fix for Sangoma BRI Daemon crashing on incoming call if chan_woomera is not installed on that system

* Tue Dec 18 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.15
====================================================================  

- Fix for caller ID value being corrupted sometimes
- Support for call confirmation in chan_woomera

* Tue Dec 18 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.14
====================================================================  

- Fix in smg_brid not releasing some b-channels properly
- Fix in wancfg_smg not setting MTU to 80 when configuring cards for SS7

* Fri Dec 14 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.13
====================================================================  

- Fix for Kernel panic on 64-bit systems when enabling hardware echo canceller


* Thu Dec 5 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.11
====================================================================  

- Support for AFT Serial Cards
- Updates for AFT PMC and MAXIM framers
  PMC - lowered LOS sensitivity
        Fixes fake up/down state changes on
        started inactive lines.

  MAXIM - lowered sensistivy
          Fixes cable cross talk on 8 port cards.
        - Enabled Unframed E1
        - Enabled Tri-State Mode
        - Fixed loopback commands

- Fixed HWEC_PERSIST_DISABLE
  This option was broken in previous release
  This option lets Asterisk control HWEC
  on each call start/stop.
  By default all hwec channels are enabled on
  device startup.

- Updated SMG/SS7 
- Updated loopback commands for AFT Maxim cards

- Updated for AstLinux
  The make file can now build all WAN and Voice Protocols
  
- Fixed add_timer warnings for ALL AFT cards
  Caused when a port is left in unconnected state.

- Updated legacy protocols for new front end architecture

- Updated Setup script 



* Thu Nov 8 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.4
====================================================================  

- Fixed A101/2 (Old) bug introduced in previous releaes

* Mon Oct 31 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.4
====================================================================  

- Updated BRI caller name
- Updaged Setup


* Mon Oct 15 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.1
====================================================================  

- Major Updates
- New BRI architecture/support
  SMG with Netbricks BRI Stack
- Support for Hardware DTMF


* Thu Aug 22 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.p3
======================================================================

- Updated wancfg_zaptel to support HW DTMF


* Thu Aug 21 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.0.p2
======================================================================  

- Major Updates
- Hardware DTMF for Asterisk and TDM API
- - END - 
