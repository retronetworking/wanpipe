#class boostbrispan
#for freeswitch isdn bri/pri boost span configuraton 

package boostspan;
#use warnings;
#use diagnostics;
use strict;

#constructor
sub new	{
	my ($class) = @_;
	my $self = {
		_span_type => undef,
		_span_name => 'OpenZap',
		_span_no   => undef,
		_chan_no   => undef,
		_trunk_type => undef,		
	};			
	bless $self, $class;
    	return $self;
}

sub span_type {
	            my ( $self, $span_type ) = @_;
		                    $self->{_span_type} = $span_type if defined($span_type);
				                        return $self->{_span_type};
}

sub span_no {
	            my ( $self, $span_no ) = @_;
		                    $self->{_span_no} = $span_no if defined($span_no);
				                        return $self->{_span_no};
}
sub span_name {
	            my ( $self, $span_name ) = @_;
		                    $self->{_span_name} = $span_name if defined($span_name);
				                        return $self->{_span_name};
}
sub chan_no {
	            my ( $self, $chan_no ) = @_;
		                    $self->{_chan_no} = $chan_no if defined($chan_no);
				                        return $self->{_chan_no};
}
sub trunk_type {
	            my ( $self, $trunk_type ) = @_;
		                    $self->{_trunk_type} = $trunk_type if defined($trunk_type);
				                        return $self->{_trunk_type};
}

sub print {
    my ($self) = @_;
    printf (" span_name: %s\n span_type: %s\n span_no: %s\n chan_no:  %s \ntrunk type: %s \n", $self->span_name, $self->span_type, $self->span_no, $self->chan_no, $self->trunk_type);

}

1;
