%define WANPIPE_VER	  wanpipe-util
%define name              %{WANPIPE_VER}
%define version           2.3.2
%define release           7
%define	serial	 	  1
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
%define START_SCRIPT      S20%{PROD}
%define OLD_START         S20router
%define STOP_SCRIPT       K10%{PROD}
%define OLD_STOP          K10router
%define ROUTER_RC         %{META_CONF}
%define WANROUTER_STARTUP_SMPL    %{PROD_HOME}/samples/wanrouter
%define WANROUTER_STARTUP         /usr/sbin/wanrouter
%define NEW_IF_TYPE               NO
%define PROD_INIT                 /usr/sbin/
 

Summary: 	Sangoma WANPIPE package for Linux. It contains WANPIPE configuration/startup/debugging utilities for Linux. This package requires the wanpipe-mod package!
Name: 		%{name}
Version: 	%{version}
Release: 	%{release}
Copyright: 	GPL
Group: 		Applications/Communicatios
#Source0:	%{WANPIPE_VER}.tgz
#Source1:	bridge-utils-0.9.1.tar.gz
Vendor:		Sangoma Technologies Inc.
Url:		www.sangoma.com
Group:		Networking/WAN
 

%description 
WANPIPE S-series is a family of intelligent multi-protocol WAN adapters that support data transfer rates up to 4Mbps. All WAN protocols supported by WANPIPE are implemented in firmware and run on the card. An advantage of an intelligent adapter is that it offloads the system CPU and improves stability. By adding a Sangoma WAN component to the Linux kernel, one can create a powerful multi-T1 router/firewall with proven reliability of Linux. Sangoma S-series cards support an optional on board CSU/DSU that eliminates all external components of a traditional routing solution. The T1 line can be directly connected to the card. WANPIPE supports the following protocols, Frame Relay, X25(API), PPP, MULTILINK PPP and CHDLC. Furthermore, WANPIPE supports custom API development such as: Credit card verification, Voice-over IP, Satellite Comm. All device drivers are part of the standard Linux Kernel distribution. 

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
# wanrouter.rc     WAN router meta-configuration file.
#
#               This file defines variables used by the router shell scripts
#               and should be located in /etc/wanpipe directory.  These are:
#
#               ROUTER_BOOT=            Boot flag (YES/NO).
#               WANPIPE_CONF_DIR=       Where to put wanpipe config files.
#               WANPIPE_INTR_DIR=       Where to put wanpipe interface files.
#               ROUTER_LOG=             Where to put start-up log file.
#               ROUTER_LOCK=            File used as a lock.
#               ROUTER_IP_FORWARD=      Enable IP Forwarding on startup.
#               WAN_DEVICES=            Name of the wanpipe devices to be
#                                       loaded on 'wanrouter start'
#                                       (ex: "wanpipe1 wanpipe2 wanpipe3...")
#               WANCFG_LIBS_DIR=        Location of wancfg libraries (lib.sh ...)
#
#                              Note:    Name of wanpipe devices correspond
#                                       to the configuration files in
#                                       WANPIPE_CONF_DIR directory:
#                                         (eg. /etc/wanpipe/interfaces/wanpipe1.conf )
#
#               Note:   This file is 'executed' by the shell script, so
#                       the usual shell syntax must be observed.
ENDOFTEXT
 echo "ROUTER_BOOT=YES"          >> %{META_CONF}
        echo "WANPIPE_CONF_DIR=%{WAN_CONF_DIR}" >> %{META_CONF}
        echo "WANPIPE_INTR_DIR=%{WAN_INTR_DIR}" >> %{META_CONF}
        echo "ROUTER_LOG=$LOG_FILE"     >> %{META_CONF}
        echo "ROUTER_LOCK=$LOCK_FILE"   >> %{META_CONF}
        echo "ROUTER_IP_FORWARD=NO" >> %{META_CONF}
        echo "NEW_IF_TYPE=%{NEW_IF_TYPE}" >> %{META_CONF}
        echo "WAN_DEVICES=\"wanpipe1\"" >> %{META_CONF}
        echo "WANCFG_LIB=/usr/lib/wanpipe/wancfg/lib" >> %{META_CONF}
 
        return 0
}


# ----------------------------------------------------------------------------
# Install initialization scripts.
# ----------------------------------------------------------------------------
install_init()
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
%{UTILS_DIR}/wanconfig
%{UTILS_DIR}/wanrouter
%{UTILS_DIR}/wancfg
%{UTILS_DIR}/wancfg_legacy
%{UTILS_DIR}/sdladump
%{UTILS_DIR}/wanpipemon
%{UTILS_DIR}/wanpipemon_legacy
%{UTILS_DIR}/wpkbdmon
%{UTILS_DIR}/wanpipe_lxdialog
%{UTILS_DIR}/wanpipe_ft1exec
%{UTILS_DIR}/cfgft1
%{UTILS_DIR}/wp_pppconfig
%{UTILS_DIR}/wanpipe_setup
%{UTILS_DIR}/wanconfig_client
%{UTILS_DIR}/wp_x25_event_read
%{UTILS_DIR}/wpbwm
%{PROD_HOME}
%{DOCS_DIR}


%changelog
* Wed Oct 26 2001 Nenad Corbic <ncorbic@sangoma.com>
- Added the API drivers and API sample codes to the
  RPM.

* Mon Sep 25 2001 Nenad Corbic <ncorbic@sangoma.com>
- The spec file is generic for any kernel
  and can be used to create custom RPMs from 
  wanpipe source packges.

* Mon Apr 23 2001 David Rokhvarg <drokhvarg@sangoma.com>
- initial version



