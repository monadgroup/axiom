#!/bin/sh

for FILE in `git diff --cached --name-only`; do
    if [[ $FILE == *.rs ]] ; then
        rustfmt --skip-children $FILE
        git add $FILE
    fi
done
