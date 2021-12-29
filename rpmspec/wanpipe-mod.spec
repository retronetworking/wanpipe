%define WANPIPE_VER	  wanpipe-modules
%define name              %{WANPIPE_VER}
%define version           3.4.8
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

* Mon Nov 30 2009 Nenad Corbic <ncorbic@sangoma.com> - Feature Frozen - 3.4.7.3
================================================================================

- Fixed RBS signalling for E1 channel 31
- Added Front end Reset Detection
  -> Support for new A108 Firmware V40
- Fixed RTP TAP bug: Caused high system load on RTP TAP usage.
- Added excessive fifo error sanity check. Fixes random pci dma errors.
- Increased EC VQE Delay: Fixes random fax failure due to hwec.
- HWEC: Check state before bypass enable.
- HWEC: Disable bypass on release


* Tue Nov 25 2009 Nenad Corbic <ncorbic@sangoma.com> - Feature Frozen - 3.4.7
================================================================================

- New A200 Firmware V12
  Fixes the fifo reporting in firmware.

- Updated driver fifo handling so that if customer is running
  older version of A200 firmware (less than V12) the fifo interrupt handling
  would still work correctly.

- Dahdi 2.2 broke Sangoma RBS support 
- Fixed the free run interrupt supported on V38 firmware
- Fixed chan_woomera inbound calls on second profile
- Fixed sangoma_mgd for multiple profiles
- Updated smg
- Fixed wanpipemon LIU alarm statistics
- Updates SMG for 32T1/E1 Support
- Backporte front end OOF alarm update.
  A OOF alarm counter was added so that link does not
  drop on a single OOF alarm as per T1/E1 spec.


* Tue Sep 18 2009 Nenad Corbic <ncorbic@sangoma.com> - Feature Frozen - 3.4.6
===============================================================================

- Fund a bug in Sangoma_mgd causing channel 31 in each span to
  fail. This bug was introduced in 3.4.5 release.


* Tue Sep 16 2009 Nenad Corbic <ncorbic@sangoma.com> - Feature Frozen - 3.4.5
===============================================================================

- New firmawre feature for A101/2/5/8: Free Run Timer Interrupt 
  The AFT T1/E1 cards will now provide perfect timing to zatpel/dahdi
  even when the ports are not connected. The free run interrupt
  will be enabled when all zaptel/dahdi ports are down, or on
  inital card start. To test this feature just start a wanpipe 
  port with zaptel/dahdi and run zttest. 
  A108 firmare V38 
  A104/2/1/ firmware V36       

- Added module inc cound when zaptel/dahdi starts.
  So wanpipe drivers do not crash if one tries to unload 
  zaptel/dahdi before stopping wanpipe drivers.

- Dahdi 2.2 Support
- Updated for 2.6.30 kernel


* Fri Jun 17 2009 Nenad Corbic <ncorbic@sangoma.com> - Feature Frozen - 3.4.4
=====================================================================

- Latest smg update in 3.4.3 broke BRI support
- This is now fixed.


* Fri Jun 17 2009 Nenad Corbic <ncorbic@sangoma.com> - Feature Frozen - 3.4.3
=====================================================================

- New firmware 
  2007-07-10: A108D Latest Firmware Release........ v37 . More info
  2007-07-10: A104D Latest Firmware Release ....... v35 . More info
  2007-07-10: A102D Latest Firmware Release ....... v35 . More info
  2007-07-10: A101D Latest Firmware Release. ...... v34 . More info
  2007-07-10: A200 Latest Firmware Release ........ v11 . More info
  2007-07-10: A400 Latest Firmware Release ........ v11 . More info
  2007-07-10: A104D Latest Firmware Release........ v26 . More info
  2007-07-10: A301-SH Latest Firmware Release...... v09 . More info
  2008-10-28: A14X Latest Firmware Release......... v07 . More Info
  2009-04-30: A500 Latest Firmware Release......... v35 . More info
  2009-05-08: B600 Latest Firmware Release......... v02 . More Info
  2009-07-03: B700 Latest Firmware Release......... v35 . More Info

- Updated Makefile Build
  make zaptel ZAPDIR=<absolute path to zaptel>  # Build for Zaptel
  make dahdi DAHDI_DIR=<absolute path to dahdi> # Build for DAHDI
  make install

  	or use original script method

  ./Setup install	# General Build
  ./Setup zaptel    # Zaptel specific build
  ./Setup dahdi     # Dahdi specific build

- For 64bit systems there is no need to use --64bit_4G flag any more.
  The check is done in the driver.

- Updated TDM ring buffer logic to battle noise and data
  corrupting in channelized voice mode.  Backport from 3.5.

- Updated for 2.6.29 kernel.

- Added DAHDI 2.2 support

- AFT TE3 Update
  Added a define AFT_TE3_IFT_FEATURE_DISABLE to disable
  the hardware feature that implements IFT timeout interrupt
  on DMA chains.  On some very slow machines this feature
  can yeild slower performance because the machine does
  not have to power to fill the T3/E3 line with traffic.

- Turned off Framer Timer Interrupt
  It was only used for stats and was not necessary

- Update for DMA chains to combat PCI IRQ latency

- Updated 64bit data protocols

- Updated smg_ctrl scripts for bri and ss7
  made them common

- Bug fix for Mixed Voice & Data mode
  On fifo overrun recovery do not disable dma if running in mixed
  voice & data mode.

- Updated to wancfg_zaptel script.
  Bug fix smg was prompting user even if bri was not installed.

- Updated SMG/Woomera
  Added Asterisk Load balancing using extension information.


* Fri Apr 30 2009 Nenad Corbic <ncorbic@sangoma.com> - Feature Frozen - 3.4.1
=======================================================================  

- Updated wancfg_zaptel configuration utility
  Use wancfg_fs to configure analog/BRI card for FreeSwitch
  Update for remove old start/stop script
  Added configuration option for TDMV_HW_FAX_DETECT
  Bug fixes in silent option

- A500 BRI - Firmware Update V35
  On some machines the A500 card caused PCI parity errors,
  causing a system reboot or a crash. The firmware V35
  fixes this problem.

- BRI DCHAN Bug Fix
  The DCHAN on some machines could become stuck due to a 
  driver interrupt race condition, causing BRI to stop
  reciving/tranmsitting calls. This bug has now been fixed.

- Updated BRI stack and sangoma_mdg      

- Added TDMV_HW_FAX_DETECT option
  To enable hardware fax detection. Used in conjunction with
  TDMV_HW_DTMF.  
  TDMV_HW_FAX_DETECT=<option>
   <option> = 0 or NO  - disabled
            = 1-8 or YES - enabled
		      The number represents fax tone timeout
		      value in seconds on call start. The fax
			  event will be valid for X about of seconds
			  at the start of the call. Default=8 seconds.
			  Option YES is equal to Default 8 seconds.
- Adsl Bug Fix
  Updated SNR reporting, fixes connecting issues on some lines.
  Updated stability on multiple restarts on some machines.

- Bug fix in chan_woomera on transfer
- Major regression testing of BRI/SS7 chan_woomera and sangoma_mgd
- Stable 500 Long Term Call load of Astersk+SMG/SS7
- Build update for latest 2.6.28 kernel.
- Build update for latest 2.6.29 kernel.
- Build update for latest Suse 


* Mon Mar 11 2009 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.16
=====================================================================

- Fixed S514 2 Byte receive

- A14X Serial Driver Update
  Added clock recovery option
  Added control of DTR/RTS on startup using 
  DTR_CTRL and RTS_CTRL config variables.

- AFT T1/E1 BERT Feature
  Ability to run Bert tests on T1/E1 cards using wanpipemon

- AFT HWEC
  Hardware DTMF detection now detects FAX tones as well,
  and passes then up to Zaptel/Asterisk

- PPP Protocol
  Added option to disable MAGIC number in negotiation
  Set MAGIC_DISABLE=YES in ppp profile section.

- Updates for new 2.6.X kernels
  Added queue check on shutdown
  Support for latest 2.6 kernels

- Updated T3/E3 Code
  The T3/E3 dma logic had a bug that was demonstrated under
  high load. Was introduced in 3.3.15 release.

- Added hardware probe DUMP 
  wanrouter hwprobe dump
  To be used by developers to easily parse the hwprobe output

- ADSL Update
  Bug fix in adsl. Could cause period disconnects on
  some equipment. The SNR value was not being properly reported
  the the upstream.


* Tue Dec 08 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.15
=====================================================================

- Added support for new B600 Analog Card

- Added task stop function on shutdown.
  This feature prevents an unlikely event that task gets
  scheduled after card has shutdown.

- Fixed tty driver for new Linux Kernel 2.6.26 and greater

- Fixed tristate mode using wanpipemon
  Used to enable/disable T1/E1 transmitter.
  wanpipemon -p aft for help.

- Fixed yellow alarm handler on T1/E1

- Added an option to run Zaptel & TDM API on same span.

- Fixed E3 Support for A301 cards.
  A301 cards need new Firmware v11 and CPLD v0.
  wanrouter hwprobe verbose shows current firmware and cpld version.
  
- Fixed Serial AFT card events
  Added X21 Support

- Added XMTP2 API for old PMC T1/E1 cards.

- Fixed Channel Bank ring problem on AFT T1/E1 cards.
  Some channel banks would ring all phones on card stop.
  This is now fixed.

- Updated AFT Core to fix XMTP2 startup with 16E1 links.

- Updated API for A14X Serial Cards
  Ability to receive Modem Events.

- Updated for Serial A14X API
  API for setting and receiving line events
  Sample code wanpipe-<version>/api/aft/aft_api_serial.c

- Updated Serial A14X for NRZI
  New firmware update v05

- Added wanrouter hwprobe dump
  Should be used for programs to parse hwprobe easily


* Tue Oct 8 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.14
=====================================================================

- Updated TDM API 
  This update helps FreeSwitch analog support.

- Analog TDM API TAPPING Feature
  Sangoma Analog cards and TDM API used in TAPPING mode can be used to build
  custom call recording applications

- Enabled Zaptel/DAHDI operation mode 
  This driver supports DAHDI/Asterisk
  Dahdi is supported on Asterisk 1.6 and 1.4
  Added dahdi to installation modes.
	-> ./Setup install 	#General installation
	-> ./Setup zaptel	#Zaptel based installation
	-> ./Setup dahdi	#Dahdi based installation
	-> ./Setup bri		#SMG BRI installation

- LibSS7 MTP2 Option
  The MTP2 option improves the performance of LibSS7
  eg: instead of using: dchan=24 use mtp2=24
  Please consult libss7 documentation.

- Updated TDM API for A200
  Fixes FreeSwitch OpenZAP with TDM API for Analog

- Bug fix in XMTP2 API
  The fifo error could cause xmtp2 buffering to fail

- wanpipemon utility
  Updated T1/E1 Loopback commands
  Bug fix on 2.6.25 kernels

- Driver compile update for Latest
  2.6.26 kernels

- Update AFT driver to implement new loopback commands

- Updated for 56K driver

- Added Asterisk DAHDI Support

- A200/A400 Analog driver update
  Bug fix possible race condition due to front end interrupt.

- AFT Core Update
  Disabled fifo overrun handling in transparent mode.
  It does not provide any improvement.
  XMTP2 API mode could run out of buffers due to overrun errors.
  This has now been fixed.

- Fixed A301 E3 Support
  New firmware V11 is needed.
  Please upgrade firmware before starting up the card.
  To check firmware run: wanrouter hwprobe
  

* Fri Sep 1 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.13
=====================================================================

- This release was never released.



* Fri Aug 1 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.12
=====================================================================

- Compilation fix for 2.6.25 kernel
  Introduced in 3.3.11 release

- Setup update
  Added bri and zaptel installation modes.
	-> ./Setup install 	#General installation
	-> ./Setup zaptel	#Zaptel based installation
	-> ./Setup bri		#SMG BRI installation

  Streamlined Setup installation script
  Added check for Asterisk directory when installing BRI
  Added check for x86 in architecture

- Fixed Makefile build

- Updated README File
  

* Wed Jul 30 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.11
=====================================================================

- BRI to Analog Faxing Update
  Improved faxing over SMG Gateway and Zaptel Analog.
  This feature drastically improves faxing performance between
  BRI and Analog cards over Asterisk.

- BRI Update
  Added overlap dialing support
  Added support for BRI with 16 channel Echo Canceler.
  Support for test start option, that only verifies configuration 
  files without applying changes.
  Support for non-syslog logging.

- BRI Bug Fix
  BRI was configured for answer on accept.  This caused a call to
  be answered before the remote leg of the call became connected.
 
- T1/E1 Loopback Update
  wanpipemon was update to check loopback status as well as set
  T1/E1 loopback commands.

- Removed S508 ISA Support from drivers

- Fixed the BRI chip security handler. Tested that all BRI 
  ports get properly shut down. 

- Updated for New PLX & Tundra PCIe Bridges chips
- Update hwprobe verbose for PMC Shark

- Updated for 2.6.25 kernel

- Fixed 64bit compilation for Octasic EC Image

- TDM API Bug Fix: 
  Added a check for buffer overflow in write function.

- New Octasic EC Image
  Improves faxing over Hardware Echo Canceler. 

- BRI Bug fix on startup
  The clock measuring function has limited and could fail on some machines.

- AFT T1/E1 - Added missing LBO configuration option WAN_T1_0_110

- Updated for 2.6.25 kernel

- Updated wancfg for interface MTU/MRU
  By default do not configure interface MTU/MRU if values have
  not changed by default.  This way the global mtu can be used
  to easily configure the MTU/MRU of all interfaces.

- Updated wancfg_smg to configure XMTP2 API

- Wireshark Tracing for MTP2
  http://wiki.sangoma.com/wanpipe-wireshark-pcap-pri-bri-wan-t1-e1-tracing

- T3/E3 Update
  Driver level update to improve code.

- MTP2 API Support
  http://wiki.sangoma.com/wanpipe-aft-ss7-mtp2-api


* Fri May 15 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.10
======================================================================

- BRI Update
  Binary segfaulted on some systems due to gcc incompabilitly.
  This is now fixed.

* Fri Apr 30 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.9
======================================================================

- AFT BRI Clock Reference Update
  Disabled reference clocking feature on original A500 BRI Cards.
  It has been determined that reference clocking feature is not always stable 
  on original A500 BRI cards.  It can cause noise and call drops in some 
  circumstances where BRI lines go up and down due to power saving mode.
  
  If you have problems with FAX synchronization on an original A500 BRI Card
  contact Sangoma Support and we will swap out the card for one with
  an updated CPLD that will work with the reference clock and provide
  reliable FAXing.

  Run wanrouter hwprobe verbose to determine your A500 BRI CPLD Version
   -> wanrouter hwprobe verbose
	-C00 -> old bri cpld (non reference clock)
	-C01 -> new bri cpld (reference clock enabled)


- Manually Disabling BRI Clock Reference
  This option is valid from 3.3.6 release and greater.
  This option can be used in case of noise and voice quality issues
  and call drop issues on BRI card.  

  In order to disable BRI clock reference manually one can add
  RM_BRI_CLOCK=OSC in each BRI wanpipe config file in /etc/wanpipe directory.
  1. vi /etc/wanpipe/wanpipe1.conf
  2. Under the TDMV_SPAN option add
     RM_BRI_CLOCK=OSC
  3. Save file
  4. Repeat for all BRI wanpipe configuration files
  5. Restart all wanpipe devices
  
  Note from 3.3.9 release on, all old CPLD A500 BRI cards
  have clock referencing disabled automatically.

- Updated BRI Stack
  Fix for RDNIS not cleared
  Support for show_spans and show_calls.
  Added support for multiple MSNs.
  Added support for timer_t3 and timer_t302
  http://wiki.sangoma.com/sangoma-wanpipe-smg-asterisk-bri-installation
  

* Fri Apr 25 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.8
======================================================================

- AFT Update
  A bug was introduced in 3.3.7 release that failed to load
  AFT cards without HWEC.  

- AFT TE1 Code
  Defaulted Maxim T1 Rx Level to 36DB
  Defaulted Maxim E1 Rx Level to 42DB
  This will improve T1/E1 connectivity on noisy or low power lines.

- AFT BRI Update
  Minor update on BRI to conform better to TBR3 Certification 


* Wed Apr 23 2008 Nenad Corbic <ncorbic@sangoma.com> - Beta - 3.3.7
======================================================================

- BRI HWEC MAJOR Bug Fix
  BRI hwec was not configured properly on startup.
  Every second channel on each span was not being configured for hwec.
  This would result in random echo problems.

- Support for 2.6.24 kernels and up
  This release will now compile properly on 2.6.24 kernels and up.

- AFT Analog Update
  Added TBR21 operation mode
  Used for most european countries.
  -> this option must be added manually in wanpipe#.conf
  -> TDMV_OPERMODE=TBR21

- AFT A144 Update
  Added MPAPI X25 support to AFT A144/A142 cards
  Use: wancfg_legacy to configure MPAPI over AFT cards.
  http://wiki.sangoma.com/wanpipe-linux-mpapi-x25

- BRI Updated
  BRI driver updated for new 512hz clock used to improve
  hardware echo canceler with clock recovery mechanism.
  This featue will solve any random hwec warning messages
  one might see in /var/log/messages.  Note these random
  warning messages did not cause any abnormal behaviour 
  in extensive lab testings.   
  To check your BRI hardware version run:	
  -> wanrouter hwprobe verbose
	-C00 -> old bri cpld
	-C01 -> new bri cpld

- Wanpipemon PRI/BRI PCAP Tracing for Wireshark
  Using wanpipemon dchan trace one can now capture
  pcap files that can be opened by Wireshark.
  http://wiki.sangoma.com/wanpipe-wireshark-pcap-pri-bri-wan-t1-e1-tracing

- S514 Secondary port bug fix
  The secondary port was not working.

- Updated wanrouter hwprobe
  New wanrouter hwprobe device summary line will only contain
  found devices. For backward compatibility we created "wanrouter hwprobe legacy"
  that can be used to revert hwprobe output to the original format.

- Add pci parity check to wanrouter 
  wanrouter parity  	-> displays current system pci parity
  wanrouter parity off 	-> disables system pci parity
  wanrouter parity on	-> enables system pci parity
  
  /etc/wanpipe/wanrouter.rc  
	WAN_PCI_PARITY=OFF -> on wanrouter start disable pci parity
			   -> event logged in /var/log/wanrouter

  On some servers pci parity can cause NMI interrupts that
  can lead to reboots.  Parity can be caused by unsuported
  or buggy pci/bridge chipsets.  The above commands can be used
  to combat pci parity reboots.

  Another option is to disable PCI parity in BIOS :)


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
    TDMV_DUMMY_REF=YES in [wanpipe1] section of wanpipe1.conf

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
- - END - 
