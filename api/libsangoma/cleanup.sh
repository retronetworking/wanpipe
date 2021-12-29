#!/bin/sh

svn status | grep -v "libsangoma.*" | xargs rm -rf     
rm -rf sample_c/regression
rm -f *.a
rm -f *.la
rm -f *.lo
