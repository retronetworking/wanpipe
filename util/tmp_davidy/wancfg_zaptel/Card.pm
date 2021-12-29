package Card;
use strict;

#constructor
sub new {
    my ($class) = @_;
    my $self = {
	_current_dir => undef,
	_cfg_dir => undef,
	_device_no => undef,
	_tdmv_span_no => undef,
        _card_model => undef,
        _pci_slot  => undef,
        _pci_bus   => undef,
	_fe_cpu	   => 'A',
	_hwec_mode => 'NO',
	_hw_dtmf => 'NO',
	_first_chan => '0',
	_zap_context => undef,
      	_zap_group => undef 	
    };
    bless $self, $class;
    return $self;
}

sub device_no {
    my ( $self, $device_no ) = @_;
    $self->{_device_no} = $device_no if defined($device_no);
    return $self->{_device_no};
}

sub tdmv_span_no {
    my ( $self, $tdmv_span_no ) = @_;
    $self->{_tdmv_span_no} = $tdmv_span_no if defined($tdmv_span_no);
    return $self->{_tdmv_span_no};
}

sub card_model {
    my ( $self, $card_model ) = @_;
    $self->{_card_model} = $card_model if defined($card_model);
    return $self->{_card_model};
}

sub pci_slot {
    my ( $self, $pci_slot ) = @_;
    $self->{_pci_slot} = $pci_slot if defined($pci_slot);
    return $self->{_pci_slot};
}

sub pci_bus {
    my ( $self, $pci_bus ) = @_;
    $self->{_pci_bus} = $pci_bus if defined($pci_bus);
    return $self->{_pci_bus};
}

sub fe_cpu {
    my ( $self, $fe_cpu ) = @_;
    $self->{_fe_cpu } = $fe_cpu if defined($fe_cpu);
    return $self->{_fe_cpu};
}

sub hwec_mode {
    my ( $self, $hwec_mode ) = @_;
    $self->{_hwec_mode} = $hwec_mode if defined($hwec_mode);
    return $self->{_hwec_mode};
}

sub hw_dtmf {
    my ( $self, $hw_dtmf ) = @_;
    $self->{_hw_dtmf} = $hw_dtmf if defined($hw_dtmf);
    return $self->{_hw_dtmf};
}

sub signalling {
    my ( $self, $signalling ) = @_;
    $self->{_signalling} = $signalling if defined($signalling);
    return $self->{_signalling};
}

sub first_chan {
    my ( $self, $first_chan ) = @_;
    $self->{_first_chan} = $first_chan if defined($first_chan);
    return $self->{_first_chan};
}

sub zap_context {
    my ( $self, $zap_context ) = @_;
    $self->{_zap_context} = $zap_context if defined($zap_context);
    return $self->{_zap_context};
}

sub zap_group {
    my ( $self, $zap_group ) = @_;
    $self->{_zap_group} = $zap_group if defined($zap_group);
    return $self->{_zap_group};
}

sub current_dir {
    my ( $self, $current_dir ) = @_;
    $self->{_current_dir} = $current_dir if defined($current_dir);
    return $self->{_current_dir};
}

sub cfg_dir {
    my ( $self, $cfg_dir ) = @_;
    $self->{_cfg_dir} = $cfg_dir if defined($cfg_dir);
    return $self->{_cfg_dir};
}

sub print {
    my ($self) = @_;
    printf (" span_no: %s\n card_model: %s\n pci_slot: %s\n pci_bus:  %s\n hwec_mode: %s\n signalling %s\n first_chan: %s\n", $self->span_no, $self->card_model, $self->pci_slot, $self->pci_bus, $self->hwec_mode, $self->signalling, $self->first_chan);

}


1;