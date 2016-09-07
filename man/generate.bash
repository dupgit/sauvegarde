#!/bin/bash
#
# This scripts generates man pages with the help of
# pandoc program from the markdown files (.md)

PANDOC_CMD="pandoc -s -f markdown -t man -V adjusting=b"

for prog in cdpfglserver cdpfglclient cdpfglrestore; do
	${PANDOC_CMD} -o ${prog}.1 ${prog}.md
done
