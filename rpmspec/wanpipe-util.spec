%define KERNEL_VERSION    %{?kern_ver}
%define WANPIPE_VER	  wanpipe-util
%define name              %{WANPIPE_VER}
%define version           3.1.4
%define release           0
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
* Tue Aug 15 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.4
==================================================================== 

- Added A101-SH old config support.
  So onld A101u or A101c config file can be used with new A101-SH cards.

- Updated KATM support in the LIP Layer.
  Used to connect Kernel ATM Layer to Wanpipe ATM AAL5 layer
  over all AFT cards.

- Added a sanity checker for enabling HWEC.
  Used to prevent duble hwec enable.

- Added wancfg_tdmapi configurator

- Updated SMG
	

* Mon Jun 30 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.3
==================================================================== 

- Update to Ocatsic Hardware Echo Canceler Library
  Turned of the NOISE suppression because it can interfere
  with faxes. If you faxes did not work properly on 3.1.2
  release they will work fine with this one.

- Cleaned up the Setup installation script.


* Mon Jun 16 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.2
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


* Fri Jun 06 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.1
==================================================================== 

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



- Updated SMG 
  Fixed DTMF synchronization


* Fri Jun 06 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.0.1
==================================================================== 

- Minor release
- Contains zaptel patch for zaptel 1.2.17 and above.
- No driver changes 

* Fri May 17 2007 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.1.0
====================================================================  

- Major new BETA wanpipe release 
  Changed wanpipe versioning:
	Release: A.B.C.D
	A - Major Relase number
	B - Indicates Stable or Beta
	    Odd number is Beta
	    Even number is Stable
	C - Minor Release number
	D - Optional pre-release and custom releases
 
- Fixed RBS Support for all Maxim cards A101/2/4/8.

- Support for 2.6.20 kernels.

- Support for New: A101D A102D A104D Maxim cards
     :
- Support for New: AFT 56K DDS card

- Redesigned TDM API Events

- TDM API Analog Support

- - END - 
