#!/bin/sh

home=`pwd`;

force=${1:-flase}

if [ ! -e /usr/local/ss7box/$ss7boost ]; then
	echo "Error: ss7boost not found in /usr/local/ss7box dir";
	exit 1
fi
if [ ! -e /usr/local/ss7box/$ss7boxd ]; then
	echo "Error: ss7boxd not found in /usr/local/ss7box dir";
	exit 1
fi

if [ $force = "force" ]; then
	echo "Stopping SMG..."
	eval "sangoma_mgd -term"
        kill -TERM $(pidof asterisk);
	echo "OK."
	echo
else 
	if [ -f /var/run/sangoma_mgd.pid ]; then
		echo "Error: sangoma_mgd is running!"
		exit 1
	fi
	if [ -f /var/run/asterisk.pid ]; then
		echo "Error: asterisk is running"
		exit 1
	fi
fi

echo
echo "Checking Syslog ...."
if [ -e  /etc/syslog.conf ]; then
eval "grep "local2.*sangoma_mgd" /etc/syslog.conf" > /dev/null 2> /dev/null
if [ $? -ne 0 ]; then
	eval "grep "local2" /etc/syslog.conf " > /dev/null 2> /dev/null
	if [ $? -eq 0 ]; then
		echo
		echo "Warning : local2 is already used in syslog.conf"
		echo
	fi
	echo -e "\nlocal2.*                /var/log/sangoma_mgd.log\n" > tmp.$$
	eval "cat /etc/syslog.conf tmp.$$ > tmp1.$$"
	\cp -f tmp1.$$ /etc/syslog.conf
	eval "/etc/init.d/syslog restart"
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


echo "Checking for SCTP...."
if [ ! -e  /usr/include/netinet/sctp.h ]; then
	echo "Please install SCTP devel package: yum install lksctp-tools-devel"
	echo 
	exit 1
fi 
echo "OK."
echo

echo "Compiling Sangoma MGD ..."
make clean
make
if [ $? -ne 0 ]; then
	exit 1;
fi
make install
echo "Ok."


echo "Compiling Woomera Channel ..."

if [ ! -e /usr/src/asterisk ]; then
	echo
	echo "Error: /usr/src/asterisk directory does not exist!"
	echo "       Please create symlink /usr/src/asterisk and"
	echo "       point to existing asterisk source!"
	echo "       Then re run ./install.sh "
	echo
	exit 1
fi

cp -f g711.h /usr/src/asterisk/
perl /usr/src/asterisk/contrib/scripts/astxs -install chan_woomera.c
if [ $? -ne 0 ]; then
	exit 1;
fi
echo "Ok."


echo "SMG Install Done"
echo


