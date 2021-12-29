#class A20x
#for A200 series cards

package A20x;
use Card;
use strict;

#constructor
sub new	{
	my ($class) = @_;
	my $self = {
		_card      => undef,		
		_tdm_opermode => 'FCC',
		_tdm_law => 'MULAW',
		_analog_modules => undef,
	};			
	bless $self, $class;
    	return $self;
}

sub card {
	    my ( $self, $card ) = @_;
	        $self->{_card} = $card if defined($card);
		return $self->{_card};
}

sub tdm_opermode {
	            my ( $self, $tdm_opermode ) = @_;
		                    $self->{_tdm_opermode} = $tdm_opermode if defined($tdm_opermode);
				                        return $self->{_tdm_opermode};
}

sub tdm_law {
	            my ( $self, $tdm_law ) = @_;
		                    $self->{_tdm_law} = $tdm_law if defined($tdm_law);
				                        return $self->{_tdm_law};
}

sub analog_modules {
	            my ( $self, $analog_modules ) = @_;
		                    $self->{_analog_modules} = $analog_modules if defined($analog_modules);
				                        return $self->{_analog_modules};
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

}

sub gen_wanpipe_conf{
	my ($self) = @_;
	my $wanpipe_conf_template = $self->card->current_dir."/templates/wanpipe.tdm.a200";
	my $wanpipe_conf_file = $self->card->current_dir."/".$self->card->cfg_dir."/wanpipe".$self->card->span_no.".conf";
		
	my $span_no = $self->card->span_no;
	my $pci_slot = $self->card->pci_slot;
	my $pci_bus = $self->card->pci_bus;
	my $tdm_law = $self->tdm_law;
	my $tdm_opermode = $self->tdm_opermode;
	my $hwec_mode = $self->card->hwec_mode;

	open(FH, $wanpipe_conf_template) or die "Can open $wanpipe_conf_template";
	my $wp_file='';
       	while (<FH>) {
       		$wp_file .= $_;
	}
	close (FH);
	open(FH, ">$wanpipe_conf_file") or die "Cant open $wanpipe_conf_file";
        $wp_file =~ s/DEVNUM/$span_no/g;
        $wp_file =~ s/SLOTNUM/$pci_slot/g;
        $wp_file =~ s/BUSNUM/$pci_bus/g;
        $wp_file =~ s/TDM_LAW/$tdm_law/g;
        $wp_file =~ s/TDM_OPERMODE/$tdm_opermode/g;
        $wp_file =~ s/HWECMODE/$hwec_mode/g;
	
	print FH $wp_file;
	close (FH);

# print "\n created $fname for A$card_model $devnum SLOT $slot BUS $bus HWEC $hwec_mode\n";
}
sub gen_zaptel_conf{
	my ($self, $channel, $type) = @_;
	my $zp_file='';
	if ( $type eq 'fxo'){
		#this is an FXS module
		$zp_file.="fxoks=$channel\n";
	}else{
		$zp_file.="fxsks=$channel\n";
	}
	return $zp_file;	
}
sub gen_zapata_conf{
	my ($self, $channel, $type) = @_;
	my $zp_file='';
	$zp_file.="context=".$self->card->zap_context."\n";
	$zp_file.="group=".$self->card->zap_group."\n";
		
	if ( $type eq 'fxo'){
		#this is an FXS module
	        $zp_file.="signalling = fxo_ks\n";
        }else{
                $zp_file.="signalling = fxs_ks\n";
        }
	$zp_file.="channel => $channel\n\n";
        return $zp_file;
}

1;
