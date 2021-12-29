%define KERNEL_VERSION    %{?kern_ver}
%define WANPIPE_VER	  wanpipe-util
%define name              %{WANPIPE_VER}
%define version           3.5.2
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

* Fri May 08 2009 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.5.2
===================================================================

- B700 PCIe cards were being desplayed as PCI cards in hwprobe
- Bug fix in wancfg_zaptel 

* Thu May 07 2009 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.5.1
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
