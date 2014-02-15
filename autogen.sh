#!/bin/sh

aclocal
libtoolize
automake --gnu --add-missing --copy
autoconf
glib-gettextize --copy --force
intltoolize --copy --force --automake

if [ -e ./configure ]; then
    echo
    echo "You can now run ./configure"
    echo
else
    echo
    echo "Failure building the configure script."
    echo "You may miss required tools (aclocal, libtoolize, automake or"
    echo "autoconf) to build Sauvegarde."
    echo
    echo "Please install the appropriated package and re-run autogen.sh"
    echo "This might also happen if you don't have the m4 provided as a"
    echo "subdirectory in this package in the directory where aclocal will find"
    echo "them. Just copy ./m4/* in, usually, /usr/share/aclocal."
    echo
    echo "Or, it can also come from a configure.ac mistake (then it's my"
    echo "business ;)). Please report your issue."
    echo
fi
