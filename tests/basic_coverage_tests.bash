#!/bin/bash
#
#  basic_coverage_tests.bash
#  Launches a set of tests in order to do some coverage tests of the
#  programs.
#
#  This file is part of "Sauvegarde" project.
#
#  (C) Copyright 2016 Olivier Delhomme
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

# This script should be launched as root.

# Here you should put the directory where the project is located
PROJECT_HOME=/home/dup/Dossiers_Perso/projets/sauvegarde

# Here you should put the directory where the project is installed
INSTALL_DIR=/home/dup/local/bin

# Directory where to store log files
LOG_DIR=/tmp

# Correct configuration files (ie without any errors)
SERVER_CONF=/home/dup/local/etc/cdpfgl/server.conf.test
CLIENT_CONF=/home/dup/local/etc/cdpfgl/client.conf.test
RESTORE_CONF=/home/dup/local/etc/cdpfgl/restore.conf.test

# Directory where to restore files
RESTORE_DIR=/tmp

###### It should not be necessary for you to change anything below #####

cd $PROJECT_HOME

# Asking for versions
$INSTALL_DIR/cdpfglserver --version -c $SERVER_CONF 1> $LOG_DIR/server.stdout 2> $LOG_DIR/server.stderr
$INSTALL_DIR/cdpfglclient --version -c $CLIENT_CONF 1> $LOG_DIR/client.stdout 2> $LOG_DIR/client.stderr
$INSTALL_DIR/cdpfglrestore  --version -c $RESTORE_CONF 1> $LOG_DIR/restore.stdout 2> $LOG_DIR/restore.stderr

# Asking for help
$INSTALL_DIR/cdpfglserver --help -c $SERVER_CONF 1>> $LOG_DIR/server.stdout 2>> $LOG_DIR/server.stderr
$INSTALL_DIR/cdpfglclient --help -c $CLIENT_CONF 1>> $LOG_DIR/client.stdout 2>> $LOG_DIR/client.stderr
$INSTALL_DIR/cdpfglrestore  --help -c $RESTORE_CONF 1>> $LOG_DIR/restore.stdout 2>> $LOG_DIR/restore.stderr


# Real test - it should not end with errors.
# Launching the server
$INSTALL_DIR/cdpfglserver -c $SERVER_CONF 1>> $LOG_DIR/server.stdout 2>> $LOG_DIR/server.stderr &

# Launching the client (must be root to be able to use fanotify).
$INSTALL_DIR/cdpfglclient -c $CLIENT_CONF 1>> $LOG_DIR/client.stdout 2>> $LOG_DIR/client.stderr &

# Waiting that the client finishes it's first pass.
echo "Waiting for client to finish its first pass 6 minutes to have at least one 'ping' of the server"
sleep 360s

# Creating files in the monitored directory
dd if=/dev/zero of=$PROJECT_HOME/tests/zerofile.dd count=3 bs=16k
dd if=/dev/urandom of=$PROJECT_HOME/tests/urandomfile.dd count=3 bs=16k

sync

md5sum $PROJECT_HOME/tests/d2/file_with_repetitions >$RESTORE_DIR/md5sums
md5sum $PROJECT_HOME/tests/urandomfile.dd >>$RESTORE_DIR/md5sums

sleep 2s

# Retrieving all saved files that contains 'ls'
$INSTALL_DIR/cdpfglrestore -c $RESTORE_CONF -l ls 1>> $LOG_DIR/restore.stdout 2>> $LOG_DIR/restore.stderr

# Saved after 2016-01-22
$INSTALL_DIR/cdpfglrestore -c $RESTORE_CONF -l ls -a 2016-01-22 1>> $LOG_DIR/restore.stdout 2>> $LOG_DIR/restore.stderr

# Saved befor 2016-01-22
$INSTALL_DIR/cdpfglrestore -c $RESTORE_CONF -l ls -b 2016-01-22 1>> $LOG_DIR/restore.stdout 2>> $LOG_DIR/restore.stderr

# Trying to restore some files
$INSTALL_DIR/cdpfglrestore -c $RESTORE_CONF -r d2/file_with_repetitions$ -w $RESTORE_DIR 1>> $LOG_DIR/restore.stdout 2>> $LOG_DIR/restore.stderr &
$INSTALL_DIR/cdpfglrestore -c $RESTORE_CONF -r urandomfile.dd$ -w $RESTORE_DIR 1>> $LOG_DIR/restore.stdout 2>> $LOG_DIR/restore.stderr &

md5sum $RESTORE_DIR/file_with_repetitions >>$RESTORE_DIR/md5sums
md5sum $RESTORE_DIR/urandomfile.dd >>$RESTORE_DIR/md5sums

cat $RESTORE_DIR/md5sums

# Waiting a bit before killing the programs
sleep 1s

# Ending programs
killall -9 cdpfglclient
killall -9 cdpfglserver

# Removing generated files (except $RESTORE_DIR/md5sums and log files).
rm -f $PROJECT_HOME/tests/urandomfile.dd $PROJECT_HOME/tests/zerofile.dd
rm -f $RESTORE_DIR/urandomfile.dd $RESTORE_DIR/file_with_repetitions
