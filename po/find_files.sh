#!/bin/sh
find ../ -type f -name "*.[ch]" | sort | grep -v bugs | grep -v tools
