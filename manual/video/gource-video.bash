#!/bin/bash
git log --pretty=format:user:%aN%n%ct --reverse --raw --encoding=UTF-8 --no-renames >gource-sauvegarde.log
gource --log-format git -r 25 -c 4 -s 1 -o - gource-sauvegarde.log | ffmpeg -y -r 25 -f image2pipe -vcodec ppm -i - -vcodec libx264 -preset ultrafast -pix_fmt yuv420p -crf 1 -threads 0 -bf 0 gource-sauvegarde.mp4
