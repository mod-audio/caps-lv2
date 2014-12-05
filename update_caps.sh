#!/bin/bash

######################################################################
# This script can be used when a new caps version is released.
# It will synchronize the two directories, the new one containing
# the most recent caps code and this one. The rsync command will be
# used to do that. After the process finishes will still some work
# to be done manually. A message will inform the necessary actions.


# check command line arguments
if [[ $# < 1 ]]; then
    echo "Usage: $0 <new_caps_dir>"
    exit 1
fi

# check if rsync is present
rsync --version 1>/dev/null 2>/dev/null || {
    echo "rsync not found. Please install it before continue." >&2
    exit 1
}

rsync -a -vv --delete $1/*.h ./ --exclude=Descriptor.h
rsync -a -vv --delete $1/*.cc ./ --exclude=interface.cc

rsync -a -vv --delete $1/waves ./
rsync -a -vv --delete $1/dsp ./

cp $1/CHANGES $1/configure.py $1/caps.rdf ./

RED="\e[0;31m"
YELLOW="\e[0;33m"
NOCOLOR="\e[0m"

echo
echo -e $RED"Please, check the following files manually: Descriptor.h, interface.cc and Makefile"
echo -e $YELLOW"The following files are no longer required, you can remove them:"$NOCOLOR
diff -qr . $1 | grep "Only in \.:.*\.h" | cut -d':' -f2
diff -qr . $1 | grep "Only in \.:.*\.cc" | cut -d':' -f2
echo
