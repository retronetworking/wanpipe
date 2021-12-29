%define WANPIPE_VER	  wanpipe-modules
%define name              %{WANPIPE_VER}
%define version           3.5.11
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


* Thu Apr 08 2010 Nenad Corbic <ncorbic@sangoma.com> -  3.5.11
===================================================================

- Fix for 2.6.31 and higher kernels
- TDM API Analog rx gain feature
- Disabled default NOISE REDUCTION feature in hwec
  that was enabled in 3.5.9 release. 
- Updates to T1/E1 Loopback and BERT Test	


* Wed Jan 11 2010 Nenad Corbic <ncorbic@sangoma.com> -  3.5.10
===================================================================

- Release cleanup script earsed libsangoma.c during release packaging.
  I have update release procedure so this does not happen again.
  This release has no functionl differences aside from the missing
  file from 3.5.9 release.

* Wed Dec 30 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.9
===================================================================

- New logger dev feature
- Bug fix in tx fifo handler
- Dahdi 2.2 broke wanpipe rbs support.
- Fixed free run interrupt supported on V38 (A108)
- Fixed RBS signalling for E1 channel 31
- Added Front end Reset Detection
  -> Support for new A108 Firmware V40
- Fixed RTP TAP bug: Caused high system load on RTP TAP usage.
- Added excessive fifo error sanity check. Fixes random pci dma errors.
- HWEC: Increased EC VQE Delay: Fixes random fax failure due to hwec.
- HWEC: Check state before bypass enable.
- HWEC: Disable bypass on release
- HWEC: Enabled Noise Reduction by default
- HWEC: Enabled Auto Gain Control by default
- HWEC: To disable Noise Reduction and Gain control set
        -> HWEC_NOISE_REDUCTION_DISABLE=NO in [wanpipe] section of wanpipe1.conf
		To check if Noise Reduction or Gain control are set
		-> wan_ec_client wanpipe1 stats 1


* Thu Oct 02 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.8
===================================================================

- Bug fix in sangoma_prid PRI stack for FreeSwitch & Asterisk.
  There was a slow memory leak.


* Thu Sep 04 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.7
===================================================================

- New Telesoft PRI Stack Support for FreeSwitch & Asterisk
  For Asterisk: The new stack uses the existing Sangoma Media Gateway
                architecture currently used by SS7 and BRI.
                -> run: wancfg_dahdi or wancfg_zaptel to configure
				   for sangoma prid stack.

  For FreeSwitch: The new stack binds to openzap directly just like
                  current SS7 and BRI.
				-> run: wancfg_fs to configure freeswitch for
				        sangoma prid, brid, ss7.

- Fixed Tx Tristate 

- Updated yellow alarm handling for Dallas maxim cards
  (A101/2/4/8)

- Autodetect USB support so that driver will compile
  correctly on kernel without USB support

- Added DAHDI Red alarm for Analog


* Thu Aug 20 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.6
===================================================================

- Update to T1 Yellow Alarm handling.
  In some cases Yellow alarm did not turn off poperly causing
  line to stay down an card startup.
- Update configuration utility
  wancfg_fs updated for sangoma_prid configuration. Added wancfg_openzap
  for OpenZap Configuration 


* Mon Aug 17 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.5
===================================================================

- Dahdi 2.2 Support
- BRI Update - Added T1 timer for NT module
- AFT Core Update - optimized dma ring buffer usage
- TDM API - refractoring and optimization
- Updated for 2.6.30 kernel

- New firmawre feature for A101/2/5/8: Free Run Timer Interrupt 
  The AFT T1/E1 cards will now provide perfect timing to zatpel/dahdi
  even when the ports are not connected. The free run interrupt
  will be enabled when all zaptel/dahdi ports are down, or on
  inital card start. To test this feature just start a wanpipe 
  port with zaptel/dahdi and run zttest. 
  A108 firmare V38 
  A104/2/1/ firmware V36

- AFT T1/E1 front end update
  Added OOF alarm treshold, so that line does not go down
  on very first OOF alarm.

- Added module inc cound when zaptel/dahdi starts.
  So wanpipe drivers do not crash if one tries to unload 
  zaptel/dahdi before stopping wanpipe drivers.


* Thu Jul 07 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4.8
===================================================================

- Updated for B700 Dchan Critical Timeout
- Fix for FAX detect on PRI
- Updated for 2.6.21 kernel TASK QUEUE REMOVAL caused 
  unexpected behaviour.
- Updated wancfg_zaptel for fax detect

* Thu Jul 03 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4.3
===================================================================

- Added DAHDI 2.2 Support


* Thu Jul 02 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4.2
===================================================================

- AFT 64bit update
  No need for --64bit_4G flag any more. 
  The 64bit check is now down in the driver.

- TDM API
  Updated the Global TDM Device
  This device can be used to read events an all cards configured in
  TDM API mode.

- Libsangoma verion 3.1.0
  Added a function to check if hwec is supported


* Tue Jun 30 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4.1
===================================================================

- Sangoma MGD update v.1.48
  Disable hwec on data calls


* Mon Jun 29 2009 Nenad Corbic <ncorbic@sangoma.com> -  3.5.4
===================================================================

- E1 Voice Bug fix introduced in 3.5.3 

- Removed NOISE REDUCTION enabled by default.
  The noise reduction is disabled by default and should be
  enabled using HWEC_NOISE_REDUCTION = YES 
 
- Fixed libsangoma enable dtmf events functionality



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
