#
# spec file for package wanpipe (Version 2.3.4)
#
# Copyright (c) 2007 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild

Name:           wanpipe
BuildRequires:  kernel-source kernel-syms module-init-tools udev
URL:            http://www.sangoma.com
Summary:        Sangoma Wanpipe Voice,TDM and Wan Drivers/Utilities
Version:        2.3.4
Release:        7
License:        GNU General Public License (GPL)
Group:          Productivity/Telephony/Utilities
PreReq:         %insserv_prereq %fillup_prereq udev
PreReq:         /usr/sbin/useradd
Excludearch:  	s390 s390x
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build
Source0:        %name-%version-%release.tar.bz2

%define UTILS_DIR         /usr/sbin
%define UTILS_LOCAL_DIR   /usr/local/sbin
%define PROD_HOME         /etc/wanpipe
%define WANCFG_LIBS_DIR   /etc/wanpipe/lib
%define API_DIR           /etc/wanpipe/api
%define DOCS_DIR          /usr/share/doc/wanpipe
%define PROD              wanrouter
%define MODULES_DIR       /lib/modules
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


%suse_kernel_module_package -p %_sourcedir/preamble kdump um iseries64 ppc64

%description
This package contains configuration files, header files, and setup
tools needed for the Sangoma wanpipe voice/TDM, Wan drivers.

See /usr/share/doc/packages/wanpipe/README for a list of supported
hardware.


Authors:
--------
    Nenad Corbic <ncorbic@sangoma.com>

%package KMP
Summary:        Sangoma Wanpipe Voice/TDM, WAN Drivers 
Group:          System/Kernel

%description KMP
This package contains the kernel modules of the Sangoma
Wanpipe Voice/TDM, Wan Drivers.



Authors:
--------
    Nenad Corbic <ncorbic@sangoma.com>

%prep
%setup -q -a 1
%patch0 -p1
%patch1
%patch2
%patch3
%patch4 -p1

%build
make prereq all OPTFLAGS="%optflags"
for flavor in %flavors_to_build; do
    ./Setup install --silent --builddir=%buildroot --with-linux=/usr/src/linux-obj/%_target_cpu/$flavor --protocols=DEF
done

%install

%clean
rm -rf %buildroot

%pre

%post
%{?fillup_and_insserv:%fillup_and_insserv}
%{?run_ldconfig:%run_ldconfig}

%postun
%{?insserv_cleanup:%insserv_cleanup}

%preun
%{?stop_on_removal:%stop_on_removal zaptel}

%files
%defattr(-,root,root,-)
%doc README* ChangeLog
%{UTILS_DIR}
%{UTILS_LOCAL_DIR}
%{PROD_HOME}
%{DOCS_DIR}
%{MODULES_DIR}

%changelog
