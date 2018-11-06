#!/bin/bash
#
#  create-package.bash
#  Bash script to create debian package.
#
#  This file is part of "Sauvegarde" project.
#
#  (C) Copyright 2015  - 2016 Olivier Delhomme
#   e-mail : olivier.delhomme@free.fr
#
#  "Sauvegarde" is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  "Sauvegarde" is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with "Sauvegarde".  If not, see <http://www.gnu.org/licenses/>
#
# Debian BTS for sauvegarde : https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=791878
#
# References :
#  . https://wiki.debian.org/IntroDebianPackaging documentation
#
export version=$(grep ' VERSION ' ../../config.h | cut -d '"' -f2)
export distfiles="../../cdpfgl-${version}.tar.xz"

cp "${distfiles}" .
mv "cdpfgl-${version}.tar.xz" "cdpfgl_${version}.orig.tar.xz"
tar Jxf "cdpfgl_${version}.orig.tar.xz"
cd "cdpfgl-${version}"
cp -a ../debian .

debuild -us -uc
