#!/bin/sh


diff -dur "$1" "$2" --exclude=*.svn --exclude=libsangoma* --exclude=sangoma_mgd* > diff
