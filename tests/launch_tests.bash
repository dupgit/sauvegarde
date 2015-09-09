#!/bin/bash
#
#  launch_tests.bash
#  Launches a set of tests for travis-ci.org architecture.
#
#  This file is part of "Sauvegarde" project.
#
#  (C) Copyright 2015 Olivier Delhomme
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

# All paths

# Launching the serveur
$HOME/local/bin/serveur -c $HOME/sauvegarde/tests/serveur.conf &

# Waiting for the serveur to achieve directories creation
sleep 40s

# Launching the client
$HOME/local/bin/client -c $HOME/sauvegarde/tests/client.conf &

# Waiting that the client finishes it's first pass (should be quick here).
sleep 5s

# Creating files in the monitored directory
dd if=/dev/zero of=$HOME/sauvegarde/tests/zerofile.dd count=3 bs=16k
dd if=/dev/urandom of=$HOME/sauvegarde/tests/urandomfile.dd count=3 bs=16k

# Waiting a bit before killing the programs
sleep 1s

# Ending programs
killall -9 client
killall -s SIGINT serveur
