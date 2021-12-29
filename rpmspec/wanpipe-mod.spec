%define WANPIPE_VER	  wanpipe-modules
%define name              %{WANPIPE_VER}
%define version           2.3.3
%define release           7
%define	serial	 	  1
%define MODULES_DIR	  /lib/modules
 

Summary: 	Sangoma WANPIPE package for Linux. It contains the WANPIPE kernel drivers.  Please install wanpipe-util package for wanpipe utilties and configuration files.
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

%post

#check dependancies for the new modules
depmod -a >> /dev/null 2>> /dev/null

modprobe wanrouter
if [ $? -eq 0 ]; then
	echo "Wanpipe kernel modules installed successfully"
	modprobe -r wanrotuer 2>> /dev/null
else
	echo "Failed to load wanpipe modules!"
	echo
	echo "Make sure you are installing correct RPMS for you system!"
	echo
	echo "Otherwise call Sangoma Tech Support"
fi


%files
%{MODULES_DIR}


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



