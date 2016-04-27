#!/bin/bash
#    update_languages.bash
#    Updates all files for the translation system.
#
#    This file is part of "Sauvegarde" project.
#
#    (C) Copyright 2015 - 2016 Olivier Delhomme
#     e-mail : olivier.delhomme@free.fr
#
#    "Sauvegarde" is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    "Sauvegarde" is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with "Sauvegarde".  If not, see <http://www.gnu.org/licenses/>
#

rm cdpfgl.po.old
mv cdpfgl.po cdpfgl.po.old

# Updating file list
./find_files.sh >./POTFILES.in

# Updating sauvegarde.po
xgettext --language=C --default-domain=cdpfgl --add-comments --from-code=UTF-8 --keyword=N_:1 --keyword=_:1 -F --files-from=./POTFILES.in

sed -i -e "s/CHARSET/us-ascii/" cdpfgl.po

# updating translations
for l in fr; do
    msgmerge -U $l.po cdpfgl.po;
done;

sed -i s,\.\.\/,, ./POTFILES.in
