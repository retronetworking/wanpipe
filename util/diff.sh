#!/bin/sh


diff -dur "$1" "$2" --exclude=*.svn --exclude=*.mod --exclude=*.cmd --exclude=libsangoma* --exclude=sangoma_mgd* > diff
