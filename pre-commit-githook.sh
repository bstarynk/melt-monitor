#!/bin/bash
# file pre-commit-githook.sh to be installed as a git hook in .git/hooks/pre-commit
echo start pre-commit-githook.sh "$@"
make dump
echo end pre-commit-githook.sh "$@"
