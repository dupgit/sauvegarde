#!/bin/bash
#
# Author : Olivier Delhomme
# Licence : GNU GPL v3
#

##
# Deletes all files passed in arguments
#
function delete_files {

    for file in "${@}"; do
        if test -f "${file}"; then
            echo "${file}"
            rm -f "${file}"
        fi
    done
}


##
# Deletes all directories (and their content) passed in arguments
#
function delete_dirs {

    for dir in "${@}"; do
        if test -d "${dir}"; then
            echo "${dir}"
            rm -fr "${dir}"
        fi
    done
}


##
# Prints the command being executed and deletes files in the whole
# project ($1 is the argument to find files to be deleted)
#
function delete_files_everywhere {

    for file in "${@}"; do
        echo "find . -name ${file} -exec rm -f {} \;"
        find . -name "${file}" -exec rm -f {} \;
    done
}

##
# Real start of the script

if [ -f Makefile ];
 then
    make distclean
fi;

delete_files "aclocal.m4" "config.guess" "config.sub" "depcomp" "install-sh"
delete_files "ltmain.sh" "missing" "configure" "config.status" "config.log"
delete_files "libtool" "compile"

delete_dirs "autom4te.cache" 'packaging/debian/cdpfgl*'

delete_files_everywhere '*~' '*#' '*.gcd[ao]' '*.gcov' '*.gcno'
delete_files_everywhere "gmon.out" "Makefile" "Makefile.in" ".libs" ".deps"
