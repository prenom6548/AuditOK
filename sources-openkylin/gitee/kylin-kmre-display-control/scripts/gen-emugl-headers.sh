#!/bin/sh

# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Run this script to re-generate headers with the gen-entries.py script.

set -e
export LANG=C
export LC_ALL=C

PROGDIR=$(dirname "$0")
OUTDIR=$1

panic () {
    echo "ERROR: $@"
    exit 1
}

if [ -z "$OUTDIR" ]; then
    panic "No output dir defined!"
fi

QEMU_TOP_DIR=$(cd $PROGDIR/../ && pwd -P)
SCRIPT_DIR=scripts
if [ ! -d "$QEMU_TOP_DIR/$SCRIPT_DIR" ]; then
    panic "Missing scripts directory: $QEMU_TOP_DIR/$SCRIPT_DIR"
fi

cd $QEMU_TOP_DIR
GEN_ENTRIES=$SCRIPT_DIR/gen-emugl-entries.py
if [ ! -f "$GEN_ENTRIES" ]; then
    panic "Missing script: $GEN_ENTRIES"
fi

FAILURES=

# $1: Source file
# $2: Target file
# $3: gen-entries script.
gen_functions_header () {
    local SRC_FILE="$1"
    local DST_FILE="$2"
    local GEN_ENTRIES="$3"
    if [ ! -f "$SRC_FILE" ]; then
        echo "ERROR: Missing source file: $SRC_FILE"
        FAILURES=true
    else
        echo "Generating $DST_FILE"
        $GEN_ENTRIES --mode=functions $SRC_FILE --output=$DST_FILE
    fi
}

# $1: Source file
# $2: Target file
# $3: gen-entries script.
gen_funcargs_header () {
    local SRC_FILE="$1"
    local DST_FILE="$2"
    local GEN_ENTRIES="$3"
    if [ ! -f "$SRC_FILE" ]; then
        echo "ERROR: Missing source file: $SRC_FILE"
        FAILURES=true
    else
        echo "Generating $DST_FILE"
        $GEN_ENTRIES --mode=funcargs $SRC_FILE --output=$DST_FILE
    fi
}


##
##  libDisplayControl headers.
##
LIBDISPLAYCONTROL_DIR=host/libs/libDisplayControl
LIBDISPLAYCONTROL_INCLUDE=$OUTDIR/host/include/DisplayControl
mkdir -p $LIBDISPLAYCONTROL_INCLUDE
gen_funcargs_header \
        "$LIBDISPLAYCONTROL_DIR"/display_control_api.entries \
        "$LIBDISPLAYCONTROL_INCLUDE"/display_control_api_functions.h \
        "$GEN_ENTRIES"

