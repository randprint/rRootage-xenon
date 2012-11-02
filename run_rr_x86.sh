#!/bin/sh
export LD_LIBRARY_PATH=$(dirname $0)/x86libs
exec ./rr

