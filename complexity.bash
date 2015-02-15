#!/bin/bash
export PATH_TO_LIZARD="../projets_externes/lizard/lizard"
$PATH_TO_LIZARD -w -s cyclomatic_complexity -C 10 monitor/ serveur/ libsauvegarde/
