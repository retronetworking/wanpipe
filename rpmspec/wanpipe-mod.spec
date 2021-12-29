%define WANPIPE_VER	  wanpipe-modules
%define name              %{WANPIPE_VER}
%define version           3.3.2
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
