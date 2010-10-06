#!/bin/sh
# Copyright (C) 2003 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General
# Public License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA 02111-1307 USA

usage () {
    echo "" >&2
    echo "usage: "`basename $0`" -k KERNEL_SOURCE_DIRECTORY -t [VERSION, PATCHLEVEL, SUBLEVEL, EXTERVERSION]" >&2
    echo "" >&2
    exit 1;
}

while [ -n "$1" ]; do
    case $1 in
	-k ) shift; if [ -n "$1" ]; then KERNEL_SOURCE="$1"; shift; else usage; fi; ;;
	-t ) shift; if [ -n "$1" ]; then TARGET="$1"; shift; else usage; fi; ;;
	-* ) usage; ;;
	* ) usage; ;;
    esac;
done;

# set current VERSION, PATCHLEVEL, SUBLEVEL, EXTERVERSION
eval `sed -n -e 's/^\([A-Z]*\) = \([0-9]*\)$/\1=\2/p' -e 's/^\([A-Z]*\) = \(-[-a-z0-9]*\)$/\1=\2/p' $KERNEL_SOURCE/Makefile`
case $TARGET in
    VERSION) echo -n $VERSION; ;;
    PATCHLEVEL) echo -n $PATCHLEVEL; ;;
    SUBLEVEL) echo -n $SUBLEVEL; ;;
    EXTERVERSION) echo -n $EXTERVERSION; ;;
    *) usage; ;;
esac;



