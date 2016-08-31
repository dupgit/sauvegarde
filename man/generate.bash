#!/bin/bash

PANDOC_CMD="pandoc -s -f markdown -t man -V adjusting=b"

for prog in cdpfglserver cdpfglclient cdpfglrestore; do
	${PANDOC_CMD} -o ${prog}.1 ${prog}.md
done
