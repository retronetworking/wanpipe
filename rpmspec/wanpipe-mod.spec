%define WANPIPE_VER	  wanpipe-modules
%define name              %{WANPIPE_VER}
%define version           3.5.3
%define release           0
%define	serial	 	  1
%define MODULES_DIR	  /lib/modules
%define USR_INCLUDE_DIR   /usr/include

%define KVERSION          %{?kern_ver}
 

Summary: 	Sangoma WANPIPE package for Linux. It contains the WANPIPE kernel drivers.  Please install wanpipe-util package for wanpipe utilties and configuration files.
Name: 		%{name}-%{?kern_ver}
Version: 	%{version}
Release: 	%{release}
License: 	GPL
Group: 		Applications/Communications
Vendor:		Sangoma Technologies Inc.
Url:		www.sangoma.com
Group:		Networking/WAN
 

%description 
Linux Drivers for Sangoma AFT Series of cards and S Series of Cards. Wanpipe supports the following protocols, TDM Voice, Frame Relay, X25(API), PPP,Multi-link PPP, CHDLC and custom API development for WAN and Voice.

Install Wanpipe-util package for wanpipe utilities and configuration files.


%prep

%build

%install

%clean

%postun

echo "Uninstalling WANPIPE..."

%post

#check dependancies for the new modules
depmod -ae -F /boot/System.map-%{KVERSION} %{KVERSION}
echo "Wanpipe Modules located in %{MODULES_DIR}/%{KVERSION}"   

%files
%{MODULES_DIR}
%{USR_INCLUDE_DIR}


%changelog

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
