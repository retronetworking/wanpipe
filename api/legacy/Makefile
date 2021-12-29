# ============================================================================
# Makefile	Make script for building Linux WAN router utilities.
# ----------------------------------------------------------------------------
# Copyright (c) 1995-2000 Sangoma Technologies Inc.  All Rights Reserved.
# ============================================================================

APIINC=/usr/include/wanpipe

####### RULES ################################################################

all:	
	make -C x25 all  APIINC=$(APIINC)
	make -C chdlc all  APIINC=$(APIINC)
	make -C fr all  APIINC=$(APIINC)
	make -C bitstrm all APIINC=$(APIINC)
	make -C bisync all APIINC=$(APIINC)
	make -C aft all APIINC=$(APIINC)

clean:
	make -C x25 clean 
	make -C chdlc clean 
	make -C fr clean 
	make -C bitstrm clean
	make -C aft clean
