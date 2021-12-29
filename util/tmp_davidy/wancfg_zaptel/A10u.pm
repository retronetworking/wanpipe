#class A10u
#for A101/2/u/c series cards

package A10u;
use Card;
use strict;

#constructor
sub new	{
	my ($class) = @_;
	my $self = {
		_card      => undef,		
 		_fe_line   => undef,
		_fe_media  => 'T1',
		_fe_lcode  => 'B8ZS',
		_fe_frame  => 'ESF',
		_fe_clock  => 'NORMAL',
		_te_sig_mode => undef,
		_te_ref_clock  => '0',
		_signalling => 'PRI_CPE',
		_pri_switchtype => 'national',
		_hw_dchan  => '0',
		_ss7_sigchan => undef,
		_ss7_option => undef,
		_ss7_tdmchan => undef,
		_ss7_subinterface => undef,
		_ss7_tdminterface => undef,
	};
	bless $self, $class;
    	return $self;
}

sub card {
	    my ( $self, $card ) = @_;
	        $self->{_card} = $card if defined($card);
		return $self->{_card};
}
sub fe_line {
	    my ( $self, $fe_line ) = @_;
	        $self->{_fe_line} = $fe_line if defined($fe_line);
		    return $self->{_fe_line};
}
sub te_ref_clock {
	    my ( $self, $te_ref_clock ) = @_;
	        $self->{_te_ref_clock} = $te_ref_clock if defined($te_ref_clock);
		    return $self->{_te_ref_clock};
}
sub signalling {
	    my ( $self, $signalling ) = @_;
	    $self->{_signalling} = $signalling if defined($signalling);
	    return $self->{_signalling};
}

sub fe_media {
	    my ( $self, $fe_media ) = @_;
	        $self->{_fe_media} = $fe_media if defined($fe_media);
		    return $self->{_fe_media};
}

sub fe_lcode {
	    my ( $self, $fe_lcode) = @_;
	        $self->{_fe_lcode} = $fe_lcode if defined($fe_lcode);
		    return $self->{_fe_lcode};
}

sub te_sig_mode {
	   my ( $self, $te_sig_mode ) = @_;
	        $self->{_te_sig_mode} = $te_sig_mode if defined($te_sig_mode);
		    return $self->{_te_sig_mode};
}


sub fe_frame {
	    my ( $self, $fe_frame ) = @_;
	        $self->{_fe_frame} = $fe_frame if defined($fe_frame);
		    return $self->{_fe_frame};
}

sub fe_clock {
	   my ( $self, $fe_clock ) = @_;
	        $self->{_fe_clock} = $fe_clock if defined($fe_clock);
		    return $self->{_fe_clock};
}

sub pri_switchtype {
	   my ( $self, $pri_switchtype ) = @_;
	        $self->{_pri_switchtype} = $pri_switchtype if defined($pri_switchtype);
		    return $self->{_pri_switchtype};
}

sub ss7_option {
          my ( $self, $ss7_option ) = @_;
                $self->{_ss7_option} = $ss7_option if defined($ss7_option);
                 return $self->{_ss7_option};     
}

sub ss7_sigchan {
          my ( $self, $ss7_sigchan ) = @_;
                $self->{_ss7_sigchan} = $ss7_sigchan if defined($ss7_sigchan);		
	        return $self->{_ss7_sigchan};     
}

sub ss7_tdmchan {
          my ( $self, $ss7_tdmchan ) = @_;
                $self->{_ss7_tdmchan} = $ss7_tdmchan if defined($ss7_tdmchan);		
	        return $self->{_ss7_tdmchan};     
}

sub ss7_subinterface {
          my ( $self, $ss7_subinterface ) = @_;
                $self->{_ss7_subinterface} = $ss7_subinterface if defined($ss7_subinterface);		
	        return $self->{_ss7_subinterface};     
}

sub ss7_tdminterface {
	  my ( $self, $ss7_tdminterface ) = @_;
                $self->{_ss7_tdminterface} = $ss7_tdminterface if defined($ss7_tdminterface);		
	        return $self->{_ss7_tdminterface};   
}

sub prompt_user{
	my($promptString, $defaultValue) = @_;
	if ($defaultValue) {
		print $promptString, "[", $defaultValue, "]: ";
	} else {
		print $promptString, ": ";
	}
	$| = 1;               # force a flush after our print
	$_ = <STDIN>;         # get the input from STDIN (presumably the keyboard)
	chomp;
	if ("$defaultValue") {
		return $_ ? $_ : $defaultValue;    # return $_ if it has a value
	} else {
		return $_;
	}
}

sub prompt_user_list{
	my @list = @_;
	my $i;
	my $valid = 0;
	for $i (0..$#list) {
		printf(" %s\. %s\n",$i+1, @list[$i]);
	}
	while ($valid == 0){
		$| = 1;               # force a flush after our print
		$_ = <STDIN>;         # get the input from STDIN (presumably the keyboard)
		chomp;
			if ( $_ =~ /(\d+)/ ){
			if ( $1 > $#list+1) {
				print "Invalid option: Value out of range \n";
			} else {
			return $1-1 ;
			}
		} else {
			print "Invalid option: Input an integer\n";
		}
	}
}

sub print {
	my ($self) = @_;
	$self->card->print();    
	printf (" fe_line: %s\n fe_media: %s\n fe_lcode: %s\n fe_frame: %s \n fe_clock:%s\n ", $self->fe_line, $self->fe_media, $self->fe_lcode, $self->fe_frame, $self->fe_clock);

}

sub gen_wanpipe_ss7_subinterfaces{
        my ($self) = @_;
	my $wanpipe_ss7_conf_file = $self->card->current_dir."/".$self->card->cfg_dir."/wanpipe".$self->card->device_no.".conf";
	my $ss7_sigchan = $self->ss7_sigchan;
	my $tdmv_span_no = $self->card->tdmv_span_no;
	my $device_no = $self->card->device_no;
	my $ss7_subinterface = $self->ss7_subinterface;
	my $ss7_tdmchan = $self->ss7_tdmchan;
	my $hwec_mode = $self->card->hwec_mode;
	my $ss7_tdminterface = $self->ss7_tdminterface;
	my $wanpipe_ss7_interfaces_template = $self->card->current_dir."/templates/ss7_a10u/wanpipe.ss7.$ss7_subinterface";

	open(FH, $wanpipe_ss7_interfaces_template) or die "Can't open $wanpipe_ss7_interfaces_template";
	my $wp_file='';
       	while (<FH>) {
       		$wp_file .= $_;
	}
	close (FH);

	open(FH, ">>$wanpipe_ss7_conf_file") or die "Cant open $wanpipe_ss7_conf_file";
	$wp_file =~ s/DEVNUM/$device_no/g;
	$wp_file =~ s/SS7SIGCHAN/$ss7_sigchan/g;
	$wp_file =~ s/TDMVOICECHAN/$ss7_tdmchan/g;
	$wp_file =~ s/VOICEINTERFACE/$ss7_tdminterface/g;
	$wp_file =~ s/TDMVSPANNO/$tdmv_span_no/g;


	print FH $wp_file;
	close (FH);
}

sub gen_wanpipe_conf{
	my ($self) = @_;
	my $wanpipe_conf_template = $self->card->current_dir."/templates/wanpipe.tdm.a10u";
	my $wanpipe_conf_file = $self->card->current_dir."/".$self->card->cfg_dir."/wanpipe".$self->card->device_no.".conf";
	my $tdmv_span_no = $self->card->tdmv_span_no;
	my $device_no = $self->card->device_no;
	my $pci_slot = $self->card->pci_slot;
	my $pci_bus = $self->card->pci_bus;
	my $fe_media = $self->fe_media;
	my $fe_lcode = $self->fe_lcode;
	my $fe_frame = $self->fe_frame;
	my $fe_line = $self->fe_line;
	my $te_sig_mode = $self->te_sig_mode;
	my $fe_clock = $self->fe_clock;
	my $ss7_option = $self->ss7_option;
	my $dchan = 0;
	my $fe_lbo;
       	my $fe_cpu;

	my $te_sig_mode_line='';


	if ($ss7_option == 1){
	        $wanpipe_conf_template = $self->card->current_dir."/templates/ss7_a10u/wanpipe.ss7.4";
	} elsif ($ss7_option == 2){
                $wanpipe_conf_template = $self->card->current_dir."/templates/ss7_a10u/wanpipe.tdmvoiceapi.a10u";
	}

        if ($self->fe_line eq '1'){
		$fe_cpu='A';
	}elsif($self->fe_line eq '2'){
		$fe_cpu='B';
	}else{
		print "Error: Invalid port on A101-2u\n";
		exit 1;
       	}

        if ($self->fe_media eq 'T1'){
		if ($self->signalling eq 'PRI CPE' | $self->signalling eq 'PRI NET'){
			  $te_sig_mode = '';	
			  $dchan = 24;
		}
		$fe_lbo='0DB';
	}else{
		if ($self->signalling eq 'PRI CPE' | $self->signalling eq 'PRI NET'){
			  $dchan = 16;
		}
		$fe_lbo='120OH';
	}

	
	if ($te_sig_mode eq ''){
		$te_sig_mode_line='';
	}else{
		$te_sig_mode_line= 'TE_SIG_MODE     = '.$te_sig_mode;
	}


	open(FH, $wanpipe_conf_template ) or die "Cannot open $wanpipe_conf_template";
	my $wp_file='';
       	while (<FH>) {
       		$wp_file .= $_;
	}
	close (FH);
	open(FH, ">>$wanpipe_conf_file") or die "Cant open $wanpipe_conf_file";
	$wp_file =~ s/DEVNUM/$device_no/g;
        $wp_file =~ s/SLOTNUM/$pci_slot/g;
        $wp_file =~ s/BUSNUM/$pci_bus/g;
        $wp_file =~ s/FEMEDIA/$fe_media/g;
        $wp_file =~ s/FELCODE/$fe_lcode/g;
	$wp_file =~ s/TESIGMODE/$te_sig_mode_line/g;
        $wp_file =~ s/FEFRAME/$fe_frame/g;
        $wp_file =~ s/FECPU/$fe_cpu/g;
        $wp_file =~ s/FECLOCK/$fe_clock/g;
        $wp_file =~ s/FELBO/$fe_lbo/g;
        $wp_file =~ s/TDMVDCHAN/$dchan/g;
	$wp_file =~ s/TDMVSPANNO/$tdmv_span_no/g;

	print FH $wp_file;
	close (FH);

# print "\n created $fname for A$card_model $devnum SLOT $slot BUS $bus HWEC $hwec_mode\n";
}
sub gen_zaptel_conf{
	my ($self, $dchan_str) = @_;
	my $zap_lcode;
	my $zap_frame;
	my $zap_crc4;

        if ( $self->fe_lcode eq 'B8ZS' ){
		$zap_lcode='b8zs';
	} elsif ( $self->fe_lcode eq 'AMI' ){
		$zap_lcode='ami';
	} elsif ( $self->fe_lcode eq 'HDB3' ){
		$zap_lcode='hdb3';
	} else {
        	printf("Error: invalid line coding %s\n", $self->fe_lcode);
		exit;
	}

        if ( $self->fe_frame eq 'ESF' ){
		$zap_frame='esf';
	} elsif ( $self->fe_frame eq 'D4' ){
		$zap_frame='d4';
	} elsif ( $self->fe_frame eq 'CRC4' ){
		$zap_frame='ccs';
		$zap_crc4=',crc4';
	} elsif ( $self->fe_frame eq 'NCRC4' ){
		$zap_frame='ccs';
	} else {
        	printf("Error: invalid line framing %s\n", $self->fe_frame);
	   	exit;
	}

 	my $zp_file='';

	$zp_file.="\n\#Sangoma A".$self->card->card_model." port ".$self->fe_line." [slot:".$self->card->pci_slot." bus:".$self->card->pci_bus." span:".$self->card->tdmv_span_no."] <wanpipe".$self->card->device_no.">\n";
	
	
       # $zp_file.="\n\#Sangoma A".$self->card->card_model." port ".$self->fe_line." [slot:".$self->card->pci_slot." bus:".$self->card->pci_bus." span: ".$self->card->span_no."]\n";
	$zp_file.="span=".$self->card->tdmv_span_no.",0,0,".$zap_frame.",".$zap_lcode.$zap_crc4."\n";	
	
	if ( $self->signalling eq 'PRI NET' | $self->signalling eq 'PRI CPE' ){
        	if ( $self->fe_media eq 'T1' ){
                	$zp_file.="bchan=".$self->card->first_chan."-".($self->card->first_chan+22)."\n"; 
			$zp_file.=$dchan_str."=".($self->card->first_chan+23)."\n";
		} else {
                	$zp_file.="bchan=".$self->card->first_chan."-".($self->card->first_chan+14).",".($self->card->first_chan+16)."-".($self->card->first_chan+30)."\n";
			$zp_file.=$dchan_str."=".($self->card->first_chan+15)."\n";
		}  
	} else {
		my $zap_signal;
        	if ( $self->signalling eq 'E & M' | $self->signalling eq 'E & M Wink' ){
			$zap_signal='e&m';
		} elsif ( $self->signalling eq 'FXS - Loop Start' ){
			$zap_signal='fxsls';
		} elsif ( $self->signalling eq 'FXS - Ground Start' ){
			$zap_signal='fxsgs';
		} elsif ( $self->signalling eq 'FXS - Kewl Start' ){
			$zap_signal='fxsks';
		} elsif ( $self->signalling eq 'FX0 - Loop Start' ){
			$zap_signal='fxols';
		} elsif ( $self->signalling eq 'FX0 - Ground Start' ){
			$zap_signal='fxogs';
		} elsif ( $self->signalling eq 'FX0 - Kewl Start' ){
			$zap_signal='fxoks';
		} else {
                 	printf("Error: invalid signalling %s\n", $self->card->signalling);
		}
		
        	if ( $self->fe_media eq 'T1' ){
                	$zp_file.=$zap_signal."=".$self->card->first_chan."-".($self->card->first_chan+23)."\n"; 
		} else {
                	$zp_file.=$zap_signal."=".$self->card->first_chan."-".($self->card->first_chan+30)."\n"; 
		}
	}
	return $zp_file;	
	
}

sub gen_zapata_conf{
	my ($self) = @_;
        
 	my $zp_file='';

	$zp_file.="\n\;Sangoma A".$self->card->card_model." port ".$self->fe_line." [slot:".$self->card->pci_slot." bus:".$self->card->pci_bus." span:".$self->card->tdmv_span_no."] <wanpipe".$self->card->device_no.">\n";

#	$zp_file.="\n\;Sangoma A".$self->card->card_model." port ".$self->fe_line." [slot:".$self->card->pci_slot." bus:".$self->card->pci_bus." span: ".$self->card->span_no."]\n";

	if ( $self->signalling eq 'PRI NET' | $self->signalling eq 'PRI CPE' ){
		$zp_file.="switchtype=".$self->pri_switchtype."\n";
	}	
       
	$zp_file.="context=".$self->card->zap_context."\n";
       	$zp_file.="group=".$self->card->zap_group."\n";
       	
	if ( $self->signalling eq 'PRI NET' ){
		$zp_file.="signalling=pri_net\n";
	} elsif ( $self->signalling eq 'PRI CPE' ){
		$zp_file.="signalling=pri_cpe\n";
	} elsif ( $self->signalling eq 'E & M' ){
		$zp_file.="signalling=em\n";
       	} elsif ($self->signalling eq 'E & M Wink' ){
		$zp_file.="signalling=em_w\n";
	} elsif ( $self->signalling eq 'FXS - Loop Start' ){
		$zp_file.="signalling=fxs_ls\n";
	} elsif ( $self->signalling eq 'FXS - Ground Start' ){
		$zp_file.="signalling=fxs_gs\n";
	} elsif ( $self->signalling eq 'FXS - Kewl Start' ){
		$zp_file.="signalling=fxs_ks\n";
	} elsif ( $self->signalling eq 'FX0 - Loop Start' ){
		$zp_file.="signalling=fxo_ls\n";
	} elsif ( $self->signalling eq 'FX0 - Ground Start' ){
		$zp_file.="signalling=fxo_gs\n";
	} elsif ( $self->signalling eq 'FX0 - Kewl Start' ){
		$zp_file.="signalling=fxo_ks\n";
	} else {
               	printf("Error: invalid signalling %s\n", $self->signalling);
	}
		
       	if ( $self->signalling eq 'PRI NET' | $self->signalling eq 'PRI CPE' ){
              	if ( $self->fe_media eq 'T1' ){
               		$zp_file.="channel => ".$self->card->first_chan."-".($self->card->first_chan+22)."\n"; 
		}else{
               		$zp_file.="channel => ".$self->card->first_chan."-".($self->card->first_chan+14).",".($self->card->first_chan+16)."-".($self->card->first_chan+30)."\n"; 
		}
	} else {
              	if ( $self->fe_media eq 'T1' ){
               		$zp_file.="channel => ".$self->card->first_chan."-".($self->card->first_chan+23)."\n"; 
		}else{
               		$zp_file.="channel => ".$self->card->first_chan."-".($self->card->first_chan+30)."\n"; 
		}
	
	}
	return $zp_file;	
	
}

1;
