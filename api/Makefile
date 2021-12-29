# ============================================================================
# Makefile	Make script for building Linux WAN router utilities.
# ----------------------------------------------------------------------------
# Copyright (c) 1995-2000 Sangoma Technologies Inc.  All Rights Reserved.
# ============================================================================

APIINC=/usr/include/wanpipe
LIB_INSTALL_DIR=/usr

####### RULES ################################################################

all:	
	make -C sample_c
	make -C sample_cpp
	make -C tdm_api
	make -C legacy

clean:
	make -C sample_c clean
	make -C sample_cpp clean
	make -C tdm_api clean 
	make -C legacy clean 
	make -C lib/hdlc clean
