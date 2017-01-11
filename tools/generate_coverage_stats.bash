#!/bin/bash
#
#  generate_coverage_stats.bash
#  Generate .info and html files based on gathered coverage information.
#
#  This file is part of "Sauvegarde" project.
#
#  (C) Copyright 2016 - 2017 Olivier Delhomme
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

lcov --capture --directory ../server --output-file ../coverage-server.info
genhtml -o ../coverage/server/ ../coverage-server.info

lcov --capture --directory ../client --output-file ../coverage-client.info
genhtml -o ../coverage/client/ ../coverage-client.info

lcov --capture --directory ../libcdpfgl --output-file ../coverage-libcdpfgl.info
genhtml -o ../coverage/libcdpfgl/ ../coverage-libcdpfgl.info

lcov --capture --directory ../restore --output-file ../coverage-restore.info
genhtml -o ../coverage/restore/ ../coverage-restore.info

