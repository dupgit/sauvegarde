#!/bin/sh
find ../ -type f -name "*.[ch]" | sort | sed s,\.\.\/,, | grep -v ^bugs | grep -v ^tools
