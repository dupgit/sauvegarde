#!/bin/bash
#    update_languages.bash
#    Updates all files for the translation system.
#
#    This file is part of "Sauvegarde" project.
# 
#    (C) Copyright 2015 Olivier Delhomme
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

# Updating file list
./find_files.sh >./POTFILES.in

# Updating sauvegarde.po
xgettext -j --default-domain=sauvegarde --add-comments --keyword=Q_:1g --keyword=N_:1g --from-code=UTF-8 --flag=g_strdup_printf:1:c-format --flag=g_string_printf:2:c-format  --flag=g_string_append_printf:2:c-format --flag=g_error_new:3:c-format --flag=g_set_error:4:c-format --flag=g_markup_printf_escaped:1:c-format --flag=g_log:3:c-format --flag=g_print:1:c-format --flag=g_printerr:1:c-format --flag=g_printf:1:c-format --flag=g_fprintf:2:c-format --flag=g_sprintf:2:c-format --flag=g_snprintf:3:c-format --flag=g_scanner_error:2:c-format --flag=g_scanner_warn:2:c-format --files-from=./POTFILES.in

# updating translations
for l in fr; do
	msgmerge -U $l.po sauvegarde.po;
done;
