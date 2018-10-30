#!/bin/sh

for FILE in `git diff --cached --name-only --diff-filter=d`; do
    if [[ $FILE == *.rs ]] ; then
        rustfmt $FILE --unstable-features --skip-children
        git add $FILE
    fi

    if [[ $FILE == *.cpp ]] || [[ $FILE == *.h ]] ; then
        clang-format -style=file -i $FILE
        git add $FILE
    fi
done
