#!/usr/bin/env zsh

if (( # == 0 )); then
       print >&2 "Usage: $0 dir"
       exit 1
fi

find $1 -exec ./check_clones \{\} \; 2>/dev/null > clone_report.txt
sort clone_report.txt

       

