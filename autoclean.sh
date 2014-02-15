#!/bin/bash
#
# Author : Olivier Delhomme
# Licence : GNU GPL v3
#

##
# Prints the file to be deleted (first argument of the function : $1) and
# deletes it.
#
function delete_file {

    echo "$1"
    rm -f $1

}

##
# Prints the command being executed and deletes files in the whole
# project ($1 is the argument to find files to be deleted)
#
function delete_evrywhere {

echo "find . -name $1 -exec rm -f {} \;"
find . -name $1 -exec rm -f {} \;

}

##
# Real start of the script

if [ -f Makefile ];
 then
    make distclean
fi;

delete_file "aclocal.m4"
delete_file "autom4te.cache"
delete_file "config.guess"
delete_file "config.sub"
delete_file "depcomp"
delete_file "install-sh"
delete_file "ltmain.sh"
delete_file "missing"
delete_file "configure"
delete_file "config.status"
delete_file "config.log"
delete_file "libtool"

delete_evrywhere "*~"
delete_evrywhere "*#"
delete_evrywhere "*.gcd[ao]"
delete_evrywhere "*.gcov"
delete_evrywhere "*.gcno"
delete_evrywhere "gmon.out"
delete_evrywhere "Makefile"
delete_evrywhere "Makefile.in"

