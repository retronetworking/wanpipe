#=============================================================================
#
#    File: oct6100api.mak    ($Revision: 1.1 $)
#
#    Description:  Makefile for building the OCT6100 API library.
#
#    $Octasic_Confidentiality: $
#
#    $Octasic_Release: $
#
#    Copyright (c) 2005 Octasic Inc. All rights reserved.
#
#=============================================================================

OBJPATH = 

OUTPATH = ../../../lib/

OUTFILE = $(OUTPATH)liboct6100api.a

PWD	:=  $(shell cd)

VPATH	= 	../../octdeviceapiw/oct6100_apiw_linux \
		../../octdeviceapi/oct6100api/oct6100_api \
		../../octdeviceapi/oct6100api/oct6100_apimi \
		../../apilib/bt ../../apilib/largmath ../../apilib/llman 
			
MAKEFILENAME = oct6100api.mak

# Build list of all source files to compile.
SRC1	= $(notdir $(foreach dir, $(VPATH), $(wildcard $(dir)/*.c)))

# Remove template oct6100_user.c to replace with linux one.
SRC	= $(patsubst ../../octdeviceapi/oct6100api/oct6100_api/oct6100_user.c,,$(SRC1))

obj	= $(patsubst %.c,%.o, $(notdir $(SRC)))
OBJ	= $(addprefix $(OBJPATH), $(obj))

AR	= ar
CC	= gcc
RM	= rm
LIST	= echo

DEBUG	= -g -O0

INC = 	-I../../include \
	-I../../include/oct6100api \
	-I../../include/apilib \
	-I../../octdeviceapi/oct6100api \
	-I/usr/include/ -I../../../

CCFLAGS		= -m32 -DDEVICE_IOCTL -DWAN_EC_USER -D__LINUX__ -L/usr/local/lib \
		-fPIC -ansi -Wall -Wpointer-arith -Winline -fno-builtin \
		-fno-defer-pop -D_REENTRANT -D_GNU_SOURCE \
		-include /usr/include/pthread.h \
		$(DEBUG) $(INC)
	#	-fno-defer-pop -fno-rtti -fvolatile -D_REENTRANT -D_GNU_SOURCE \
			
CCFLAGS2	=   -D__LINUX__ -L/usr/local/lib -MM $(INC)

# OUTDEP = Executed to create depedencies
OUTDEP		= | sed "1s/^/\$$(OBJPath)/" > $(objpath)$(subst .c,.d,$(notdir $<))

CHCKMKDIR	=	@(if [ -d $(dir $@) ]; then :; else $(MKDIR) $(dir $@); fi)
			
.PHONY		:   all clean rebuild

ECHOSTR =	Linking and building archive...
AR_LD	= 	$(AR) -rcs $@ $(OBJ) 

# -------------------------- RULES -----------------------------
# When there isn't any target given to make, make will automatically execute the first
# rule it detects. In our case, the "all" rule.

all		: $(OUTFILE)

clean		:
		- @$(RM) $(OBJPATH)*.o 
		- @$(RM) $(OUTFILE)

rebuild 	:
		    $(MAKE) -f $(MAKEFILENAME) clean
		    $(MAKE) -f $(MAKEFILENAME) all

# Once we have all .o files, we can create the .a
$(OUTFILE)	:	$(OBJ) $(MAKEFILENAME)
				@echo $(ECHOSTR) $@
				@$(CHCKMKDIR)
				@$(AR_LD) 
		
#Third Rule : Make an object file from the current source file. This object file 
#              is created in the $(OBJPATH) directory.

$(OBJPATH)%.o : %.c
			@echo compiling : $<
			@$(CC) -c $(CCFLAGS2) $< $(OUTDEP)
			@$(CC) -c $(CCFLAGS) $< -o $@
		 	
#
# Dependencies
#
		 	
DEPS := $(wildcard $(OBJPath)*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
	 	

# *****************************************************************
