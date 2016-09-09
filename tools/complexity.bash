#!/bin/bash
export PATH_TO_LIZARD="../../projets_externes/lizard/lizard"
$PATH_TO_LIZARD -s cyclomatic_complexity -C 10 -L 100 -a 10 ../client/ ../server/ ../libcdpfgl/ ../restore/
